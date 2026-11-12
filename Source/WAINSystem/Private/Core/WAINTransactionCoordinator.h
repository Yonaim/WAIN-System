/**
 * @file WAINTransactionCoordinator.h
 * @brief Proposal 검증부터 Prepared Snapshot 생성과 Commit까지 한 턴을 조율한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINInterfaces.h"
#include "Core/WAINStateStore.h"

struct FWAINCommitOutcome
{
    EWAINInteractionError Error = EWAINInteractionError::None;
    FString Message;
    uint64 Revision = 0;

    bool Succeeded() const
    {
        return Error == EWAINInteractionError::None;
    }
};

class FWAINTransactionCoordinator final
{
public:
    FWAINCommitOutcome Execute(
        const FWAINInteractionRequest& Request,
        const FWAINResponseProposal& Proposal,
        FWAINStateStore& StateStore,
        const IWAINGameSnapshotAdapter& GameAdapter,
        const IWAINNarrativeEvaluator& NarrativeEvaluator) const;

private:
    bool ValidateProposal(const FWAINResponseProposal& Proposal, FString& OutError) const;

    TArray<FWAINStateMutation> CollectGameMutations(const FWAINResponseProposal& Proposal) const;

    bool PrepareWAINState(
        const FWAINInteractionRequest& Request,
        const FWAINSessionRoot& Current,
        const FWAINResponseProposal& Proposal,
        FWAINStateSnapshot& OutPrepared,
        FString& OutError) const;

};
