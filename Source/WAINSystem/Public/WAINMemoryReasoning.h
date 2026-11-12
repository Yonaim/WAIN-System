/**
 * @file WAINMemoryReasoning.h
 * @brief Reflection과 Plan의 문구 생성기를 Host 또는 AI Provider에서 주입하기 위한 계약을 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINRuntimeTypes.h"

// ============================================================
// 00. Plan Draft
// ============================================================
#pragma region 00_Plan_Draft

    /** @brief 서사 Goal은 현재 Beat를 기본 단위로 하며, 실제 단계 텍스트는 생성기가 제공한다. */
    struct FWAINPlanDraft
    {
        FString         Summary;
        TArray<FString> Steps;
    };

#pragma endregion

// ============================================================
// 01. Memory Reasoner Contract
// ============================================================
#pragma region 01_Memory_Reasoner_Contract

    /**
     * @brief Reflection/Plan 문구 생성만 수행한다. State 수정과 Memory ID 생성은 WAIN이 담당한다.
     *
     * 호출은 현재 동기 Mock 경로의 Game Thread에서 수행한다.
     * 실제 네트워크 LLM은 추후 비동기 파이프라인에서 호출해야 한다.
     */
    class IWAINMemoryReasoner
    {
        public:
        virtual ~IWAINMemoryReasoner() = default;

        virtual bool GenerateReflection(const FWAINInteractionRequest   &Request,
                                        const TArray<FWAINMemoryRecord> &Sources,
                                        FString                         &OutInsight,
                                        FString                         &OutError) const = 0;

        virtual bool GeneratePlan(const FWAINInteractionRequest   &Request,
                                  FName                            ActiveBeatId,
                                  const TArray<FWAINMemoryRecord> &Sources,
                                  FWAINPlanDraft                  &OutPlan,
                                  FString                         &OutError) const = 0;
    };

#pragma endregion
