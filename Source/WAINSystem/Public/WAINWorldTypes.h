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

        virtual bool ReadInteger(const IWAINGameStateSnapshot &Snapshot, FName TargetId, int32 &OutValue) const = 0;
    };

#pragma endregion

// ============================================================
// 01. Fact Definition And Visibility
// ============================================================
#pragma region 01_Fact_Definition_And_Visibility

    // Schema는 게임이 제공하는 정적 권한 정의이고 실제 값은 Host Game Snapshot에 남는다.

    struct FWAINWorldFactDefinition
    {
        FName                Key;
        int32                MinValue = 0;
        int32                MaxValue = MAX_int32;
        bool                 bExposeToAgent = false;
        bool                 bWritableByProposal = false;
        bool                 bWritableByScript = false; // 작가 이벤트만 해당 Fact를 변경하도록 허가.
        FString              Description;               // Character Agent에게 공개되는 Fact의 의미.
        TMap<int32, FString> ValueLabels;               // enum 값이 공개된 경우에만 사용한다.
    };

    struct FWAINWorldFactValue
    {
        FName   Key;
        int32   Value = 0;
        FString Description;
        FString AllowedValues;
    };

    /** @brief Context에는 공개된 사실만 포함하며, 숨겨진 Fact의 ID도 노출하지 않는다. */
    struct FWAINWorldContext
    {
        TArray<FWAINWorldFactValue> VisibleFacts;
    };

#pragma endregion
