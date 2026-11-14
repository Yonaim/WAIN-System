/**
 * @file WAINRelationshipStepTests.cpp
 * @brief 관계 5종·전역 대사 규칙·복수 PresentationStep의 단일 Commit을 검증한다.
 */
#include "Character/WAINCharacterStatePreparer.h"
#include "Core/WAINInteractionProcessor.h"
#include "LLM/Prompt/WAINPromptBuilder.h"
#include "LLM/Validation/WAINDialogueValidator.h"
#include "Testing/WAINMapGameAdapter.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

// ============================================================
// 00. Relationship State Contract
// ============================================================
#pragma region 00_Relationship_State_Contract
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINRelationshipFiveTargetsTest,
                                     "WAIN.Character.Relationship.FiveTargets",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINRelationshipFiveTargetsTest::RunTest(const FString &Parameters)
    {
        FWAINStateSnapshot Prepared;
        Prepared.Characters.Add(TEXT("MANA"), FWAINCharacterState{});
        const TArray<FName>        Targets = {TEXT("relationship.trust"),
                                              TEXT("relationship.affection"),
                                              TEXT("relationship.familiarity"),
                                              TEXT("relationship.comfort"),
                                              TEXT("relationship.romantic_attraction")};
        TArray<FWAINStateMutation> Changes;
        for (const FName Target : Targets)
        {
            FWAINStateMutation Change;
            Change.Owner = EWAINMutationOwner::Character;
            Change.TargetId = Target;
            Change.Operation = EWAINMutationOperation::SetInteger;
            Change.IntegerValue = 73;
            Changes.Add(Change);
        }
        FString Error;
        TestTrue(TEXT("다섯 관계값 반영"), FWAINCharacterStatePreparer().Apply(TEXT("MANA"), Changes, Prepared, Error));
        const FWAINCharacterState &State = Prepared.Characters.FindChecked(TEXT("MANA"));
        TestEqual(TEXT("Trust"), State.Trust, 73);
        TestEqual(TEXT("Affection"), State.Affection, 73);
        TestEqual(TEXT("Familiarity"), State.Familiarity, 73);
        TestEqual(TEXT("Comfort"), State.Comfort, 73);
        TestEqual(TEXT("RomanticAttraction"), State.RomanticAttraction, 73);
        Changes[0].TargetId = TEXT("relationship.fake");
        TestFalse(TEXT("알 수 없는 파라미터 거절"),
                  FWAINCharacterStatePreparer().Apply(TEXT("MANA"), Changes, Prepared, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINRelationshipOverflowTest,
                                     "WAIN.Character.Relationship.OutOfRangeRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINRelationshipOverflowTest::RunTest(const FString &Parameters)
    {
        FWAINStateSnapshot Prepared;
        Prepared.Characters.Add(TEXT("MANA"), FWAINCharacterState{});
        FWAINStateMutation Mutation;
        Mutation.Owner = EWAINMutationOwner::Character;
        Mutation.TargetId = TEXT("relationship.romantic_attraction");
        Mutation.Operation = EWAINMutationOperation::SetInteger;
        Mutation.IntegerValue = 101;
        FString Error;
        TestFalse(TEXT("101 거절"), FWAINCharacterStatePreparer().Apply(TEXT("MANA"), {Mutation}, Prepared, Error));
        TestEqual(TEXT("Prepared 값 유지"), Prepared.Characters.FindChecked(TEXT("MANA")).RomanticAttraction, 0);
        return true;
    }
#pragma endregion

// ============================================================
// 01. Global Rules And Context
// ============================================================
#pragma region 01_Global_Rules_And_Context
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINDialoguePolicyTest,
                                     "WAIN.LLM.Dialogue.PolicyValidation",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINDialoguePolicyTest::RunTest(const FString &Parameters)
    {
        FWAINResponseProposal Proposal;
        Proposal.PresentationSteps.Add({TEXT("괜찮아."), FName(), FName()});
        FWAINGlobalDialogueRules Rules;
        Rules.ForbiddenExpressions.Add(TEXT("ㅋㅋ"));
        FString Error;
        TestTrue(TEXT("정상 대사"), FWAINDialogueValidator::Validate(Proposal, Rules, Error));
        Proposal.PresentationSteps.Add({TEXT("응ㅎㅎ"), FName(), FName()});
        TestFalse(TEXT("두 번째 Step의 채팅체 차단"), FWAINDialogueValidator::Validate(Proposal, Rules, Error));
        Proposal.PresentationSteps[1].Dialogue = TEXT("반가워 😀");
        TestFalse(TEXT("이모지 차단"), FWAINDialogueValidator::Validate(Proposal, Rules, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINRelationshipContextPromptTest,
                                     "WAIN.LLM.Prompt.RelationshipSemantics",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINRelationshipContextPromptTest::RunTest(const FString &Parameters)
    {
        FWAINCharacterContext Context;
        Context.CharacterId = TEXT("MANA");
        Context.State.Trust = 18;
        Context.bHasPersona = true;
        Context.Persona.SpeechStyle = TEXT("나긋나긋한 반말");
        FWAINRelationshipDefinition Definition;
        Definition.Low = {20, TEXT("쉽게 믿지 않는다")};
        Definition.Neutral = {50, TEXT("관찰한다")};
        Definition.High = {80, TEXT("신뢰한다")};
        Context.RelationshipDefinitions.Add(TEXT("trust"), Definition);
        FWAINCharacterRelationshipResponses Responses;
        Responses.Low.Attitude = TEXT("경계한다");
        Responses.Low.EmotionalResponse = TEXT("서운하면 울상을 짓는다");
        Responses.Low.DialogueStyle = TEXT("조심스럽게 항의한다");
        Context.Persona.RelationshipResponses.Add(TEXT("trust"), Responses);
        FWAINInteractionRequest Request;
        Request.Text = TEXT("바보");
        const FString Prompt = FWAINPromptBuilder::BuildUserPrompt(Request, Context);
        TestTrue(TEXT("Current value"), Prompt.Contains(TEXT("trust=18")));
        TestTrue(TEXT("Anchor semantics"), Prompt.Contains(TEXT("쉽게 믿지 않는다")));
        TestTrue(TEXT("Persona reaction"), Prompt.Contains(TEXT("울상을 짓는다")));
        return true;
    }
#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
