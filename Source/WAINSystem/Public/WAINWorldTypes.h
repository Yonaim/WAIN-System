/**
 * @file WAINWorldTypes.h
 * @brief Game 소유 World Fact의 공통 정수 조회·정의·공개 Context 계약을 선언한다.
 */

#pragma once

#include "CoreMinimal.h"

class IWAINGameStateSnapshot;

// ============================================================
// 00. Read-Only World Fact Source
// ============================================================
#pragma region 00_Read_Only_World_Fact_Source

    // Game-owned State는 Host Snapshot에서만 조회하며 Plugin에 별도 정본을 만들지 않는다.

    class IWAINWorldFactReader
    {
        public:
        virtual ~IWAINWorldFactReader() = default;

        virtual bool ReadInteger(const IWAINGameStateSnapshot &Snapshot,
                                 FName TargetId,
                                 int32 &OutValue) const = 0;
    };

#pragma endregion

// ============================================================
// 01. Fact Definition And Visibility
// ============================================================
#pragma region 01_Fact_Definition_And_Visibility

    // Schema는 게임이 제공하는 정적 권한 정의이고 실제 값은 Host Game Snapshot에 남는다.

    struct FWAINWorldFactDefinition
    {
        FName Key;
        int32 MinValue = 0;
        int32 MaxValue = MAX_int32;
        bool bExposeToAgent = false;
        bool bWritableByProposal = false;
    };

    struct FWAINWorldFactValue
    {
        FName Key;
        int32 Value = 0;
    };

    /** @brief Context에는 공개된 사실만 포함하며, 숨겨진 Fact의 ID도 노출하지 않는다. */
    struct FWAINWorldContext
    {
        TArray<FWAINWorldFactValue> VisibleFacts;
    };

#pragma endregion
