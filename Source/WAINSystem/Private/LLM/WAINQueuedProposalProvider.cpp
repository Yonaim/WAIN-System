/**
 * @file WAINQueuedProposalProvider.cpp
 * @brief 비동기 생성 Proposal을 TurnId·Revision 기준으로 한 번만 소비한다.
 */

#include "LLM/WAINLLMTypes.h"

// ============================================================
// 00. Queued Provider Consumption
// ============================================================
#pragma region 00_Queued_Provider_Consumption

    void FWAINQueuedProposalProvider::SetReady(const FGuid                 &TurnId,
                                               const uint64                 Revision,
                                               const FWAINResponseProposal &Proposal)
    {
        check(IsInGameThread());
        PreparedTurn = TurnId;
        PreparedRevision = Revision;
        PreparedProposal = Proposal;
        PreparedError.Empty();
        bHasPrepared = true;
    }

    void FWAINQueuedProposalProvider::SetFailure(const FGuid &TurnId, const uint64 Revision, const FString &Error)
    {
        check(IsInGameThread());
        PreparedTurn = TurnId;
        PreparedRevision = Revision;
        PreparedError = Error;
        PreparedProposal = FWAINResponseProposal();
        bHasPrepared = true;
    }

    bool FWAINQueuedProposalProvider::Generate(const FWAINInteractionRequest &Request,
                                               const FWAINSessionRoot        &Current,
                                               FWAINResponseProposal         &OutProposal,
                                               FString                       &OutError) const
    {
        OutError = TEXT("LLM Provider에는 준비된 Turn Context가 필요합니다.");
        return false;
    }

    bool FWAINQueuedProposalProvider::GenerateWithContext(const FWAINInteractionRequest &Request,
                                                          const FWAINSessionRoot        &Current,
                                                          const FWAINCharacterContext   &Context,
                                                          FWAINResponseProposal         &OutProposal,
                                                          FString                       &OutError) const
    {
        check(IsInGameThread());
        if (!bHasPrepared || PreparedTurn != Request.TurnId || PreparedRevision != Current.Revision ||
            Context.StateRevision != Current.Revision)
        {
            bHasPrepared = false;
            OutError = TEXT("LLM Proposal이 없거나 TurnId/Revision이 변경되었습니다.");
            return false;
        }
        bHasPrepared = false;
        if (!PreparedError.IsEmpty())
        {
            OutError = PreparedError;
            PreparedError.Empty();
            return false;
        }
        OutProposal = MoveTemp(PreparedProposal);
        return true;
    }

#pragma endregion
