/**
 * @file WAINLLMConfig.cpp
 * @brief 게임별 Config/WAIN의 공개·로컬 설정 병합과 검증을 처리한다.
 */

#include "LLM/WAINLLMConfig.h"
#include "Dom/JsonObject.h"
#include "HAL/PlatformMisc.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace WAINLLMConfigPrivate
{
    TSharedPtr<FJsonObject> ParseObject(const FString &Json)
    {
        TSharedPtr<FJsonObject> Root;
        if (Json.IsEmpty() || Json.Len() > 16384)
            return nullptr;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
        return FJsonSerializer::Deserialize(Reader, Root) ? Root : nullptr;
    }

    void OverrideIfSet(const TCHAR *Variable, FString &OutValue)
    {
        const FString Override = FPlatformMisc::GetEnvironmentVariable(Variable);
        if (!Override.IsEmpty())
            OutValue = Override;
    }
} // namespace WAINLLMConfigPrivate

// ============================================================
// 00. Project Config Loading
// ============================================================
#pragma region 00_Project_Config_Loading

    // Host의 Config/WAIN 폴더에서 Secret과 일반 설정을 분리해 읽는다.

    bool FWAINLLMConfigLoader::Load(FWAINLLMConfig &OutConfig, FString &OutError)
    {
        FWAINLLMConfig Config;
        const FString  Folder = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("WAIN"));
        const FString  PublicPath = FPaths::Combine(Folder, TEXT("LLMConfig.json"));
        const FString  SecretPath = FPaths::Combine(Folder, TEXT("LLMConfig.local.json"));

        FString Json;
        if (FPaths::FileExists(PublicPath))
        {
            if (!FFileHelper::LoadFileToString(Json, *PublicPath) || !ParsePublicJson(Json, Config, OutError))
            {
                if (OutError.IsEmpty())
                    OutError = TEXT("LLM 공개 설정 파일을 읽지 못했습니다.");
                return false;
            }
        }
        if (FPaths::FileExists(SecretPath))
        {
            if (!FFileHelper::LoadFileToString(Json, *SecretPath) || !ParseLocalJson(Json, Config, OutError))
            {
                if (OutError.IsEmpty())
                    OutError = TEXT("LLM 로컬 Secret 파일을 읽지 못했습니다.");
                return false;
            }
        }

        WAINLLMConfigPrivate::OverrideIfSet(TEXT("WAIN_LLM_MODE"), Config.Provider);
        WAINLLMConfigPrivate::OverrideIfSet(TEXT("WAIN_OPENAI_MODEL"), Config.Model);
        WAINLLMConfigPrivate::OverrideIfSet(TEXT("WAIN_OPENAI_REASONING_EFFORT"), Config.ReasoningEffort);
        WAINLLMConfigPrivate::OverrideIfSet(TEXT("WAIN_OPENAI_API_KEY"), Config.ApiKey);
        WAINLLMConfigPrivate::OverrideIfSet(TEXT("WAIN_OPENAI_ADMIN_API_KEY"), Config.AdminApiKey);

        if (!Validate(Config, OutError))
            return false;
        OutConfig = MoveTemp(Config);
        return true;
    }

#pragma endregion

// ============================================================
// 01. Public And Local JSON Parsing
// ============================================================
#pragma region 01_Public_And_Local_JSON_Parsing

    bool FWAINLLMConfigLoader::ParsePublicJson(const FString &Json, FWAINLLMConfig &InOutConfig, FString &OutError)
    {
        const TSharedPtr<FJsonObject> Root = WAINLLMConfigPrivate::ParseObject(Json);
        if (!Root.IsValid())
        {
            OutError = TEXT("LLMConfig.json은 유효한 JSON 객체여야 합니다.");
            return false;
        }
        if (Root->HasField(TEXT("apiKey")) || Root->HasField(TEXT("adminApiKey")))
        {
            OutError = TEXT("공개 설정에는 API Key를 기록할 수 없습니다. local.json을 사용하세요.");
            return false;
        }

        FString Value;
        if (Root->HasField(TEXT("provider")))
        {
            if (!Root->TryGetStringField(TEXT("provider"), Value))
            {
                OutError = TEXT("provider는 문자열이어야 합니다.");
                return false;
            }
            InOutConfig.Provider = Value;
        }
        if (Root->HasField(TEXT("model")))
        {
            if (!Root->TryGetStringField(TEXT("model"), Value))
            {
                OutError = TEXT("model은 문자열이어야 합니다.");
                return false;
            }
            InOutConfig.Model = Value;
        }

        const TSharedPtr<FJsonObject> *Reasoning = nullptr;
        if (Root->TryGetObjectField(TEXT("reasoning"), Reasoning))
        {
            if (!(*Reasoning)->TryGetStringField(TEXT("effort"), InOutConfig.ReasoningEffort))
            {
                OutError = TEXT("reasoning.effort는 문자열이어야 합니다.");
                return false;
            }
        }
        const TSharedPtr<FJsonObject> *Request = nullptr;
        if (Root->TryGetObjectField(TEXT("request"), Request))
        {
            double Timeout = 0.0;
            if (!(*Request)->TryGetNumberField(TEXT("timeoutSeconds"), Timeout))
            {
                OutError = TEXT("request.timeoutSeconds는 숫자여야 합니다.");
                return false;
            }
            InOutConfig.TimeoutSeconds = static_cast<float>(Timeout);
        }
        const TSharedPtr<FJsonObject> *Monitor = nullptr;
        if (Root->TryGetObjectField(TEXT("usageMonitor"), Monitor))
        {
            if ((*Monitor)->HasField(TEXT("enabled")) &&
                !(*Monitor)->TryGetBoolField(TEXT("enabled"), InOutConfig.bUsageMonitorEnabled))
            {
                OutError = TEXT("usageMonitor.enabled는 Boolean이어야 합니다.");
                return false;
            }
            double       Number = 0.0;
            const TCHAR *Names[] = {TEXT("dailyQuotaTokens"), TEXT("warningThreshold"), TEXT("criticalThreshold")};
            int64       *Outputs[] = {&InOutConfig.DailyQuotaTokens,
                                      &InOutConfig.WarningThresholdTokens,
                                      &InOutConfig.CriticalThresholdTokens};
            for (int32 Index = 0; Index < 3; ++Index)
            {
                if ((*Monitor)->HasField(Names[Index]))
                {
                    if (!(*Monitor)->TryGetNumberField(Names[Index], Number) || !FMath::IsFinite(Number) || Number < 1.0 ||
                        Number > 1000000000.0 || Number != FMath::FloorToDouble(Number))
                    {
                        OutError = TEXT("usageMonitor 임계값은 양의 정수여야 합니다.");
                        return false;
                    }
                    *Outputs[Index] = static_cast<int64>(Number);
                }
            }
        }
        return true;
    }

    bool FWAINLLMConfigLoader::ParseLocalJson(const FString &Json, FWAINLLMConfig &InOutConfig, FString &OutError)
    {
        const TSharedPtr<FJsonObject> Root = WAINLLMConfigPrivate::ParseObject(Json);
        if (!Root.IsValid())
        {
            OutError = TEXT("LLMConfig.local.json은 유효한 JSON 객체여야 합니다.");
            return false;
        }
        if (Root->HasField(TEXT("apiKey")) && !Root->TryGetStringField(TEXT("apiKey"), InOutConfig.ApiKey))
        {
            OutError = TEXT("LLMConfig.local.json의 apiKey는 문자열이어야 합니다.");
            return false;
        }
        if (Root->HasField(TEXT("adminApiKey")) && !Root->TryGetStringField(TEXT("adminApiKey"), InOutConfig.AdminApiKey))
        {
            OutError = TEXT("LLMConfig.local.json의 adminApiKey는 문자열이어야 합니다.");
            return false;
        }
        return true;
    }

#pragma endregion

// ============================================================
// 02. Safe Configuration Validation
// ============================================================
#pragma region 02_Safe_Configuration_Validation

    bool FWAINLLMConfigLoader::Validate(const FWAINLLMConfig &Config, FString &OutError)
    {
        if (!Config.Provider.Equals(TEXT("mock"), ESearchCase::IgnoreCase) &&
            !Config.Provider.Equals(TEXT("openai"), ESearchCase::IgnoreCase))
        {
            OutError = TEXT("현재는 mock/openai 단일 Provider만 지원합니다.");
            return false;
        }
        if (Config.Model.TrimStartAndEnd().IsEmpty() || Config.Model.Len() > 100)
        {
            OutError = TEXT("LLM Model ID가 유효하지 않습니다.");
            return false;
        }
        const bool bKnownEffort = Config.ReasoningEffort == TEXT("none") || Config.ReasoningEffort == TEXT("low") ||
                                  Config.ReasoningEffort == TEXT("medium") || Config.ReasoningEffort == TEXT("high") ||
                                  Config.ReasoningEffort == TEXT("xhigh");
        if (!bKnownEffort)
        {
            OutError = TEXT("Reasoning Effort는 none/low/medium/high/xhigh 중 하나여야 합니다.");
            return false;
        }
        if (!FMath::IsFinite(Config.TimeoutSeconds) || Config.TimeoutSeconds < 5.0F || Config.TimeoutSeconds > 300.0F)
        {
            OutError = TEXT("LLM Timeout은 5~300초 사이여야 합니다.");
            return false;
        }
        if (Config.DailyQuotaTokens <= 0 || Config.WarningThresholdTokens <= 0 ||
            Config.CriticalThresholdTokens < Config.WarningThresholdTokens ||
            Config.DailyQuotaTokens < Config.CriticalThresholdTokens)
        {
            OutError = TEXT("무료 할당량 추정 기준은 0 < 경고 <= 위험 <= 설정 한도를 만족해야 합니다.");
            return false;
        }
        if (Config.IsOpenAI() && Config.ApiKey.TrimStartAndEnd().IsEmpty())
        {
            OutError = TEXT("OpenAI API Key가 없습니다. 로컬 JSON 또는 환경변수를 확인하세요.");
            return false;
        }
        return true;
    }

#pragma endregion
