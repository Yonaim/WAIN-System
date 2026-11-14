/**
 * @file WAINLLMTurnService.cpp
 * @brief LLM 사용량 Preflight와 Proposal 생성을 Game Thread 비동기 단계로 연결한다.
 */

#include "LLM/WAINLLMTurnService.h"

// ============================================================
// 00. Single-Provider Turn Submission
// ============================================================
#pragma region 00_Single_Provider_Turn_Submission

    // HTTP와 데이터 변환은 Plugin이 책임지고, Host는 경고 표시와 Commit 승인 결과만 담당한다.

    bool FWAINLLMTurnService::Submit(const FWAINInteractionRequest &Request,
                                     const FWAINCharacterContext &Context,
                                     const FWAINLLMConfig &Config,
                                     FIsActive IsActive,
                                     FOnUsage OnUsage,
                                     FCompletion OnComplete,
                                     FString &OutError)
    {
        if (!IsInGameThread() || !Config.IsOpenAI() || !OnComplete || !IsActive)
        {
            OutError = TEXT("LLM Turn Service는 Game Thread 및 OpenAI 설정과 완료 콜백이 필요합니다.");
            return false;
        }

        // Lambda가 비동기 완료까지 살아 있도록 Move-only TFunction을 공유한다.
        const TSharedRef<FIsActive> Active = MakeShared<FIsActive>(MoveTemp(IsActive));
        const TSharedRef<FOnUsage> UsageHandler = MakeShared<FOnUsage>(MoveTemp(OnUsage));
        const TSharedRef<FCompletion> CompleteHandler = MakeShared<FCompletion>(MoveTemp(OnComplete));

        const auto SendResponse = [Request, Context, Config, Active, CompleteHandler]()
        {
            if (!(*Active)()) return;
            FString PostError;
            const bool bStarted = FWAINOpenAIClient::Submit(Request, Context, Config,
                [Active, CompleteHandler](const bool bSucceeded, const FWAINResponseProposal &Proposal,
                                          const FString &Error, const FWAINLLMRequestMetrics &Metrics)
            {
                if (!(*Active)()) return;
                FWAINLLMTurnResult Result;
                Result.bSucceeded = bSucceeded;
                Result.Proposal = Proposal;
                Result.Error = Error;
                Result.Metrics = Metrics;
                (*CompleteHandler)(Result);
            }, PostError);
            if (!bStarted && (*Active)())
            {
                FWAINLLMTurnResult Result;
                Result.Error = PostError.IsEmpty() ? TEXT("Responses 요청을 시작하지 못했습니다.") : PostError;
                (*CompleteHandler)(Result);
            }
        };

        if (!Config.bUsageMonitorEnabled)
        {
            SendResponse();
            return true;
        }

        const auto AfterUsage = [Active, UsageHandler, SendResponse](const FWAINOrganizationUsageSnapshot &Usage)
        {
            if (!(*Active)()) return;
            if (*UsageHandler) (*UsageHandler)(Usage);
            SendResponse(); // 경고만 출력하며 호출은 허용한다.
        };

        FString UsageError;
        if (!FWAINOrganizationUsageMonitor::Query(Config.AdminApiKey, 10.0F, AfterUsage, UsageError))
        {
            FWAINOrganizationUsageSnapshot Unavailable;
            Unavailable.Error = UsageError.IsEmpty() ? TEXT("조직 사용량 조회를 시작하지 못했습니다.") : UsageError;
            AfterUsage(Unavailable);
        }
        return true;
    }

#pragma endregion
