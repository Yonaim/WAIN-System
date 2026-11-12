/**
 * @file WAINPresentationTests.cpp
 * @brief Host Catalog 조회·표정 선택 검증·No Commit·중복 턴 재생을 검증한다.
 */

#include "Core/WAINInteractionProcessor.h"
#include "Testing/WAINMapGameAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Presentation Test Fixtures
// ============================================================
#pragma region 00_Presentation_Test_Fixtures

    namespace WAINPresentationTests
    {
        class FStaticCatalog final : public IWAINPresentationCatalog
        {
            public:
            FWAINPresentationOptions       Options;
            mutable int32                  QueryCount = 0;
            mutable FWAINPresentationQuery LastQuery;
            bool                           bFailQuery = false;

            virtual bool QueryAvailable(const FWAINPresentationQuery &Query,
                                        FWAINPresentationOptions     &OutOptions,
                                        FString                      &OutError) const override
            {
                ++QueryCount;
                LastQuery = Query;
                if (bFailQuery)
                {
                    OutError = TEXT("표정 카탈로그 조회 실패");
                    return false;
                }

                OutOptions = Options;
                return true;
            }
        };

        class FSelectingProvider final : public IWAINProposalProvider
        {
            public:
            FName                                 SelectedExpression;
            FName                                 SelectedVariant;
            mutable int32                         GenerateCount = 0;
            mutable TArray<FWAINCharacterContext> Captured;

            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                OutError = TEXT("Context-aware 경로가 필요합니다.");
                return false;
            }

            virtual bool GenerateWithContext(const FWAINInteractionRequest &Request,
                                             const FWAINSessionRoot        &Current,
                                             const FWAINCharacterContext   &Context,
                                             FWAINResponseProposal         &OutProposal,
                                             FString                       &OutError) const override
            {
                ++GenerateCount;
                Captured.Add(Context);
                OutProposal.Dialogue = TEXT("안녕!");
                OutProposal.ExpressionId = SelectedExpression;
                OutProposal.ExpressionVariantId = SelectedVariant;

                FWAINStateMutation Mutation;
                Mutation.Owner = EWAINMutationOwner::Character;
                Mutation.TargetId = TEXT("relationship.trust");
                Mutation.Operation = EWAINMutationOperation::AddInteger;
                Mutation.IntegerValue = 5;
                OutProposal.StateChanges.Add(Mutation);
                return true;
            }
        };

        class FLegacyProvider final : public IWAINProposalProvider
        {
            public:
            mutable int32 GenerateCount = 0;

            virtual bool Generate(const FWAINInteractionRequest &Request,
                                  const FWAINSessionRoot        &Current,
                                  FWAINResponseProposal         &OutProposal,
                                  FString                       &OutError) const override
            {
                ++GenerateCount;
                OutProposal.Dialogue = TEXT("Legacy");
                return true;
            }
        };

        TSharedRef<FStaticCatalog> MakeCatalog()
        {
            auto                   Catalog = MakeShared<FStaticCatalog>();
            FWAINPresentationEntry Smile;
            Smile.ExpressionId = TEXT("EXP_MANA_SMILE");
            Smile.VariantId = TEXT("soft");
            Smile.Description = TEXT("입꼬리가 살짝 올라간 온화한 미소");
            Catalog->Options.Entries.Add(Smile);
            return Catalog;
        }

        FWAINStateSnapshot MakeState()
        {
            FWAINStateSnapshot  State;
            FWAINCharacterState Character;
            Character.Trust = 30;
            Character.Emotion.Anxiety = 0.8F;
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
            Request.Text = TEXT("안녕");
            return Request;
        }

        TUniquePtr<FWAINInteractionProcessor> MakeProcessor(TSharedRef<IWAINProposalProvider>          Provider,
                                                            TSharedPtr<const IWAINPresentationCatalog> Catalog,
                                                            FString                                   &OutError)
        {
            TMap<FName, int32> Values;
            Values.Add(TEXT("inventory.gift"), 2);
            auto Game = MakeShared<FWAINMapGameAdapter>(MoveTemp(Values));
            auto Processor = MakeUnique<FWAINInteractionProcessor>(Game,
                                                                   MoveTemp(Provider),
                                                                   MakeShared<FWAINNoOpNarrativeEvaluator>(),
                                                                   nullptr,
                                                                   MoveTemp(Catalog));
            if (!Processor->StartSession(MakeState(), OutError))
            {
                return nullptr;
            }
            return Processor;
        }

        bool IsUnchanged(const FWAINInteractionProcessor &Processor)
        {
            const auto State = Processor.GetSnapshot();
            return State.IsValid() && State->Revision == 0 && State->WAIN.Characters.FindChecked(TEXT("MANA")).Trust == 30;
        }
    } // namespace WAINPresentationTests

#pragma endregion

// ============================================================
// 01. Allowed Expression And Context
// ============================================================
#pragma region 01_Allowed_Expression_And_Context

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationAllowedExpressionTest,
                                     "WAIN.Presentation.AllowedExpression",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationAllowedExpressionTest::RunTest(const FString &Parameters)
    {
        auto Catalog = WAINPresentationTests::MakeCatalog();
        auto Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        Provider->SelectedExpression = TEXT("EXP_MANA_SMILE");
        Provider->SelectedVariant = TEXT("soft");
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, Catalog, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        const auto Before = Processor->GetSnapshot();
        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("허용된 Expression Commit"), Result.Succeeded());
        TestEqual(TEXT("결과 Expression"), Result.ExpressionId, FName(TEXT("EXP_MANA_SMILE")));
        TestEqual(TEXT("결과 Variant"), Result.ExpressionVariantId, FName(TEXT("soft")));
        TestEqual(TEXT("확정 Revision"), Processor->GetSnapshot()->Revision, uint64(1));
        TestEqual(TEXT("확정 Trust"), Processor->GetSnapshot()->WAIN.Characters.FindChecked(TEXT("MANA")).Trust, 35);
        TestTrue(TEXT("높은 불안과 미소 표현 독립"),
                 FMath::IsNearlyEqual(Processor->GetSnapshot()->WAIN.Characters.FindChecked(TEXT("MANA")).Emotion.Anxiety,
                                      0.8F));
        TestEqual(TEXT("이전 Root는 불변"), Before->WAIN.Characters.FindChecked(TEXT("MANA")).Trust, 30);
        TestEqual(TEXT("Catalog 조회 1회"), Catalog->QueryCount, 1);
        TestEqual(TEXT("Context 후보 1개"), Provider->Captured[0].AvailablePresentations.Entries.Num(), 1);
        TestTrue(TEXT("Context에 Catalog 존재"), Provider->Captured[0].bHasPresentationCatalog);
        TestEqual(TEXT("Catalog 요청 Character"), Catalog->LastQuery.CharacterId, FName(TEXT("MANA")));
        TestEqual(TEXT("Catalog 요청 Scene"), Catalog->LastQuery.SceneId, FName(TEXT("CLASSROOM")));
        TestEqual(TEXT("Catalog 요청 State Revision"), Catalog->LastQuery.StateRevision, uint64(0));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationNoneAllowedTest,
                                     "WAIN.Presentation.NoExpressionAllowed",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationNoneAllowedTest::RunTest(const FString &Parameters)
    {
        auto Catalog = WAINPresentationTests::MakeCatalog();
        Catalog->Options.Entries.Reset();
        auto    Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, Catalog, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
        {
            return false;
        }

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("Expression 미선택 턴 정상"), Result.Succeeded());
        TestTrue(TEXT("Expression 자동 대체 없음"), Result.ExpressionId.IsNone());
        return true;
    }

#pragma endregion

// ============================================================
// 02. Invalid Proposal And Catalog Failures
// ============================================================
#pragma region 02_Invalid_Proposal_And_Catalog_Failures

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationUnknownExpressionTest,
                                     "WAIN.Presentation.UnknownExpressionNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationUnknownExpressionTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        Provider->SelectedExpression = TEXT("EXP_UNKNOWN");
        Provider->SelectedVariant = TEXT("soft");
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, WAINPresentationTests::MakeCatalog(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("알 수 없는 ID 거절"), Result.Error == EWAINInteractionError::InvalidProposal);
        TestTrue(TEXT("변경 없음"), WAINPresentationTests::IsUnchanged(*Processor));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationUnknownVariantTest,
                                     "WAIN.Presentation.UnknownVariantNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationUnknownVariantTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        Provider->SelectedExpression = TEXT("EXP_MANA_SMILE");
        Provider->SelectedVariant = TEXT("angry");
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, WAINPresentationTests::MakeCatalog(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("미허용 Variant 거절"), Result.Error == EWAINInteractionError::InvalidProposal);
        TestTrue(TEXT("변경 없음"), WAINPresentationTests::IsUnchanged(*Processor));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationMissingCatalogTest,
                                     "WAIN.Presentation.MissingCatalogNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationMissingCatalogTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        Provider->SelectedExpression = TEXT("EXP_MANA_SMILE");
        Provider->SelectedVariant = TEXT("soft");
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, nullptr, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("검증할 Catalog 없어서 거절"), Result.Error == EWAINInteractionError::InvalidProposal);
        TestTrue(TEXT("변경 없음"), WAINPresentationTests::IsUnchanged(*Processor));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationVariantWithoutExpressionTest,
                                     "WAIN.Presentation.VariantWithoutExpressionNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationVariantWithoutExpressionTest::RunTest(const FString &Parameters)
    {
        auto Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        Provider->SelectedVariant = TEXT("soft");
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, WAINPresentationTests::MakeCatalog(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("Expression 없는 Variant 거절"), Result.Error == EWAINInteractionError::InvalidProposal);
        TestTrue(TEXT("변경 없음"), WAINPresentationTests::IsUnchanged(*Processor));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationInvalidCatalogEntryTest,
                                     "WAIN.Presentation.InvalidCatalogEntryNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationInvalidCatalogEntryTest::RunTest(const FString &Parameters)
    {
        auto Catalog = WAINPresentationTests::MakeCatalog();
        Catalog->Options.Entries[0].Description = TEXT(" ");
        auto    Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, Catalog, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("설명 없는 후보 거절"), Result.Error == EWAINInteractionError::ContextPreparationFailed);
        TestEqual(TEXT("생성기 미호출"), Provider->GenerateCount, 0);
        TestTrue(TEXT("변경 없음"), WAINPresentationTests::IsUnchanged(*Processor));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationDuplicateEntriesTest,
                                     "WAIN.Presentation.DuplicateEntriesNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationDuplicateEntriesTest::RunTest(const FString &Parameters)
    {
        auto Catalog = WAINPresentationTests::MakeCatalog();
        Catalog->Options.Entries.Add(Catalog->Options.Entries[0]);
        auto    Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, Catalog, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("중복 Catalog 거절"), Result.Error == EWAINInteractionError::ContextPreparationFailed);
        TestEqual(TEXT("생성기 미호출"), Provider->GenerateCount, 0);
        TestTrue(TEXT("변경 없음"), WAINPresentationTests::IsUnchanged(*Processor));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationLookupFailureTest,
                                     "WAIN.Presentation.LookupFailureNoCommit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationLookupFailureTest::RunTest(const FString &Parameters)
    {
        auto Catalog = WAINPresentationTests::MakeCatalog();
        Catalog->bFailQuery = true;
        auto    Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, Catalog, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("조회 실패 거절"), Result.Error == EWAINInteractionError::ContextPreparationFailed);
        TestEqual(TEXT("생성기 미호출"), Provider->GenerateCount, 0);
        TestTrue(TEXT("변경 없음"), WAINPresentationTests::IsUnchanged(*Processor));
        return true;
    }

#pragma endregion

// ============================================================
// 03. Provider Compatibility And Turn Replay
// ============================================================
#pragma region 03_Provider_Compatibility_And_Turn_Replay

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationLegacyProviderTest,
                                     "WAIN.Presentation.LegacyProviderRejectsCatalog",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationLegacyProviderTest::RunTest(const FString &Parameters)
    {
        auto    Provider = MakeShared<WAINPresentationTests::FLegacyProvider>();
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, WAINPresentationTests::MakeCatalog(), Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Result = Processor->ProcessTurn(WAINPresentationTests::MakeRequest());
        TestTrue(TEXT("Catalog 무시 Provider 거절"), Result.Error == EWAINInteractionError::ProposalGenerationFailed);
        TestEqual(TEXT("Legacy 생성기 미호출"), Provider->GenerateCount, 0);
        TestTrue(TEXT("변경 없음"), WAINPresentationTests::IsUnchanged(*Processor));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPresentationReplayTest,
                                     "WAIN.Presentation.ReplaySkipsCatalog",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPresentationReplayTest::RunTest(const FString &Parameters)
    {
        auto Catalog = WAINPresentationTests::MakeCatalog();
        auto Provider = MakeShared<WAINPresentationTests::FSelectingProvider>();
        Provider->SelectedExpression = TEXT("EXP_MANA_SMILE");
        Provider->SelectedVariant = TEXT("soft");
        FString Error;
        auto    Processor = WAINPresentationTests::MakeProcessor(Provider, Catalog, Error);
        if (!TestTrue(TEXT("세션 시작"), Processor != nullptr))
            return false;

        const auto Request = WAINPresentationTests::MakeRequest();
        const auto First = Processor->ProcessTurn(Request);
        const auto Replay = Processor->ProcessTurn(Request);
        TestTrue(TEXT("첫 턴 성공"), First.Succeeded());
        TestTrue(TEXT("재전송 상태"), Replay.bReplayed);
        TestEqual(TEXT("Expression 재생"), Replay.ExpressionId, First.ExpressionId);
        TestEqual(TEXT("Catalog 한 번 호출"), Catalog->QueryCount, 1);
        TestEqual(TEXT("생성기 한 번 호출"), Provider->GenerateCount, 1);
        TestEqual(TEXT("Commit 한 번"), Processor->GetSnapshot()->Revision, uint64(1));
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
