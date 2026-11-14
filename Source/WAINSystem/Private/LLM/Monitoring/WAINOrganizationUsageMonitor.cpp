/**
 * @file WAINOrganizationUsageMonitor.cpp
 * @brief 매 대화 전 비동기 Usage 조회와 실패 안전성을 유지하는 추정 사용량 계산을 구현한다.
 */

#include "WAINOrganizationUsageMonitor.h"
#include "Dom/JsonObject.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace WAINOrganizationUsagePrivate
{
    constexpr int32 MaxUsageResponseChars = 1048576;

    TSharedPtr<FJsonObject> ParseJson(const FString &Json)
    {
        if (Json.IsEmpty() || Json.Len() > MaxUsageResponseChars)
            return nullptr;
        TSharedPtr<FJsonObject>         Root;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
        return FJsonSerializer::Deserialize(Reader, Root) ? Root : nullptr;
    }

    bool ExtractTokenValue(const TSharedPtr<FJsonObject> &Object, const TCHAR *Field, int64 &OutTokens)
    {
        double Number = 0.0;
        if (!Object.IsValid() || !Object->TryGetNumberField(Field, Number) || !FMath::IsFinite(Number) ||
            Number < 0.0 || Number > 1000000000000.0 || Number != FMath::FloorToDouble(Number))
            return false;
        OutTokens = static_cast<int64>(Number);
        return true;
    }
} // namespace WAINOrganizationUsagePrivate

// ============================================================
// 00. Model Group Classification
// ============================================================
#pragma region 00_Model_Group_Classification

    // 공개된 Data Sharing Incentive 소형 모델 그룹만 추정에 포함한다. 미분류 모델은 별도로 보고한다.

    bool FWAINOrganizationUsageMonitor::IsMiniIncentiveGroupModel(const FString &Model)
    {
        static const TSet<FString> Eligible = {TEXT("gpt-5.6-terra"),
                                               TEXT("gpt-5.6-luna"),
                                               TEXT("gpt-5.4-mini"),
                                               TEXT("gpt-5.4-mini-2026-03-17"),
                                               TEXT("gpt-5.4-nano"),
                                               TEXT("gpt-5.4-nano-2026-03-17"),
                                               TEXT("gpt-5.1-codex-mini"),
                                               TEXT("gpt-5-mini"),
                                               TEXT("gpt-5-mini-2025-08-07"),
                                               TEXT("gpt-5-nano"),
                                               TEXT("gpt-5-nano-2025-08-07"),
                                               TEXT("gpt-4.1-mini"),
                                               TEXT("gpt-4.1-mini-2025-04-14"),
                                               TEXT("gpt-4.1-nano"),
                                               TEXT("gpt-4.1-nano-2025-04-14"),
                                               TEXT("gpt-4o-mini"),
                                               TEXT("gpt-4o-mini-2024-07-18"),
                                               TEXT("o4-mini"),
                                               TEXT("o4-mini-2025-04-16"),
                                               TEXT("o1-mini"),
                                               TEXT("o1-mini-2024-09-12"),
                                               TEXT("codex-mini-latest")};
        if (Eligible.Contains(Model))
            return true;
        // API 응답은 alias 대신 dated snapshot 이름을 돌려줄 수 있다.
        return Model.StartsWith(TEXT("gpt-5.4-mini-")) || Model.StartsWith(TEXT("gpt-5.4-nano-")) ||
               Model.StartsWith(TEXT("gpt-5.6-terra-")) || Model.StartsWith(TEXT("gpt-5.6-luna-"));
    }

#pragma endregion

// ============================================================
// 01. Bounded Response Parsing
// ============================================================
#pragma region 01_Bounded_Response_Parsing

    // group_by=model 결과 중 입력+출력 토큰을 읽는다. service_tier는 무료 잔액 보증이 아니다.

    bool FWAINOrganizationUsageMonitor::ParseResponse(const FString                  &Json,
                                                      FWAINOrganizationUsageSnapshot &OutUsage,
                                                      FString                        &OutError)
    {
        OutUsage = FWAINOrganizationUsageSnapshot();
        OutError.Empty();
        const TSharedPtr<FJsonObject> Root = WAINOrganizationUsagePrivate::ParseJson(Json);
        if (!Root.IsValid())
        {
            OutError = TEXT("Organization Usage API가 유효한 JSON을 반환하지 않았습니다.");
            return false;
        }
        bool                                  bHasMore = false;
        const TArray<TSharedPtr<FJsonValue>> *Buckets = nullptr;
        if (!Root->TryGetBoolField(TEXT("has_more"), bHasMore) || !Root->TryGetArrayField(TEXT("data"), Buckets) ||
            !Buckets)
        {
            OutError = TEXT("Organization Usage API의 응답 형식이 누락되었습니다.");
            return false;
        }
        if (bHasMore)
        {
            // 1일 1버킷 쿼리의 페이지네이션이 바뀌면 불완전한 값을 정확한 것으로 표시하지 않는다.
            OutError = TEXT("사용량 집계에 추가 페이지가 있습니다. 현재 결과를 신뢰할 수 없습니다.");
            OutUsage.bHasMorePages = true;
            return false;
        }
        for (const TSharedPtr<FJsonValue> &BucketValue : *Buckets)
        {
            if (!BucketValue.IsValid() || BucketValue->Type != EJson::Object)
            {
                OutError = TEXT("잘못된 Organization Usage 버킷입니다.");
                return false;
            }
            const TSharedPtr<FJsonObject>         Bucket = BucketValue->AsObject();
            const TArray<TSharedPtr<FJsonValue>> *Results = nullptr;
            if (!Bucket->TryGetArrayField(TEXT("results"), Results) || !Results)
            {
                OutError = TEXT("Organization Usage 결과 배열이 없습니다.");
                return false;
            }
            for (const TSharedPtr<FJsonValue> &ResultValue : *Results)
            {
                if (!ResultValue.IsValid() || ResultValue->Type != EJson::Object)
                {
                    OutError = TEXT("Organization Usage 집계 행 형식이 잘못되었습니다.");
                    return false;
                }
                const TSharedPtr<FJsonObject> Row = ResultValue->AsObject();
                int64                         InputTokens = 0, OutputTokens = 0;
                if (!WAINOrganizationUsagePrivate::ExtractTokenValue(Row, TEXT("input_tokens"), InputTokens) ||
                    !WAINOrganizationUsagePrivate::ExtractTokenValue(Row, TEXT("output_tokens"), OutputTokens))
                {
                    OutError = TEXT("Organization Usage 토큰 집계에 잘못된 숫자가 있습니다.");
                    return false;
                }
                const int64 Tokens = InputTokens + OutputTokens;
                if (Tokens > 1000000000000LL || OutUsage.OrganizationTokens > 1000000000000LL - Tokens)
                {
                    OutError = TEXT("Organization Usage 집계 크기가 허용 범위를 초과했습니다.");
                    return false;
                }
                OutUsage.OrganizationTokens += Tokens;
                FString Model;
                if (!Row->TryGetStringField(TEXT("model"), Model) || Model.IsEmpty())
                {
                    OutUsage.bHasUnclassifiedModels = true;
                    OutUsage.UnclassifiedTokens += Tokens;
                }
                else if (IsMiniIncentiveGroupModel(Model))
                {
                    OutUsage.GroupTokens += Tokens;
                }
            }
        }
        OutUsage.bAvailable = true;
        OutUsage.CheckedAtUtc = FDateTime::UtcNow();
        return true;
    }

#pragma endregion

// ============================================================
// 02. Warning And Inspector Formatting
// ============================================================
#pragma region 02_Warning_And_Inspector_Formatting

    // 경고는 정보용이며 경고/조회 실패만으로 플레이어나 Transaction을 중단하지 않는다.

    FString FWAINLLMUsageDiagnostics::WarningText(const FWAINOrganizationUsageSnapshot &Usage, const FWAINLLMConfig &Config)
    {
        if (!Config.bUsageMonitorEnabled)
            return FString();
        if (!Usage.bAvailable)
        {
            return TEXT("조직 사용량을 확인하지 못했습니다. 다른 API 호출의 사용량과 무료 혜택 잔량을 알 수 없습니다. ") +
                   (Usage.Error.IsEmpty() ? FString(TEXT("Admin API Key/연결을 확인하세요.")) : Usage.Error) +
                   TEXT("\n경고만 표시하며 LLM 요청은 계속 진행합니다.");
        }
        if (Usage.bHasUnclassifiedModels && Usage.UnclassifiedTokens > 0)
        {
            return TEXT("조직 사용량 중 모델 정보가 없는 항목이 있습니다. 무료 토큰 사용량을 정확하게 추정할 수 없습니다. ")
                TEXT("추가 요청은 과금될 수 있습니다.\n경고만 표시하며 LLM 요청은 계속 진행합니다.");
        }
        const int64 Tokens = Usage.GroupTokens;
        if (Tokens < Config.WarningThresholdTokens)
            return FString();
        const FString Level = Tokens >= Config.CriticalThresholdTokens ? TEXT("위험") : TEXT("주의");
        return FString::Printf(TEXT("[%s] 소형 모델 그룹의 조직 사용량이 %lld / %lld 토큰입니다. ")
                                   TEXT("이 수치는 무료 잔액이 아닌 집계 지연이 있을 수 있는 추정값입니다. ") TEXT(
                                       "요청 전체가 유료 처리될 수 있습니다.\n경고만 표시하며 LLM 요청은 계속 진행합니다."),
                               *Level,
                               static_cast<long long>(Tokens),
                               static_cast<long long>(Config.DailyQuotaTokens));
    }

    FString FWAINLLMUsageDiagnostics::FormatDebug(const FWAINOrganizationUsageSnapshot &Usage, const FWAINLLMConfig &Config)
    {
        if (!Config.bUsageMonitorEnabled)
            return TEXT("Org Usage: monitoring disabled");
        if (!Usage.bAvailable)
        {
            return TEXT("Org Usage: UNAVAILABLE (free balance unknown)\n") +
                   (Usage.Error.IsEmpty() ? FString(TEXT("사용량 조회 전")) : Usage.Error);
        }
        return FString::Printf(TEXT("Org Usage (UTC %s):\nMini-group estimate: %lld / %lld tokens\n")
                                   TEXT("Organization completions total: %lld tokens\n")
                                       TEXT("Unclassified model tokens: %lld\n")
                                           TEXT("NOT a free balance. Data-sharing eligibility and billing are unverified."),
                               *Usage.CheckedAtUtc.ToString(TEXT("%Y-%m-%d %H:%M:%S")),
                               static_cast<long long>(Usage.GroupTokens),
                               static_cast<long long>(Config.DailyQuotaTokens),
                               static_cast<long long>(Usage.OrganizationTokens),
                               static_cast<long long>(Usage.UnclassifiedTokens));
    }

#pragma endregion

// ============================================================
// 03. Non-blocking Admin HTTP Query
// ============================================================
#pragma region 03_Non_blocking_Admin_HTTP_Query

    // 매 턴 시작 전 조직 전체의 당일 집계를 조회한다. 결과를 받으면 Host가 반드시 LLM POST로 이어간다.

    bool FWAINOrganizationUsageMonitor::Query(const FString &AdminApiKey,
                                              const float    TimeoutSeconds,
                                              FCompletion    Callback,
                                              FString       &OutError)
    {
        OutError.Empty();
        if (!IsInGameThread())
        {
            OutError = TEXT("Usage 조회는 Game Thread에서 시작해야 합니다.");
            return false;
        }
        if (AdminApiKey.IsEmpty())
        {
            OutError = TEXT("Admin API Key가 설정되지 않았습니다.");
            return false;
        }
        const int64   StartTime = FDateTime::UtcNow().GetDate().ToUnixTimestamp();
        const FString Url = FString::Printf(TEXT("https://api.openai.com/v1/organization/usage/completions?")
                                                TEXT("start_time=%lld&bucket_width=1d&limit=1&group_by=model"),
                                            static_cast<long long>(StartTime));
        const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Http = FHttpModule::Get().CreateRequest();
        Http->SetURL(Url);
        Http->SetVerb(TEXT("GET"));
        Http->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + AdminApiKey);
        Http->SetHeader(TEXT("Accept"), TEXT("application/json"));
        Http->SetTimeout(FMath::Clamp(TimeoutSeconds, 3.0F, 30.0F));
        Http->SetDelegateThreadPolicy(EHttpRequestDelegateThreadPolicy::CompleteOnGameThread);

        const TSharedRef<FCompletion> OnComplete = MakeShared<FCompletion>(MoveTemp(Callback));
        Http->OnProcessRequestComplete().BindLambda(
            [OnComplete](FHttpRequestPtr CompletedRequest, FHttpResponsePtr Response, const bool bTransportOK)
            {
                FWAINOrganizationUsageSnapshot Usage;
                if (!bTransportOK || !Response.IsValid())
                {
                    Usage.Error = TEXT("Admin Usage API 연결 또는 시간 초과 오류입니다.");
                }
                else
                {
                    Usage.HttpStatus = Response->GetResponseCode();
                    if (Usage.HttpStatus != 200)
                    {
                        Usage.Error =
                            FString::Printf(TEXT("Admin Usage API HTTP %d (응답 본문은 기록하지 않음)"), Usage.HttpStatus);
                    }
                    else
                    {
                        FString ParseError;
                        if (!FWAINOrganizationUsageMonitor::ParseResponse(Response->GetContentAsString(),
                                                                          Usage,
                                                                          ParseError))
                        {
                            Usage.Error = ParseError;
                        }
                        Usage.HttpStatus = 200;
                    }
                }
                (*OnComplete)(Usage);
            });
        if (!Http->ProcessRequest())
        {
            Http->OnProcessRequestComplete().Unbind();
            OutError = TEXT("Admin Usage HTTP 요청을 시작하지 못했습니다.");
            return false;
        }
        return true;
    }

#pragma endregion
