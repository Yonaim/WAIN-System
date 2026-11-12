/**
 * @file WAINRuntimeSubsystem.h
 * @brief Unreal GameInstance에 WAIN 세션의 초기화·Mock 턴·확정 상태 조회를 제공한다.
 */

#pragma once

// clang-format off
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WAINInterfaces.h"
#include "WAINRuntimeTypes.h"
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
    bool StartSession(const FWAINStateSnapshot            &InitialWAIN,
                      TSharedRef<IWAINGameSnapshotAdapter> GameAdapter,
                      TSharedRef<IWAINProposalProvider>    ProposalProvider,
                      TSharedRef<IWAINNarrativeEvaluator>  NarrativeEvaluator,
                      FString                             &OutError,
                      TSharedPtr<const IWAINPersonaSource> PersonaSource = nullptr,
                      TSharedPtr<const IWAINPresentationCatalog> PresentationCatalog = nullptr);

    FWAINInteractionResult ProcessMockTurn(const FWAINInteractionRequest &Request);

    TSharedPtr<const FWAINSessionRoot> GetStateSnapshot() const;

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
