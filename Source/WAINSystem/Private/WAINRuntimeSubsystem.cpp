/**
 * @file WAINRuntimeSubsystem.cpp
 * @brief GameInstance 진입점과 실제 Interaction Processor의 수명 및 알림을 연결한다.
 */

#include "WAINRuntimeSubsystem.h"

#include "Core/WAINInteractionProcessor.h"

bool UWAINRuntimeSubsystem::StartSession(
    const FWAINStateSnapshot& InitialWAIN,
    TSharedRef<IWAINGameSnapshotAdapter> GameAdapter,
    TSharedRef<IWAINProposalProvider> ProposalProvider,
    TSharedRef<IWAINNarrativeEvaluator> NarrativeEvaluator,
    FString& OutError)
{
    check(IsInGameThread());

    if (Processor.IsValid())
    {
        OutError = TEXT("기존 세션을 종료한 뒤 새 세션을 시작해야 합니다.");
        return false;
    }

    TSharedPtr<FWAINInteractionProcessor> NextProcessor = MakeShared<FWAINInteractionProcessor>(
        MoveTemp(GameAdapter), MoveTemp(ProposalProvider), MoveTemp(NarrativeEvaluator));

    if (!NextProcessor->StartSession(InitialWAIN, OutError))
    {
        return false;
    }

    Processor = MoveTemp(NextProcessor);
    return true;
}

FWAINInteractionResult UWAINRuntimeSubsystem::ProcessMockTurn(const FWAINInteractionRequest& Request)
{
    check(IsInGameThread());

    if (!Processor)
    {
        FWAINInteractionResult Result;
        Result.TurnId = Request.TurnId;
        Result.Error = EWAINInteractionError::SessionNotStarted;
        Result.ErrorMessage = TEXT("WAIN 세션이 시작되지 않았습니다.");
        return Result;
    }

    const FWAINInteractionResult Result = Processor->ProcessTurn(Request);
    if (Result.Succeeded() && !Result.bReplayed)
    {
        StateCommitted.Broadcast(Request.TurnId, Result.CommittedRevision);
    }

    return Result;
}

TSharedPtr<const FWAINSessionRoot> UWAINRuntimeSubsystem::GetStateSnapshot() const
{
    check(IsInGameThread());
    return Processor ? Processor->GetSnapshot() : nullptr;
}

void UWAINRuntimeSubsystem::EndSession()
{
    check(IsInGameThread());
    Processor.Reset();
}

void UWAINRuntimeSubsystem::Deinitialize()
{
    EndSession();
    Super::Deinitialize();
}
