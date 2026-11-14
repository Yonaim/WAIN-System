/**
 * @file WAINRuntimeSubsystem.h
 * @brief Unreal GameInstance에 WAIN 세션의 초기화·Mock 턴·확정 상태 조회를 제공한다.
 */

#pragma once

// clang-format off
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WAINInterfaces.h"
#include "WAINMemoryReasoning.h"
#include "WAINRuntimeTypes.h"
#include "WAINWorldStateManager.h"
#include "WAINInteractionDebug.h"
#include "WAINRuntimeSubsystem.generated.h"
// clang-format on

class FWAINInteractionProcessor;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWAINStateCommitted, const FGuid & /*TurnId*/, uint64 /*Revision*/);

/**
 * @brief 이번 버전은 Game Thread의 동기 Mock Turn만 지원한다.
 *
 * 실제 비동기 LLM SubmitTurn / Blueprint 계약은 이후 확정한다.
 */
UCLASS()
class WAINSYSTEM_API UWAINRuntimeSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

  public:
    bool StartSession(const FWAINStateSnapshot                  &InitialWAIN,
                      TSharedRef<IWAINGameSnapshotAdapter>       GameAdapter,
                      TSharedRef<IWAINProposalProvider>          ProposalProvider,
                      TSharedRef<IWAINNarrativeEvaluator>        NarrativeEvaluator,
                      FString                                   &OutError,
                      TSharedPtr<const IWAINPersonaSource>       PersonaSource = nullptr,
                      TSharedPtr<const IWAINPresentationCatalog> PresentationCatalog = nullptr,
                      TSharedPtr<const IWAINMemoryReasoner>      MemoryReasoner = nullptr,
                      TSharedPtr<const FWAINWorldStateManager>   WorldManager = nullptr);

    FWAINInteractionResult ProcessTurn(const FWAINInteractionRequest &Request);

    /** @brief 이전 Testbed 호출부 호환성 유지. Provider 구현에 따라 LLM Proposal도 처리할 수 있다. */
    FWAINInteractionResult ProcessMockTurn(const FWAINInteractionRequest &Request);

    TSharedPtr<const FWAINSessionRoot> GetStateSnapshot() const;

    bool PrepareTurnContext(const FWAINInteractionRequest &Request,
                            FWAINCharacterContext         &OutContext,
                            FString                       &OutError) const;

    /** @brief 개발 패널 전용 조회. State 정본이나 Turn 실행 결과를 변경하지 않는다. */
    bool GetDebugRecord(const FGuid &TurnId, FWAINInteractionDebugRecord &OutRecord) const;

    FOnWAINStateCommitted &OnStateCommitted()
    {
        return StateCommitted;
    }

    void EndSession();

    virtual void Deinitialize() override;

  private:
    TSharedPtr<FWAINInteractionProcessor> Processor;
    FOnWAINStateCommitted                 StateCommitted;
};
