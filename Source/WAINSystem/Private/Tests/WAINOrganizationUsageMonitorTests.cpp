/**
 * @file WAINOrganizationUsageMonitorTests.cpp
 * @brief 외부 HTTP 없이 조직 Usage 응답, 모델 그룹, 임계값, Secret Config 검증을 테스트한다.
 */

#include "../LLM/Monitoring/WAINOrganizationUsageMonitor.h"
#include "../LLM/Providers/WAINOpenAIClient.h"
#include "LLM/WAINLLMConfig.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

// ============================================================
// 00. Offline Usage Fixtures
// ============================================================
#pragma region 00_Offline_Usage_Fixtures

    namespace WAINUsageTests
    {
        FString BuildResponse(const FString &Results, const bool bHasMore = false)
        {
            return FString::Printf(
                TEXT("{\"object\":\"page\",\"has_more\":%s,\"data\":[{\"object\":\"bucket\",\"results\":[%s]}]}"),
                bHasMore ? TEXT("true") : TEXT("false"),
                *Results);
        }

        FString Row(const FString &Model, const int64 InTokens, const int64 OutTokens)
        {
            return FString::Printf(TEXT("{\"object\":\"organization.usage.completions.result\",\"model\":\"%s\",\"input_"
                                        "tokens\":%lld,\"output_tokens\":%lld}"),
                                   *Model,
                                   static_cast<long long>(InTokens),
                                   static_cast<long long>(OutTokens));
        }
    } // namespace WAINUsageTests

#pragma endregion

// ============================================================
// 01. Model And Organization Usage Parsing
// ============================================================
#pragma region 01_Model_And_Organization_Usage_Parsing

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsageGroupParsingTest,
                                     "WAIN.LLM.Usage.GroupVsTotal",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsageGroupParsingTest::RunTest(const FString &Parameters)
    {
        const FString Json =
            WAINUsageTests::BuildResponse(WAINUsageTests::Row(TEXT("gpt-5.4-mini-2026-03-17"), 120, 30) + TEXT(",") +
                                          WAINUsageTests::Row(TEXT("gpt-4.1-mini-2025-04-14"), 80, 20) + TEXT(",") +
                                          WAINUsageTests::Row(TEXT("gpt-4o-2024-08-06"), 300, 100));
        FWAINOrganizationUsageSnapshot Usage;
        FString                        Error;
        TestTrue(TEXT("Valid response"), FWAINOrganizationUsageMonitor::ParseResponse(Json, Usage, Error));
        TestEqual(TEXT("Mini group combined"), Usage.GroupTokens, static_cast<int64>(250));
        TestEqual(TEXT("Organization total"), Usage.OrganizationTokens, static_cast<int64>(650));
        TestTrue(TEXT("Available"), Usage.bAvailable);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsageUnknownModelTest,
                                     "WAIN.LLM.Usage.Unclassified",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsageUnknownModelTest::RunTest(const FString &Parameters)
    {
        const FString Json =
            WAINUsageTests::BuildResponse(TEXT("{\"input_tokens\":50,\"output_tokens\":10,\"model\":null}"));
        FWAINOrganizationUsageSnapshot Usage;
        FString                        Error;
        TestTrue(TEXT("Parse"), FWAINOrganizationUsageMonitor::ParseResponse(Json, Usage, Error));
        TestTrue(TEXT("Unclassified"), Usage.bHasUnclassifiedModels);
        TestEqual(TEXT("Unclassified tokens"), Usage.UnclassifiedTokens, static_cast<int64>(60));
        TestFalse(TEXT("No assumption about incentive group"), Usage.GroupTokens != 0);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsagePaginationTest,
                                     "WAIN.LLM.Usage.PaginationRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsagePaginationTest::RunTest(const FString &Parameters)
    {
        FWAINOrganizationUsageSnapshot Usage;
        FString                        Error;
        TestFalse(TEXT("Do not trust partial page"),
                  FWAINOrganizationUsageMonitor::ParseResponse(
                      WAINUsageTests::BuildResponse(WAINUsageTests::Row(TEXT("gpt-5.4-mini"), 10, 5), true),
                      Usage,
                      Error));
        TestFalse(TEXT("Unavailable"), Usage.bAvailable);
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsageMalformedTest,
                                     "WAIN.LLM.Usage.MalformedRejected",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsageMalformedTest::RunTest(const FString &Parameters)
    {
        FWAINOrganizationUsageSnapshot Usage;
        FString                        Error;
        TestFalse(
            TEXT("Negative input rejected"),
            FWAINOrganizationUsageMonitor::ParseResponse(
                WAINUsageTests::BuildResponse(TEXT("{\"model\":\"gpt-5.4-mini\",\"input_tokens\":-1,\"output_tokens\":2}")),
                Usage,
                Error));
        TestFalse(TEXT("Non-JSON rejected"), FWAINOrganizationUsageMonitor::ParseResponse(TEXT("broken"), Usage, Error));
        return true;
    }

#pragma endregion

// ============================================================
// 02. Warnings And Safe Configuration
// ============================================================
#pragma region 02_Warnings_And_Safe_Configuration

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsageThresholdTest,
                                     "WAIN.LLM.Usage.WarningThreshold",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsageThresholdTest::RunTest(const FString &Parameters)
    {
        FWAINLLMConfig                 Config;
        FWAINOrganizationUsageSnapshot Usage;
        Usage.bAvailable = true;
        Usage.GroupTokens = 2300000;
        TestTrue(TEXT("Below threshold"), FWAINLLMUsageDiagnostics::WarningText(Usage, Config).IsEmpty());
        Usage.GroupTokens = 2400000;
        TestTrue(TEXT("Warning"), FWAINLLMUsageDiagnostics::WarningText(Usage, Config).Contains(TEXT("주의")));
        Usage.GroupTokens = 2490000;
        TestTrue(TEXT("Critical"), FWAINLLMUsageDiagnostics::WarningText(Usage, Config).Contains(TEXT("위험")));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsageUnavailableTest,
                                     "WAIN.LLM.Usage.UnavailableWarnOnly",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsageUnavailableTest::RunTest(const FString &Parameters)
    {
        FWAINOrganizationUsageSnapshot Usage;
        Usage.Error = TEXT("Admin API Key가 없습니다.");
        FWAINLLMConfig Config;
        const FString  Message = FWAINLLMUsageDiagnostics::WarningText(Usage, Config);
        TestTrue(TEXT("Warn on unavailable"), Message.Contains(TEXT("확인하지 못했습니다")));
        TestTrue(TEXT("Warn-only"), Message.Contains(TEXT("계속 진행")));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsageLocalConfigTest,
                                     "WAIN.LLM.Usage.ConfigLocalKey",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsageLocalConfigTest::RunTest(const FString &Parameters)
    {
        FWAINLLMConfig Config;
        FString        Error;
        TestTrue(
            TEXT("Local keys"),
            FWAINLLMConfigLoader::ParseLocalJson(TEXT("{\"apiKey\":\"sk-example\",\"adminApiKey\":\"sk-admin-example\"}"),
                                                 Config,
                                                 Error));
        TestEqual(TEXT("Admin secret loaded"), Config.AdminApiKey, FString(TEXT("sk-admin-example")));
        TestFalse(TEXT("Public secret rejected"),
                  FWAINLLMConfigLoader::ParsePublicJson(TEXT("{\"adminApiKey\":\"oops\"}"), Config, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsageConfigThresholdTest,
                                     "WAIN.LLM.Usage.ConfigThresholdValidation",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsageConfigThresholdTest::RunTest(const FString &Parameters)
    {
        FWAINLLMConfig Config;
        FString        Error;
        TestTrue(TEXT("Public config"),
                 FWAINLLMConfigLoader::ParsePublicJson(
                     TEXT("{\"provider\":\"mock\",\"usageMonitor\":{\"enabled\":true,\"criticalThreshold\":2490000}}"),
                     Config,
                     Error));
        TestTrue(TEXT("Valid thresholds"), FWAINLLMConfigLoader::Validate(Config, Error));
        Config.CriticalThresholdTokens = 200;
        TestFalse(TEXT("Misordered thresholds"), FWAINLLMConfigLoader::Validate(Config, Error));
        return true;
    }

    IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWAINUsageHttpErrorTest,
                                     "WAIN.LLM.Usage.CreditVsRateLimit",
                                     EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
    bool FWAINUsageHttpErrorTest::RunTest(const FString &Parameters)
    {
        FWAINLLMRequestMetrics Metrics;
        FWAINOpenAIClient::ClassifyError(429, TEXT("{\"error\":{\"code\":\"insufficient_quota\"}}"), Metrics);
        TestTrue(TEXT("Credit exhausted"), Metrics.bCreditExhausted);
        TestFalse(TEXT("Not temporary rate"), Metrics.bRateLimited);
        Metrics = FWAINLLMRequestMetrics();
        FWAINOpenAIClient::ClassifyError(429, TEXT("{\"error\":{\"code\":\"rate_limit_exceeded\"}}"), Metrics);
        TestTrue(TEXT("Temporary rate limit"), Metrics.bRateLimited);
        TestFalse(TEXT("Not credit"), Metrics.bCreditExhausted);
        return true;
    }

#pragma endregion

#endif // WITH_DEV_AUTOMATION_TESTS
