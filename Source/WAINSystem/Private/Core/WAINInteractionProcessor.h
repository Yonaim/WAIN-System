/**
 * @file WAINInteractionProcessor.h
 * @brief Mock 턴의 검증·제안 생성·Transaction 실행 및 TurnId 재처리를 조율한다.
 */

#pragma once

#include "Character/WAINCharacterContextBuilder.h"
#include "Core/WAINStateStore.h"
#include "Core/WAINTransactionCoordinator.h"
#include "CoreMinimal.h"
#include "WAINInterfaces.h"
#include "WAINWorldStateManager.h"
#include "WAINInteractionDebug.h"

class FWAINInteractionProcessor final
{
  public:
// ============================================================
// 00. Public Interaction API
// ============================================================
#pragma region 00_Public_Interaction_API

    FWAINInteractionProcessor(TSharedRef<IWAINGameSnapshotAdapter>       InGameAdapter,
                              TSharedRef<IWAINProposalProvider>          InProposalProvider,
                              TSharedRef<IWAINNarrativeEvaluator>        InNarrativeEvaluator,
                              TSharedPtr<const IWAINPersonaSource>       InPersonaSource = nullptr,
                              TSharedPtr<const IWAINPresentationCatalog> InPresentationCatalog = nullptr,
                              TSharedPtr<const IWAINMemoryReasoner>      InMemoryReasoner = nullptr,
                              TSharedPtr<const FWAINWorldStateManager>   InWorldManager = nullptr);

    bool StartSession(const FWAINStateSnapshot &InitialWAIN, FString &OutError);

    FWAINInteractionResult ProcessTurn(const FWAINInteractionRequest &Request);

    TSharedPtr<const FWAINSessionRoot> GetSnapshot() const;

    /** @brief 최신 48개 턴 기록에서 해당 TurnId를 조회한다. Replay는 기존 기록을 변경하지 않는다. */
    bool GetDebugRecord(const FGuid &TurnId, FWAINInteractionDebugRecord &OutRecord) const;

    private:
#pragma endregion

// ============================================================
// 01. Turn Execution And Validation
// ============================================================
#pragma region 01_Turn_Execution_And_Validation

    struct FRecordedTurn
    {
        FWAINInteractionRequest Request;
        FWAINInteractionResult  Result;
    };

    bool ValidateRequest(const FWAINInteractionRequest &Request, FString &OutError) const;

    FWAINInteractionResult ExecuteTurn(const FWAINInteractionRequest &Request,
                                      FWAINInteractionDebugRecord &OutDebug);

    void SaveDebugRecord(FWAINInteractionDebugRecord &Record, const FWAINInteractionResult &Result);

    static bool IsSameRequest(const FWAINInteractionRequest &Left, const FWAINInteractionRequest &Right);

    static FWAINInteractionResult
    Fail(const FWAINInteractionRequest &Request, EWAINInteractionError Error, const FString &Message);

#pragma endregion

// ============================================================
// 02. Dependencies And Session State
// ============================================================
#pragma region 02_Dependencies_And_Session_State

    TSharedRef<IWAINGameSnapshotAdapter>       GameAdapter;
    TSharedRef<IWAINProposalProvider>          ProposalProvider;
    TSharedRef<IWAINNarrativeEvaluator>        NarrativeEvaluator;
    TSharedPtr<const IWAINPersonaSource>       PersonaSource;
    TSharedPtr<const IWAINPresentationCatalog> PresentationCatalog;
    TSharedPtr<const IWAINMemoryReasoner>      MemoryReasoner;
    TSharedPtr<const FWAINWorldStateManager>   WorldManager;
    FWAINCharacterContextBuilder               ContextBuilder;

    FWAINStateStore             StateStore;
    FWAINTransactionCoordinator Coordinator;
    TMap<FGuid, FRecordedTurn>  CompletedTurns;
    TMap<FGuid, FWAINInteractionDebugRecord> DebugRecords;
    TArray<FGuid> DebugOrder;
    bool                        bProcessing = false;
#pragma endregion
};
