/**
 * @file WAINStructuredProposalTests.cpp
 * @brief 모델 JSON의 필수 필드·타입·Mutation 규칙 및 Schema 생성 결과를 검증한다.
 */

#include "../LLM/Validation/WAINStructuredProposal.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Structured Proposal Fixtures
// ============================================================
#pragma region 00_Structured_Proposal_Fixtures

    namespace WAINStructuredTests
    {
        FString Valid()
        {
            return TEXT("{\"presentationSteps\":[{\"dialogue\":\"안녕!\",\"expressionId\":\"EXP_MANA_NEUTRAL\",")
                TEXT("\"expressionVariantId\":\"default\"}],\"actionId\":\"\",\"mutations\":[]}");
        }

        FString WithMutation(const FString &Mutation)
        {
            FString       Json = Valid();
            const FString Replacement = FString(TEXT("\"mutations\":[")) + Mutation + TEXT("]");
            Json.ReplaceInline(TEXT("\"mutations\":[]"), *Replacement);
            return Json;
        }

    } // namespace WAINStructuredTests

#pragma endregion

// ============================================================
// 01. Valid Output And Schema
// ============================================================
#pragma region 01_Valid_Output_And_Schema

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredValidTest,
                                     "WAIN.StructuredProposal.Valid",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredValidTest::RunTest(const FString &Parameters)
    {
        FString               Error;
        FWAINResponseProposal Proposal;
        TestTrue(TEXT("Valid Proposal"), FWAINStructuredProposal::Parse(WAINStructuredTests::Valid(), Proposal, Error));
        TestEqual(TEXT("대사"), Proposal.PresentationSteps[0].Dialogue, FString(TEXT("안녕!")));
        TestTrue(TEXT("표정 ID"), Proposal.PresentationSteps[0].ExpressionId == TEXT("EXP_MANA_NEUTRAL"));
        TestEqual(TEXT("Mutation 없음"), Proposal.StateChanges.Num(), 0);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredSchemaTest,
                                     "WAIN.StructuredProposal.SchemaValid",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredSchemaTest::RunTest(const FString &Parameters)
    {
        TSharedPtr<FJsonObject>         Schema;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FWAINStructuredProposal::MakeJsonSchema());
        TestTrue(TEXT("JSON Schema 파싱"), FJsonSerializer::Deserialize(Reader, Schema) && Schema.IsValid());
        if (!Schema.IsValid())
            return false;
        bool bAdditional = true;
        TestTrue(TEXT("additionalProperties 선언"), Schema->TryGetBoolField(TEXT("additionalProperties"), bAdditional));
        TestFalse(TEXT("미정의 필드 금지"), bAdditional);
        return true;
    }

#pragma endregion

// ============================================================
// 02. Reject Malformed Or Unsupported Output
// ============================================================
#pragma region 02_Reject_Malformed_Or_Unsupported_Output

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredUnknownFieldTest,
                                     "WAIN.StructuredProposal.UnknownFieldRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredUnknownFieldTest::RunTest(const FString &Parameters)
    {
        FString               Error;
        FWAINResponseProposal Proposal;
        FString               Json = WAINStructuredTests::Valid();
        Json.ReplaceInline(TEXT("\"mutations\":[]"), TEXT("\"mutations\":[],\"overrideState\":true"));
        TestFalse(TEXT("Extra field"), FWAINStructuredProposal::Parse(Json, Proposal, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredMissingTest,
                                     "WAIN.StructuredProposal.MissingRequiredRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredMissingTest::RunTest(const FString &Parameters)
    {
        FString               Error;
        FWAINResponseProposal Proposal;
        FString               Json = WAINStructuredTests::Valid();
        Json.ReplaceInline(TEXT("\"actionId\":\"\","), TEXT(""));
        TestFalse(TEXT("Missing field"), FWAINStructuredProposal::Parse(Json, Proposal, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredWrongTypeTest,
                                     "WAIN.StructuredProposal.WrongTypeRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredWrongTypeTest::RunTest(const FString &Parameters)
    {
        FString               Error;
        FWAINResponseProposal Proposal;
        FString               Json = WAINStructuredTests::Valid();
        Json.ReplaceInline(TEXT("\"dialogue\":\"안녕!\""), TEXT("\"dialogue\":33"));
        TestFalse(TEXT("Wrong type"), FWAINStructuredProposal::Parse(Json, Proposal, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredInvalidVariantTest,
                                     "WAIN.StructuredProposal.UnpairedVariantRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredInvalidVariantTest::RunTest(const FString &Parameters)
    {
        FString               Error;
        FWAINResponseProposal Proposal;
        FString               Json = WAINStructuredTests::Valid();
        Json.ReplaceInline(TEXT("\"expressionId\":\"EXP_MANA_NEUTRAL\""), TEXT("\"expressionId\":\"\""));
        TestFalse(TEXT("Variant without expression"), FWAINStructuredProposal::Parse(Json, Proposal, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredMutationTest,
                                     "WAIN.StructuredProposal.MutationAccepted",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredMutationTest::RunTest(const FString &Parameters)
    {
        FString               Error;
        FWAINResponseProposal Proposal;
        const FString         Json =
            WAINStructuredTests::WithMutation(TEXT("{\"owner\":\"character\","
                                                   "\"targetId\":\"relationship.trust\",\"operation\":\"addInteger\","
                                                   "\"integerValue\":1,\"floatValue\":0}"));
        TestTrue(TEXT("Trust mutation parse"), FWAINStructuredProposal::Parse(Json, Proposal, Error));
        TestEqual(TEXT("Mutation one"), Proposal.StateChanges.Num(), 1);
        if (Proposal.StateChanges.Num())
            TestEqual(TEXT("Delta"), Proposal.StateChanges[0].IntegerValue, 1);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredFractionTest,
                                     "WAIN.StructuredProposal.FractionIntegerRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredFractionTest::RunTest(const FString &Parameters)
    {
        FString               Error;
        FWAINResponseProposal Proposal;
        const FString         Json =
            WAINStructuredTests::WithMutation(TEXT("{\"owner\":\"character\","
                                                   "\"targetId\":\"relationship.trust\",\"operation\":\"addInteger\","
                                                   "\"integerValue\":1.5,\"floatValue\":0}"));
        TestFalse(TEXT("Fractional integer"), FWAINStructuredProposal::Parse(Json, Proposal, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINStructuredMultiStepTest,
                                     "WAIN.StructuredProposal.MultipleStepsStrict",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINStructuredMultiStepTest::RunTest(const FString &Parameters)
    {
        FString Json = WAINStructuredTests::Valid();
        Json.ReplaceInline(
            TEXT("}],\"actionId\":"),
            TEXT("},{\"dialogue\":\"또 보자.\",\"expressionId\":\"\",\"expressionVariantId\":\"\"}],\"actionId\":"));
        FWAINResponseProposal Proposal;
        FString               Error;
        TestTrue(TEXT("Step 2개 허용"), FWAINStructuredProposal::Parse(Json, Proposal, Error));
        TestEqual(TEXT("2 Steps"), Proposal.PresentationSteps.Num(), 2);
        if (Proposal.PresentationSteps.Num() == 2)
            TestEqual(TEXT("Second dialogue"), Proposal.PresentationSteps[1].Dialogue, FString(TEXT("또 보자.")));
        TestFalse(TEXT("이전 단일 Dialogue 계약 거절"),
                  FWAINStructuredProposal::Parse(TEXT("{\"dialogue\":\"안녕\",\"actionId\":\"\",\"mutations\":[]}"),
                                                 Proposal,
                                                 Error));
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
