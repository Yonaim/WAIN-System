/**
 * @file WAINMemoryTests.cpp
 * @brief Observation 기록·검색·Context 반영 및 Commit 실패 시 불변성을 확인한다.
 */

#include "Core/WAINInteractionProcessor.h"
#include "Memory/WAINMemoryRetriever.h"
#include "Testing/WAINMapGameAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Memory Test Fixtures
// ============================================================
#pragma region 00_Memory_Test_Fixtures

    namespace WAINMemoryTests
    {
        class FCapturingProvider final : public IWAINProposalProvider
        {
            public:
            mutable TArray<FWAINCharacterContext> Contexts;
            bool                                  bInvalidGameMutation = false;
            bool                                  bEmptyDialogue = false;

            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                OutError = TEXT("Memory 테스트는 Context-aware 경로를 사용합니다.");
                return false;
            }

            virtual bool GenerateWithContext(const FWAINInteractionRequest &Request,
                                             const FWAINSessionRoot        &Current,
                                             const FWAINCharacterContext   &Context,
                                             FWAINResponseProposal         &OutProposal,
                                             FString                       &OutError) const override
            {
                Contexts.Add(Context);
                OutProposal.Dialogue = bEmptyDialogue ? TEXT("") : TEXT("그래, 기억할게.");
                if (bInvalidGameMutation)
                {
                    FWAINStateMutation Mutation;
                    Mutation.Owner = EWAINMutationOwner::Game;
                    Mutation.TargetId = TEXT("inventory.unregistered");
                    Mutation.Operation = EWAINMutationOperation::AddInteger;
                    Mutation.IntegerValue = -1;
                    OutProposal.StateChanges.Add(Mutation);
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

        FWAINInteractionRequest MakeRequest(const FName    CharacterId = TEXT("MANA"),
                                            const FString &Text = TEXT("약속 기억해"))
        {
            FWAINInteractionRequest Request;
            Request.TurnId = FGuid::NewGuid();
            Request.CharacterId = CharacterId;
            Request.SceneId = TEXT("CLASSROOM");
            Request.InputKind = EWAINInputKind::Text;
            Request.Text = Text;
            return Request;
        }

        FWAINMemoryRecord MakeMemory(const FName CharacterId, const FString &Content, const int32 CreatedTurn)
        {
            FWAINMemoryRecord Memory;
            Memory.Id = FGuid::NewGuid();
            Memory.CharacterId = CharacterId;
            Memory.Speaker = EWAINMemorySpeaker::Player;
            Memory.Content = Content;
            Memory.CreatedTurn = CreatedTurn;
            return Memory;
        }

        TUniquePtr<FWAINInteractionProcessor> MakeProcessor(TSharedRef<IWAINProposalProvider> Provider,
                                                            FString                          &OutError,
                                                            const FWAINStateSnapshot         *Initial = nullptr)
        {
            TMap<FName, int32> Values;
            Values.Add(TEXT("inventory.gift"), 2);
            auto Adapter = MakeShared<FWAINMapGameAdapter>(MoveTemp(Values));
            auto Processor = MakeUnique<FWAINInteractionProcessor>(Adapter,
                                                                   MoveTemp(Provider),
                                                                   MakeShared<FWAINNoOpNarrativeEvaluator>());
            const FWAINStateSnapshot DefaultState = MakeState();
            if (!Processor->StartSession(Initial ? *Initial : DefaultState, OutError))
            {
                return nullptr;
            }
            return Processor;
        }
    } // namespace WAINMemoryTests

#pragma endregion

// ============================================================
// 01. Observation Storage And Read-Only Context
// ============================================================
#pragma region 01_Observation_Storage_And_Context

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINMemoryObservationCommitTest,
                                     "WAIN.Memory.ObservationCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINMemoryObservationCommitTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINMemoryTests::FCapturingProvider>();
        FString Error;
        auto    Processor = WAINMemoryTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINMemoryTests::MakeRequest());
        const auto After = Processor->GetSnapshot();

        TestTrue(TEXT("턴 성공"), Result.Succeeded());
        TestEqual(TEXT("두 Observation 기록"), After->WAIN.MemoryStream.Num(), 2);
        TestEqual(TEXT("이전 Snapshot 유지"), Before->WAIN.MemoryStream.Num(), 0);
        TestEqual(TEXT("Player 입력"), After->WAIN.MemoryStream[0].Content, FString(TEXT("약속 기억해")));
        TestTrue(TEXT("Character 대사"), After->WAIN.MemoryStream[1].Speaker == EWAINMemorySpeaker::Character);
        TestTrue(TEXT("Observation ID 유효"), After->WAIN.MemoryStream[0].Id.IsValid());
        TestTrue(TEXT("기억 ID 중복 없음"), After->WAIN.MemoryStream[0].Id != After->WAIN.MemoryStream[1].Id);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINMemoryNextTurnContextTest,
                                     "WAIN.Memory.NextTurnContext",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINMemoryNextTurnContextTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINMemoryTests::FCapturingProvider>();
        FString Error;
        auto    Processor = WAINMemoryTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto First = Processor->ProcessTurn(WAINMemoryTests::MakeRequest());
        const auto Second = Processor->ProcessTurn(WAINMemoryTests::MakeRequest(TEXT("MANA"), TEXT("약속")));
        TestTrue(TEXT("첫 턴 성공"), First.Succeeded());
        TestTrue(TEXT("다음 턴 성공"), Second.Succeeded());
        TestEqual(TEXT("Context 기록 횟수"), Provider->Contexts.Num(), 2);
        TestEqual(TEXT("첫 턴 Context에는 Memory 없음"), Provider->Contexts[0].RelevantMemories.Num(), 0);
        TestEqual(TEXT("다음 턴 Context에 직전 Observation 두 건"), Provider->Contexts[1].RelevantMemories.Num(), 2);
        TestEqual(TEXT("다음 턴 Context는 확정 Revision"), Provider->Contexts[1].StateRevision, uint64(1));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINMemoryCharacterIsolationTest,
                                     "WAIN.Memory.CharacterIsolation",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINMemoryCharacterIsolationTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINMemoryTests::FCapturingProvider>();
        FString Error;
        auto    Processor = WAINMemoryTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        Processor->ProcessTurn(WAINMemoryTests::MakeRequest(TEXT("MANA"), TEXT("마나만 아는 약속")));
        const auto Other = Processor->ProcessTurn(WAINMemoryTests::MakeRequest(TEXT("HANA"), TEXT("다른 사람")));
        TestTrue(TEXT("다른 Character 턴 성공"), Other.Succeeded());
        TestEqual(TEXT("상대 캐릭터 기억은 전달하지 않음"), Provider->Contexts[1].RelevantMemories.Num(), 0);
        TestEqual(TEXT("통합 Stream에는 두 캐릭터 기록"), Processor->GetSnapshot()->WAIN.MemoryStream.Num(), 4);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINMemoryQueryRankingTest,
                                     "WAIN.Memory.QueryRanking",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINMemoryQueryRankingTest::RunTest(const FString &Parameters)
    {
        auto State = WAINMemoryTests::MakeState();
        State.TurnCount = 5;
        State.MemoryStream.Add(WAINMemoryTests::MakeMemory(TEXT("MANA"), TEXT("도서관에서 만남"), 4));
        State.MemoryStream.Add(WAINMemoryTests::MakeMemory(TEXT("MANA"), TEXT("딸기우유를 나눔"), 2));
        State.MemoryStream.Add(WAINMemoryTests::MakeMemory(TEXT("HANA"), TEXT("딸기우유 이야기"), 5));

        const auto Found =
            FWAINMemoryRetriever().Retrieve(WAINMemoryTests::MakeRequest(TEXT("MANA"), TEXT("딸기우유")), State);
        TestEqual(TEXT("MANA의 기록만 검색"), Found.Num(), 2);
        if (Found.Num() == 2)
        {
            TestEqual(TEXT("관련성이 높은 오래된 기억 우선"), Found[0].Content, FString(TEXT("딸기우유를 나눔")));
        }
        return true;
    }

#pragma endregion

// ============================================================
// 02. Transaction Failure And Replay
// ============================================================
#pragma region 02_Transaction_Failure_And_Replay

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINMemoryPrepareFailureNoCommitTest,
                                     "WAIN.Memory.PrepareFailureNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINMemoryPrepareFailureNoCommitTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINMemoryTests::FCapturingProvider>();
        Provider->bInvalidGameMutation = true;
        FString Error;
        auto    Processor = WAINMemoryTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINMemoryTests::MakeRequest());
        const auto Current = Processor->GetSnapshot();
        TestFalse(TEXT("Game Prepare 실패"), Result.Succeeded());
        TestEqual(TEXT("Revision 무변경"), Current->Revision, uint64(0));
        TestEqual(TEXT("Memory 추가 없음"), Current->WAIN.MemoryStream.Num(), 0);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINMemoryInvalidProposalNoCommitTest,
                                     "WAIN.Memory.InvalidProposalNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINMemoryInvalidProposalNoCommitTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINMemoryTests::FCapturingProvider>();
        Provider->bEmptyDialogue = true;
        FString Error;
        auto    Processor = WAINMemoryTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINMemoryTests::MakeRequest());
        TestTrue(TEXT("잘못된 Proposal 오류"), Result.Error == EWAINInteractionError::InvalidProposal);
        TestEqual(TEXT("Memory No Commit"), Processor->GetSnapshot()->WAIN.MemoryStream.Num(), 0);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINMemoryDuplicateTurnTest,
                                     "WAIN.Memory.DuplicateTurn",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINMemoryDuplicateTurnTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINMemoryTests::FCapturingProvider>();
        FString Error;
        auto    Processor = WAINMemoryTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Request = WAINMemoryTests::MakeRequest();
        const auto First = Processor->ProcessTurn(Request);
        const auto Replay = Processor->ProcessTurn(Request);
        TestTrue(TEXT("첫 턴 성공"), First.Succeeded());
        TestTrue(TEXT("Replay 성공"), Replay.Succeeded());
        TestTrue(TEXT("Replay 플래그"), Replay.bReplayed);
        TestEqual(TEXT("Memory 이중 기록 방지"), Processor->GetSnapshot()->WAIN.MemoryStream.Num(), 2);
        TestEqual(TEXT("Provider 한 번 호출"), Provider->Contexts.Num(), 1);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINMemorySilenceTurnTest,
                                     "WAIN.Memory.SilenceTurn",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINMemorySilenceTurnTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINMemoryTests::FCapturingProvider>();
        FString Error;
        auto    Processor = WAINMemoryTests::MakeProcessor(Provider, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        auto Request = WAINMemoryTests::MakeRequest();
        Request.InputKind = EWAINInputKind::Silence;
        Request.Text.Empty();
        const auto Result = Processor->ProcessTurn(Request);
        TestTrue(TEXT("침묵 턴 성공"), Result.Succeeded());
        TestEqual(TEXT("캐릭터 Observation만 저장"), Processor->GetSnapshot()->WAIN.MemoryStream.Num(), 1);
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
