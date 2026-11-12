/**
 * @file WAINMemoryWriter.h
 * @brief 커밋 후보 Snapshot에 이번 턴의 Observation을 추가한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINRuntimeTypes.h"

class FWAINMemoryWriter final
{
  public:
    bool PrepareObservations(const FWAINInteractionRequest &Request,
                             const FWAINResponseProposal   &Proposal,
                             FWAINStateSnapshot            &Prepared,
                             FString                       &OutError) const;

  private:
    static bool AppendObservation(FName               CharacterId,
                                  EWAINMemorySpeaker  Speaker,
                                  const FString      &Content,
                                  int32               CreatedTurn,
                                  FWAINStateSnapshot &Prepared,
                                  FString            &OutError);
};
