/**
 * @file WAINInteractionProcessor.cpp
 * @brief 턴별 Top-Down 실행 흐름과 중복 실행 방지를 구현한다.
 */

#include "Core/WAINInteractionProcessor.h"

// ============================================================
// 00. Session And Public API
// ============================================================
#pragma region 00_Session_And_Public_API

FWAINInteractionProcessor::FWAINInteractionProcessor(
    TSharedRef<IWAINGameSnapshotAdapter> InGameAdapter,
    TSharedRef<IWAINProposalProvider> InProposalProvider,
    TSharedRef<IWAINNarrativeEvaluator> InNarrativeEvaluator)
    : GameAdapter(MoveTemp(InGameAdapter))
    , ProposalProvider(MoveTemp(InProposalProvider))
    , NarrativeEvaluator(MoveTemp(InNarrativeEvaluator))
{
}

bool FWAINInteractionProcessor::StartSession(const FWAINStateSnapshot& InitialWAIN, FString& OutError)
{
    check(IsInGameThread());

    TSharedPtr<const IWAINGameStateSnapshot> InitialGame = GameAdapter->CreateInitialSnapshot(OutError);
    if (!InitialGame.IsValid())
    {
        return false;
    }

    if (!GameAdapter->Validate(*InitialGame, OutError))
    {
        return false;
    }

    return StateStore.Initialize(InitialWAIN, MoveTemp(InitialGame), OutError);
}

FWAINInteractionResult FWAINInteractionProcessor::ProcessTurn(const FWAINInteractionRequest& Request)
{
    check(IsInGameThread());

    if (const FRecordedTurn* Previous = CompletedTurns.Find(Request.TurnId))
    {
        if (!IsSameRequest(Previous->Request, Request))
        {
            return Fail(Request, EWAINInteractionError::DuplicateTurnConflict, TEXT("동일 TurnId에 다른 입력을 사용할 수 없습니다."));
        }

        FWAINInteractionResult Replay = Previous->Result;
        Replay.bReplayed = true;
        return Replay;
    }

    FString Error;
    if (!ValidateRequest(Request, Error))
    {
        return Fail(Request, EWAINInteractionError::InvalidRequest, Error);
    }

    if (bProcessing)
    {
        return Fail(Request, EWAINInteractionError::Busy, TEXT("다른 Turn을 처리하고 있습니다."));
    }

    bProcessing = true;
    const FWAINInteractionResult Result = ExecuteTurn(Request);
    bProcessing = false;

    FRecordedTurn Record;
    Record.Request = Request;
    Record.Result = Result;
    CompletedTurns.Add(Request.TurnId, MoveTemp(Record));
    return Result;
}

TSharedPtr<const FWAINSessionRoot> FWAINInteractionProcessor::GetSnapshot() const
{
    return StateStore.Read();
}

#pragma endregion


// ============================================================
// 01. Turn Execution
// ============================================================
#pragma region 01_Turn_Execution

FWAINInteractionResult FWAINInteractionProcessor::ExecuteTurn(const FWAINInteractionRequest& Request)
{
    const TSharedPtr<const FWAINSessionRoot> Current = StateStore.Read();
    if (!Current.IsValid())
    {
        return Fail(Request, EWAINInteractionError::SessionNotStarted, TEXT("WAIN 세션이 시작되지 않았습니다."));
    }

    FWAINResponseProposal Proposal;
    FString Error;
    if (!ProposalProvider->Generate(Request, *Current, Proposal, Error))
    {
        return Fail(Request, EWAINInteractionError::ProposalGenerationFailed, Error);
    }

    const FWAINCommitOutcome Commit = Coordinator.Execute(
        Request, Proposal, StateStore, *GameAdapter, *NarrativeEvaluator);
    if (!Commit.Succeeded())
    {
        return Fail(Request, Commit.Error, Commit.Message);
    }

    FWAINInteractionResult Result;
    Result.TurnId = Request.TurnId;
    Result.CommittedRevision = Commit.Revision;
    Result.Dialogue = Proposal.Dialogue;
    return Result;
}

#pragma endregion


// ============================================================
// 02. Request Validation And Errors
// ============================================================
#pragma region 02_Request_Validation_And_Errors

bool FWAINInteractionProcessor::ValidateRequest(const FWAINInteractionRequest& Request, FString& OutError) const
{
    if (!Request.TurnId.IsValid() || Request.CharacterId.IsNone() || Request.SceneId.IsNone())
    {
        OutError = TEXT("TurnId, CharacterId, SceneId는 필수입니다.");
        return false;
    }

    if (Request.InputKind != EWAINInputKind::Text && Request.InputKind != EWAINInputKind::Silence)
    {
        OutError = TEXT("지원하지 않는 InputKind입니다.");
        return false;
    }

    if ((Request.InputKind == EWAINInputKind::Text && Request.Text.TrimStartAndEnd().IsEmpty()) ||
        (Request.InputKind == EWAINInputKind::Silence && !Request.Text.IsEmpty()))
    {
        OutError = TEXT("Text와 Silence 입력 형식이 일치하지 않습니다.");
        return false;
    }

    const TSharedPtr<const FWAINSessionRoot> Current = StateStore.Read();
    if (!Current.IsValid() || !Current->WAIN.Characters.Contains(Request.CharacterId))
    {
        OutError = TEXT("현재 세션에 등록되지 않은 CharacterId입니다.");
        return false;
    }

    return true;
}

bool FWAINInteractionProcessor::IsSameRequest(
    const FWAINInteractionRequest& Left,
    const FWAINInteractionRequest& Right)
{
    return Left.TurnId == Right.TurnId && Left.CharacterId == Right.CharacterId && Left.SceneId == Right.SceneId &&
           Left.InputKind == Right.InputKind && Left.Text == Right.Text;
}

FWAINInteractionResult FWAINInteractionProcessor::Fail(
    const FWAINInteractionRequest& Request,
    const EWAINInteractionError Error,
    const FString& Message)
{
    FWAINInteractionResult Result;
    Result.TurnId = Request.TurnId;
    Result.Error = Error;
    Result.ErrorMessage = Message;
    return Result;
}

#pragma endregion
