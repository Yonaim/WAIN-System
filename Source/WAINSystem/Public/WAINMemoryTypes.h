/**
 * @file WAINMemoryTypes.h
 * @brief WAIN이 소유하는 통합 Memory Stream의 공통 레코드 형식을 선언한다.
 */

#pragma once

#include "CoreMinimal.h"

// ============================================================
// 00. Unified Memory Record
// ============================================================
#pragma region 00_Unified_Memory_Record

    /** @brief 모든 기억은 동일한 Stream에 저장되며 출처는 ID로 연결한다. */
    enum class EWAINMemoryType : uint8
    {
        Observation,
        Reflection,
        Plan
    };

    enum class EWAINMemorySpeaker : uint8
    {
        Player,
        Character,
        System
    };

    /** @brief Memory는 확정 Snapshot에 속하며, 검색에서는 레코드를 수정하지 않는다. */
    struct FWAINMemoryRecord
    {
        FGuid              Id;
        FName              CharacterId;
        EWAINMemoryType    Type = EWAINMemoryType::Observation;
        EWAINMemorySpeaker Speaker = EWAINMemorySpeaker::Player;
        FString            Content;
        int32              CreatedTurn = 0;
        float              Importance = 0.5F;
        TArray<FGuid>      SourceMemoryIds;

        // Plan 전용 메타데이터. Reflection과 Observation은 기본값을 유지한다.
        FName           SceneId;
        FName           BeatId;
        FName           GoalId;
        TArray<FString> PlanSteps;
        int32           CurrentStepIndex = 0;

        // Embedding 및 접근 기록은 후속 구현용 선택 필드다.
        TArray<float> Embedding;
        int32         LastAccessedTurn = INDEX_NONE;
    };

#pragma endregion
