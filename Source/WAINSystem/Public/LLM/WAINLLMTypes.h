/**
 * @file WAINLLMTypes.h
 * @brief Host가 사용하는 Proposal 인계 및 LLM·Usage 진단의 최소 공개 계약을 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINInterfaces.h"

struct FWAINLLMConfig;

// ============================================================
// 00. Revision-Bound Proposal Handoff
// ============================================================
#pragma region 00_Revision_Bound_Proposal_Handoff
    // HTTP 완료 후 게임 스레드에서 준비된 Proposal 한 번만 Transaction에 전달한다.

    class WAINSYSTEM_API FWAINQueuedProposalProvider final : public IWAINProposalProvider
    {
        public:
        void SetReady(const FGuid &TurnId, uint64 Revision, const FWAINResponseProposal &Proposal);
        void SetFailure(const FGuid &TurnId, uint64 Revision, const FString &Error);

        virtual bool Generate(const FWAINInteractionRequest &Request,
                              const FWAINSessionRoot        &Current,
                              FWAINResponseProposal         &OutProposal,
                              FString                       &OutError) const override;

        virtual bool GenerateWithContext(const FWAINInteractionRequest &Request,
                                         const FWAINSessionRoot        &Current,
                                         const FWAINCharacterContext   &Context,
                                         FWAINResponseProposal         &OutProposal,
                                         FString                       &OutError) const override;

        private:
        mutable FGuid                 PreparedTurn;
        mutable uint64                PreparedRevision = 0;
        mutable FWAINResponseProposal PreparedProposal;
        mutable FString               PreparedError;
        mutable bool                  bHasPrepared = false;
    };

#pragma endregion

// ============================================================
// 01. Sanitized Request Diagnostics
// ============================================================
#pragma region 01_Sanitized_Request_Diagnostics
    // API Key, 원시 요청·응답 및 임의 서버 메시지는 외부로 전달하지 않는다.

    struct WAINSYSTEM_API FWAINLLMRequestMetrics
    {
        int32   HttpStatus = 0;
        int32   InputTokens = 0;
        int32   OutputTokens = 0;
        FString ErrorCode;
        bool    bCreditExhausted = false;
        bool    bRateLimited = false;
    };

#pragma endregion

// ============================================================
// 02. Read-Only Organization Usage Snapshot
// ============================================================
#pragma region 02_Read_Only_Organization_Usage_Snapshot
    // GroupTokens는 지연될 수 있는 추정 집계이며 무료 잔량이 아니다.

    struct WAINSYSTEM_API FWAINOrganizationUsageSnapshot
    {
        bool      bAvailable = false;
        bool      bHasMorePages = false;
        bool      bHasUnclassifiedModels = false;
        int32     HttpStatus = 0;
        int64     GroupTokens = 0;
        int64     OrganizationTokens = 0;
        int64     UnclassifiedTokens = 0;
        FDateTime CheckedAtUtc;
        FString   Error;
    };

#pragma endregion

// ============================================================
// 03. Host-Facing Usage Formatting
// ============================================================
#pragma region 03_Host_Facing_Usage_Formatting
    // 경고는 정보용이며 LLM 호출 차단·키 전환을 수행하지 않는다.

    class WAINSYSTEM_API FWAINLLMUsageDiagnostics final
    {
        public:
        static FString FormatDebug(const FWAINOrganizationUsageSnapshot &Usage, const FWAINLLMConfig &Config);
        static FString WarningText(const FWAINOrganizationUsageSnapshot &Usage, const FWAINLLMConfig &Config);
    };

#pragma endregion
