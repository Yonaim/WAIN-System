/**
 * @file WAINPresentationCatalog.h
 * @brief Host가 제공하는 표정 후보와 턴별 선택 가능 목록의 읽기 전용 계약을 정의한다.
 */

#pragma once

#include "CoreMinimal.h"

// ============================================================
// 00. Presentation Query
// ============================================================
#pragma region 00_Presentation_Query

    /** @brief Context Preparation 시점의 캐릭터·장면 문맥으로 허용된 표정 목록을 요청한다. */
    struct FWAINPresentationQuery
    {
        FName CharacterId;
        FName SceneId;
        FName ActiveBeatId;
        uint64 StateRevision = 0;
    };

#pragma endregion

// ============================================================
// 01. Expression Candidate And Catalog Contract
// ============================================================
#pragma region 01_Expression_Candidate_And_Catalog_Contract

    /** @brief Expression/Variant의 허용 조합. VariantId가 None이면 기본 표현을 뜻한다. */
    struct FWAINPresentationEntry
    {
        FName ExpressionId;
        FName VariantId;
        FString Description;
    };

    /** @brief Host가 한 턴 동안 고정하여 제공하는 읽기 전용 Expression 후보 집합. */
    struct FWAINPresentationOptions
    {
        TArray<FWAINPresentationEntry> Entries;
    };

    /** @brief Expression ID와 실제 PNG·Part Mapping의 소유자는 Host Game이다. */
    class IWAINPresentationCatalog
    {
        public:
        virtual ~IWAINPresentationCatalog() = default;

        virtual bool QueryAvailable(const FWAINPresentationQuery &Query,
                                    FWAINPresentationOptions &OutOptions,
                                    FString &OutError) const = 0;
    };

#pragma endregion
