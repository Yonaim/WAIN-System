/**
 * @file WAINTransactionCoordinator.h
 * @brief Proposal 검증부터 Prepared Snapshot 생성과 Commit까지 한 턴을 조율한다.
 */

#pragma once

#include "Core/WAINStateStore.h"
#include "CoreMinimal.h"
#include "WAINInterfaces.h"
#include "WAINMemoryReasoning.h"
#include "WAINWorldStateManager.h"

// ============================================================
// 00. Commit Outcome
// ============================================================
#pragma region 00_Commit_Outcome

    struct FWAINCommitOutcome
    {
        EWAINInteractionError Error = EWAINInteractionError::None;
        FString               Message;
        uint64                Revision = 0;

        bool Succeeded() const
        {
            return Error == EWAINInteractionError::None;
        }
    };

#pragma endregion

// ============================================================
// 01. Coordinator Interface
// ============================================================
#pragma region 01_Coordinator_Interface

    class FWAINTransactionCoordinator final
    {
        public:
        FWAINCommitOutcome Execute(const FWAINInteractionRequest  &Request,
                                   const FWAINResponseProposal    &Proposal,
                                   FWAINStateStore                &StateStore,
                                   const IWAINGameSnapshotAdapter &GameAdapter,
                                   const IWAINNarrativeEvaluator  &NarrativeEvaluator,
                                   const FWAINPresentationOptions *AvailablePresentations = nullptr,
                                   const IWAINMemoryReasoner *MemoryReasoner = nullptr,
                                   const FWAINWorldStateManager *WorldManager = nullptr) const;

        private:
        bool ValidateProposal(const FWAINResponseProposal &Proposal,
                              const FWAINPresentationOptions *AvailablePresentations,
                              FString &OutError) const;

        TArray<FWAINStateMutation> CollectGameMutations(const FWAINResponseProposal &Proposal) const;

        bool PrepareWAINState(const FWAINInteractionRequest &Request,
                              const FWAINSessionRoot        &Current,
                              const FWAINResponseProposal   &Proposal,
                              FWAINStateSnapshot            &OutPrepared,
                              EWAINInteractionError         &OutErrorCode,
                              FString                       &OutError) const;
    };

#pragma endregion
