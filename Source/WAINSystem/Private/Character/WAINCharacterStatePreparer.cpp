/**
 * @file WAINCharacterStatePreparer.cpp
 * @brief 다섯 관계 파라미터와 독립 6차원 Emotion Delta를 Prepared Character State에 반영한다.
 */

#include "Character/WAINCharacterStatePreparer.h"

// ============================================================
// 00. Prepared Character Mutation Pipeline
// ============================================================
#pragma region 00_Prepared_Character_Mutation_Pipeline

    bool FWAINCharacterStatePreparer::Apply(const FName                       CharacterId,
                                            const TArray<FWAINStateMutation> &Mutations,
                                            FWAINStateSnapshot               &Prepared,
                                            FString                          &OutError) const
    {
        FWAINCharacterState *Character = Prepared.Characters.Find(CharacterId);
        if (!Character)
        {
            OutError = TEXT("존재하지 않는 CharacterId입니다.");
            return false;
        }

        for (const FWAINStateMutation &Mutation : Mutations)
        {
            if (Mutation.Owner != EWAINMutationOwner::Character)
            {
                continue;
            }

            if (!ApplyMutation(Mutation, *Character, OutError))
            {
                return false;
            }
        }

        return true;
    }

    bool FWAINCharacterStatePreparer::ApplyMutation(const FWAINStateMutation &Mutation,
                                                    FWAINCharacterState      &Character,
                                                    FString                  &OutError) const
    {
        if (Mutation.TargetId.ToString().StartsWith(TEXT("relationship.")))
        {
            return ApplyRelationship(Mutation, Character, OutError);
        }

        return ApplyEmotionDelta(Mutation, Character, OutError);
    }

#pragma endregion

// ============================================================
// 01. Relationship Update
// ============================================================
#pragma region 01_Relationship_Update

    bool FWAINCharacterStatePreparer::ApplyRelationship(const FWAINStateMutation &Mutation,
                                                        FWAINCharacterState      &Character,
                                                        FString                  &OutError) const
    {
        int32 *Target = nullptr;
        if (Mutation.TargetId == TEXT("relationship.trust"))
            Target = &Character.Trust;
        else if (Mutation.TargetId == TEXT("relationship.affection"))
            Target = &Character.Affection;
        else if (Mutation.TargetId == TEXT("relationship.familiarity"))
            Target = &Character.Familiarity;
        else if (Mutation.TargetId == TEXT("relationship.comfort"))
            Target = &Character.Comfort;
        else if (Mutation.TargetId == TEXT("relationship.romantic_attraction"))
            Target = &Character.RomanticAttraction;
        if (!Target)
        {
            OutError = TEXT("등록되지 않은 Relationship Target입니다.");
            return false;
        }
        if (Mutation.Operation != EWAINMutationOperation::AddInteger &&
            Mutation.Operation != EWAINMutationOperation::SetInteger)
        {
            OutError = TEXT("관계 파라미터에는 정수 Set/Add만 허용됩니다.");
            return false;
        }
        const int64 Next = Mutation.Operation == EWAINMutationOperation::AddInteger
                               ? static_cast<int64>(*Target) + Mutation.IntegerValue
                               : static_cast<int64>(Mutation.IntegerValue);
        if (Next < 0 || Next > 100)
        {
            OutError = TEXT("관계 파라미터는 0부터 100 사이여야 합니다.");
            return false;
        }
        *Target = static_cast<int32>(Next);
        return true;
    }

#pragma endregion

// ============================================================
// 02. Emotion Delta Update
// ============================================================
#pragma region 02_Emotion_Delta_Update

    bool FWAINCharacterStatePreparer::ApplyEmotionDelta(const FWAINStateMutation &Mutation,
                                                        FWAINCharacterState      &Character,
                                                        FString                  &OutError) const
    {
        float *Dimension = FindEmotionDimension(Character.Emotion, Mutation.TargetId);
        if (!Dimension)
        {
            OutError = TEXT("등록되지 않은 Character State Target입니다.");
            return false;
        }

        if (Mutation.Operation != EWAINMutationOperation::AddFloat || !FMath::IsFinite(Mutation.FloatValue))
        {
            OutError = TEXT("Emotion에는 유한한 float Delta만 허용됩니다.");
            return false;
        }

        // 여섯 감정값은 독립적이며 정규화하지 않는다. 각 차원만 [0, 1]로 제한한다.
        *Dimension = FMath::Clamp(*Dimension + Mutation.FloatValue, 0.0F, 1.0F);
        return true;
    }

    float *FWAINCharacterStatePreparer::FindEmotionDimension(FWAINEmotionState &Emotion, const FName TargetId)
    {
        if (TargetId == TEXT("emotion.joy"))
        {
            return &Emotion.Joy;
        }
        if (TargetId == TEXT("emotion.sadness"))
        {
            return &Emotion.Sadness;
        }
        if (TargetId == TEXT("emotion.anger"))
        {
            return &Emotion.Anger;
        }
        if (TargetId == TEXT("emotion.disgust"))
        {
            return &Emotion.Disgust;
        }
        if (TargetId == TEXT("emotion.anxiety"))
        {
            return &Emotion.Anxiety;
        }
        if (TargetId == TEXT("emotion.surprise"))
        {
            return &Emotion.Surprise;
        }
        return nullptr;
    }

#pragma endregion
