/**
 * @file WAINStateValidation.cpp
 * @brief 캐릭터·감정 범위와 필수 Narrative State의 유효성을 검사한다.
 */

#include "Core/WAINStateValidation.h"

namespace WAINStateValidation
{
    namespace
    {
        bool HasValidEmotionValues(const FWAINEmotionState &Emotion)
        {
            const float Values[] =
                {Emotion.Joy, Emotion.Sadness, Emotion.Anger, Emotion.Disgust, Emotion.Anxiety, Emotion.Surprise};

            for (const float Value : Values)
            {
                if (!FMath::IsFinite(Value) || Value < 0.0F || Value > 1.0F)
                {
                    return false;
                }
            }

            return true;
        }
    } // namespace

    bool Validate(const FWAINStateSnapshot &Snapshot, FString &OutError)
    {
        if (Snapshot.ActiveBeatId.IsNone() || Snapshot.TurnCount < 0 || Snapshot.Characters.IsEmpty())
        {
            OutError = TEXT("Snapshot에 유효한 ActiveBeat, TurnCount, Character가 필요합니다.");
            return false;
        }

        for (const TPair<FName, FWAINCharacterState> &Pair : Snapshot.Characters)
        {
            if (Pair.Key.IsNone() || Pair.Value.Trust < 0 || Pair.Value.Trust > 100 ||
                !HasValidEmotionValues(Pair.Value.Emotion))
            {
                OutError = TEXT("Character Trust 또는 6차원 Emotion이 허용 범위를 벗어났습니다.");
                return false;
            }
        }

        return true;
    }
} // namespace WAINStateValidation
