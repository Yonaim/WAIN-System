/**
 * @file WAINLLMConfig.h
 * @brief Host Config/WAIN의 단일 LLM 설정을 해석하는 Plugin API를 정의한다.
 */

#pragma once

#include "CoreMinimal.h"

// ============================================================
// 00. Single Provider Configuration
// ============================================================
#pragma region 00_Single_Provider_Configuration

    /** @brief API Key는 메모리에만 보관하며 DEV/로그/진단 이력에 복사하지 않는다. */
    struct WAINSYSTEM_API FWAINLLMConfig
    {
        FString Provider = TEXT("mock");
        FString Model = TEXT("gpt-5.4-mini");
        FString ReasoningEffort = TEXT("high");
        float TimeoutSeconds = 60.0F;
        FString ApiKey;
        FString AdminApiKey;

        // Organization Usage API counts are an estimate, NOT a free-token balance.
        bool bUsageMonitorEnabled = true;
        int64 DailyQuotaTokens = 2500000;
        int64 WarningThresholdTokens = 2400000;
        int64 CriticalThresholdTokens = 2490000;

        bool IsOpenAI() const { return Provider.Equals(TEXT("openai"), ESearchCase::IgnoreCase); }
    };

#pragma endregion

// ============================================================
// 01. Host Configuration Loader
// ============================================================
#pragma region 01_Host_Configuration_Loader

    class WAINSYSTEM_API FWAINLLMConfigLoader final
    {
      public:
        /** @brief Public JSON -> Local secret JSON -> 환경변수 Override 순서로 합친다. */
        static bool Load(FWAINLLMConfig &OutConfig, FString &OutError);

        /** @brief 네트워크 없는 테스트용 설정 파싱, ApiKey를 오류나 로그에 포함하지 않는다. */
        static bool ParsePublicJson(const FString &Json, FWAINLLMConfig &InOutConfig, FString &OutError);
        static bool ParseLocalJson(const FString &Json, FWAINLLMConfig &InOutConfig, FString &OutError);
        static bool Validate(const FWAINLLMConfig &Config, FString &OutError);
    };

#pragma endregion
