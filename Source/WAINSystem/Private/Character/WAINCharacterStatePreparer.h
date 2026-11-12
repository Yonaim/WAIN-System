/**
 * @file WAINCharacterStatePreparer.h
 * @brief Character-owned Mutation을 Prepared Snapshot에만 적용하는 상태 변경기를 선언한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINRuntimeTypes.h"

/**
 * @brief Relationship 및 6차원 Emotion 변경을 준비한다.
 *
 * 확정된 Session Root에 접근하지 않으며, 호출자가 소유한 Prepared Snapshot만 수정한다.
 */
class FWAINCharacterStatePreparer final
{
  public:
    bool Apply(const FName                       CharacterId,
               const TArray<FWAINStateMutation> &Mutations,
               FWAINStateSnapshot               &Prepared,
               FString                          &OutError) const;

  private:
    bool ApplyMutation(const FWAINStateMutation &Mutation, FWAINCharacterState &Character, FString &OutError) const;
    bool ApplyTrust(const FWAINStateMutation &Mutation, FWAINCharacterState &Character, FString &OutError) const;
    bool ApplyEmotionDelta(const FWAINStateMutation &Mutation, FWAINCharacterState &Character, FString &OutError) const;

    static float *FindEmotionDimension(FWAINEmotionState &Emotion, const FName TargetId);
};
