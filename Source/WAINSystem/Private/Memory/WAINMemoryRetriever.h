/**
 * @file WAINMemoryRetriever.h
 * @brief 확정 Memory Stream에서 현재 캐릭터와 입력에 맞는 Observation을 검색한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINRuntimeTypes.h"

class FWAINMemoryRetriever final
{
  public:
    TArray<FWAINMemoryRecord> Retrieve(const FWAINInteractionRequest &Request, const FWAINStateSnapshot &Current) const;

  private:
    struct FRankedMemory
    {
        int32 Index;
        float Score;
    };

    static TArray<FRankedMemory> RankCandidates(const FWAINInteractionRequest &Request,
                                                const FWAINStateSnapshot      &Current);

    static TArray<FWAINMemoryRecord> SelectTopMemories(const TArray<FRankedMemory> &Ranked,
                                                       const FWAINStateSnapshot    &Current);

    static void
    AppendActivePlan(FName CharacterId, const FWAINStateSnapshot &Current, TArray<FWAINMemoryRecord> &OutResults);

    static float CalculateRelevance(const FString &Query, const FString &Content);
    static float
    CalculateScore(const FWAINInteractionRequest &Request, const FWAINMemoryRecord &Memory, int32 CurrentTurn);
};
