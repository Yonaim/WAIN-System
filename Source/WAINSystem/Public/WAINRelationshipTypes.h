/**
 * @file WAINRelationshipTypes.h
 * @brief 관계 파라미터 공통 의미와 캐릭터별 반응, 전역 대사 규칙을 선언한다.
 */
#pragma once
#include "CoreMinimal.h"

// ============================================================
// 00. Semantic Anchors And Character Responses
// ============================================================
#pragma region 00_Semantic_Anchors_And_Character_Responses
    struct FWAINRelationshipAnchor
    {
        int32   Value = 0;
        FString Description;
    };

    struct FWAINRelationshipDefinition
    {
        FString                 DisplayName;
        FString                 Description;
        int32                   Min = 0;
        int32                   Max = 100;
        FWAINRelationshipAnchor Low;
        FWAINRelationshipAnchor Neutral;
        FWAINRelationshipAnchor High;
    };

    struct FWAINCharacterRelationshipReaction
    {
        FString Attitude;
        FString EmotionalResponse;
        FString DialogueStyle;
    };

    struct FWAINCharacterRelationshipResponses
    {
        FWAINCharacterRelationshipReaction Low;
        FWAINCharacterRelationshipReaction Neutral;
        FWAINCharacterRelationshipReaction High;
    };

    struct FWAINGlobalDialogueRules
    {
        FString         Language = TEXT("ko");
        bool            bForbidEmoji = true;
        bool            bForbidChatEmoticons = true;
        TArray<FString> ForbiddenExpressions;
    };
#pragma endregion
