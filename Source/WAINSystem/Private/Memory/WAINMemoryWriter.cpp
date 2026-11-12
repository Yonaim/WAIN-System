/**
 * @file WAINMemoryWriter.cpp
 * @brief 검증된 입력과 대사를 실제 State 변경 없이 Prepared Memory Stream에 기록한다.
 */

#include "Memory/WAINMemoryWriter.h"

// ============================================================
// 00. Transactional Observation Preparation
// ============================================================
#pragma region 00_Transactional_Observation_Preparation

    bool FWAINMemoryWriter::PrepareObservations(const FWAINInteractionRequest &Request,
                                                const FWAINResponseProposal &Proposal,
                                                FWAINStateSnapshot &Prepared,
                                                FString &OutError) const
    {
        if (Request.InputKind == EWAINInputKind::Text &&
            !AppendObservation(Request.CharacterId, EWAINMemorySpeaker::Player, Request.Text,
                               Prepared.TurnCount, Prepared, OutError))
        {
            return false;
        }

        return AppendObservation(Request.CharacterId, EWAINMemorySpeaker::Character, Proposal.Dialogue,
                                 Prepared.TurnCount, Prepared, OutError);
    }

#pragma endregion

// ============================================================
// 01. Observation Construction
// ============================================================
#pragma region 01_Observation_Construction

    bool FWAINMemoryWriter::AppendObservation(const FName CharacterId,
                                              const EWAINMemorySpeaker Speaker,
                                              const FString &Content,
                                              const int32 CreatedTurn,
                                              FWAINStateSnapshot &Prepared,
                                              FString &OutError)
    {
        // MVP는 긴 입력의 잘림과 무제한 Memory 증가를 암묵적으로 허용하지 않는다.
        if (Content.TrimStartAndEnd().IsEmpty() || Content.Len() > 4096 || Prepared.MemoryStream.Num() >= 2000)
        {
            OutError = TEXT("Observation 내용이 비었거나 길이/저장 개수 제한을 초과했습니다.");
            return false;
        }

        FWAINMemoryRecord Observation;
        Observation.Id = FGuid::NewGuid();
        Observation.CharacterId = CharacterId;
        Observation.Type = EWAINMemoryType::Observation;
        Observation.Speaker = Speaker;
        Observation.Content = Content;
        Observation.CreatedTurn = CreatedTurn;
        Prepared.MemoryStream.Add(MoveTemp(Observation));
        return true;
    }

#pragma endregion
