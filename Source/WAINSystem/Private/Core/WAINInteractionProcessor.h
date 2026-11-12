/**
 * @file WAINInteractionProcessor.h
 * @brief Mock 턴의 검증·제안 생성·Transaction 실행 및 TurnId 재처리를 조율한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINInterfaces.h"
#include "Core/WAINStateStore.h"
#include "Core/WAINTransactionCoordinator.h"

class FWAINInteractionProcessor final
{
public:
    FWAINInteractionProcessor(
        TSharedRef<IWAINGameSnapshotAdapter> InGameAdapter,
        TSharedRef<IWAINProposalProvider> InProposalProvider,
        TSharedRef<IWAINNarrativeEvaluator> InNarrativeEvaluator);

    bool StartSession(const FWAINStateSnapshot& InitialWAIN, FString& OutError);

    FWAINInteractionResult ProcessTurn(const FWAINInteractionRequest& Request);

    TSharedPtr<const FWAINSessionRoot> GetSnapshot() const;

private:
    struct FRecordedTurn
    {
        FWAINInteractionRequest Request;
        FWAINInteractionResult Result;
    };

    bool ValidateRequest(const FWAINInteractionRequest& Request, FString& OutError) const;

    FWAINInteractionResult ExecuteTurn(const FWAINInteractionRequest& Request);

    static bool IsSameRequest(const FWAINInteractionRequest& Left, const FWAINInteractionRequest& Right);

    static FWAINInteractionResult Fail(
        const FWAINInteractionRequest& Request,
        EWAINInteractionError Error,
        const FString& Message);

    TSharedRef<IWAINGameSnapshotAdapter> GameAdapter;
    TSharedRef<IWAINProposalProvider> ProposalProvider;
    TSharedRef<IWAINNarrativeEvaluator> NarrativeEvaluator;

    FWAINStateStore StateStore;
    FWAINTransactionCoordinator Coordinator;
    TMap<FGuid, FRecordedTurn> CompletedTurns;
    bool bProcessing = false;
};
