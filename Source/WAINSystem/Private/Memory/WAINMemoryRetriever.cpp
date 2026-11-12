/**
 * @file WAINMemoryRetriever.cpp
 * @brief 모델 연동 전 임시 어휘 일치·최근성·중요도 검색을 수행한다.
 */

#include "Memory/WAINMemoryRetriever.h"

// ============================================================
// 00. Read-Only Retrieval Pipeline
// ============================================================
#pragma region 00_Read_Only_Retrieval_Pipeline

    TArray<FWAINMemoryRecord> FWAINMemoryRetriever::Retrieve(const FWAINInteractionRequest &Request,
                                                              const FWAINStateSnapshot &Current) const
    {
        const TArray<FRankedMemory> Ranked = RankCandidates(Request, Current);
        return SelectTopObservations(Ranked, Current);
    }

#pragma endregion

// ============================================================
// 01. Candidate Ranking And Selection
// ============================================================
#pragma region 01_Candidate_Ranking_And_Selection

    TArray<FWAINMemoryRetriever::FRankedMemory>
    FWAINMemoryRetriever::RankCandidates(const FWAINInteractionRequest &Request,
                                          const FWAINStateSnapshot &Current)
    {
        TArray<FRankedMemory> Ranked;
        for (int32 Index = 0; Index < Current.MemoryStream.Num(); ++Index)
        {
            const FWAINMemoryRecord &Memory = Current.MemoryStream[Index];
            if (Memory.CharacterId == Request.CharacterId && Memory.Type == EWAINMemoryType::Observation)
            {
                Ranked.Add({Index, CalculateScore(Request, Memory, Current.TurnCount)});
            }
        }

        Ranked.Sort([](const FRankedMemory &Left, const FRankedMemory &Right)
        {
            if (Left.Score != Right.Score)
            {
                return Left.Score > Right.Score;
            }
            return Left.Index > Right.Index;
        });
        return Ranked;
    }

    TArray<FWAINMemoryRecord> FWAINMemoryRetriever::SelectTopObservations(const TArray<FRankedMemory> &Ranked,
                                                                          const FWAINStateSnapshot &Current)
    {
        TArray<FWAINMemoryRecord> Results;
        const int32 Count = FMath::Min(Ranked.Num(), 6);
        Results.Reserve(Count);
        for (int32 Index = 0; Index < Count; ++Index)
        {
            Results.Add(Current.MemoryStream[Ranked[Index].Index]);
        }
        return Results;
    }

#pragma endregion

// ============================================================
// 02. Temporary Lexical Scoring
// ============================================================
#pragma region 02_Temporary_Lexical_Scoring

    float FWAINMemoryRetriever::CalculateScore(const FWAINInteractionRequest &Request,
                                                const FWAINMemoryRecord &Memory,
                                                const int32 CurrentTurn)
    {
        const float Relevance = CalculateRelevance(Request.Text, Memory.Content);
        const float Recency = 1.0F / (1.0F + FMath::Max(0, CurrentTurn - Memory.CreatedTurn) / 8.0F);
        return 0.60F * Relevance + 0.25F * Recency + 0.15F * Memory.Importance;
    }

    float FWAINMemoryRetriever::CalculateRelevance(const FString &Query, const FString &Content)
    {
        TArray<FString> Terms;
        Query.ParseIntoArrayWS(Terms);
        if (Terms.IsEmpty())
        {
            return 0.0F;
        }

        int32 Matches = 0;
        for (const FString &Term : Terms)
        {
            if (Content.Contains(Term, ESearchCase::IgnoreCase))
            {
                ++Matches;
            }
        }
        return static_cast<float>(Matches) / static_cast<float>(Terms.Num());
    }

#pragma endregion
