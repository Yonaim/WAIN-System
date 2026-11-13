/**
 * @file WAINTransactionCoordinator.cpp
 * @brief 단계별 검증·준비·서사 평가 후 WAIN/Host Snapshot을 함께 확정한다.
 */

#include "Core/WAINTransactionCoordinator.h"
#include "Character/WAINCharacterStatePreparer.h"
#include "Core/WAINStateValidation.h"
#include "Memory/WAINMemoryReasoningPreparer.h"
#include "Memory/WAINMemoryWriter.h"
#include "Presentation/WAINPresentationValidator.h"

// ============================================================
// 00. Commit Result Helper
// ============================================================
#pragma region 00_Commit_Result_Helper

    namespace WAINTransaction
    {
        FWAINCommitOutcome Fail(const EWAINInteractionError Error, const FString &Message)
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

    FWAINCommitOutcome FWAINTransactionCoordinator::Execute(const FWAINInteractionRequest  &Request,
                                                            const FWAINResponseProposal    &Proposal,
                                                            FWAINStateStore                &StateStore,
                                                            const IWAINGameSnapshotAdapter &GameAdapter,
                                                            const IWAINNarrativeEvaluator  &NarrativeEvaluator,
                                                            const FWAINPresentationOptions *AvailablePresentations,
                                                            const IWAINMemoryReasoner      *MemoryReasoner,
                                                            const FWAINWorldStateManager   *WorldManager,
                                                            FWAINInteractionDebugRecord   *OutDebug) const
    {
        check(IsInGameThread());

        FString Error;
        auto Trace = [OutDebug](const FName Name, const EWAINTraceStatus Status, const FString &Detail = FString())
        {
            if (OutDebug) OutDebug->AddStep(Name, Status, Detail);
        };

        if (!ValidateProposal(Proposal, AvailablePresentations, Error))
        {
            Trace(TEXT("Proposal Validation"), EWAINTraceStatus::Failed, Error);
            return WAINTransaction::Fail(EWAINInteractionError::InvalidProposal, Error);
        }
        Trace(TEXT("Proposal Validation"), EWAINTraceStatus::Passed);

        const TSharedPtr<const FWAINSessionRoot> Current = StateStore.Read();
        if (!Current.IsValid() || !Current->Game.IsValid())
        {
            Trace(TEXT("Snapshot Read"), EWAINTraceStatus::Failed, TEXT("확정된 Session Root가 없습니다."));
            return WAINTransaction::Fail(EWAINInteractionError::SessionNotStarted, TEXT("확정된 Session Root가 없습니다."));
        }
        Trace(TEXT("Snapshot Read"), EWAINTraceStatus::Passed);

        FWAINStateSnapshot    PreparedWAIN;
        EWAINInteractionError PreparationError = EWAINInteractionError::UnsupportedMutation;
        if (!PrepareWAINState(Request, *Current, Proposal, PreparedWAIN, PreparationError, Error))
        {
            Trace(TEXT("WAIN Preparation"), EWAINTraceStatus::Failed, Error);
            return WAINTransaction::Fail(PreparationError, Error);
        }
        Trace(TEXT("WAIN Preparation"), EWAINTraceStatus::Passed);

        const TArray<FWAINStateMutation> GameMutations = CollectGameMutations(Proposal);
        if (WorldManager && !WorldManager->ValidateMutations(*Current->Game, GameMutations, Error))
        {
            Trace(TEXT("World Mutation Validation"), EWAINTraceStatus::Failed, Error);
            return WAINTransaction::Fail(EWAINInteractionError::InvalidProposal, Error);
        }
        Trace(TEXT("World Mutation Validation"), WorldManager ? EWAINTraceStatus::Passed : EWAINTraceStatus::Skipped);

        TSharedPtr<const IWAINGameStateSnapshot> PreparedGame = GameAdapter.Prepare(*Current->Game, GameMutations, Error);
        if (!PreparedGame.IsValid())
        {
            Trace(TEXT("Game Preparation"), EWAINTraceStatus::Failed, Error);
            return WAINTransaction::Fail(EWAINInteractionError::GamePrepareFailed, Error);
        }
        Trace(TEXT("Game Preparation"), EWAINTraceStatus::Passed);

        if (WorldManager && !WorldManager->ValidatePrepared(*Current->Game, *PreparedGame, GameMutations, Error))
        {
            Trace(TEXT("Prepared World Validation"), EWAINTraceStatus::Failed, Error);
            return WAINTransaction::Fail(EWAINInteractionError::GamePrepareFailed, Error);
        }
        Trace(TEXT("Prepared World Validation"), WorldManager ? EWAINTraceStatus::Passed : EWAINTraceStatus::Skipped);

        if (!NarrativeEvaluator.Evaluate(Request, *Current, PreparedWAIN, *PreparedGame, Error))
        {
            Trace(TEXT("Narrative Evaluation"), EWAINTraceStatus::Failed, Error);
            return WAINTransaction::Fail(EWAINInteractionError::NarrativeEvaluationFailed, Error);
        }
        Trace(TEXT("Narrative Evaluation"), EWAINTraceStatus::Passed);

        if (MemoryReasoner && !FWAINMemoryReasoningPreparer().Prepare(Request, PreparedWAIN, *MemoryReasoner, Error))
        {
            Trace(TEXT("Memory Reasoning"), EWAINTraceStatus::Failed, Error);
            return WAINTransaction::Fail(EWAINInteractionError::MemoryReasoningFailed, Error);
        }
        Trace(TEXT("Memory Reasoning"), MemoryReasoner ? EWAINTraceStatus::Passed : EWAINTraceStatus::Skipped);

        if (!WAINStateValidation::Validate(PreparedWAIN, Error) || !GameAdapter.Validate(*PreparedGame, Error))
        {
            Trace(TEXT("Final Validation"), EWAINTraceStatus::Failed, Error);
            return WAINTransaction::Fail(EWAINInteractionError::FinalValidationFailed, Error);
        }
        Trace(TEXT("Final Validation"), EWAINTraceStatus::Passed);

        if (!StateStore.Publish(Current->Revision, MoveTemp(PreparedWAIN), MoveTemp(PreparedGame)))
        {
            Trace(TEXT("Atomic Publish"), EWAINTraceStatus::Failed, TEXT("확정 Revision이 변경되었습니다."));
            return WAINTransaction::Fail(EWAINInteractionError::RevisionConflict, TEXT("확정 Revision이 변경되었습니다."));
        }
        Trace(TEXT("Atomic Publish"), EWAINTraceStatus::Passed);

        FWAINCommitOutcome Outcome;
        Outcome.Revision = Current->Revision + 1;
        return Outcome;
    }

#pragma endregion

// ============================================================
// 02. Proposal Validation
// ============================================================
#pragma region 02_Proposal_Validation

    bool FWAINTransactionCoordinator::ValidateProposal(const FWAINResponseProposal    &Proposal,
                                                       const FWAINPresentationOptions *AvailablePresentations,
                                                       FString                        &OutError) const
    {
        if (Proposal.Dialogue.TrimStartAndEnd().IsEmpty())
        {
            OutError = TEXT("Proposal에 대사가 없습니다.");
            return false;
        }

        // Action Schema는 아직 구현되지 않았으므로 미검증 Action은 거절한다.
        if (!Proposal.ActionId.IsNone())
        {
            OutError = TEXT("Action ID 검증은 아직 구현되지 않았습니다.");
            return false;
        }

        if (!FWAINPresentationValidator::ValidateSelection(Proposal, AvailablePresentations, OutError))
        {
            return false;
        }

        for (const FWAINStateMutation &Mutation : Proposal.StateChanges)
        {
            if (Mutation.TargetId.IsNone())
            {
                OutError = TEXT("Mutation TargetId가 없습니다.");
                return false;
            }

            if (Mutation.Owner != EWAINMutationOwner::Character && Mutation.Owner != EWAINMutationOwner::Game)
            {
                OutError = TEXT("지원하지 않는 Mutation Owner입니다.");
                return false;
            }

            if (Mutation.Operation != EWAINMutationOperation::AddInteger &&
                Mutation.Operation != EWAINMutationOperation::SetInteger &&
                Mutation.Operation != EWAINMutationOperation::AddFloat)
            {
                OutError = TEXT("지원하지 않는 Mutation Operation입니다.");
                return false;
            }

            if (Mutation.Owner == EWAINMutationOwner::Game && Mutation.Operation == EWAINMutationOperation::AddFloat)
            {
                OutError = TEXT("초기 Game Adapter에는 float Mutation을 전달하지 않습니다.");
                return false;
            }

            if (Mutation.Operation == EWAINMutationOperation::AddFloat && !FMath::IsFinite(Mutation.FloatValue))
            {
                OutError = TEXT("Emotion Delta는 유한한 float 값이어야 합니다.");
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

    TArray<FWAINStateMutation>
    FWAINTransactionCoordinator::CollectGameMutations(const FWAINResponseProposal &Proposal) const
    {
        TArray<FWAINStateMutation> GameMutations;
        for (const FWAINStateMutation &Mutation : Proposal.StateChanges)
        {
            if (Mutation.Owner == EWAINMutationOwner::Game)
            {
                GameMutations.Add(Mutation);
            }
        }

        return GameMutations;
    }

    bool FWAINTransactionCoordinator::PrepareWAINState(const FWAINInteractionRequest &Request,
                                                       const FWAINSessionRoot        &Current,
                                                       const FWAINResponseProposal   &Proposal,
                                                       FWAINStateSnapshot            &OutPrepared,
                                                       EWAINInteractionError         &OutErrorCode,
                                                       FString                       &OutError) const
    {
        OutPrepared = Current.WAIN;
        const FWAINCharacterStatePreparer CharacterPreparer;
        if (!CharacterPreparer.Apply(Request.CharacterId, Proposal.StateChanges, OutPrepared, OutError))
        {
            return false;
        }

        if (OutPrepared.TurnCount == MAX_int32)
        {
            OutError = TEXT("TurnCount가 최대 범위에 도달했습니다.");
            return false;
        }

        ++OutPrepared.TurnCount;
        if (!FWAINMemoryWriter().PrepareObservations(Request, Proposal, OutPrepared, OutError))
        {
            OutErrorCode = EWAINInteractionError::MemoryPreparationFailed;
            return false;
        }
        return true;
    }

#pragma endregion
