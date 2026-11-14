/**
 * @file WAINOpenAIClient.h
 * @brief 비동기 OpenAI 요청 및 Revision-bound Proposal Provider의 Plugin API를 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINInterfaces.h"
#include "LLM/WAINLLMConfig.h"

// ============================================================
// 00. One-Turn Proposal Handoff
// ============================================================
#pragma region 00_One_Turn_Proposal_Handoff

    // HTTP가 끝나기 전에는 기존 동기 Provider를 호출하지 않는다.

    class WAINSYSTEM_API FWAINQueuedProposalProvider final : public IWAINProposalProvider
    {
      public:
        void SetReady(const FGuid &TurnId, uint64 Revision, const FWAINResponseProposal &Proposal);
        void SetFailure(const FGuid &TurnId, uint64 Revision, const FString &Error);

        virtual bool Generate(const FWAINInteractionRequest &Request,
                              const FWAINSessionRoot &Current,
                              FWAINResponseProposal &OutProposal,
                              FString &OutError) const override;

        virtual bool GenerateWithContext(const FWAINInteractionRequest &Request,
                                         const FWAINSessionRoot &Current,
                                         const FWAINCharacterContext &Context,
                                         FWAINResponseProposal &OutProposal,
                                         FString &OutError) const override;
      private:
        mutable FGuid PreparedTurn;
        mutable uint64 PreparedRevision = 0;
        mutable FWAINResponseProposal PreparedProposal;
        mutable FString PreparedError;
        mutable bool bHasPrepared = false;
    };

#pragma endregion

// ============================================================
// 01. Asynchronous LLM Client
// ============================================================
#pragma region 01_Asynchronous_LLM_Client

    // Callback은 Unreal HTTP의 Game Thread 완료 시점에 호출된다. API Key는 보존/기록하지 않는다.

    /** @brief API Key/원시 응답을 저장하지 않는 요청별 진단값. */
    struct WAINSYSTEM_API FWAINLLMRequestMetrics
    {
        int32 HttpStatus = 0;
        int32 InputTokens = 0;
        int32 OutputTokens = 0;
        FString ErrorCode;
        bool bCreditExhausted = false;
        bool bRateLimited = false;
    };

    class WAINSYSTEM_API FWAINOpenAIClient final
    {
      public:
        using FCompletion = TFunction<void(bool, const FWAINResponseProposal &, const FString &,
                                           const FWAINLLMRequestMetrics &)>;

        static bool Submit(const FWAINInteractionRequest &Request,
                           const FWAINCharacterContext &Context,
                           const FWAINLLMConfig &Config,
                           FCompletion Callback, FString &OutError);

        static FString ClassifyError(int32 HttpStatus, const FString &ResponseJson,
                                     FWAINLLMRequestMetrics &OutMetrics);
        static void ExtractUsage(const FString &ResponseJson,
                                 FWAINLLMRequestMetrics &OutMetrics);

        static bool ExtractProposal(const FString &ResponseJson,
                                    FWAINResponseProposal &OutProposal, FString &OutError);

        static FString BuildPrompt(const FWAINInteractionRequest &Request,
                                   const FWAINCharacterContext &Context);
    };

#pragma endregion
