/**
 * @file WAINLLMTurnService.h
 * @brief 게임과 독립된 비동기 Usage Preflight → Structured LLM Proposal 호출 계약을 제공한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "LLM/WAINOpenAIClient.h"
#include "LLM/WAINOrganizationUsageMonitor.h"

// ============================================================
// 00. One-Turn Request Result
// ============================================================
#pragma region 00_One_Turn_Request_Result

    // 원시 응답·인증 키를 Host에 공개하지 않고 검증된 Proposal과 진단 메타데이터만 전달한다.

    struct WAINSYSTEM_API FWAINLLMTurnResult
    {
        bool bSucceeded = false;
        FWAINResponseProposal Proposal;
        FString Error;
        FWAINLLMRequestMetrics Metrics;
    };

#pragma endregion

// ============================================================
// 01. Async LLM Turn Orchestration
// ============================================================
#pragma region 01_Async_LLM_Turn_Orchestration

    // Usage 실패는 경고 데이터로 전달하고 LLM POST를 차단하지 않는다.

    class WAINSYSTEM_API FWAINLLMTurnService final
    {
      public:
        using FIsActive = TFunction<bool()>;
        using FOnUsage = TFunction<void(const FWAINOrganizationUsageSnapshot &)>;
        using FCompletion = TFunction<void(const FWAINLLMTurnResult &)>;

        /** @brief 정상 호출 시 Query(GET) 후 Generate(POST). 호출자가 취소되면 POST를 생략한다. */
        static bool Submit(const FWAINInteractionRequest &Request,
                           const FWAINCharacterContext &Context,
                           const FWAINLLMConfig &Config,
                           FIsActive IsActive,
                           FOnUsage OnUsage,
                           FCompletion OnComplete,
                           FString &OutError);
    };

#pragma endregion
