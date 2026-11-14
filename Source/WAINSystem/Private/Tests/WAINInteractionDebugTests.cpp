/**
 * @file WAINInteractionDebugTests.cpp
 * @brief Turn Trace가 실제 Proposal/Commit/실패 단계만 기록하고 Replay에 영향을 주지 않는지 검증한다.
 */

#include "Core/WAINInteractionProcessor.h"
#include "Testing/WAINMapGameAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Debug Trace Fixtures
// ============================================================
#pragma region 00_Debug_Trace_Fixtures

    namespace WAINInteractionDebugTests
    {
        class FProposalProvider final : public IWAINProposalProvider
        {
            public:
            bool         bEmptyDialogue = false;
            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                OutProposal.Dialogue = bEmptyDialogue ? TEXT("") : TEXT("약속을 기억해.");
                FWAINStateMutation Mutation;
                Mutation.Owner = EWAINMutationOwner::Character;
                Mutation.TargetId = TEXT("relationship.trust");
                Mutation.Operation = EWAINMutationOperation::AddInteger;
                Mutation.IntegerValue = 2;
                OutProposal.StateChanges.Add(Mutation);
                return true;
            }
        };

        FWAINInteractionRequest MakeRequest()
        {
            FWAINInteractionRequest Request;
            Request.TurnId = FGuid::NewGuid();
            Request.CharacterId = TEXT("MANA");
            Request.SceneId = TEXT("CLASSROOM");
            Request.InputKind = EWAINInputKind::Text;
            Request.Text = TEXT("안녕");
            return Request;
        }

        TUniquePtr<FWAINInteractionProcessor> MakeProcessor(TSharedRef<FProposalProvider> Provider, FString &OutError)
        {
            TMap<FName, int32> World;
            World.Add(TEXT("flag.a"), 0);
            auto               Adapter = MakeShared<FWAINMapGameAdapter>(MoveTemp(World));
            auto               Processor = MakeUnique<FWAINInteractionProcessor>(Adapter,
                                                                                 MoveTemp(Provider),
                                                                                 MakeShared<FWAINNoOpNarrativeEvaluator>());
            FWAINStateSnapshot State;
            State.ActiveBeatId = TEXT("INTRO");
            State.Characters.Add(TEXT("MANA"), FWAINCharacterState());
            if (!Processor->StartSession(State, OutError))
                return nullptr;
            return Processor;
        }

        const FWAINTraceStep *FindStep(const FWAINInteractionDebugRecord &Record, FName Name)
        {
            return Record.Steps.FindByPredicate([Name](const FWAINTraceStep &Step) { return Step.Name == Name; });
        }
    } // namespace WAINInteractionDebugTests

#pragma endregion

// ============================================================
// 01. Captured Pipeline Results
// ============================================================
#pragma region 01_Captured_Pipeline_Results

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINTraceCommitTest,
                                     "WAIN.DebugTrace.SuccessfulTurn",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINTraceCommitTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINInteractionDebugTests::FProposalProvider>();
        FString Error;
        auto    Processor = WAINInteractionDebugTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto                  Request = WAINInteractionDebugTests::MakeRequest();
        const auto                  Result = Processor->ProcessTurn(Request);
        FWAINInteractionDebugRecord Trace;
        TestTrue(TEXT("턴 성공"), Result.Succeeded());
        if (!TestTrue(TEXT("기록 조회"), Processor->GetDebugRecord(Request.TurnId, Trace)))
            return false;
        TestTrue(TEXT("Context 기록"), Trace.bHasContext);
        TestTrue(TEXT("Proposal 기록"), Trace.bHasProposal);
        TestEqual(TEXT("제안 변경 건수"), Trace.Proposal.StateChanges.Num(), 1);
        TestEqual(TEXT("Revision 이전"), Trace.RevisionBefore, uint64(0));
        TestEqual(TEXT("Revision 이후"), Trace.RevisionAfter, uint64(1));
        const auto *Commit = WAINInteractionDebugTests::FindStep(Trace, TEXT("Atomic Publish"));
        TestTrue(TEXT("Commit 단계 존재"), Commit != nullptr);
        if (Commit)
            TestTrue(TEXT("실제로 Commit된 단계만 PASS"), Commit->Status == EWAINTraceStatus::Passed);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINTraceFailedProposalTest,
                                     "WAIN.DebugTrace.FailedProposalNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINTraceFailedProposalTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINInteractionDebugTests::FProposalProvider>();
        Provider->bEmptyDialogue = true;
        FString Error;
        auto    Processor = WAINInteractionDebugTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto                  Request = WAINInteractionDebugTests::MakeRequest();
        const auto                  Result = Processor->ProcessTurn(Request);
        FWAINInteractionDebugRecord Trace;
        if (!TestTrue(TEXT("실패 기록 조회"), Processor->GetDebugRecord(Request.TurnId, Trace)))
            return false;
        TestTrue(TEXT("제안 실패"), Result.Error == EWAINInteractionError::InvalidProposal);
        const auto *Invalid = WAINInteractionDebugTests::FindStep(Trace, TEXT("Proposal Validation"));
        TestTrue(TEXT("거절 단계 존재"), Invalid != nullptr);
        if (Invalid)
            TestTrue(TEXT("실제 FAIL"), Invalid->Status == EWAINTraceStatus::Failed);
        TestTrue(TEXT("수행되지 않은 Commit 단계 없음"),
                 WAINInteractionDebugTests::FindStep(Trace, TEXT("Atomic Publish")) == nullptr);
        TestEqual(TEXT("Revision 무변경"), Trace.RevisionAfter, uint64(0));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINTraceReplayTest,
                                     "WAIN.DebugTrace.ReplayPreservesOriginal",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINTraceReplayTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINInteractionDebugTests::FProposalProvider>();
        FString Error;
        auto    Processor = WAINInteractionDebugTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Request = WAINInteractionDebugTests::MakeRequest();
        Processor->ProcessTurn(Request);
        FWAINInteractionDebugRecord Before;
        Processor->GetDebugRecord(Request.TurnId, Before);
        const auto                  Replay = Processor->ProcessTurn(Request);
        FWAINInteractionDebugRecord After;
        Processor->GetDebugRecord(Request.TurnId, After);
        TestTrue(TEXT("Replay 반환"), Replay.bReplayed);
        TestEqual(TEXT("원본 단계 수 불변"), After.Steps.Num(), Before.Steps.Num());
        TestEqual(TEXT("원본 Commit Revision 불변"), After.RevisionAfter, uint64(1));
        TestEqual(TEXT("권위 Revision 중복 증가 없음"), Processor->GetSnapshot()->Revision, uint64(1));
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
