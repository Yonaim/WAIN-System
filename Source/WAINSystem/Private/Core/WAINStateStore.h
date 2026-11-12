/**
 * @file WAINStateStore.h
 * @brief 확정된 Session Root를 유지하고 Revision 조건부 단일 교체를 수행한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINInterfaces.h"

class FWAINStateStore final
{
  public:
    bool Initialize(const FWAINStateSnapshot                &InitialWAIN,
                    TSharedPtr<const IWAINGameStateSnapshot> InitialGame,
                    FString                                 &OutError);

    TSharedPtr<const FWAINSessionRoot> Read() const;

    bool Publish(uint64                                   ExpectedRevision,
                 FWAINStateSnapshot                     &&PreparedWAIN,
                 TSharedPtr<const IWAINGameStateSnapshot> PreparedGame);

  private:
    // 단일 Game Thread에서만 읽고 교체한다. 외부로는 const Root만 공개한다.
    TSharedPtr<const FWAINSessionRoot> CurrentRoot;
};
