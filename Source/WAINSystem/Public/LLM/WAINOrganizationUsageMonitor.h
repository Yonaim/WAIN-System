/**
 * @file WAINOrganizationUsageMonitor.h
 * @brief Admin Usage API의 조직 단위 집계와 경고용 추정값을 조회하는 Plugin API를 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "LLM/WAINLLMConfig.h"

// ============================================================
// 00. Safe Usage Snapshot
// ============================================================
#pragma region 00_Safe_Usage_Snapshot

    // Admin 비밀 키, 원시 HTTP 본문, 프롬프트는 진단 Snapshot에 저장하지 않는다.

    struct WAINSYSTEM_API FWAINOrganizationUsageSnapshot
    {
        bool bAvailable = false;
        bool bHasMorePages = false;
        bool bHasUnclassifiedModels = false;
        int32 HttpStatus = 0;
        int64 GroupTokens = 0;
        int64 OrganizationTokens = 0;
        int64 UnclassifiedTokens = 0;
        FDateTime CheckedAtUtc;
        FString Error;
    };

#pragma endregion

// ============================================================
// 01. Async Organization Usage Query
// ============================================================
#pragma region 01_Async_Organization_Usage_Query

    class WAINSYSTEM_API FWAINOrganizationUsageMonitor final
    {
      public:
        using FCompletion = TFunction<void(const FWAINOrganizationUsageSnapshot &)>;

        /** @brief 조회 실패도 Snapshot으로 반환하지만 게임의 LLM 요청을 차단하지 않는다. */
        static bool Query(const FString &AdminApiKey, float TimeoutSeconds,
                          FCompletion Callback, FString &OutError);

        /** @brief 모델 단위 집계를 읽는다. 실제 무료 잔액과 동일하지 않다. */
        static bool ParseResponse(const FString &Json, FWAINOrganizationUsageSnapshot &OutUsage,
                                  FString &OutError);
        static bool IsMiniIncentiveGroupModel(const FString &Model);
        static FString FormatDebug(const FWAINOrganizationUsageSnapshot &Usage,
                                   const FWAINLLMConfig &Config);
        static FString WarningText(const FWAINOrganizationUsageSnapshot &Usage,
                                   const FWAINLLMConfig &Config);
    };

#pragma endregion
