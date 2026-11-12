/**
 * @file WAINTransactionCoordinator.cpp
 * @brief 단계별 검증·준비·서사 평가 후 WAIN/Host Snapshot을 함께 확정한다.
 */

#include "Core/WAINTransactionCoordinator.h"

#include "Core/WAINStateValidation.h"

// ============================================================
// 00. Commit Result Helper
// ============================================================
#pragma region 00_Commit_Result_Helper

namespace WAINTransaction
{
FWAINCommitOutcome Fail(const EWAINInteractionError Error, const FString& Message)
{
    FWAINCommitOutcome Outcome;
    Outcome.Error = Error;
    Outcome.Message = Message;
    return Outcome;
}

} // namespace WAINTransaction

#pragma endregion


// ============================================================
// 01. Transaction Pipeline
// ============================================================
#pragma region 01_Transaction_Pipeline

FWAINCommitOutcome FWAINTransactionCoordinator::Execute(
    const FWAINInteractionRequest& Request,
    const FWAINResponseProposal& Proposal,
    FWAINStateStore& StateStore,
    const IWAINGameSnapshotAdapter& GameAdapter,
    const IWAINNarrativeEvaluator& NarrativeEvaluator) const
{
    check(IsInGameThread());

    FString Error;
    if (!ValidateProposal(Proposal, Error))
    {
        return WAINTransaction::Fail(EWAINInteractionError::InvalidProposal, Error);
    }

    const TSharedPtr<const FWAINSessionRoot> Current = StateStore.Read();
    if (!Current.IsValid() || !Current->Game.IsValid())
    {
        return WAINTransaction::Fail(EWAINInteractionError::SessionNotStarted, TEXT("확정된 Session Root가 없습니다."));
    }

    FWAINStateSnapshot PreparedWAIN;
    if (!PrepareWAINState(Request, *Current, Proposal, PreparedWAIN, Error))
    {
        return WAINTransaction::Fail(EWAINInteractionError::UnsupportedMutation, Error);
    }

    const TArray<FWAINStateMutation> GameMutations = CollectGameMutations(Proposal);

    TSharedPtr<const IWAINGameStateSnapshot> PreparedGame = GameAdapter.Prepare(*Current->Game, GameMutations, Error);
    if (!PreparedGame.IsValid())
    {
        return WAINTransaction::Fail(EWAINInteractionError::GamePrepareFailed, Error);
    }

    if (!NarrativeEvaluator.Evaluate(Request, *Current, PreparedWAIN, *PreparedGame, Error))
    {
        return WAINTransaction::Fail(EWAINInteractionError::NarrativeEvaluationFailed, Error);
    }

    if (!WAINStateValidation::Validate(PreparedWAIN, Error) || !GameAdapter.Validate(*PreparedGame, Error))
    {
        return WAINTransaction::Fail(EWAINInteractionError::FinalValidationFailed, Error);
    }

    if (!StateStore.Publish(Current->Revision, MoveTemp(PreparedWAIN), MoveTemp(PreparedGame)))
    {
        return WAINTransaction::Fail(EWAINInteractionError::RevisionConflict, TEXT("확정 Revision이 변경되었습니다."));
    }

    FWAINCommitOutcome Outcome;
    Outcome.Revision = Current->Revision + 1;
    return Outcome;
}

#pragma endregion


// ============================================================
// 02. Proposal Validation
// ============================================================
#pragma region 02_Proposal_Validation

bool FWAINTransactionCoordinator::ValidateProposal(const FWAINResponseProposal& Proposal, FString& OutError) const
{
    if (Proposal.Dialogue.TrimStartAndEnd().IsEmpty())
    {
        OutError = TEXT("Proposal에 대사가 없습니다.");
        return false;
    }

    // Presentation Catalog와 Action Schema를 연결하기 전까지 미검증 ID는 거절한다.
    if (!Proposal.ExpressionId.IsNone() || !Proposal.ExpressionVariantId.IsNone() || !Proposal.ActionId.IsNone())
    {
        OutError = TEXT("Presentation / Action ID 검증은 아직 구현되지 않았습니다.");
        return false;
    }

    for (const FWAINStateMutation& Mutation : Proposal.StateChanges)
    {
        if (Mutation.TargetId.IsNone())
        {
            OutError = TEXT("Mutation TargetId가 없습니다.");
            return false;
        }

        if (Mutation.Operation != EWAINMutationOperation::AddInteger &&
            Mutation.Operation != EWAINMutationOperation::SetInteger)
        {
            OutError = TEXT("지원하지 않는 Mutation Operation입니다.");
            return false;
        }
    }

    return true;
}

#pragma endregion


// ============================================================
// 03. Mutation Preparation
// ============================================================
#pragma region 03_Mutation_Preparation

TArray<FWAINStateMutation> FWAINTransactionCoordinator::CollectGameMutations(
    const FWAINResponseProposal& Proposal) const
{
    TArray<FWAINStateMutation> GameMutations;
    for (const FWAINStateMutation& Mutation : Proposal.StateChanges)
    {
        if (Mutation.Owner == EWAINMutationOwner::Game)
        {
            GameMutations.Add(Mutation);
        }
    }

    return GameMutations;
}

bool FWAINTransactionCoordinator::PrepareWAINState(
    const FWAINInteractionRequest& Request,
    const FWAINSessionRoot& Current,
    const FWAINResponseProposal& Proposal,
    FWAINStateSnapshot& OutPrepared,
    FString& OutError) const
{
    OutPrepared = Current.WAIN;
    FWAINCharacterState* Character = OutPrepared.Characters.Find(Request.CharacterId);
    if (!Character)
    {
        OutError = TEXT("존재하지 않는 CharacterId입니다.");
        return false;
    }

    for (const FWAINStateMutation& Mutation : Proposal.StateChanges)
    {
        if (Mutation.Owner == EWAINMutationOwner::Game)
        {
            continue;
        }

        if (Mutation.Owner != EWAINMutationOwner::Character || Mutation.TargetId != TEXT("relationship.trust"))
        {
            OutError = TEXT("MVP에서 지원하지 않는 WAIN Mutation Target입니다.");
            return false;
        }

        const int64 NextTrust = Mutation.Operation == EWAINMutationOperation::AddInteger
                                    ? static_cast<int64>(Character->Trust) + Mutation.IntegerValue
                                    : static_cast<int64>(Mutation.IntegerValue);
        if (NextTrust < 0 || NextTrust > 100)
        {
            OutError = TEXT("Trust는 0부터 100 사이여야 합니다.");
            return false;
        }

        Character->Trust = static_cast<int32>(NextTrust);
    }

    if (OutPrepared.TurnCount == MAX_int32)
    {
        OutError = TEXT("TurnCount가 최대 범위에 도달했습니다.");
        return false;
    }

    ++OutPrepared.TurnCount;
    return true;
}

#pragma endregion
