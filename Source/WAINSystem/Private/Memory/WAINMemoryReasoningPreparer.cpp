/**
 * @file WAINMemoryReasoningPreparer.cpp
 * @brief Reflection/Plan Trigger, 근거 연결 및 Prepared Snapshot 변경을 조율한다.
 */

#include "Algo/Reverse.h"
#include "Memory/WAINMemoryReasoningPreparer.h"

namespace WAINMemoryReasoning
{
    constexpr float ReflectionThreshold = 2.0F; // MVP 실험값: Observation 중요도 합
    constexpr int32 MaxReflectionSources = 12;
    constexpr int32 MaxPlanSources = 6;
    constexpr int32 MaxPlanSteps = 8;
}

// ============================================================
// 00. Reasoning Pipeline
// ============================================================
#pragma region 00_Reasoning_Pipeline

    // 턴의 Reasoning 처리 순서를 Reflection 평가와 Plan 평가로 분리한다.

    bool FWAINMemoryReasoningPreparer::Prepare(const FWAINInteractionRequest &Request,
                                              FWAINStateSnapshot &Prepared,
                                              const IWAINMemoryReasoner &Reasoner,
                                              FString &OutError) const
    {
        if (!PrepareReflection(Request, Prepared, Reasoner, OutError))
        {
            return false;
        }

        return PreparePlan(Request, Prepared, Reasoner, OutError);
    }

#pragma endregion

// ============================================================
// 01. Reflection Preparation
// ============================================================
#pragma region 01_Reflection_Preparation

    // 새 Observation의 중요도 누적량이 임계값을 넘으면 출처가 연결된 Reflection 후보를 생성한다.

    bool FWAINMemoryReasoningPreparer::PrepareReflection(const FWAINInteractionRequest &Request,
                                                 FWAINStateSnapshot &Prepared,
                                                 const IWAINMemoryReasoner &Reasoner,
                                                 FString &OutError)
    {
        const TArray<FWAINMemoryRecord> Sources = CollectUnreflectedObservations(Request.CharacterId, Prepared);
        float TotalImportance = 0.0F;
        for (const FWAINMemoryRecord &Memory : Sources)
        {
            TotalImportance += Memory.Importance;
        }

        if (Sources.Num() < 2 || TotalImportance < WAINMemoryReasoning::ReflectionThreshold)
        {
            return true;
        }

        FString Insight;
        if (!Reasoner.GenerateReflection(Request, Sources, Insight, OutError) ||
            !ValidateDraftContent(Insight, OutError))
        {
            if (OutError.IsEmpty()) OutError = TEXT("Reflection을 생성하지 못했습니다.");
            return false;
        }

        FWAINMemoryRecord Reflection;
        Reflection.Id = FGuid::NewGuid();
        Reflection.CharacterId = Request.CharacterId;
        Reflection.Type = EWAINMemoryType::Reflection;
        Reflection.Speaker = EWAINMemorySpeaker::System;
        Reflection.Content = MoveTemp(Insight);
        Reflection.CreatedTurn = Prepared.TurnCount;
        Reflection.Importance = 0.7F; // MVP 실험값
        for (const FWAINMemoryRecord &Source : Sources)
        {
            Reflection.SourceMemoryIds.Add(Source.Id);
        }

        return AppendDerivedMemory(MoveTemp(Reflection), Prepared, OutError);
    }

    TArray<FWAINMemoryRecord> FWAINMemoryReasoningPreparer::CollectUnreflectedObservations(
        const FName CharacterId, const FWAINStateSnapshot &Prepared)
    {
        int32 LastReflectionTurn = INDEX_NONE;
        for (const FWAINMemoryRecord &Memory : Prepared.MemoryStream)
        {
            if (Memory.CharacterId == CharacterId && Memory.Type == EWAINMemoryType::Reflection)
            {
                LastReflectionTurn = FMath::Max(LastReflectionTurn, Memory.CreatedTurn);
            }
        }

        TArray<FWAINMemoryRecord> Sources;
        for (int32 Index = Prepared.MemoryStream.Num() - 1;
             Index >= 0 && Sources.Num() < WAINMemoryReasoning::MaxReflectionSources; --Index)
        {
            const FWAINMemoryRecord &Memory = Prepared.MemoryStream[Index];
            if (Memory.CharacterId == CharacterId && Memory.Type == EWAINMemoryType::Observation &&
                Memory.CreatedTurn > LastReflectionTurn)
            {
                Sources.Add(Memory);
            }
        }
        Algo::Reverse(Sources);
        return Sources;
    }

#pragma endregion

// ============================================================
// 02. Plan Preparation And Reuse
// ============================================================
#pragma region 02_Plan_Preparation_And_Reuse

    // 기존 Plan의 Scene/Beat 유효성을 확인한 뒤 필요할 때만 새 Plan 후보를 준비한다.

    bool FWAINMemoryReasoningPreparer::PreparePlan(const FWAINInteractionRequest &Request,
                                           FWAINStateSnapshot &Prepared,
                                           const IWAINMemoryReasoner &Reasoner,
                                           FString &OutError)
    {
        if (!RequiresPlan(Request, Prepared))
        {
            return true;
        }

        const TArray<FWAINMemoryRecord> Sources = CollectPlanSources(Request.CharacterId, Prepared);
        if (Sources.IsEmpty())
        {
            OutError = TEXT("Plan을 생성할 근거 Memory가 없습니다.");
            return false;
        }

        FWAINPlanDraft Draft;
        if (!Reasoner.GeneratePlan(Request, Prepared.ActiveBeatId, Sources, Draft, OutError) ||
            !ValidateDraftContent(Draft.Summary, OutError))
        {
            if (OutError.IsEmpty()) OutError = TEXT("Plan을 생성하지 못했습니다.");
            return false;
        }

        if (Draft.Steps.IsEmpty() || Draft.Steps.Num() > WAINMemoryReasoning::MaxPlanSteps)
        {
            OutError = TEXT("Plan의 Step 개수가 허용 범위를 벗어났습니다.");
            return false;
        }
        for (const FString &Step : Draft.Steps)
        {
            if (!ValidateDraftContent(Step, OutError)) return false;
        }

        FWAINMemoryRecord Plan;
        Plan.Id = FGuid::NewGuid();
        Plan.CharacterId = Request.CharacterId;
        Plan.Type = EWAINMemoryType::Plan;
        Plan.Speaker = EWAINMemorySpeaker::System;
        Plan.Content = MoveTemp(Draft.Summary);
        Plan.CreatedTurn = Prepared.TurnCount;
        Plan.Importance = 0.7F;
        Plan.SceneId = Request.SceneId;
        Plan.BeatId = Prepared.ActiveBeatId;
        Plan.GoalId = Prepared.ActiveBeatId; // MVP: Beat를 계획 목표 키로 사용
        Plan.PlanSteps = MoveTemp(Draft.Steps);
        for (const FWAINMemoryRecord &Source : Sources)
        {
            Plan.SourceMemoryIds.Add(Source.Id);
        }

        const FGuid NewPlanId = Plan.Id;
        if (!AppendDerivedMemory(MoveTemp(Plan), Prepared, OutError))
        {
            return false;
        }
        Prepared.ActivePlanIds.Add(Request.CharacterId, NewPlanId);
        return true;
    }

    bool FWAINMemoryReasoningPreparer::RequiresPlan(const FWAINInteractionRequest &Request,
                                           const FWAINStateSnapshot &Prepared)
    {
        const FGuid *PlanId = Prepared.ActivePlanIds.Find(Request.CharacterId);
        if (!PlanId) return true;

        for (const FWAINMemoryRecord &Memory : Prepared.MemoryStream)
        {
            if (Memory.Id == *PlanId && Memory.CharacterId == Request.CharacterId &&
                Memory.Type == EWAINMemoryType::Plan)
            {
                return Memory.SceneId != Request.SceneId || Memory.BeatId != Prepared.ActiveBeatId;
            }
        }
        return true;
    }

    TArray<FWAINMemoryRecord> FWAINMemoryReasoningPreparer::CollectPlanSources(const FName CharacterId,
                                                                       const FWAINStateSnapshot &Prepared)
    {
        TArray<FWAINMemoryRecord> Sources;
        for (int32 Index = Prepared.MemoryStream.Num() - 1;
             Index >= 0 && Sources.Num() < WAINMemoryReasoning::MaxPlanSources; --Index)
        {
            const FWAINMemoryRecord &Memory = Prepared.MemoryStream[Index];
            if (Memory.CharacterId == CharacterId && Memory.Type != EWAINMemoryType::Plan)
            {
                Sources.Add(Memory);
            }
        }
        Algo::Reverse(Sources);
        return Sources;
    }

#pragma endregion

// ============================================================
// 03. Draft Validation And Append
// ============================================================
#pragma region 03_Draft_Validation_And_Append

    // 생성된 메모리의 내용과 용량을 검사한 뒤 준비 중인 Snapshot에만 추가한다.

    bool FWAINMemoryReasoningPreparer::ValidateDraftContent(const FString &Content, FString &OutError)
    {
        if (Content.TrimStartAndEnd().IsEmpty() || Content.Len() > 4096)
        {
            OutError = TEXT("생성된 Memory 내용이 비었거나 4096자를 초과했습니다.");
            return false;
        }
        return true;
    }

    bool FWAINMemoryReasoningPreparer::AppendDerivedMemory(FWAINMemoryRecord &&Record,
                                                    FWAINStateSnapshot &Prepared,
                                                    FString &OutError)
    {
        if (Prepared.MemoryStream.Num() >= 2000)
        {
            OutError = TEXT("Memory Stream의 저장 한도를 초과했습니다.");
            return false;
        }
        Prepared.MemoryStream.Add(MoveTemp(Record));
        return true;
    }

#pragma endregion
