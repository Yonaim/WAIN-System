/**
 * @file WAINInteractionProcessor.cpp
 * @brief 턴별 Top-Down 실행 흐름과 중복 실행 방지를 구현한다.
 */

#include "Core/WAINInteractionProcessor.h"
#include "HAL/PlatformTime.h"

// ============================================================
// 00. Session And Public API
// ============================================================
#pragma region 00_Session_And_Public_API

    FWAINInteractionProcessor::FWAINInteractionProcessor(TSharedRef<IWAINGameSnapshotAdapter>       InGameAdapter,
                                                         TSharedRef<IWAINProposalProvider>          InProposalProvider,
                                                         TSharedRef<IWAINNarrativeEvaluator>        InNarrativeEvaluator,
                                                         TSharedPtr<const IWAINPersonaSource>       InPersonaSource,
                                                         TSharedPtr<const IWAINPresentationCatalog> InPresentationCatalog,
                                                         TSharedPtr<const IWAINMemoryReasoner>      InMemoryReasoner,
                                                         TSharedPtr<const FWAINWorldStateManager>   InWorldManager)
        : GameAdapter(MoveTemp(InGameAdapter)),
          ProposalProvider(MoveTemp(InProposalProvider)),
          NarrativeEvaluator(MoveTemp(InNarrativeEvaluator)),
          PersonaSource(MoveTemp(InPersonaSource)),
          PresentationCatalog(MoveTemp(InPresentationCatalog)),
          MemoryReasoner(MoveTemp(InMemoryReasoner)),
          WorldManager(MoveTemp(InWorldManager))
    {
    }

    bool FWAINInteractionProcessor::StartSession(const FWAINStateSnapshot &InitialWAIN, FString &OutError)
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

        if (WorldManager.IsValid() && !WorldManager->ValidateInitial(*InitialGame, OutError))
        {
            return false;
        }

        if (!NarrativeEvaluator->ValidateInitialState(InitialWAIN, *InitialGame, OutError))
        {
            return false;
        }

        return StateStore.Initialize(InitialWAIN, MoveTemp(InitialGame), OutError);
    }

    FWAINInteractionResult FWAINInteractionProcessor::ProcessTurn(const FWAINInteractionRequest &Request)
    {
        check(IsInGameThread());

        if (const FRecordedTurn *Previous = CompletedTurns.Find(Request.TurnId))
        {
            if (!IsSameRequest(Previous->Request, Request))
            {
                return Fail(Request, EWAINInteractionError::DuplicateTurnConflict,
                            TEXT("동일 TurnId에 다른 입력을 사용할 수 없습니다."));
            }
            FWAINInteractionResult Replay = Previous->Result;
            Replay.bReplayed = true;
            return Replay;
        }

        FWAINInteractionDebugRecord Debug;
        Debug.Request = Request;
        const double StartSeconds = FPlatformTime::Seconds();
        const TSharedPtr<const FWAINSessionRoot> Before = StateStore.Read();
        Debug.RevisionBefore = Before.IsValid() ? Before->Revision : 0;

        FString Error;
        if (!ValidateRequest(Request, Error))
        {
            Debug.AddStep(TEXT("Request Validation"), EWAINTraceStatus::Failed, Error);
            const FWAINInteractionResult Result = Fail(Request, EWAINInteractionError::InvalidRequest, Error);
            Debug.ElapsedMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
            SaveDebugRecord(Debug, Result);
            return Result;
        }
        Debug.AddStep(TEXT("Request Validation"), EWAINTraceStatus::Passed);

        if (bProcessing)
        {
            Debug.AddStep(TEXT("Turn Availability"), EWAINTraceStatus::Failed, TEXT("다른 Turn 처리 중"));
            const FWAINInteractionResult Result = Fail(Request, EWAINInteractionError::Busy,
                                                        TEXT("다른 Turn을 처리하고 있습니다."));
            Debug.ElapsedMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
            SaveDebugRecord(Debug, Result);
            return Result;
        }

        bProcessing = true;
        const FWAINInteractionResult Result = ExecuteTurn(Request, Debug);
        bProcessing = false;

        FRecordedTurn Record;
        Record.Request = Request;
        Record.Result = Result;
        CompletedTurns.Add(Request.TurnId, MoveTemp(Record));
        Debug.ElapsedMs = (FPlatformTime::Seconds() - StartSeconds) * 1000.0;
        SaveDebugRecord(Debug, Result);
        return Result;
    }

    TSharedPtr<const FWAINSessionRoot> FWAINInteractionProcessor::GetSnapshot() const
    {
        return StateStore.Read();
    }

    bool FWAINInteractionProcessor::GetDebugRecord(const FGuid &TurnId,
                                                   FWAINInteractionDebugRecord &OutRecord) const
    {
        if (const FWAINInteractionDebugRecord *Found = DebugRecords.Find(TurnId))
        {
            OutRecord = *Found;
            return true;
        }
        return false;
    }

    void FWAINInteractionProcessor::SaveDebugRecord(FWAINInteractionDebugRecord &Record,
                                                     const FWAINInteractionResult &Result)
    {
        Record.Result = Result;
        const TSharedPtr<const FWAINSessionRoot> After = StateStore.Read();
        Record.RevisionAfter = After.IsValid() ? After->Revision : Record.RevisionBefore;

        // TurnId가 없는 잘못된 요청은 조회 키가 없으므로 기록을 남기지 않는다.
        if (!Record.Request.TurnId.IsValid()) return;

        // 최근 48건만 진단 Context/Proposal 사본을 보관한다. 권위 State와 Replay 캐시는 별개다.
        if (DebugOrder.Num() >= 48)
        {
            DebugRecords.Remove(DebugOrder[0]);
            DebugOrder.RemoveAt(0);
        }
        DebugOrder.Add(Record.Request.TurnId);
        DebugRecords.Add(Record.Request.TurnId, MoveTemp(Record));
    }

#pragma endregion

// ============================================================
// 01. Turn Execution
// ============================================================
#pragma region 01_Turn_Execution

    FWAINInteractionResult FWAINInteractionProcessor::ExecuteTurn(const FWAINInteractionRequest &Request,
                                                                   FWAINInteractionDebugRecord &OutDebug)
    {
        const TSharedPtr<const FWAINSessionRoot> Current = StateStore.Read();
        if (!Current.IsValid())
        {
            const FString Error = TEXT("WAIN 세션이 시작되지 않았습니다.");
            OutDebug.AddStep(TEXT("Snapshot Read"), EWAINTraceStatus::Failed, Error);
            return Fail(Request, EWAINInteractionError::SessionNotStarted, Error);
        }
        OutDebug.AddStep(TEXT("Snapshot Read"), EWAINTraceStatus::Passed);

        FWAINCharacterContext Context;
        FString Error;
        if (!ContextBuilder.Build(Request, *Current, PersonaSource.Get(), PresentationCatalog.Get(), Context, Error))
        {
            OutDebug.AddStep(TEXT("Character Context"), EWAINTraceStatus::Failed, Error);
            return Fail(Request, EWAINInteractionError::ContextPreparationFailed, Error);
        }
        OutDebug.AddStep(TEXT("Character Context"), EWAINTraceStatus::Passed);

        if (!NarrativeEvaluator->BuildContext(*Current, Context.Narrative, Error))
        {
            OutDebug.AddStep(TEXT("Narrative Context"), EWAINTraceStatus::Failed, Error);
            return Fail(Request, EWAINInteractionError::ContextPreparationFailed, Error);
        }
        Context.bHasNarrativeContext = !Context.Narrative.BeatId.IsNone();
        OutDebug.AddStep(TEXT("Narrative Context"), EWAINTraceStatus::Passed);

        if (WorldManager.IsValid())
        {
            if (!WorldManager->BuildContext(*Current->Game, Context.World, Error))
            {
                OutDebug.AddStep(TEXT("World Context"), EWAINTraceStatus::Failed, Error);
                return Fail(Request, EWAINInteractionError::ContextPreparationFailed, Error);
            }
            Context.bHasWorldContext = true;
            OutDebug.AddStep(TEXT("World Context"), EWAINTraceStatus::Passed);
        }
        else
        {
            OutDebug.AddStep(TEXT("World Context"), EWAINTraceStatus::Skipped);
        }

        OutDebug.Context = Context;
        OutDebug.bHasContext = true;
        FWAINResponseProposal Proposal;
        if (!ProposalProvider->GenerateWithContext(Request, *Current, Context, Proposal, Error))
        {
            OutDebug.AddStep(TEXT("Proposal Generation"), EWAINTraceStatus::Failed, Error);
            return Fail(Request, EWAINInteractionError::ProposalGenerationFailed, Error);
        }
        OutDebug.AddStep(TEXT("Proposal Generation"), EWAINTraceStatus::Passed);
        OutDebug.Proposal = Proposal;
        OutDebug.bHasProposal = true;

        const FWAINCommitOutcome Commit = Coordinator.Execute(
            Request, Proposal, StateStore, *GameAdapter, *NarrativeEvaluator,
            Context.bHasPresentationCatalog ? &Context.AvailablePresentations : nullptr,
            MemoryReasoner.Get(), WorldManager.Get(), &OutDebug);
        if (!Commit.Succeeded())
        {
            return Fail(Request, Commit.Error, Commit.Message);
        }

        FWAINInteractionResult Result;
        Result.TurnId = Request.TurnId;
        Result.CommittedRevision = Commit.Revision;
        Result.Dialogue = Proposal.Dialogue;
        Result.ExpressionId = Proposal.ExpressionId;
        Result.ExpressionVariantId = Proposal.ExpressionVariantId;
        return Result;
    }

#pragma endregion

// ============================================================
// 02. Request Validation And Errors
// ============================================================
#pragma region 02_Request_Validation_And_Errors

    bool FWAINInteractionProcessor::ValidateRequest(const FWAINInteractionRequest &Request, FString &OutError) const
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

        if ((Request.InputKind == EWAINInputKind::Text &&
             (Request.Text.TrimStartAndEnd().IsEmpty() || Request.Text.Len() > 4096)) ||
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

    bool FWAINInteractionProcessor::IsSameRequest(const FWAINInteractionRequest &Left, const FWAINInteractionRequest &Right)
    {
        return Left.TurnId == Right.TurnId && Left.CharacterId == Right.CharacterId && Left.SceneId == Right.SceneId &&
               Left.InputKind == Right.InputKind && Left.Text == Right.Text;
    }

    FWAINInteractionResult FWAINInteractionProcessor::Fail(const FWAINInteractionRequest &Request,
                                                           const EWAINInteractionError    Error,
                                                           const FString                 &Message)
    {
        FWAINInteractionResult Result;
        Result.TurnId = Request.TurnId;
        Result.Error = Error;
        Result.ErrorMessage = Message;
        return Result;
    }

#pragma endregion
