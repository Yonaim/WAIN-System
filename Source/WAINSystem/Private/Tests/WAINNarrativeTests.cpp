/**
 * @file WAINNarrativeTests.cpp
 * @brief Beat Graph 서사 Context·조건·Hard Rule·원자적 No Commit을 검증한다.
 */

#include "Core/WAINInteractionProcessor.h"
#include "Testing/WAINMapGameAdapter.h"
#include "WAINBeatGraphNarrativeEvaluator.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Narrative Test Fixtures
// ============================================================
#pragma region 00_Narrative_Test_Fixtures

    // Host Fact와 Graph를 고정하여 전이·실패 상태를 재현 가능하게 만든다.

    namespace WAINNarrativeTests
    {
        FWAINNarrativeCondition TrustAtLeast(const int32 Value)
        {
            FWAINNarrativeCondition Condition;
            Condition.Source = EWAINNarrativeFact::CharacterTrust;
            Condition.Comparison = EWAINNarrativeComparison::AtLeast;
            Condition.Value = Value;
            return Condition;
        }

        FWAINNarrativeCondition GameAtLeast(const FName TargetId, const int32 Value)
        {
            FWAINNarrativeCondition Condition;
            Condition.Source = EWAINNarrativeFact::GameInteger;
            Condition.TargetId = TargetId;
            Condition.Comparison = EWAINNarrativeComparison::AtLeast;
            Condition.Value = Value;
            return Condition;
        }

        FWAINNarrativeCondition TurnAtLeast(const int32 Value)
        {
            FWAINNarrativeCondition Condition;
            Condition.Source = EWAINNarrativeFact::TurnCount;
            Condition.Comparison = EWAINNarrativeComparison::AtLeast;
            Condition.Value = Value;
            return Condition;
        }

        FWAINNarrativeGraph MakeGraph()
        {
            FWAINNarrativeGraph Graph;

            FWAINNarrativeChapter Chapter;
            Chapter.Id = TEXT("CH01");
            Chapter.Goal = TEXT("두 인물의 신뢰를 쌓는다.");
            Chapter.SoftConstraints.Add(TEXT("아직 밝히지 않은 비밀을 직접 언급하지 않는다."));
            FWAINNarrativeHardRule SecretRule;
            SecretRule.GuardedGameTargetId = TEXT("story.secret");
            SecretRule.Prerequisites.Add(TrustAtLeast(60));
            Chapter.HardRules.Add(SecretRule);
            Graph.Chapters.Add(Chapter);

            FWAINNarrativeSequence Sequence;
            Sequence.Id = TEXT("SQ01");
            Sequence.ChapterId = Chapter.Id;
            Sequence.Goal = TEXT("전학생과 친해진다.");
            Graph.Sequences.Add(Sequence);

            FWAINNarrativeBeat Intro;
            Intro.Id = TEXT("INTRO");
            Intro.SequenceId = Sequence.Id;
            Intro.Goal = TEXT("첫 대화를 마친다.");
            Intro.CompletionConditions.Add(GameAtLeast(TEXT("story.step"), 1));
            FWAINBeatTransition HighTrust;
            HighTrust.TargetBeatId = TEXT("TRANSFER_HINT");
            HighTrust.Priority = 10;
            HighTrust.Conditions.Add(TrustAtLeast(50));
            Intro.Transitions.Add(HighTrust);
            FWAINBeatTransition LowTrust;
            LowTrust.TargetBeatId = TEXT("LOW_TRUST");
            LowTrust.Priority = 1;
            LowTrust.Conditions.Add(TrustAtLeast(0));
            Intro.Transitions.Add(LowTrust);
            Graph.Beats.Add(Intro);

            FWAINNarrativeBeat Hint;
            Hint.Id = TEXT("TRANSFER_HINT");
            Hint.SequenceId = Sequence.Id;
            Hint.Goal = TEXT("전학 단서를 이해한다.");
            Hint.CompletionConditions.Add(TurnAtLeast(3));
            FWAINBeatTransition Next;
            Next.TargetBeatId = TEXT("MEMORY_DOUBT");
            Next.Priority = 1;
            Hint.Transitions.Add(Next);
            Graph.Beats.Add(Hint);

            FWAINNarrativeBeat Low;
            Low.Id = TEXT("LOW_TRUST");
            Low.SequenceId = Sequence.Id;
            Low.Goal = TEXT("거리를 유지한다.");
            Graph.Beats.Add(Low);

            FWAINNarrativeBeat Doubt;
            Doubt.Id = TEXT("MEMORY_DOUBT");
            Doubt.SequenceId = Sequence.Id;
            Doubt.Goal = TEXT("과거의 기억을 의심한다.");
            Graph.Beats.Add(Doubt);
            return Graph;
        }

        FWAINStateSnapshot MakeState(const int32 Trust = 45)
        {
            FWAINStateSnapshot  State;
            FWAINCharacterState Character;
            Character.Trust = Trust;
            State.Characters.Add(TEXT("MANA"), Character);
            State.ActiveBeatId = TEXT("INTRO");
            return State;
        }

        FWAINInteractionRequest MakeRequest()
        {
            FWAINInteractionRequest Request;
            Request.TurnId = FGuid::NewGuid();
            Request.CharacterId = TEXT("MANA");
            Request.SceneId = TEXT("CLASSROOM");
            Request.InputKind = EWAINInputKind::Text;
            Request.Text = TEXT("약속 기억하지?");
            return Request;
        }

        FWAINStateMutation GameSet(const FName Id, const int32 Value)
        {
            FWAINStateMutation Mutation;
            Mutation.Owner = EWAINMutationOwner::Game;
            Mutation.TargetId = Id;
            Mutation.Operation = EWAINMutationOperation::SetInteger;
            Mutation.IntegerValue = Value;
            return Mutation;
        }

        FWAINStateMutation AddTrust(const int32 Delta)
        {
            FWAINStateMutation Mutation;
            Mutation.Owner = EWAINMutationOwner::Character;
            Mutation.TargetId = TEXT("relationship.trust");
            Mutation.Operation = EWAINMutationOperation::AddInteger;
            Mutation.IntegerValue = Delta;
            return Mutation;
        }

        class FContextCapturingProvider final : public IWAINProposalProvider
        {
            public:
            FWAINResponseProposal         Proposal;
            mutable int32                 ContextCalls = 0;
            mutable FWAINNarrativeContext LastNarrative;

            FContextCapturingProvider()
            {
                Proposal.Dialogue = TEXT("좋아, 기억하고 있어.");
            }

            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                OutProposal = Proposal;
                return true;
            }

            virtual bool GenerateWithContext(const FWAINInteractionRequest &Request,
                                             const FWAINSessionRoot        &Current,
                                             const FWAINCharacterContext   &Context,
                                             FWAINResponseProposal         &OutProposal,
                                             FString                       &OutError) const override
            {
                ++ContextCalls;
                LastNarrative = Context.Narrative;
                return Generate(Request, Current, OutProposal, OutError);
            }
        };

        TUniquePtr<FWAINInteractionProcessor> MakeProcessor(TSharedRef<IWAINProposalProvider> Provider,
                                                            FWAINNarrativeGraph               Graph,
                                                            FString                          &OutError,
                                                            const int32                       Trust = 45)
        {
            TMap<FName, int32> Values;
            Values.Add(TEXT("story.step"), 0);
            Values.Add(TEXT("story.secret"), 0);
            auto                                           Adapter = MakeShared<FWAINMapGameAdapter>(MoveTemp(Values));
            TSharedPtr<const IWAINNarrativeGameFactReader> FactReader = Adapter;
            auto Evaluator = MakeShared<FWAINBeatGraphNarrativeEvaluator>(MoveTemp(Graph), MoveTemp(FactReader));
            auto Processor = MakeUnique<FWAINInteractionProcessor>(Adapter, MoveTemp(Provider), Evaluator);
            if (!Processor->StartSession(MakeState(Trust), OutError))
            {
                return nullptr;
            }
            return Processor;
        }
    } // namespace WAINNarrativeTests

#pragma endregion

// ============================================================
// 01. Context And Transition Tests
// ============================================================
#pragma region 01_Context_And_Transition_Tests

    // Current Context와 Prepared State 기반 단일 전이·우선순위를 검증한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativeContextTest,
                                     "WAIN.Narrative.ContextGoals",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativeContextTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, WAINNarrativeTests::MakeGraph(), Error);
        if (!TestTrue(TEXT("Graph/세션 검증"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        TestTrue(TEXT("상태 변경 없는 대화"), Result.Succeeded());
        TestEqual(TEXT("Context 전달 횟수"), Provider->ContextCalls, 1);
        TestTrue(TEXT("Chapter ID"), Provider->LastNarrative.ChapterId == TEXT("CH01"));
        TestTrue(TEXT("현재 Beat 목표"), Provider->LastNarrative.BeatId == TEXT("INTRO"));
        TestEqual(TEXT("부모 Constraint 포함"), Provider->LastNarrative.SoftConstraints.Num(), 1);
        TestTrue(TEXT("미완료 Beat 유지"), Processor->GetSnapshot()->WAIN.ActiveBeatId == TEXT("INTRO"));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativePreparedTransitionTest,
                                     "WAIN.Narrative.PreparedTransition",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativePreparedTransitionTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::GameSet(TEXT("story.step"), 1));
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::AddTrust(10));
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, WAINNarrativeTests::MakeGraph(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;
        const auto Before = Processor->GetSnapshot();

        const auto Result = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        const auto After = Processor->GetSnapshot();
        TestTrue(TEXT("Commit 성공"), Result.Succeeded());
        TestTrue(TEXT("Prepared Trust 55 기준 상위 전이"), After->WAIN.ActiveBeatId == TEXT("TRANSFER_HINT"));
        TestEqual(TEXT("전이 이력 1건"), After->WAIN.BeatHistory.Num(), 1);
        TestTrue(TEXT("이전 Root 유지"), Before->WAIN.ActiveBeatId == TEXT("INTRO"));
        TestEqual(TEXT("Revision 1"), static_cast<int32>(After->Revision), 1);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativeNotCompletedTest,
                                     "WAIN.Narrative.IncompleteBeat",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativeNotCompletedTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::AddTrust(10));
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, WAINNarrativeTests::MakeGraph(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        TestTrue(TEXT("Commit 성공"), Result.Succeeded());
        TestTrue(TEXT("완료 조건 불충족"), Processor->GetSnapshot()->WAIN.ActiveBeatId == TEXT("INTRO"));
        TestEqual(TEXT("전이 이력 없음"), Processor->GetSnapshot()->WAIN.BeatHistory.Num(), 0);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativeTurnProgressTest,
                                     "WAIN.Narrative.TurnProgress",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativeTurnProgressTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::GameSet(TEXT("story.step"), 1));
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::AddTrust(5));
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, WAINNarrativeTests::MakeGraph(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto First = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        const auto Second = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        TestTrue(TEXT("첫 두 턴 성공"), First.Succeeded() && Second.Succeeded());
        TestTrue(TEXT("두 번째 턴까지 Hint 유지"), Processor->GetSnapshot()->WAIN.ActiveBeatId == TEXT("TRANSFER_HINT"));
        const auto Third = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        TestTrue(TEXT("세 번째 턴 성공"), Third.Succeeded());
        TestTrue(TEXT("TurnCount 3에서 이동"), Processor->GetSnapshot()->WAIN.ActiveBeatId == TEXT("MEMORY_DOUBT"));
        TestEqual(TEXT("턴당 1회, 누적 2회 전이"), Processor->GetSnapshot()->WAIN.BeatHistory.Num(), 2);
        return true;
    }

#pragma endregion

// ============================================================
// 02. Failure And Idempotency Tests
// ============================================================
#pragma region 02_Failure_And_Idempotency_Tests

    // 정의 오류·조건 조회 실패·규칙 위반은 Root Publication 이전에 거절한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativeInvalidGraphTest,
                                     "WAIN.Narrative.InvalidGraph",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativeInvalidGraphTest::RunTest(const FString &Parameters)
    {
        auto Graph = WAINNarrativeTests::MakeGraph();
        Graph.Beats[0].Transitions[0].TargetBeatId = TEXT("UNKNOWN");
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(MakeShared<WAINNarrativeTests::FContextCapturingProvider>(),
                                                              MoveTemp(Graph),
                                                              Error);
        TestTrue(TEXT("미존재 Beat 참조는 세션 시작 거절"), Processor == nullptr);
        TestFalse(TEXT("오류 설명 존재"), Error.IsEmpty());
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativeHardRuleRejectedTest,
                                     "WAIN.Narrative.HardRuleReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativeHardRuleRejectedTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::GameSet(TEXT("story.secret"), 1));
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, WAINNarrativeTests::MakeGraph(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        TestTrue(TEXT("Narrative 거절"), Result.Error == EWAINInteractionError::NarrativeEvaluationFailed);
        TestTrue(TEXT("Revision 무변경"), Before == Processor->GetSnapshot());
        TestEqual(TEXT("Observation 미기록"), Processor->GetSnapshot()->WAIN.MemoryStream.Num(), 0);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativeHardRulePreparedTest,
                                     "WAIN.Narrative.HardRulePreparedTrust",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativeHardRulePreparedTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::GameSet(TEXT("story.secret"), 1));
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::AddTrust(15));
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, WAINNarrativeTests::MakeGraph(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        TestTrue(TEXT("같은 턴 Prepared Trust로 Rule 통과"), Result.Succeeded());
        TestEqual(TEXT("Trust 확정"), Processor->GetSnapshot()->WAIN.Characters.FindChecked(TEXT("MANA")).Trust, 60);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativePriorityTieTest,
                                     "WAIN.Narrative.PriorityTieReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativePriorityTieTest::RunTest(const FString &Parameters)
    {
        auto Graph = WAINNarrativeTests::MakeGraph();
        Graph.Beats[0].Transitions[1].Priority = 10;
        auto Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::GameSet(TEXT("story.step"), 1));
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::AddTrust(10));
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, MoveTemp(Graph), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        TestTrue(TEXT("동점 전이는 거절"), Result.Error == EWAINInteractionError::NarrativeEvaluationFailed);
        TestTrue(TEXT("No Commit"), Before == Processor->GetSnapshot());
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativeUnknownFactTest,
                                     "WAIN.Narrative.UnknownFactReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativeUnknownFactTest::RunTest(const FString &Parameters)
    {
        auto Graph = WAINNarrativeTests::MakeGraph();
        Graph.Beats[0].CompletionConditions[0].TargetId = TEXT("story.unknown");
        auto    Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, MoveTemp(Graph), Error);
        if (!TestTrue(TEXT("정의는 유효하지만 값 미등록"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINNarrativeTests::MakeRequest());
        TestTrue(TEXT("미등록 Fact 조회 실패"), Result.Error == EWAINInteractionError::NarrativeEvaluationFailed);
        TestEqual(TEXT("Commit 없음"), static_cast<int32>(Processor->GetSnapshot()->Revision), 0);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINNarrativeReplayTest,
                                     "WAIN.Narrative.ReplayNoDoubleTransition",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINNarrativeReplayTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINNarrativeTests::FContextCapturingProvider>();
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::GameSet(TEXT("story.step"), 1));
        Provider->Proposal.StateChanges.Add(WAINNarrativeTests::AddTrust(10));
        FString Error;
        auto    Processor = WAINNarrativeTests::MakeProcessor(Provider, WAINNarrativeTests::MakeGraph(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Request = WAINNarrativeTests::MakeRequest();
        const auto First = Processor->ProcessTurn(Request);
        const auto Replayed = Processor->ProcessTurn(Request);
        TestTrue(TEXT("최초 처리 성공"), First.Succeeded());
        TestTrue(TEXT("재전송 캐시 결과"), Replayed.bReplayed);
        TestEqual(TEXT("전이 이력 중복 없음"), Processor->GetSnapshot()->WAIN.BeatHistory.Num(), 1);
        TestEqual(TEXT("Revision 중복 없음"), static_cast<int32>(Processor->GetSnapshot()->Revision), 1);
        TestEqual(TEXT("Context 최초 1회만 생성"), Provider->ContextCalls, 1);
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
