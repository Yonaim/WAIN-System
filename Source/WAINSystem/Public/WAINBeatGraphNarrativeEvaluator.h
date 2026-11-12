/**
 * @file WAINBeatGraphNarrativeEvaluator.h
 * @brief Host가 제공한 Beat Graph를 Prepared Snapshot에 대해 결정론적으로 평가한다.
 */

#pragma once

#include "WAINInterfaces.h"
#include "WAINNarrativeTypes.h"

// ============================================================
// 00. Beat Graph Evaluator
// ============================================================
#pragma region 00_Beat_Graph_Evaluator

    // State 조회·조건 검사·전이를 단일 평가기로 구성한다.

    /** @brief Graph 정의 데이터는 생성 후 변경하지 않는다는 MVP 계약을 가진다. */
    class FWAINBeatGraphNarrativeEvaluator final : public IWAINNarrativeEvaluator
    {
        public:
        FWAINBeatGraphNarrativeEvaluator(FWAINNarrativeGraph InGraph,
                                        TSharedPtr<const IWAINNarrativeGameFactReader> InGameFacts = nullptr);

        virtual bool ValidateInitialState(const FWAINStateSnapshot &InitialWAIN,
                                          const IWAINGameStateSnapshot &InitialGame,
                                          FString &OutError) const override;

        virtual bool BuildContext(const FWAINSessionRoot &Current,
                                  FWAINNarrativeContext &OutContext,
                                  FString &OutError) const override;

        virtual bool Evaluate(const FWAINInteractionRequest &Request,
                              const FWAINSessionRoot &Current,
                              FWAINStateSnapshot &PreparedWAIN,
                              const IWAINGameStateSnapshot &PreparedGame,
                              FString &OutError) const override;

        private:
        bool ValidateGraph(FString &OutError) const;
        bool ValidateChapters(FString &OutError) const;
        bool ValidateSequences(FString &OutError) const;
        bool ValidateBeatsAndTransitions(FString &OutError) const;
        bool ValidateConditions(const TArray<FWAINNarrativeCondition> &Conditions,
                                FString &OutError) const;
        bool ValidateHardRules(const TArray<FWAINNarrativeHardRule> &Rules,
                               FString &OutError) const;
        bool EnforceHardRules(const FWAINInteractionRequest &Request,
                              const FWAINSessionRoot &Current,
                              const FWAINStateSnapshot &PreparedWAIN,
                              const IWAINGameStateSnapshot &PreparedGame,
                              const FWAINNarrativeBeat &Beat,
                              FString &OutError) const;
        bool EvaluateConditions(const TArray<FWAINNarrativeCondition> &Conditions,
                                const FWAINInteractionRequest &Request,
                                const FWAINStateSnapshot &PreparedWAIN,
                                const IWAINGameStateSnapshot &PreparedGame,
                                bool &OutSatisfied,
                                FString &OutError) const;
        bool ReadConditionValue(const FWAINNarrativeCondition &Condition,
                                const FWAINInteractionRequest &Request,
                                const FWAINStateSnapshot &PreparedWAIN,
                                const IWAINGameStateSnapshot &PreparedGame,
                                int32 &OutValue,
                                FString &OutError) const;
        bool SelectNextBeat(const FWAINNarrativeBeat &Beat,
                            const FWAINInteractionRequest &Request,
                            const FWAINStateSnapshot &PreparedWAIN,
                            const IWAINGameStateSnapshot &PreparedGame,
                            FName &OutNextBeat,
                            FString &OutError) const;

        const FWAINNarrativeChapter *FindChapter(FName ChapterId) const;
        const FWAINNarrativeSequence *FindSequence(FName SequenceId) const;
        const FWAINNarrativeBeat *FindBeat(FName BeatId) const;

        FWAINNarrativeGraph Graph;
        TSharedPtr<const IWAINNarrativeGameFactReader> GameFacts;
    };

#pragma endregion
