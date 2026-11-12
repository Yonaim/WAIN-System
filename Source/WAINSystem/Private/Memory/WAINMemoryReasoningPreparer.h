/**
 * @file WAINMemoryReasoningPreparer.h
 * @brief Snapshot에 Reflection과 활성 Plan 후보를 작성하는 동기 MVP 처리기를 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINMemoryReasoning.h"

class FWAINMemoryReasoningPreparer final
{
  public:
    bool Prepare(const FWAINInteractionRequest &Request,
                 FWAINStateSnapshot            &Prepared,
                 const IWAINMemoryReasoner     &Reasoner,
                 FString                       &OutError) const;

  private:
    static bool PrepareReflection(const FWAINInteractionRequest &Request,
                                  FWAINStateSnapshot            &Prepared,
                                  const IWAINMemoryReasoner     &Reasoner,
                                  FString                       &OutError);

    static bool PreparePlan(const FWAINInteractionRequest &Request,
                            FWAINStateSnapshot            &Prepared,
                            const IWAINMemoryReasoner     &Reasoner,
                            FString                       &OutError);

    static TArray<FWAINMemoryRecord> CollectUnreflectedObservations(FName                     CharacterId,
                                                                    const FWAINStateSnapshot &Prepared);

    static TArray<FWAINMemoryRecord> CollectPlanSources(FName CharacterId, const FWAINStateSnapshot &Prepared);

    static bool RequiresPlan(const FWAINInteractionRequest &Request, const FWAINStateSnapshot &Prepared);
    static bool ValidateDraftContent(const FString &Content, FString &OutError);
    static bool AppendDerivedMemory(FWAINMemoryRecord &&Record, FWAINStateSnapshot &Prepared, FString &OutError);
};
