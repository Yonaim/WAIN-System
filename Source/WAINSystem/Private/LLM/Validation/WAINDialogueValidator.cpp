/**
 * @file WAINDialogueValidator.cpp
 * @brief 응답이 확정되기 전에 이모지·채팅형 금지 표현을 검사한다.
 */
#include "WAINDialogueValidator.h"

// ============================================================
// 00. Dialogue Rule Validation
// ============================================================
#pragma region 00_Dialogue_Rule_Validation
    namespace
    {
        bool ContainsEmoji(const FString &Text)
        {
            for (int32 Index = 0; Index < Text.Len(); ++Index)
            {
                uint32 CodePoint = static_cast<uint32>(Text[Index]);
                if (CodePoint >= 0xD800 && CodePoint <= 0xDBFF && Index + 1 < Text.Len())
                {
                    const uint32 Low = static_cast<uint32>(Text[Index + 1]);
                    if (Low >= 0xDC00 && Low <= 0xDFFF)
                    {
                        CodePoint = 0x10000 + ((CodePoint - 0xD800) << 10) + (Low - 0xDC00);
                        ++Index;
                    }
                }
                if ((CodePoint >= 0x1F300 && CodePoint <= 0x1FAFF) || (CodePoint >= 0x2600 && CodePoint <= 0x27BF))
                    return true;
            }
            return false;
        }
    } // namespace

    bool FWAINDialogueValidator::Validate(const FWAINResponseProposal    &Proposal,
                                          const FWAINGlobalDialogueRules &Rules,
                                          FString                        &OutError)
    {
        for (const FWAINPresentationStep &Step : Proposal.PresentationSteps)
        {
            if (Rules.bForbidEmoji && ContainsEmoji(Step.Dialogue))
            {
                OutError = TEXT("Global Dialogue Rules: 이모지는 사용할 수 없습니다.");
                return false;
            }
            if (Rules.bForbidChatEmoticons &&
                (Step.Dialogue.Contains(TEXT("ㅎㅎ")) || Step.Dialogue.Contains(TEXT("ㅋㅋ")) ||
                 Step.Dialogue.Contains(TEXT("^^")) || Step.Dialogue.Contains(TEXT(":)"))))
            {
                OutError = TEXT("Global Dialogue Rules: 채팅식 감정 표현은 사용할 수 없습니다.");
                return false;
            }
            for (const FString &Forbidden : Rules.ForbiddenExpressions)
            {
                if (!Forbidden.IsEmpty() && Step.Dialogue.Contains(Forbidden))
                {
                    OutError = TEXT("Global Dialogue Rules: 금지 표현이 포함되었습니다.");
                    return false;
                }
            }
        }
        return true;
    }
#pragma endregion
