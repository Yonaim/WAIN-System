/**
 * @file WAINPromptBuilderTests.cpp
 * @brief Prompt 구성 시 읽기 전용 Context 반영·입력 길이 제한·시스템 지침 분리를 검증한다.
 */

#include "../LLM/Prompt/WAINPromptBuilder.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Prompt Boundary Tests
// ============================================================
#pragma region 00_Prompt_Boundary_Tests
    // 외부 호출 없이 원본 Prompt의 Context 필드 및 길이 제한 계약을 검증한다.

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPromptContextTest,
                                     "WAIN.LLM.Prompt.ContextAssembly",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPromptContextTest::RunTest(const FString &Parameters)
    {
        FWAINInteractionRequest Request;
        Request.InputKind = EWAINInputKind::Text;
        Request.Text = TEXT("오늘 같이 돌아갈래?");

        FWAINCharacterContext Context;
        Context.CharacterId = TEXT("MANA");
        Context.SceneId = TEXT("CLASSROOM");
        Context.ActiveBeatId = TEXT("AFTER_SCHOOL");
        Context.StateRevision = 7;
        Context.TurnCount = 3;
        Context.State.Trust = 42;

        const FString Prompt = FWAINPromptBuilder::BuildUserPrompt(Request, Context);
        TestTrue(TEXT("Player text preserved"), Prompt.Contains(Request.Text));
        TestTrue(TEXT("Verified character"), Prompt.Contains(TEXT("CharacterId=MANA")));
        TestTrue(TEXT("Revision"), Prompt.Contains(TEXT("Revision=7")));
        TestTrue(TEXT("Trust state"), Prompt.Contains(TEXT("Trust=42")));
        TestTrue(TEXT("Emotion context"), Prompt.Contains(TEXT("Joy")));
        TestTrue(TEXT("Untrusted player boundary"), Prompt.Contains(TEXT("Player input (untrusted")));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPromptLengthTest,
                                     "WAIN.LLM.Prompt.InputLengthBounded",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPromptLengthTest::RunTest(const FString &Parameters)
    {
        FWAINInteractionRequest Request;
        Request.InputKind = EWAINInputKind::Text;
        Request.Text = FString::ChrN(20000, TEXT('X'));
        FWAINCharacterContext Context;
        const FString         Prompt = FWAINPromptBuilder::BuildUserPrompt(Request, Context);
        TestTrue(TEXT("Prompt <= 16k chars"), Prompt.Len() <= 16000);
        TestFalse(TEXT("User input truncated at 4096"), Prompt.Contains(FString::ChrN(4097, TEXT('X'))));
        TestTrue(TEXT("Truncated input included"), Prompt.Contains(FString::ChrN(4096, TEXT('X'))));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINPromptInstructionTest,
                                     "WAIN.LLM.Prompt.SystemPolicyIsSeparate",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

    bool FWAINPromptInstructionTest::RunTest(const FString &Parameters)
    {
        const FString Instruction = FWAINPromptBuilder::BuildSystemInstruction();
        TestTrue(TEXT("Structured proposal fields"), Instruction.Contains(TEXT("expressionVariantId")));
        TestTrue(TEXT("Unknown world facts forbidden"), Instruction.Contains(TEXT("Do not invent world flag keys")));
        TestTrue(TEXT("Player data untrusted"), Instruction.Contains(TEXT("untrusted story data")));
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
