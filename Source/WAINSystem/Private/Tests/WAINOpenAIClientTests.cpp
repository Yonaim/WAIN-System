/**
 * @file WAINOpenAIClientTests.cpp
 * @brief HTTP 없이 Responses API 결과·비동기 Proposal 인계·Revision 방어를 검증한다.
 */

#include "../LLM/Providers/WAINOpenAIClient.h"
#include "../LLM/Validation/WAINStructuredProposal.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. API Response Fixtures
// ============================================================
#pragma region 00_API_Response_Fixtures

    namespace WAINLLMPrivateTests
    {
        FString ProposalText()
        {
            return TEXT("{\"dialogue\":\"좋아.\",\"actionId\":\"\",\"expressionId\":\"\","
                        "\"expressionVariantId\":\"\",\"mutations\":[]}");
        }

        FWAINInteractionRequest MakeRequest()
        {
            FWAINInteractionRequest Request;
            Request.TurnId = FGuid::NewGuid();
            Request.CharacterId = TEXT("MANA");
            Request.SceneId = TEXT("CLASSROOM");
            Request.Text = TEXT("안녕");
            return Request;
        }
    } // namespace WAINLLMPrivateTests

#pragma endregion

// ============================================================
// 01. Offline API And Queue Tests
// ============================================================
#pragma region 01_Offline_API_And_Queue_Tests

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINLLMResponseValidTest,
                                     "WAIN.LLM.CompletedResponse",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINLLMResponseValidTest::RunTest(const FString &Parameters)
    {
        const FString Envelope = TEXT("{\"status\":\"completed\",\"output\":[{\"type\":\"message\","
                                      "\"content\":[{\"type\":\"output_text\",\"text\":\"{\\\"dialogue\\\":\\\"좋아.\\\","
                                      "\\\"actionId\\\":\\\"\\\",\\\"expressionId\\\":\\\"\\\","
                                      "\\\"expressionVariantId\\\":\\\"\\\",\\\"mutations\\\":[]}\"}]}]}");
        FWAINResponseProposal Proposal;
        FString               Error;
        TestTrue(TEXT("Response parse"), FWAINOpenAIClient::ExtractProposal(Envelope, Proposal, Error));
        TestEqual(TEXT("Dialogue"), Proposal.Dialogue, FString(TEXT("좋아.")));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINLLMResponseIncompleteTest,
                                     "WAIN.LLM.IncompleteRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINLLMResponseIncompleteTest::RunTest(const FString &Parameters)
    {
        FWAINResponseProposal Proposal;
        FString               Error;
        TestFalse(TEXT("Incomplete"),
                  FWAINOpenAIClient::ExtractProposal(TEXT("{\"status\":\"incomplete\",\"output\":[]}"), Proposal, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINLLMResponseRefusalTest,
                                     "WAIN.LLM.RefusalRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINLLMResponseRefusalTest::RunTest(const FString &Parameters)
    {
        FWAINResponseProposal Proposal;
        FString               Error;
        TestFalse(TEXT("Refusal"),
                  FWAINOpenAIClient::ExtractProposal(TEXT("{\"status\":\"completed\",\"output\":[{\"type\":\"message\","
                                                          "\"content\":[{\"type\":\"refusal\",\"refusal\":\"No\"}]}]}"),
                                                     Proposal,
                                                     Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINLLMQueueRevisionTest,
                                     "WAIN.LLM.QueueRevisionMismatch",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINLLMQueueRevisionTest::RunTest(const FString &Parameters)
    {
        FWAINQueuedProposalProvider Provider;
        FWAINResponseProposal       Submitted, Output;
        Submitted.Dialogue = TEXT("안녕");
        const auto Request = WAINLLMPrivateTests::MakeRequest();
        Provider.SetReady(Request.TurnId, 5, Submitted);
        FWAINSessionRoot Current;
        Current.Revision = 6;
        FWAINCharacterContext Context;
        Context.StateRevision = 6;
        FString Error;
        TestFalse(TEXT("Stale proposal"), Provider.GenerateWithContext(Request, Current, Context, Output, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINLLMQueueConsumeTest,
                                     "WAIN.LLM.QueueConsumeOnce",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINLLMQueueConsumeTest::RunTest(const FString &Parameters)
    {
        FWAINQueuedProposalProvider Provider;
        FWAINResponseProposal       Submitted, Output;
        Submitted.Dialogue = TEXT("안녕");
        const auto Request = WAINLLMPrivateTests::MakeRequest();
        Provider.SetReady(Request.TurnId, 2, Submitted);
        FWAINSessionRoot Current;
        Current.Revision = 2;
        FWAINCharacterContext Context;
        Context.StateRevision = 2;
        FString Error;
        TestTrue(TEXT("First consume"), Provider.GenerateWithContext(Request, Current, Context, Output, Error));
        TestEqual(TEXT("Dialogue"), Output.Dialogue, FString(TEXT("안녕")));
        TestFalse(TEXT("Second consume rejected"), Provider.GenerateWithContext(Request, Current, Context, Output, Error));
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
