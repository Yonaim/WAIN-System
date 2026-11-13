/**
 * @file WAINStructuredProposal.h
 * @brief 구조화된 모델 출력을 검증하고 미확정 WAIN Proposal로 변환한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINRuntimeTypes.h"

// ============================================================
// 00. Strict Proposal Contract
// ============================================================
#pragma region 00_Strict_Proposal_Contract

    // JSON Schema는 생성 형식만 제한하며 실제 State 규칙은 Transaction에서 재검증한다.

    class WAINSYSTEM_API FWAINStructuredProposal final
    {
      public:
        static bool Parse(const FString &Json, FWAINResponseProposal &OutProposal, FString &OutError);
        static FString MakeJsonSchema();
    };

#pragma endregion
