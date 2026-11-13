/**
 * @file WAINInteractionDebug.h
 * @brief 개발용 Turn 단계 기록과 Proposal/Context의 읽기 전용 조회 계약을 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINCharacterContext.h"

// ============================================================
// 00. Verified Stage Records
// ============================================================
#pragma region 00_Verified_Stage_Records

    enum class EWAINTraceStatus : uint8
    {
        Passed,
        Failed,
        Skipped
    };

    struct FWAINTraceStep
    {
        FName Name;
        EWAINTraceStatus Status = EWAINTraceStatus::Skipped;
        FString Detail;
    };

    /** @brief WAIN이 실제 실행한 단계만 기록한다. 실행되지 않은 단계는 PASS로 추정하지 않는다. */
    struct FWAINInteractionDebugRecord
    {
        FWAINInteractionRequest Request;
        FWAINInteractionResult Result;
        uint64 RevisionBefore = 0;
        uint64 RevisionAfter = 0;
        double ElapsedMs = 0.0;
        bool bHasContext = false;
        FWAINCharacterContext Context;
        bool bHasProposal = false;
        FWAINResponseProposal Proposal;
        TArray<FWAINTraceStep> Steps;

        void AddStep(FName Name, EWAINTraceStatus Status, const FString &Detail = FString())
        {
            FWAINTraceStep Entry;
            Entry.Name = Name;
            Entry.Status = Status;
            Entry.Detail = Detail;
            Steps.Add(MoveTemp(Entry));
        }
    };

#pragma endregion
