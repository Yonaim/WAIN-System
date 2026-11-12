/**
 * @file WAINWorldStateTests.cpp
 * @brief World Schema 조회 권한·변경 제한·Prepared Game 검증·No Commit을 시험한다.
 */

#include "Core/WAINInteractionProcessor.h"
#include "Testing/WAINMapGameAdapter.h"
#include "WAINWorldStateManager.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. World Test Fixtures
// ============================================================
#pragma region 00_World_Test_Fixtures

    // Host 정의와 World Manager를 명시적으로 주입해 실제 Game State 소유권을 유지한다.

    namespace WAINWorldTests
    {
        FWAINWorldFactDefinition
        Definition(const FName Key, const int32 Min, const int32 Max, const bool bVisible, const bool bWritable)
        {
            FWAINWorldFactDefinition Fact;
            Fact.Key = Key;
            Fact.MinValue = Min;
            Fact.MaxValue = Max;
            Fact.bExposeToAgent = bVisible;
            Fact.bWritableByProposal = bWritable;
            return Fact;
        }

        TArray<FWAINWorldFactDefinition> MakeSchema()
        {
            TArray<FWAINWorldFactDefinition> Schema;
            Schema.Add(Definition(TEXT("story.step"), 0, 3, true, true));
            Schema.Add(Definition(TEXT("story.secret"), 0, 1, false, false));
            Schema.Add(Definition(TEXT("inventory.gift"), 0, 5, true, true));
            return Schema;
        }

        TMap<FName, int32> MakeValues()
        {
            TMap<FName, int32> Values;
            Values.Add(TEXT("story.step"), 0);
            Values.Add(TEXT("story.secret"), 1);
            Values.Add(TEXT("inventory.gift"), 2);
            return Values;
        }

        FWAINStateMutation Mutation(const FName Key, const EWAINMutationOperation Operation, const int32 Value)
        {
            FWAINStateMutation Change;
            Change.Owner = EWAINMutationOwner::Game;
            Change.TargetId = Key;
            Change.Operation = Operation;
            Change.IntegerValue = Value;
            return Change;
        }

        class FProvider final : public IWAINProposalProvider
        {
            public:
            FWAINResponseProposal     Proposal;
            mutable int32             Calls = 0;
            mutable FWAINWorldContext LastWorld;
            mutable bool              bReceivedWorld = false;

            FProvider()
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
                ++Calls;
                bReceivedWorld = Context.bHasWorldContext;
                LastWorld = Context.World;
                return Generate(Request, Current, OutProposal, OutError);
            }
        };

        /** @brief Prepared Snapshot을 의도적으로 왜곡해 Manager의 최종 검증을 확인한다. */
        class FDriftingAdapter final : public IWAINGameSnapshotAdapter, public IWAINWorldFactReader
        {
            public:
            FWAINMapGameAdapter Delegate;

            FDriftingAdapter()
                : Delegate(MakeValues())
            {
            }

            virtual TSharedPtr<const IWAINGameStateSnapshot> CreateInitialSnapshot(FString &OutError) const override
            {
                return Delegate.CreateInitialSnapshot(OutError);
            }

            virtual TSharedPtr<const IWAINGameStateSnapshot> Prepare(const IWAINGameStateSnapshot     &Current,
                                                                     const TArray<FWAINStateMutation> &Changes,
                                                                     FString &OutError) const override
            {
                const TSharedPtr<const IWAINGameStateSnapshot> Result = Delegate.Prepare(Current, Changes, OutError);
                if (!Result.IsValid())
                    return nullptr;

                auto Changed = MakeShared<FWAINMapGameSnapshot>();
                Changed->Values = static_cast<const FWAINMapGameSnapshot &>(*Result).Values;
                Changed->Values.FindChecked(TEXT("story.secret")) = 0; // 요청하지 않은 숨겨진 State 오염
                return Changed;
            }

            virtual bool Validate(const IWAINGameStateSnapshot &Prepared, FString &OutError) const override
            {
                return Delegate.Validate(Prepared, OutError);
            }

            virtual bool ReadInteger(const IWAINGameStateSnapshot &Snapshot, FName Key, int32 &OutValue) const override
            {
                return Delegate.ReadInteger(Snapshot, Key, OutValue);
            }
        };

        FWAINStateSnapshot MakeWAIN()
        {
            FWAINStateSnapshot State;
            State.ActiveBeatId = TEXT("INTRO");
            State.Characters.Add(TEXT("MANA"), FWAINCharacterState());
            return State;
        }

        FWAINInteractionRequest MakeRequest()
        {
            FWAINInteractionRequest Request;
            Request.TurnId = FGuid::NewGuid();
            Request.CharacterId = TEXT("MANA");
            Request.SceneId = TEXT("CLASSROOM");
            Request.InputKind = EWAINInputKind::Text;
            Request.Text = TEXT("교실은 어때?");
            return Request;
        }

        TUniquePtr<FWAINInteractionProcessor> MakeProcessor(const TSharedRef<FProvider>         &Provider,
                                                            FString                             &OutError,
                                                            TArray<FWAINWorldFactDefinition>     Schema = MakeSchema(),
                                                            TSharedPtr<IWAINGameSnapshotAdapter> OverrideAdapter = nullptr,
                                                            TSharedPtr<const IWAINWorldFactReader> OverrideReader = nullptr)
        {
            TSharedRef<FWAINMapGameAdapter>      DefaultAdapter = MakeShared<FWAINMapGameAdapter>(MakeValues());
            TSharedRef<IWAINGameSnapshotAdapter> Adapter =
                OverrideAdapter.IsValid() ? OverrideAdapter.ToSharedRef()
                                          : StaticCastSharedRef<IWAINGameSnapshotAdapter>(DefaultAdapter);
            TSharedPtr<const IWAINWorldFactReader> Reader =
                OverrideReader.IsValid() ? MoveTemp(OverrideReader)
                                         : StaticCastSharedRef<const IWAINWorldFactReader>(DefaultAdapter);
            auto Manager = MakeShared<FWAINWorldStateManager>(MoveTemp(Schema), MoveTemp(Reader));
            auto Processor = MakeUnique<FWAINInteractionProcessor>(Adapter,
                                                                   Provider,
                                                                   MakeShared<FWAINNoOpNarrativeEvaluator>(),
                                                                   nullptr,
                                                                   nullptr,
                                                                   nullptr,
                                                                   Manager);
            if (!Processor->StartSession(MakeWAIN(), OutError))
                return nullptr;
            return Processor;
        }
    } // namespace WAINWorldTests

#pragma endregion

// ============================================================
// 01. Context And Allowed Mutations
// ============================================================
#pragma region 01_Context_And_Allowed_Mutations

    // Schema에서 공개한 Fact만 Context에 전달하고 연속 Mutation을 올바르게 적용한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldVisibleFactsTest,
                                     "WAIN.World.VisibleContext",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldVisibleFactsTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINWorldTests::FProvider>();
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINWorldTests::MakeRequest());
        TestTrue(TEXT("대사 Commit 성공"), Result.Succeeded());
        TestTrue(TEXT("World Context 사용"), Provider->bReceivedWorld);
        TestEqual(TEXT("공개된 Fact 두 개"), Provider->LastWorld.VisibleFacts.Num(), 2);
        for (const auto &Fact : Provider->LastWorld.VisibleFacts)
        {
            TestFalse(TEXT("숨겨진 Fact Key도 공개하지 않음"), Fact.Key == TEXT("story.secret"));
        }
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldMutationCommitTest,
                                     "WAIN.World.MutationCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldMutationCommitTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINWorldTests::FProvider>();
        Provider->Proposal.StateChanges.Add(
            WAINWorldTests::Mutation(TEXT("story.step"), EWAINMutationOperation::AddInteger, 1));
        Provider->Proposal.StateChanges.Add(
            WAINWorldTests::Mutation(TEXT("story.step"), EWAINMutationOperation::AddInteger, 1));
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINWorldTests::MakeRequest());
        TestTrue(TEXT("연속 Mutation Commit"), Result.Succeeded());
        TestEqual(TEXT("Revision 1"), Processor->GetSnapshot()->Revision, uint64(1));
        TestTrue(TEXT("기존 Root 불변"), Before.Get() != Processor->GetSnapshot().Get());
        const auto &Current = static_cast<const FWAINMapGameSnapshot &>(*Processor->GetSnapshot()->Game);
        TestEqual(TEXT("Prepared State의 누적 결과"), Current.Values.FindChecked(TEXT("story.step")), 2);
        return true;
    }

#pragma endregion

// ============================================================
// 02. Schema And Proposal Rejection
// ============================================================
#pragma region 02_Schema_And_Proposal_Rejection

    // 권한·범위·존재성 위반은 Commit 전에 실패한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldReadOnlyRejectionTest,
                                     "WAIN.World.ReadOnlyReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldReadOnlyRejectionTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINWorldTests::FProvider>();
        Provider->Proposal.StateChanges.Add(
            WAINWorldTests::Mutation(TEXT("story.secret"), EWAINMutationOperation::SetInteger, 0));
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINWorldTests::MakeRequest());
        TestTrue(TEXT("Read-only Mutation 거절"), Result.Error == EWAINInteractionError::InvalidProposal);
        TestTrue(TEXT("No Commit"), Before.Get() == Processor->GetSnapshot().Get());
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldBoundsTest,
                                     "WAIN.World.BoundsReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldBoundsTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINWorldTests::FProvider>();
        Provider->Proposal.StateChanges.Add(
            WAINWorldTests::Mutation(TEXT("inventory.gift"), EWAINMutationOperation::AddInteger, 4));
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINWorldTests::MakeRequest());
        TestTrue(TEXT("범위 이탈 거절"), Result.Error == EWAINInteractionError::InvalidProposal);
        TestEqual(TEXT("확정 Revision 0"), Processor->GetSnapshot()->Revision, uint64(0));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldUnregisteredKeyTest,
                                     "WAIN.World.UnregisteredKeyReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldUnregisteredKeyTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINWorldTests::FProvider>();
        Provider->Proposal.StateChanges.Add(
            WAINWorldTests::Mutation(TEXT("unknown"), EWAINMutationOperation::SetInteger, 2));
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINWorldTests::MakeRequest());
        TestTrue(TEXT("등록되지 않은 Key 거절"), Result.Error == EWAINInteractionError::InvalidProposal);
        TestEqual(TEXT("확정 Revision 0"), Processor->GetSnapshot()->Revision, uint64(0));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldDuplicateSchemaTest,
                                     "WAIN.World.DuplicateSchemaReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldDuplicateSchemaTest::RunTest(const FString &Parameters)
    {
        auto Schema = WAINWorldTests::MakeSchema();
        Schema.Add(Schema[0]);
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(MakeShared<WAINWorldTests::FProvider>(), Error, MoveTemp(Schema));
        TestTrue(TEXT("중복 Schema는 세션 시작 실패"), Processor == nullptr);
        TestFalse(TEXT("실패 원인 기재"), Error.IsEmpty());
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldInvalidInitialTest,
                                     "WAIN.World.InvalidInitialReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldInvalidInitialTest::RunTest(const FString &Parameters)
    {
        auto Schema = WAINWorldTests::MakeSchema();
        Schema[0].MinValue = 1; // 초기 story.step=0
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(MakeShared<WAINWorldTests::FProvider>(), Error, MoveTemp(Schema));
        TestTrue(TEXT("초기 Snapshot이 Schema 범위 밖이면 세션 거절"), Processor == nullptr);
        return true;
    }

#pragma endregion

// ============================================================
// 03. Prepared Validation And Replay
// ============================================================
#pragma region 03_Prepared_Validation_And_Replay

    // Adapter의 원치 않는 변경과 TurnId 재생의 이중 Commit을 차단한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldPreparedDriftTest,
                                     "WAIN.World.PreparedDriftReject",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldPreparedDriftTest::RunTest(const FString &Parameters)
    {
        auto                                   Adapter = MakeShared<WAINWorldTests::FDriftingAdapter>();
        TSharedPtr<IWAINGameSnapshotAdapter>   AdapterInterface = Adapter;
        TSharedPtr<const IWAINWorldFactReader> Reader = Adapter;
        auto                                   Provider = MakeShared<WAINWorldTests::FProvider>();
        Provider->Proposal.StateChanges.Add(
            WAINWorldTests::Mutation(TEXT("story.step"), EWAINMutationOperation::AddInteger, 1));
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(Provider,
                                                          Error,
                                                          WAINWorldTests::MakeSchema(),
                                                          MoveTemp(AdapterInterface),
                                                          MoveTemp(Reader));
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINWorldTests::MakeRequest());
        TestTrue(TEXT("준비된 State의 숨겨진 오염 검출"), Result.Error == EWAINInteractionError::GamePrepareFailed);
        TestTrue(TEXT("State Publish 하지 않음"), Before.Get() == Processor->GetSnapshot().Get());
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINWorldReplayTest,
                                     "WAIN.World.ReplayNoDoubleCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINWorldReplayTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINWorldTests::FProvider>();
        Provider->Proposal.StateChanges.Add(
            WAINWorldTests::Mutation(TEXT("story.step"), EWAINMutationOperation::AddInteger, 1));
        FString Error;
        auto    Processor = WAINWorldTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Request = WAINWorldTests::MakeRequest();
        const auto First = Processor->ProcessTurn(Request);
        const auto Second = Processor->ProcessTurn(Request);
        TestTrue(TEXT("첫 Commit 정상"), First.Succeeded());
        TestTrue(TEXT("Replay 표시"), Second.bReplayed);
        TestEqual(TEXT("Provider 한 번"), Provider->Calls, 1);
        const auto &Current = static_cast<const FWAINMapGameSnapshot &>(*Processor->GetSnapshot()->Game);
        TestEqual(TEXT("Game 값 한 번만 증가"), Current.Values.FindChecked(TEXT("story.step")), 1);
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
