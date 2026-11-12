/**
 * @file WAINBeatGraphNarrativeEvaluator.cpp
 * @brief Prepared State에서 Goal 완료·Hard Rule·우선순위 전이를 결정론적으로 평가한다.
 */

#include "WAINBeatGraphNarrativeEvaluator.h"

// ============================================================
// 00. Initialization And Read-Only Guidance
// ============================================================
#pragma region 00_Initialization_And_Read_Only_Guidance

    // Graph 초기 검증과 현재 활성 경로의 읽기 전용 Context를 생성한다.

    FWAINBeatGraphNarrativeEvaluator::FWAINBeatGraphNarrativeEvaluator(
        FWAINNarrativeGraph                            InGraph,
        TSharedPtr<const IWAINNarrativeGameFactReader> InGameFacts)
        : Graph(MoveTemp(InGraph)),
          GameFacts(MoveTemp(InGameFacts))
    {
    }

    bool FWAINBeatGraphNarrativeEvaluator::ValidateInitialState(const FWAINStateSnapshot     &InitialWAIN,
                                                                const IWAINGameStateSnapshot &InitialGame,
                                                                FString                      &OutError) const
    {
        if (!ValidateGraph(OutError))
        {
            return false;
        }

        if (!FindBeat(InitialWAIN.ActiveBeatId))
        {
            OutError = TEXT("초기 ActiveBeat가 Narrative Graph에 없습니다.");
            return false;
        }

        return true;
    }

    bool FWAINBeatGraphNarrativeEvaluator::BuildContext(const FWAINSessionRoot &Current,
                                                        FWAINNarrativeContext  &OutContext,
                                                        FString                &OutError) const
    {
        const FWAINNarrativeBeat     *Beat = FindBeat(Current.WAIN.ActiveBeatId);
        const FWAINNarrativeSequence *Sequence = Beat ? FindSequence(Beat->SequenceId) : nullptr;
        const FWAINNarrativeChapter  *Chapter = Sequence ? FindChapter(Sequence->ChapterId) : nullptr;
        if (!Beat || !Sequence || !Chapter)
        {
            OutError = TEXT("현재 Beat 또는 상위 Narrative 정의를 찾을 수 없습니다.");
            return false;
        }

        FWAINNarrativeContext Context;
        Context.ChapterId = Chapter->Id;
        Context.SequenceId = Sequence->Id;
        Context.BeatId = Beat->Id;
        Context.ChapterGoal = Chapter->Goal;
        Context.SequenceGoal = Sequence->Goal;
        Context.BeatGoal = Beat->Goal;
        Context.SoftConstraints.Append(Chapter->SoftConstraints);
        Context.SoftConstraints.Append(Sequence->SoftConstraints);
        Context.SoftConstraints.Append(Beat->SoftConstraints);
        OutContext = MoveTemp(Context);
        return true;
    }

#pragma endregion

// ============================================================
// 01. Prepared Beat Evaluation
// ============================================================
#pragma region 01_Prepared_Beat_Evaluation

    // Hard Rule, Completion, Transition을 순서대로 평가한다.

    // Hard Rule → Completion → Transition을 평가하며 확정 State는 수정하지 않는다.

    bool FWAINBeatGraphNarrativeEvaluator::Evaluate(const FWAINInteractionRequest &Request,
                                                    const FWAINSessionRoot        &Current,
                                                    FWAINStateSnapshot            &PreparedWAIN,
                                                    const IWAINGameStateSnapshot  &PreparedGame,
                                                    FString                       &OutError) const
    {
        const FWAINNarrativeBeat *Beat = FindBeat(Current.WAIN.ActiveBeatId);
        if (!Beat)
        {
            OutError = TEXT("현재 ActiveBeat가 Narrative Graph에 존재하지 않습니다.");
            return false;
        }

        if (!EnforceHardRules(Request, Current, PreparedWAIN, PreparedGame, *Beat, OutError))
        {
            return false;
        }

        // Completion 조건이 비어 있는 Beat는 명시적 완료 판정 전까지 유지한다.
        if (Beat->CompletionConditions.IsEmpty())
        {
            return true;
        }

        bool bCompleted = false;
        if (!EvaluateConditions(Beat->CompletionConditions, Request, PreparedWAIN, PreparedGame, bCompleted, OutError))
        {
            return false;
        }
        if (!bCompleted)
        {
            return true;
        }

        FName NextBeat;
        if (!SelectNextBeat(*Beat, Request, PreparedWAIN, PreparedGame, NextBeat, OutError))
        {
            return false;
        }
        if (NextBeat.IsNone())
        {
            return true;
        }

        FWAINBeatTransitionRecord History;
        History.FromBeatId = Beat->Id;
        History.ToBeatId = NextBeat;
        History.Turn = PreparedWAIN.TurnCount;
        PreparedWAIN.BeatHistory.Add(History);
        PreparedWAIN.ActiveBeatId = NextBeat;
        return true;
    }

    bool FWAINBeatGraphNarrativeEvaluator::SelectNextBeat(const FWAINNarrativeBeat      &Beat,
                                                          const FWAINInteractionRequest &Request,
                                                          const FWAINStateSnapshot      &PreparedWAIN,
                                                          const IWAINGameStateSnapshot  &PreparedGame,
                                                          FName                         &OutNextBeat,
                                                          FString                       &OutError) const
    {
        bool  bHasCandidate = false;
        int32 BestPriority = MIN_int32;
        for (const FWAINBeatTransition &Transition : Beat.Transitions)
        {
            bool bSatisfied = false;
            if (!EvaluateConditions(Transition.Conditions, Request, PreparedWAIN, PreparedGame, bSatisfied, OutError))
            {
                return false;
            }
            if (!bSatisfied)
            {
                continue;
            }

            if (!bHasCandidate || Transition.Priority > BestPriority)
            {
                bHasCandidate = true;
                BestPriority = Transition.Priority;
                OutNextBeat = Transition.TargetBeatId;
            }
            else if (Transition.Priority == BestPriority)
            {
                OutError = TEXT("동일 우선순위의 Beat 전이 조건이 동시에 충족되었습니다.");
                return false;
            }
        }
        return true;
    }

#pragma endregion

// ============================================================
// 02. Inherited Hard Rules
// ============================================================
#pragma region 02_Inherited_Hard_Rules

    // Chapter·Sequence·Beat의 Hard Rule을 누적 적용한다.

    bool FWAINBeatGraphNarrativeEvaluator::EnforceHardRules(const FWAINInteractionRequest &Request,
                                                            const FWAINSessionRoot        &Current,
                                                            const FWAINStateSnapshot      &PreparedWAIN,
                                                            const IWAINGameStateSnapshot  &PreparedGame,
                                                            const FWAINNarrativeBeat      &Beat,
                                                            FString                       &OutError) const
    {
        const FWAINNarrativeSequence *Sequence = FindSequence(Beat.SequenceId);
        const FWAINNarrativeChapter  *Chapter = Sequence ? FindChapter(Sequence->ChapterId) : nullptr;
        if (!Sequence || !Chapter || !Current.Game.IsValid())
        {
            OutError = TEXT("Narrative 부모 관계 또는 기존 Game Snapshot이 없습니다.");
            return false;
        }

        const TArray<FWAINNarrativeHardRule> *RuleSets[] = {&Chapter->HardRules, &Sequence->HardRules, &Beat.HardRules};
        for (const TArray<FWAINNarrativeHardRule> *Rules : RuleSets)
        {
            for (const FWAINNarrativeHardRule &Rule : *Rules)
            {
                int32 Previous = 0;
                int32 Next = 0;
                if (!GameFacts.IsValid() || !GameFacts->ReadInteger(*Current.Game, Rule.GuardedGameTargetId, Previous) ||
                    !GameFacts->ReadInteger(PreparedGame, Rule.GuardedGameTargetId, Next))
                {
                    OutError = TEXT("Hard Rule 평가에 필요한 Game Fact를 읽지 못했습니다.");
                    return false;
                }
                if (Next <= Previous)
                {
                    continue;
                }

                bool bAllowed = false;
                if (!EvaluateConditions(Rule.Prerequisites, Request, PreparedWAIN, PreparedGame, bAllowed, OutError))
                {
                    return false;
                }
                if (!bAllowed)
                {
                    OutError = TEXT("Narrative Hard Rule의 선행 조건을 만족하지 못했습니다.");
                    return false;
                }
            }
        }
        return true;
    }

#pragma endregion

// ============================================================
// 03. Structured Condition Evaluation
// ============================================================
#pragma region 03_Structured_Condition_Evaluation

    // Game/Character Fact를 읽고 비교 연산을 평가한다.

    bool FWAINBeatGraphNarrativeEvaluator::EvaluateConditions(const TArray<FWAINNarrativeCondition> &Conditions,
                                                              const FWAINInteractionRequest         &Request,
                                                              const FWAINStateSnapshot              &PreparedWAIN,
                                                              const IWAINGameStateSnapshot          &PreparedGame,
                                                              bool                                  &OutSatisfied,
                                                              FString                               &OutError) const
    {
        OutSatisfied = true;
        for (const FWAINNarrativeCondition &Condition : Conditions)
        {
            int32 Actual = 0;
            if (!ReadConditionValue(Condition, Request, PreparedWAIN, PreparedGame, Actual, OutError))
            {
                return false;
            }

            bool bMatches = false;
            switch (Condition.Comparison)
            {
                case EWAINNarrativeComparison::Equal:
                bMatches = Actual == Condition.Value;
                break;
                case EWAINNarrativeComparison::AtLeast:
                bMatches = Actual >= Condition.Value;
                break;
                case EWAINNarrativeComparison::AtMost:
                bMatches = Actual <= Condition.Value;
                break;
                default:
                OutError = TEXT("지원하지 않는 Narrative 비교 연산입니다.");
                return false;
            }
            if (!bMatches)
            {
                OutSatisfied = false;
                return true;
            }
        }
        return true;
    }

    bool FWAINBeatGraphNarrativeEvaluator::ReadConditionValue(const FWAINNarrativeCondition &Condition,
                                                              const FWAINInteractionRequest &Request,
                                                              const FWAINStateSnapshot      &PreparedWAIN,
                                                              const IWAINGameStateSnapshot  &PreparedGame,
                                                              int32                         &OutValue,
                                                              FString                       &OutError) const
    {
        switch (Condition.Source)
        {
            case EWAINNarrativeFact::CharacterTrust:
            {
                const FWAINCharacterState *Character = PreparedWAIN.Characters.Find(Request.CharacterId);
                if (Character)
                {
                    OutValue = Character->Trust;
                    return true;
                }
                break;
            }
            case EWAINNarrativeFact::TurnCount:
            OutValue = PreparedWAIN.TurnCount;
            return true;
            case EWAINNarrativeFact::GameInteger:
            if (GameFacts.IsValid() && GameFacts->ReadInteger(PreparedGame, Condition.TargetId, OutValue))
            {
                return true;
            }
            break;
            default:
            break;
        }

        OutError = TEXT("Narrative Condition에 필요한 Prepared Fact를 읽지 못했습니다.");
        return false;
    }

#pragma endregion

// ============================================================
// 04. Graph Definition Validation
// ============================================================
#pragma region 04_Graph_Definition_Validation

    // Chapter·Sequence·Beat 참조와 규칙의 유효성을 검사한다.

    bool FWAINBeatGraphNarrativeEvaluator::ValidateGraph(FString &OutError) const
    {
        if (Graph.Chapters.IsEmpty() || Graph.Sequences.IsEmpty() || Graph.Beats.IsEmpty())
        {
            OutError = TEXT("Narrative Graph에 Chapter, Sequence, Beat가 필요합니다.");
            return false;
        }

        return ValidateChapters(OutError) && ValidateSequences(OutError) && ValidateBeatsAndTransitions(OutError);
    }

    bool FWAINBeatGraphNarrativeEvaluator::ValidateChapters(FString &OutError) const
    {
        TSet<FName> Ids;
        for (const FWAINNarrativeChapter &Chapter : Graph.Chapters)
        {
            if (Chapter.Id.IsNone() || Ids.Contains(Chapter.Id) || Chapter.Goal.TrimStartAndEnd().IsEmpty() ||
                !ValidateHardRules(Chapter.HardRules, OutError))
            {
                if (OutError.IsEmpty())
                    OutError = TEXT("Chapter 정의가 비었거나 중복되었습니다.");
                return false;
            }
            Ids.Add(Chapter.Id);
        }
        return true;
    }

    bool FWAINBeatGraphNarrativeEvaluator::ValidateSequences(FString &OutError) const
    {
        TSet<FName> Ids;
        for (const FWAINNarrativeSequence &Sequence : Graph.Sequences)
        {
            if (Sequence.Id.IsNone() || Ids.Contains(Sequence.Id) || !FindChapter(Sequence.ChapterId) ||
                Sequence.Goal.TrimStartAndEnd().IsEmpty() || !ValidateHardRules(Sequence.HardRules, OutError))
            {
                if (OutError.IsEmpty())
                    OutError = TEXT("Sequence의 ID 또는 부모 Chapter가 유효하지 않습니다.");
                return false;
            }
            Ids.Add(Sequence.Id);
        }
        return true;
    }

    bool FWAINBeatGraphNarrativeEvaluator::ValidateBeatsAndTransitions(FString &OutError) const
    {
        TSet<FName> Ids;
        for (const FWAINNarrativeBeat &Beat : Graph.Beats)
        {
            if (Beat.Id.IsNone() || Ids.Contains(Beat.Id) || !FindSequence(Beat.SequenceId) ||
                Beat.Goal.TrimStartAndEnd().IsEmpty() || !ValidateHardRules(Beat.HardRules, OutError) ||
                !ValidateConditions(Beat.CompletionConditions, OutError))
            {
                if (OutError.IsEmpty())
                    OutError = TEXT("Beat의 ID 또는 부모 Sequence가 유효하지 않습니다.");
                return false;
            }
            Ids.Add(Beat.Id);
        }

        for (const FWAINNarrativeBeat &Beat : Graph.Beats)
        {
            TSet<FName> Targets;
            for (const FWAINBeatTransition &Transition : Beat.Transitions)
            {
                if (!Ids.Contains(Transition.TargetBeatId) || Transition.TargetBeatId == Beat.Id ||
                    Targets.Contains(Transition.TargetBeatId) || !ValidateConditions(Transition.Conditions, OutError))
                {
                    if (OutError.IsEmpty())
                        OutError = TEXT("Beat 전이 대상 또는 조건이 유효하지 않습니다.");
                    return false;
                }
                Targets.Add(Transition.TargetBeatId);
            }
        }
        return true;
    }

    bool FWAINBeatGraphNarrativeEvaluator::ValidateConditions(const TArray<FWAINNarrativeCondition> &Conditions,
                                                              FString                               &OutError) const
    {
        for (const FWAINNarrativeCondition &Condition : Conditions)
        {
            if ((Condition.Source != EWAINNarrativeFact::CharacterTrust &&
                 Condition.Source != EWAINNarrativeFact::TurnCount &&
                 Condition.Source != EWAINNarrativeFact::GameInteger) ||
                (Condition.Source == EWAINNarrativeFact::GameInteger && Condition.TargetId.IsNone()) ||
                (Condition.Comparison != EWAINNarrativeComparison::Equal &&
                 Condition.Comparison != EWAINNarrativeComparison::AtLeast &&
                 Condition.Comparison != EWAINNarrativeComparison::AtMost))
            {
                OutError = TEXT("Narrative 조건의 Source/Target/비교 연산이 유효하지 않습니다.");
                return false;
            }
        }
        return true;
    }

    bool FWAINBeatGraphNarrativeEvaluator::ValidateHardRules(const TArray<FWAINNarrativeHardRule> &Rules,
                                                             FString                              &OutError) const
    {
        for (const FWAINNarrativeHardRule &Rule : Rules)
        {
            if (Rule.GuardedGameTargetId.IsNone() || Rule.Prerequisites.IsEmpty() ||
                !ValidateConditions(Rule.Prerequisites, OutError))
            {
                if (OutError.IsEmpty())
                    OutError = TEXT("Hard Rule에 대상 및 조건이 필요합니다.");
                return false;
            }
        }
        return true;
    }

#pragma endregion

// ============================================================
// 05. Graph Lookups
// ============================================================
#pragma region 05_Graph_Lookups

    // 데이터 정의를 ID로 탐색한다.

    const FWAINNarrativeChapter *FWAINBeatGraphNarrativeEvaluator::FindChapter(const FName ChapterId) const
    {
        return Graph.Chapters.FindByPredicate([ChapterId](const FWAINNarrativeChapter &Item)
                                              { return Item.Id == ChapterId; });
    }

    const FWAINNarrativeSequence *FWAINBeatGraphNarrativeEvaluator::FindSequence(const FName SequenceId) const
    {
        return Graph.Sequences.FindByPredicate([SequenceId](const FWAINNarrativeSequence &Item)
                                               { return Item.Id == SequenceId; });
    }

    const FWAINNarrativeBeat *FWAINBeatGraphNarrativeEvaluator::FindBeat(const FName BeatId) const
    {
        return Graph.Beats.FindByPredicate([BeatId](const FWAINNarrativeBeat &Item) { return Item.Id == BeatId; });
    }

#pragma endregion
