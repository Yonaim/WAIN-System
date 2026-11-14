/**
 * @file WAINOrganizationUsageMonitor.h
 * @brief OpenAI 조직 Usage API 조회·집계 파싱의 Plugin 내부 계약을 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "LLM/WAINLLMConfig.h"
#include "LLM/WAINLLMTypes.h"

// ============================================================
// 00. Private Organization Usage Transport
// ============================================================
#pragma region 00_Private_Organization_Usage_Transport
    // Admin Key는 사용량 조회에서만 사용하고 게임 UI와 외부 코드에 노출하지 않는다.

    class WAINSYSTEM_API FWAINOrganizationUsageMonitor final
    {
        public:
        using FCompletion = TFunction<void(const FWAINOrganizationUsageSnapshot &)>;

        /** @brief 조회 실패도 Snapshot으로 반환하지만 게임의 LLM 요청을 차단하지 않는다. */
        static bool Query(const FString &AdminApiKey, float TimeoutSeconds, FCompletion Callback, FString &OutError);

        /** @brief 모델 단위 집계를 읽는다. 실제 무료 잔액과 동일하지 않다. */
        static bool ParseResponse(const FString &Json, FWAINOrganizationUsageSnapshot &OutUsage, FString &OutError);
        static bool IsMiniIncentiveGroupModel(const FString &Model);
    };

#pragma endregion
