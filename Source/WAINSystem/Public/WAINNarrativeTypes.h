/**
 * @file WAINNarrativeTypes.h
 * @brief Beat Graph, 상태 조건식, 전이와 읽기 전용 서사 Context의 자료형을 선언한다.
 */

#pragma once

#include "CoreMinimal.h"

class IWAINGameStateSnapshot;

// ============================================================
// 00. Deterministic Narrative Conditions
// ============================================================
#pragma region 00_Deterministic_Narrative_Conditions

    // 숫자 기반 Condition과 Host Game Fact 조회 계약을 정의한다.

    enum class EWAINNarrativeFact : uint8
    {
        CharacterTrust,
        TurnCount,
        GameInteger
    };

    enum class EWAINNarrativeComparison : uint8
    {
        Equal,
        AtLeast,
        AtMost
    };

    /** @brief GameInteger인 경우에만 TargetId를 사용한다. 조건 배열은 논리 AND다. */
    struct FWAINNarrativeCondition
    {
        EWAINNarrativeFact Source = EWAINNarrativeFact::CharacterTrust;
        FName TargetId;
        EWAINNarrativeComparison Comparison = EWAINNarrativeComparison::AtLeast;
        int32 Value = 0;
    };

    /** @brief Game Integer의 증가를 감지하면 준비된 State에서 선행 조건을 검사한다. */
    struct FWAINNarrativeHardRule
    {
        FName GuardedGameTargetId;
        TArray<FWAINNarrativeCondition> Prerequisites;
    };

    /** @brief Host Game의 불변 Prepared Snapshot에서 조건용 숫자만 읽는다. */
    class IWAINNarrativeGameFactReader
    {
        public:
        virtual ~IWAINNarrativeGameFactReader() = default;

        virtual bool ReadInteger(const IWAINGameStateSnapshot &Snapshot,
                                 FName TargetId,
                                 int32 &OutValue) const = 0;
    };

#pragma endregion

// ============================================================
// 01. Authored Narrative Graph
// ============================================================
#pragma region 01_Authored_Narrative_Graph

    // Chapter·Sequence·Beat의 Goal, 규칙과 전이를 정의한다.

    struct FWAINNarrativeChapter
    {
        FName Id;
        FString Goal;
        TArray<FString> SoftConstraints;
        TArray<FWAINNarrativeHardRule> HardRules;
    };

    struct FWAINNarrativeSequence
    {
        FName Id;
        FName ChapterId;
        FString Goal;
        TArray<FString> SoftConstraints;
        TArray<FWAINNarrativeHardRule> HardRules;
    };

    struct FWAINBeatTransition
    {
        FName TargetBeatId;
        int32 Priority = 0;
        TArray<FWAINNarrativeCondition> Conditions;
    };

    struct FWAINNarrativeBeat
    {
        FName Id;
        FName SequenceId;
        FString Goal;
        TArray<FString> SoftConstraints;
        TArray<FWAINNarrativeHardRule> HardRules;
        TArray<FWAINNarrativeCondition> CompletionConditions;
        TArray<FWAINBeatTransition> Transitions;
    };

    struct FWAINNarrativeGraph
    {
        TArray<FWAINNarrativeChapter> Chapters;
        TArray<FWAINNarrativeSequence> Sequences;
        TArray<FWAINNarrativeBeat> Beats;
    };

#pragma endregion

// ============================================================
// 02. Read-Only Narrative Guidance
// ============================================================
#pragma region 02_Read_Only_Narrative_Guidance

    // 현재 활성 부모 경로의 지침만 공개 Context로 제공한다.

    /** @brief 현재 활성 부모 경로의 공개 가능한 Goal/Soft Constraint만 Provider에 전달한다. */
    struct FWAINNarrativeContext
    {
        FName ChapterId;
        FName SequenceId;
        FName BeatId;
        FString ChapterGoal;
        FString SequenceGoal;
        FString BeatGoal;
        TArray<FString> SoftConstraints;
    };

#pragma endregion
