/**
 * @file WAINStateValidation.cpp
 * @brief 확정 전 Character·Emotion·Memory Stream의 공통 불변조건을 검증한다.
 */

#include "Core/WAINStateValidation.h"

namespace WAINStateValidation
{
    namespace
    {
        bool ValidateCharacters(const FWAINStateSnapshot &Snapshot, FString &OutError);
        bool ValidateMemoryStream(const FWAINStateSnapshot &Snapshot, FString &OutError);
        bool HasValidEmotionValues(const FWAINEmotionState &Emotion);
    }

// ============================================================
// 00. Snapshot Validation Pipeline
// ============================================================
#pragma region 00_Snapshot_Validation_Pipeline

    bool Validate(const FWAINStateSnapshot &Snapshot, FString &OutError)
    {
        if (Snapshot.ActiveBeatId.IsNone() || Snapshot.TurnCount < 0 || Snapshot.Characters.IsEmpty())
        {
            OutError = TEXT("Snapshot에 유효한 ActiveBeat, TurnCount, Character가 필요합니다.");
            return false;
        }

        if (!ValidateCharacters(Snapshot, OutError))
        {
            return false;
        }

        return ValidateMemoryStream(Snapshot, OutError);
    }

#pragma endregion

// ============================================================
// 01. Character And Emotion Invariants
// ============================================================
#pragma region 01_Character_And_Emotion_Invariants

    namespace
    {
        bool ValidateCharacters(const FWAINStateSnapshot &Snapshot, FString &OutError)
        {
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

#pragma endregion

// ============================================================
// 02. Unified Memory Stream Invariants
// ============================================================
#pragma region 02_Unified_Memory_Stream_Invariants

    namespace
    {
        bool ValidateMemoryStream(const FWAINStateSnapshot &Snapshot, FString &OutError)
        {
            if (Snapshot.MemoryStream.Num() > 2000)
            {
                OutError = TEXT("Memory Stream의 저장 개수 제한을 초과했습니다.");
                return false;
            }

            TMap<FGuid, FName> MemoryOwners;
            for (const FWAINMemoryRecord &Memory : Snapshot.MemoryStream)
            {
                if (!Memory.Id.IsValid() || MemoryOwners.Contains(Memory.Id) ||
                    !Snapshot.Characters.Contains(Memory.CharacterId) || Memory.Content.TrimStartAndEnd().IsEmpty() ||
                    Memory.Content.Len() > 4096 || Memory.CreatedTurn < 0 || Memory.CreatedTurn > Snapshot.TurnCount ||
                    !FMath::IsFinite(Memory.Importance) || Memory.Importance < 0.0F || Memory.Importance > 1.0F ||
                    (Memory.Type != EWAINMemoryType::Observation && Memory.Type != EWAINMemoryType::Reflection &&
                     Memory.Type != EWAINMemoryType::Plan) ||
                    (Memory.Speaker != EWAINMemorySpeaker::Player && Memory.Speaker != EWAINMemorySpeaker::Character &&
                     Memory.Speaker != EWAINMemorySpeaker::System))
                {
                    OutError = TEXT("Memory Stream의 ID, Owner, Content, Turn, Importance 또는 Type이 유효하지 않습니다.");
                    return false;
                }
                MemoryOwners.Add(Memory.Id, Memory.CharacterId);
            }

            for (const FWAINMemoryRecord &Memory : Snapshot.MemoryStream)
            {
                for (const FGuid &SourceId : Memory.SourceMemoryIds)
                {
                    const FName *Owner = MemoryOwners.Find(SourceId);
                    if (!Owner || *Owner != Memory.CharacterId || SourceId == Memory.Id)
                    {
                        OutError = TEXT("Memory 출처 ID가 없거나 다른 Character의 기억을 참조합니다.");
                        return false;
                    }
                }
            }
            return true;
        }
    } // namespace

#pragma endregion

} // namespace WAINStateValidation
