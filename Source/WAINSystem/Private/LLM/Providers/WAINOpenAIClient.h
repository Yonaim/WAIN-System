/**
 * @file WAINOpenAIClient.h
 * @brief OpenAI Responses API 전용 비동기 HTTP 클라이언트의 Plugin 내부 계약을 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "LLM/WAINLLMConfig.h"
#include "LLM/WAINLLMTypes.h"

// ============================================================
// 00. OpenAI Responses Transport
// ============================================================
#pragma region 00_OpenAI_Responses_Transport
    // 외부 HTTP와 Envelope 변환만 담당하고 Game State는 직접 변경하지 않는다.

    class WAINSYSTEM_API FWAINOpenAIClient final
    {
        public:
        using FCompletion =
            TFunction<void(bool, const FWAINResponseProposal &, const FString &, const FWAINLLMRequestMetrics &)>;

        static bool Submit(const FWAINInteractionRequest &Request,
                           const FWAINCharacterContext   &Context,
                           const FWAINLLMConfig          &Config,
                           FCompletion                    Callback,
                           FString                       &OutError);

        static FString ClassifyError(int32 HttpStatus, const FString &ResponseJson, FWAINLLMRequestMetrics &OutMetrics);
        static void    ExtractUsage(const FString &ResponseJson, FWAINLLMRequestMetrics &OutMetrics);

        static bool ExtractProposal(const FString &ResponseJson, FWAINResponseProposal &OutProposal, FString &OutError);
    };

#pragma endregion
