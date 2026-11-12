/**
 * @file WAINCharacterContextTests.cpp
 * @brief Persona 조회 및 확정 Character State 기반 Context의 읽기 전용 동작과 실패 시 No Commit을 검증한다.
 */

#include "Character/WAINCharacterContextBuilder.h"
#include "Core/WAINInteractionProcessor.h"
#include "Testing/WAINMapGameAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Context Test Fixtures
// ============================================================
#pragma region 00_Context_Test_Fixtures

    namespace WAINContextTests
    {
        class FStaticPersonaSource final : public IWAINPersonaSource
        {
            public:
            FWAINCharacterPersona Definition;
            mutable int32         QueryCount = 0;
            bool                  bFailLookup = false;

            virtual bool GetPersona(FName CharacterId, FWAINCharacterPersona &OutPersona, FString &OutError) const override
            {
                ++QueryCount;
                if (bFailLookup)
                {
                    OutError = TEXT("Persona 정의를 찾을 수 없습니다.");
                    return false;
                }

                OutPersona = Definition;
                return true;
            }
        };

        class FContextCapturingProvider final : public IWAINProposalProvider
        {
            public:
            mutable TArray<FWAINCharacterContext> Captured;

            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                OutError = TEXT("이 테스트는 Context-aware 생성 경로를 사용해야 합니다.");
                return false;
            }

            virtual bool GenerateWithContext(const FWAINInteractionRequest &Request,
                                             const FWAINSessionRoot        &Current,
                                             const FWAINCharacterContext   &Context,
                                             FWAINResponseProposal         &OutProposal,
                                             FString                       &OutError) const override
            {
                Captured.Add(Context);
                OutProposal.Dialogue = TEXT("반가워!");

                FWAINStateMutation Trust;
                Trust.Owner = EWAINMutationOwner::Character;
                Trust.TargetId = TEXT("relationship.trust");
                Trust.Operation = EWAINMutationOperation::AddInteger;
                Trust.IntegerValue = 5;
                OutProposal.StateChanges.Add(Trust);

                FWAINStateMutation Joy;
                Joy.Owner = EWAINMutationOwner::Character;
                Joy.TargetId = TEXT("emotion.joy");
                Joy.Operation = EWAINMutationOperation::AddFloat;
                Joy.FloatValue = 0.2F;
                OutProposal.StateChanges.Add(Joy);
                return true;
            }
        };

        class FLegacyProvider final : public IWAINProposalProvider
        {
            public:
            mutable int32 Calls = 0;

            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                ++Calls;
                OutProposal.Dialogue = TEXT("이전 Provider 호출");
                return true;
            }
        };

        FWAINStateSnapshot MakeState()
        {
            FWAINStateSnapshot  State;
            FWAINCharacterState Character;
            Character.Trust = 40;
            Character.Emotion.Anxiety = 0.6F;
            State.Characters.Add(TEXT("MANA"), Character);
            State.ActiveBeatId = TEXT("INTRO");
            return State;
        }

        FWAINInteractionRequest MakeRequest()
        {
            FWAINInteractionRequest Request;
            Request.TurnId = FGuid::NewGuid();
            Request.CharacterId = TEXT("MANA");
            Request.SceneId = TEXT("SC_CLASSROOM");
            Request.InputKind = EWAINInputKind::Text;
            Request.Text = TEXT("안녕");
            return Request;
        }

        TSharedRef<FWAINMapGameAdapter> MakeGameAdapter()
        {
            TMap<FName, int32> Values;
            Values.Add(TEXT("inventory.gift"), 2);
            return MakeShared<FWAINMapGameAdapter>(MoveTemp(Values));
        }

        TSharedRef<FStaticPersonaSource> MakePersonaSource()
        {
            auto Source = MakeShared<FStaticPersonaSource>();
            Source->Definition.CharacterId = TEXT("MANA");
            Source->Definition.DisplayName = TEXT("마나");
            Source->Definition.Traits.Add(TEXT("다정함"));
            Source->Definition.SpeechStyle = TEXT("친근한 반말");
            Source->Definition.Background = TEXT("전학을 앞둔 학생");
            Source->Definition.Preferences.Add(TEXT("딸기우유"));
            return Source;
        }

        TUniquePtr<FWAINInteractionProcessor> MakeProcessor(TSharedRef<IWAINProposalProvider>    Provider,
                                                            TSharedPtr<const IWAINPersonaSource> PersonaSource,
                                                            FString                             &OutError)
        {
            auto Processor = MakeUnique<FWAINInteractionProcessor>(MakeGameAdapter(),
                                                                   MoveTemp(Provider),
                                                                   MakeShared<FWAINNoOpNarrativeEvaluator>(),
                                                                   MoveTemp(PersonaSource));
            if (!Processor->StartSession(MakeState(), OutError))
            {
                return nullptr;
            }

            return Processor;
        }
    } // namespace WAINContextTests

#pragma endregion

// ============================================================
// 01. Context Construction And Revision Tests
// ============================================================
#pragma region 01_Context_Construction_And_Revision_Tests

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINContextUsesCommittedStateTest,
                                     "WAIN.CharacterContext.UsesCommittedState",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINContextUsesCommittedStateTest::RunTest(const FString &Parameters)
    {
        auto    Source = WAINContextTests::MakePersonaSource();
        auto    Provider = MakeShared<WAINContextTests::FContextCapturingProvider>();
        FString Error;
        auto    Processor = WAINContextTests::MakeProcessor(Provider, Source, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        const auto Before = Processor->GetSnapshot();
        const auto First = Processor->ProcessTurn(WAINContextTests::MakeRequest());
        TestTrue(TEXT("첫 턴 성공"), First.Succeeded());
        if (!TestEqual(TEXT("첫 Context 수"), Provider->Captured.Num(), 1))
        {
            return false;
        }

        const auto &Context = Provider->Captured[0];
        TestTrue(TEXT("Persona 포함"), Context.bHasPersona);
        TestEqual(TEXT("Persona 이름"), Context.Persona.DisplayName, FString(TEXT("마나")));
        TestEqual(TEXT("Persona 말투"), Context.Persona.SpeechStyle, FString(TEXT("친근한 반말")));
        TestEqual(TEXT("Context 이전 Revision"), Context.StateRevision, uint64(0));
        TestEqual(TEXT("Context 이전 Trust"), Context.State.Trust, 40);
        TestEqual(TEXT("Context Beat"), Context.ActiveBeatId, FName(TEXT("INTRO")));
        TestTrue(TEXT("Context 불안 유지"), FMath::IsNearlyEqual(Context.State.Emotion.Anxiety, 0.6F));
        TestEqual(TEXT("원래 Root의 Trust 불변"), Before->WAIN.Characters.FindChecked(TEXT("MANA")).Trust, 40);
        TestEqual(TEXT("확정 Root의 Trust"), Processor->GetSnapshot()->WAIN.Characters.FindChecked(TEXT("MANA")).Trust, 45);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINContextRefreshAfterCommitTest,
                                     "WAIN.CharacterContext.RefreshAfterCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINContextRefreshAfterCommitTest::RunTest(const FString &Parameters)
    {
        auto    Source = WAINContextTests::MakePersonaSource();
        auto    Provider = MakeShared<WAINContextTests::FContextCapturingProvider>();
        FString Error;
        auto    Processor = WAINContextTests::MakeProcessor(Provider, Source, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        TestTrue(TEXT("첫 턴"), Processor->ProcessTurn(WAINContextTests::MakeRequest()).Succeeded());
        TestTrue(TEXT("두 번째 턴"), Processor->ProcessTurn(WAINContextTests::MakeRequest()).Succeeded());
        if (!TestEqual(TEXT("Context 두 개"), Provider->Captured.Num(), 2))
        {
            return false;
        }

        TestEqual(TEXT("두 번째 턴은 Revision 1 조회"), Provider->Captured[1].StateRevision, uint64(1));
        TestEqual(TEXT("두 번째 턴은 Trust 45 조회"), Provider->Captured[1].State.Trust, 45);
        TestTrue(TEXT("두 번째 턴은 Joy 0.2 조회"), FMath::IsNearlyEqual(Provider->Captured[1].State.Emotion.Joy, 0.2F));
        TestEqual(TEXT("최종 Revision 2"), Processor->GetSnapshot()->Revision, uint64(2));
        return true;
    }

#pragma endregion

// ============================================================
// 02. Failure And Backward Compatibility Tests
// ============================================================
#pragma region 02_Failure_And_Backward_Compatibility_Tests

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINContextMissingPersonaNoCommitTest,
                                     "WAIN.CharacterContext.MissingPersonaNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINContextMissingPersonaNoCommitTest::RunTest(const FString &Parameters)
    {
        auto Source = WAINContextTests::MakePersonaSource();
        Source->bFailLookup = true;
        auto    Provider = MakeShared<WAINContextTests::FContextCapturingProvider>();
        FString Error;
        auto    Processor = WAINContextTests::MakeProcessor(Provider, Source, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINContextTests::MakeRequest());
        TestTrue(TEXT("Context 조회 실패"), Result.Error == EWAINInteractionError::ContextPreparationFailed);
        TestEqual(TEXT("Proposal 생성 호출 없음"), Provider->Captured.Num(), 0);
        TestEqual(TEXT("Revision 변화 없음"), Processor->GetSnapshot()->Revision, uint64(0));
        TestTrue(TEXT("같은 Root 유지"), Processor->GetSnapshot() == Before);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINContextWrongIdentityNoCommitTest,
                                     "WAIN.CharacterContext.WrongIdentityNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINContextWrongIdentityNoCommitTest::RunTest(const FString &Parameters)
    {
        auto Source = WAINContextTests::MakePersonaSource();
        Source->Definition.CharacterId = TEXT("OTHER_CHARACTER");
        auto    Provider = MakeShared<WAINContextTests::FContextCapturingProvider>();
        FString Error;
        auto    Processor = WAINContextTests::MakeProcessor(Provider, Source, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        const auto Result = Processor->ProcessTurn(WAINContextTests::MakeRequest());
        TestTrue(TEXT("ID 불일치 거절"), Result.Error == EWAINInteractionError::ContextPreparationFailed);
        TestEqual(TEXT("Provider 미호출"), Provider->Captured.Num(), 0);
        TestEqual(TEXT("Revision 유지"), Processor->GetSnapshot()->Revision, uint64(0));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINContextLegacyProviderFallbackTest,
                                     "WAIN.CharacterContext.LegacyProviderFallback",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINContextLegacyProviderFallbackTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINContextTests::FLegacyProvider>();
        FString Error;
        auto    Processor = WAINContextTests::MakeProcessor(Provider, nullptr, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        const auto Result = Processor->ProcessTurn(WAINContextTests::MakeRequest());
        TestTrue(TEXT("기존 Generate 경로 성공"), Result.Succeeded());
        TestEqual(TEXT("기존 Provider 한 번 호출"), Provider->Calls, 1);
        TestEqual(TEXT("확정 Revision"), Processor->GetSnapshot()->Revision, uint64(1));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINContextLegacyProviderRejectsPersonaTest,
                                     "WAIN.CharacterContext.LegacyProviderRejectsPersona",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINContextLegacyProviderRejectsPersonaTest::RunTest(const FString &Parameters)
    {
        auto    Source = WAINContextTests::MakePersonaSource();
        auto    Provider = MakeShared<WAINContextTests::FLegacyProvider>();
        FString Error;
        auto    Processor = WAINContextTests::MakeProcessor(Provider, Source, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        const auto Result = Processor->ProcessTurn(WAINContextTests::MakeRequest());
        TestTrue(TEXT("Persona 무시 생성 거절"), Result.Error == EWAINInteractionError::ProposalGenerationFailed);
        TestEqual(TEXT("기존 Generate 미호출"), Provider->Calls, 0);
        TestEqual(TEXT("Revision 유지"), Processor->GetSnapshot()->Revision, uint64(0));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINContextReplaySkipsAssemblyTest,
                                     "WAIN.CharacterContext.ReplaySkipsAssembly",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINContextReplaySkipsAssemblyTest::RunTest(const FString &Parameters)
    {
        auto    Source = WAINContextTests::MakePersonaSource();
        auto    Provider = MakeShared<WAINContextTests::FContextCapturingProvider>();
        FString Error;
        auto    Processor = WAINContextTests::MakeProcessor(Provider, Source, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        const auto Request = WAINContextTests::MakeRequest();
        TestTrue(TEXT("첫 턴 성공"), Processor->ProcessTurn(Request).Succeeded());
        const auto Replay = Processor->ProcessTurn(Request);
        TestTrue(TEXT("중복 Turn 재사용"), Replay.bReplayed);
        TestEqual(TEXT("Persona 1회 조회"), Source->QueryCount, 1);
        TestEqual(TEXT("Context 1회 전달"), Provider->Captured.Num(), 1);
        TestEqual(TEXT("Revision 1 유지"), Processor->GetSnapshot()->Revision, uint64(1));
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
