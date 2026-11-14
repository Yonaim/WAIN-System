/**
 * @file WAINRuntimeSubsystem.cpp
 * @brief GameInstance 진입점과 실제 Interaction Processor의 수명 및 알림을 연결한다.
 */

#include "WAINRuntimeSubsystem.h"
#include "Core/WAINInteractionProcessor.h"

// ============================================================
// 00. Session And Mock Turn
// ============================================================
#pragma region 00_Session_And_Mock_Turn

    bool UWAINRuntimeSubsystem::StartSession(const FWAINStateSnapshot                  &InitialWAIN,
                                             TSharedRef<IWAINGameSnapshotAdapter>       GameAdapter,
                                             TSharedRef<IWAINProposalProvider>          ProposalProvider,
                                             TSharedRef<IWAINNarrativeEvaluator>        NarrativeEvaluator,
                                             FString                                   &OutError,
                                             TSharedPtr<const IWAINPersonaSource>       PersonaSource,
                                             TSharedPtr<const IWAINPresentationCatalog> PresentationCatalog,
                                             TSharedPtr<const IWAINMemoryReasoner>      MemoryReasoner,
                                             TSharedPtr<const FWAINWorldStateManager>   WorldManager)
    {
        check(IsInGameThread());

        if (Processor.IsValid())
        {
            OutError = TEXT("기존 세션을 종료한 뒤 새 세션을 시작해야 합니다.");
            return false;
        }

        TSharedPtr<FWAINInteractionProcessor> NextProcessor =
            MakeShared<FWAINInteractionProcessor>(MoveTemp(GameAdapter),
                                                  MoveTemp(ProposalProvider),
                                                  MoveTemp(NarrativeEvaluator),
                                                  MoveTemp(PersonaSource),
                                                  MoveTemp(PresentationCatalog),
                                                  MoveTemp(MemoryReasoner),
                                                  MoveTemp(WorldManager));

        if (!NextProcessor->StartSession(InitialWAIN, OutError))
        {
            return false;
        }

        Processor = MoveTemp(NextProcessor);
        return true;
    }

    bool UWAINRuntimeSubsystem::AdvanceScriptedBeat(FName ExpectedBeatId, FString &OutError)
    {
        check(IsInGameThread());
        if (!Processor.IsValid())
        {
            OutError = TEXT("시작된 Session이 없습니다.");
            return false;
        }
        return Processor->AdvanceScriptedBeat(ExpectedBeatId, OutError);
    }

    bool UWAINRuntimeSubsystem::ApplyScriptedWorldChanges(const TArray<FWAINStateMutation> &Mutations, FString &OutError)
    {
        check(IsInGameThread());
        if (!Processor.IsValid())
        {
            OutError = TEXT("시작된 Session이 없습니다.");
            return false;
        }
        return Processor->ApplyScriptedWorldChanges(Mutations, OutError);
    }

    bool UWAINRuntimeSubsystem::SetAuthoredGameFact(FName TargetId, int32 Value, FString &OutError)
    {
        check(IsInGameThread());
        if (!Processor.IsValid())
        {
            OutError = TEXT("시작된 Session이 없습니다.");
            return false;
        }
        return Processor->SetAuthoredGameFact(TargetId, Value, OutError);
    }

    bool UWAINRuntimeSubsystem::RestoreSnapshot(const FWAINStateSnapshot                &Snapshot,
                                                TSharedPtr<const IWAINGameStateSnapshot> Game,
                                                FString                                 &OutError)
    {
        check(IsInGameThread());
        if (!Processor.IsValid())
        {
            OutError = TEXT("시작된 Session이 없습니다.");
            return false;
        }
        return Processor->RestoreSnapshot(Snapshot, MoveTemp(Game), OutError);
    }

    FWAINInteractionResult UWAINRuntimeSubsystem::ProcessMockTurn(const FWAINInteractionRequest &Request)
    {
        return ProcessTurn(Request);
    }

    FWAINInteractionResult UWAINRuntimeSubsystem::ProcessTurn(const FWAINInteractionRequest &Request)
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

#pragma endregion

// ============================================================
// 01. Snapshot And Lifecycle
// ============================================================
#pragma region 01_Snapshot_And_Lifecycle

    TSharedPtr<const FWAINSessionRoot> UWAINRuntimeSubsystem::GetStateSnapshot() const
    {
        check(IsInGameThread());
        return Processor ? Processor->GetSnapshot() : nullptr;
    }

    bool UWAINRuntimeSubsystem::PrepareTurnContext(const FWAINInteractionRequest &Request,
                                                   FWAINCharacterContext         &OutContext,
                                                   FString                       &OutError) const
    {
        check(IsInGameThread());
        if (!Processor)
        {
            OutError = TEXT("WAIN 세션이 시작되지 않았습니다.");
            return false;
        }
        return Processor->PrepareTurnContext(Request, OutContext, OutError);
    }

    bool UWAINRuntimeSubsystem::GetDebugRecord(const FGuid &TurnId, FWAINInteractionDebugRecord &OutRecord) const
    {
        check(IsInGameThread());
        return Processor && Processor->GetDebugRecord(TurnId, OutRecord);
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

#pragma endregion
