/**
 * @file WAINMemoryReasoningTests.cpp
 * @brief Reflection 출처, Plan 재사용/재생성 및 실패 시 Snapshot 불변성을 검증한다.
 */

#include "Core/WAINInteractionProcessor.h"
#include "Core/WAINStateValidation.h"
#include "Testing/WAINMapGameAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Reasoning Test Fixtures
// ============================================================
#pragma region 00_Reasoning_Test_Fixtures

    // 결정론적 생성기와 최소 세션을 사용해 실제 LLM 없이 상태 전이를 검증한다.

    namespace WAINMemoryReasoningTests
    {
        class FProposalProvider final : public IWAINProposalProvider
        {
            public:
            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                OutProposal.Dialogue = TEXT("약속을 기억하고 있어.");
                return true;
            }
        };

        class FCapturingProposalProvider final : public IWAINProposalProvider
        {
            public:
            mutable TArray<FWAINCharacterContext> Contexts;
            bool                                  bInvalidGameMutation = false;

            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                return false;
            }

            virtual bool GenerateWithContext(const FWAINInteractionRequest &Request,
                                             const FWAINSessionRoot        &Current,
                                             const FWAINCharacterContext   &Context,
                                             FWAINResponseProposal         &OutProposal,
                                             FString                       &OutError) const override
            {
                Contexts.Add(Context);
                OutProposal.Dialogue = TEXT("약속을 기억하고 있어.");
                if (bInvalidGameMutation)
                {
                    FWAINStateMutation Mutation;
                    Mutation.Owner = EWAINMutationOwner::Game;
                    Mutation.TargetId = TEXT("inventory.unknown");
                    Mutation.Operation = EWAINMutationOperation::AddInteger;
                    Mutation.IntegerValue = -1;
                    OutProposal.StateChanges.Add(Mutation);
                }
                return true;
            }
        };

        class FDeterministicReasoner final : public IWAINMemoryReasoner
        {
            public:
            mutable int32                             ReflectionCalls = 0;
            mutable int32                             PlanCalls = 0;
            mutable TArray<TArray<FWAINMemoryRecord>> ReflectionSources;
            mutable TArray<TArray<FWAINMemoryRecord>> PlanSources;
            bool                                      bInvalidReflection = false;
            bool                                      bInvalidPlan = false;
            bool                                      bFailReflection = false;

            virtual bool GenerateReflection(const FWAINInteractionRequest   &Request,
                                            const TArray<FWAINMemoryRecord> &Sources,
                                            FString                         &OutInsight,
                                            FString                         &OutError) const override
            {
                ++ReflectionCalls;
                ReflectionSources.Add(Sources);
                if (bFailReflection)
                {
                    OutError = TEXT("Mock Reflection 실패");
                    return false;
                }
                OutInsight = bInvalidReflection ? TEXT("") : TEXT("약속을 반복해서 중요하게 여긴다.");
                return true;
            }

            virtual bool GeneratePlan(const FWAINInteractionRequest   &Request,
                                      const FName                      ActiveBeatId,
                                      const TArray<FWAINMemoryRecord> &Sources,
                                      FWAINPlanDraft                  &OutPlan,
                                      FString                         &OutError) const override
            {
                ++PlanCalls;
                PlanSources.Add(Sources);
                OutPlan.Summary = TEXT("이번 장면의 약속을 확인한다.");
                OutPlan.Steps =
                    bInvalidPlan ? TArray<FString>() : TArray<FString>{TEXT("대화 이어가기"), TEXT("다음 단서 확인")};
                return true;
            }
        };

        class FBeatTransitionEvaluator final : public IWAINNarrativeEvaluator
        {
            public:
            virtual bool Evaluate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINStateSnapshot            &PreparedWAIN,
                                  const IWAINGameStateSnapshot  &PreparedGame,
                                  FString                       &OutError) const override
            {
                if (Current.WAIN.TurnCount == 1)
                {
                    PreparedWAIN.ActiveBeatId = TEXT("NEXT_BEAT");
                }
                return true;
            }
        };

        FWAINStateSnapshot MakeState()
        {
            FWAINStateSnapshot State;
            State.ActiveBeatId = TEXT("INTRO");
            State.Characters.Add(TEXT("MANA"), FWAINCharacterState());
            State.Characters.Add(TEXT("HANA"), FWAINCharacterState());
            return State;
        }

        FWAINInteractionRequest MakeRequest(const FName CharacterId = TEXT("MANA"), const FName SceneId = TEXT("CLASSROOM"))
        {
            FWAINInteractionRequest Request;
            Request.TurnId = FGuid::NewGuid();
            Request.CharacterId = CharacterId;
            Request.SceneId = SceneId;
            Request.InputKind = EWAINInputKind::Text;
            Request.Text = TEXT("우리 약속 기억하지?");
            return Request;
        }

        int32 CountType(const FWAINStateSnapshot &State, const EWAINMemoryType Type)
        {
            int32 Count = 0;
            for (const FWAINMemoryRecord &Memory : State.MemoryStream)
            {
                if (Memory.Type == Type)
                    ++Count;
            }
            return Count;
        }

        TUniquePtr<FWAINInteractionProcessor>
        MakeProcessor(TSharedRef<IWAINProposalProvider>     ProposalProvider,
                      TSharedPtr<const IWAINMemoryReasoner> Reasoner,
                      FString                              &OutError,
                      TSharedRef<IWAINNarrativeEvaluator>   Evaluator = MakeShared<FWAINNoOpNarrativeEvaluator>())
        {
            TMap<FName, int32> Inventory;
            Inventory.Add(TEXT("inventory.gift"), 2);
            auto Adapter = MakeShared<FWAINMapGameAdapter>(MoveTemp(Inventory));
            auto Processor = MakeUnique<FWAINInteractionProcessor>(Adapter,
                                                                   MoveTemp(ProposalProvider),
                                                                   MoveTemp(Evaluator),
                                                                   nullptr,
                                                                   nullptr,
                                                                   MoveTemp(Reasoner));
            if (!Processor->StartSession(MakeState(), OutError))
                return nullptr;
            return Processor;
        }
    } // namespace WAINMemoryReasoningTests

#pragma endregion

// ============================================================
// 01. Reflection And Plan Commit
// ============================================================
#pragma region 01_Reflection_And_Plan_Commit

    // 첫 계획 생성과 Reflection Trigger, 다음 턴 문맥 조회를 검사한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningPlanCreatedTest,
                                     "WAIN.MemoryReasoning.PlanCreated",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningPlanCreatedTest::RunTest(const FString &Parameters)
    {
        auto    Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        FString Error;
        auto Processor = WAINMemoryReasoningTests::MakeProcessor(MakeShared<WAINMemoryReasoningTests::FProposalProvider>(),
                                                                 Reasoner,
                                                                 Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Previous = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        const auto Current = Processor->GetSnapshot();
        TestTrue(TEXT("Commit 성공"), Result.Succeeded());
        TestEqual(TEXT("첫 턴은 Plan 하나"), WAINMemoryReasoningTests::CountType(Current->WAIN, EWAINMemoryType::Plan), 1);
        TestEqual(TEXT("첫 턴은 Reflection 없음"), Reasoner->ReflectionCalls, 0);
        TestEqual(TEXT("Plan 생성 한 번"), Reasoner->PlanCalls, 1);
        TestEqual(TEXT("이전 Snapshot 불변"), Previous->WAIN.MemoryStream.Num(), 0);
        const FGuid *ActiveId = Current->WAIN.ActivePlanIds.Find(TEXT("MANA"));
        if (!TestNotNull(TEXT("활성 Plan ID"), ActiveId))
            return false;
        const auto &Plan = Current->WAIN.MemoryStream.Last();
        TestTrue(TEXT("Plan ID 연결"), Plan.Id == *ActiveId);
        TestEqual(TEXT("Plan Step"), Plan.PlanSteps.Num(), 2);
        TestEqual(TEXT("출처 Observation 2건"), Plan.SourceMemoryIds.Num(), 2);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningReflectionTriggerTest,
                                     "WAIN.MemoryReasoning.ReflectionTrigger",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningReflectionTriggerTest::RunTest(const FString &Parameters)
    {
        auto    Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        FString Error;
        auto Processor = WAINMemoryReasoningTests::MakeProcessor(MakeShared<WAINMemoryReasoningTests::FProposalProvider>(),
                                                                 Reasoner,
                                                                 Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        const auto Result = Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        const auto State = Processor->GetSnapshot();
        TestTrue(TEXT("두 번째 Commit 성공"), Result.Succeeded());
        TestEqual(TEXT("Reflection은 두 번째 턴에서 생성"), Reasoner->ReflectionCalls, 1);
        TestEqual(TEXT("Plan은 재사용"), Reasoner->PlanCalls, 1);
        TestEqual(TEXT("Memory 4 Obs + 1 Plan + 1 Reflection"), State->WAIN.MemoryStream.Num(), 6);
        const auto &Reflection = State->WAIN.MemoryStream.Last();
        TestTrue(TEXT("Reflection 타입"), Reflection.Type == EWAINMemoryType::Reflection);
        TestEqual(TEXT("Reflection 출처 4건"), Reflection.SourceMemoryIds.Num(), 4);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningContextTest,
                                     "WAIN.MemoryReasoning.ContextIncludesDerivedMemory",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningContextTest::RunTest(const FString &Parameters)
    {
        auto    Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        auto    Provider = MakeShared<WAINMemoryReasoningTests::FCapturingProposalProvider>();
        FString Error;
        auto    Processor = WAINMemoryReasoningTests::MakeProcessor(Provider, Reasoner, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        TestEqual(TEXT("Context 캡처"), Provider->Contexts.Num(), 3);
        if (Provider->Contexts.Num() != 3)
            return false;

        bool bHasReflection = false;
        bool bHasPlan = false;
        for (const FWAINMemoryRecord &Memory : Provider->Contexts.Last().RelevantMemories)
        {
            bHasReflection |= Memory.Type == EWAINMemoryType::Reflection;
            bHasPlan |= Memory.Type == EWAINMemoryType::Plan;
        }
        TestTrue(TEXT("Reflection Context 포함"), bHasReflection);
        TestTrue(TEXT("활성 Plan Context 포함"), bHasPlan);
        return true;
    }

#pragma endregion

// ============================================================
// 02. Plan Reuse And Invalidation
// ============================================================
#pragma region 02_Plan_Reuse_And_Invalidation

    // Scene·Beat·Character 경계에서 활성 계획의 교체 여부를 검사한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningSceneReplanTest,
                                     "WAIN.MemoryReasoning.SceneChangeReplans",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningSceneReplanTest::RunTest(const FString &Parameters)
    {
        auto    Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        FString Error;
        auto Processor = WAINMemoryReasoningTests::MakeProcessor(MakeShared<WAINMemoryReasoningTests::FProposalProvider>(),
                                                                 Reasoner,
                                                                 Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        const auto FirstPlanId = *Processor->GetSnapshot()->WAIN.ActivePlanIds.Find(TEXT("MANA"));
        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        TestEqual(TEXT("같은 Scene은 Plan 재사용"), Reasoner->PlanCalls, 1);

        const auto NewScene = Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest(TEXT("MANA"), TEXT("ROOFTOP")));
        TestTrue(TEXT("Scene 변경 턴 성공"), NewScene.Succeeded());
        TestEqual(TEXT("새 Scene은 Plan 재생성"), Reasoner->PlanCalls, 2);
        TestTrue(TEXT("활성 Plan ID 변경"),
                 *Processor->GetSnapshot()->WAIN.ActivePlanIds.Find(TEXT("MANA")) != FirstPlanId);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningBeatReplanTest,
                                     "WAIN.MemoryReasoning.BeatChangeReplans",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningBeatReplanTest::RunTest(const FString &Parameters)
    {
        auto    Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        FString Error;
        auto    Processor =
            WAINMemoryReasoningTests::MakeProcessor(MakeShared<WAINMemoryReasoningTests::FProposalProvider>(),
                                                    Reasoner,
                                                    Error,
                                                    MakeShared<WAINMemoryReasoningTests::FBeatTransitionEvaluator>());
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        const auto Second = Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        TestTrue(TEXT("Beat 변경 성공"), Second.Succeeded());
        TestEqual(TEXT("Beat 변경 시 Plan 새로 생성"), Reasoner->PlanCalls, 2);
        TestEqual(TEXT("새 Plan Beat"),
                  Processor->GetSnapshot()->WAIN.MemoryStream.Last().BeatId,
                  FName(TEXT("NEXT_BEAT")));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningCharacterIsolationTest,
                                     "WAIN.MemoryReasoning.CharacterIsolation",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningCharacterIsolationTest::RunTest(const FString &Parameters)
    {
        auto    Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        FString Error;
        auto Processor = WAINMemoryReasoningTests::MakeProcessor(MakeShared<WAINMemoryReasoningTests::FProposalProvider>(),
                                                                 Reasoner,
                                                                 Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest(TEXT("MANA")));
        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest(TEXT("HANA")));
        TestEqual(TEXT("서로 다른 캐릭터의 Plan 생성"), Reasoner->PlanCalls, 2);
        TestEqual(TEXT("캐릭터별 Active Plan"), Processor->GetSnapshot()->WAIN.ActivePlanIds.Num(), 2);
        TestEqual(TEXT("서로 다른 캐릭터는 Reflection 없음"), Reasoner->ReflectionCalls, 0);
        return true;
    }

#pragma endregion

// ============================================================
// 03. Failure, Replay, And State Validation
// ============================================================
#pragma region 03_Failure_Replay_And_State_Validation

    // 생성 실패·Game Prepare 실패·Replay 시 Commit 불변성을 검사한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningInvalidPlanNoCommitTest,
                                     "WAIN.MemoryReasoning.InvalidPlanNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningInvalidPlanNoCommitTest::RunTest(const FString &Parameters)
    {
        auto Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        Reasoner->bInvalidPlan = true;
        FString Error;
        auto Processor = WAINMemoryReasoningTests::MakeProcessor(MakeShared<WAINMemoryReasoningTests::FProposalProvider>(),
                                                                 Reasoner,
                                                                 Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        TestTrue(TEXT("Plan 오류 반환"), Result.Error == EWAINInteractionError::MemoryReasoningFailed);
        TestTrue(TEXT("Current Root 동일"), Before.Get() == Processor->GetSnapshot().Get());
        TestEqual(TEXT("Memory 미적용"), Processor->GetSnapshot()->WAIN.MemoryStream.Num(), 0);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningInvalidReflectionNoCommitTest,
                                     "WAIN.MemoryReasoning.InvalidReflectionNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningInvalidReflectionNoCommitTest::RunTest(const FString &Parameters)
    {
        auto    Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        FString Error;
        auto Processor = WAINMemoryReasoningTests::MakeProcessor(MakeShared<WAINMemoryReasoningTests::FProposalProvider>(),
                                                                 Reasoner,
                                                                 Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        const auto Before = Processor->GetSnapshot();
        Reasoner->bInvalidReflection = true;
        const auto Result = Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        TestTrue(TEXT("Reflection 실패 반환"), Result.Error == EWAINInteractionError::MemoryReasoningFailed);
        TestTrue(TEXT("확정 Snapshot 불변"), Before.Get() == Processor->GetSnapshot().Get());
        TestEqual(TEXT("Reflection 미기록"), Reasoner->ReflectionCalls, 1);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningGamePrepareFailureTest,
                                     "WAIN.MemoryReasoning.GameFailureSkipsReasoning",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningGamePrepareFailureTest::RunTest(const FString &Parameters)
    {
        auto Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        auto Provider = MakeShared<WAINMemoryReasoningTests::FCapturingProposalProvider>();
        Provider->bInvalidGameMutation = true;
        FString Error;
        auto    Processor = WAINMemoryReasoningTests::MakeProcessor(Provider, Reasoner, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINMemoryReasoningTests::MakeRequest());
        TestTrue(TEXT("Game Prepare 실패"), Result.Error == EWAINInteractionError::GamePrepareFailed);
        TestEqual(TEXT("Reasoner 미호출"), Reasoner->PlanCalls + Reasoner->ReflectionCalls, 0);
        TestTrue(TEXT("No Commit"), Before.Get() == Processor->GetSnapshot().Get());
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningReplayTest,
                                     "WAIN.MemoryReasoning.ReplayDoesNotRegenerate",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningReplayTest::RunTest(const FString &Parameters)
    {
        auto    Reasoner = MakeShared<WAINMemoryReasoningTests::FDeterministicReasoner>();
        FString Error;
        auto Processor = WAINMemoryReasoningTests::MakeProcessor(MakeShared<WAINMemoryReasoningTests::FProposalProvider>(),
                                                                 Reasoner,
                                                                 Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Request = WAINMemoryReasoningTests::MakeRequest();
        const auto First = Processor->ProcessTurn(Request);
        const auto Replay = Processor->ProcessTurn(Request);
        TestTrue(TEXT("첫 응답 성공"), First.Succeeded());
        TestTrue(TEXT("재전송 Replay"), Replay.bReplayed);
        TestEqual(TEXT("Plan 한 번"), Reasoner->PlanCalls, 1);
        TestEqual(TEXT("Revision 증가 없음"), Processor->GetSnapshot()->Revision, uint64(1));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINReasoningInvalidActivePlanTest,
                                     "WAIN.MemoryReasoning.InvalidActivePlanRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINReasoningInvalidActivePlanTest::RunTest(const FString &Parameters)
    {
        FWAINStateSnapshot Invalid = WAINMemoryReasoningTests::MakeState();
        Invalid.ActivePlanIds.Add(TEXT("MANA"), FGuid::NewGuid());
        FString Error;
        TestFalse(TEXT("존재하지 않는 활성 Plan 거절"), WAINStateValidation::Validate(Invalid, Error));
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
