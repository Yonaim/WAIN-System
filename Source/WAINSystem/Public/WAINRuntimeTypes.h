/**
 * @file WAINRuntimeTypes.h
 * @brief 한 턴의 입력·제안·결과와 WAIN 내부 Snapshot의 최소 자료형을 선언한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINMemoryTypes.h"

// ============================================================
// 00. Interaction Enums
// ============================================================
#pragma region 00_Interaction_Enums

    /** @brief 사용자 입력 형태. Silence는 빈 Text와 구별되는 명시적인 턴이다. */
    enum class EWAINInputKind : uint8
    {
        Text,
        Silence
    };

    /** @brief 초기 MVP에서 실제로 지원하는 State Mutation 소유자. */
    enum class EWAINMutationOwner : uint8
    {
        Character,
        Game
    };

    enum class EWAINMutationOperation : uint8
    {
        AddInteger,
        SetInteger,
        AddFloat
    };

    /** @brief 아직 처리하지 않는 종류를 조용히 성공시키지 않기 위한 실패 코드. */
    enum class EWAINInteractionError : uint8
    {
        None,
        SessionNotStarted,
        Busy,
        InvalidRequest,
        DuplicateTurnConflict,
        ProposalGenerationFailed,
        ContextPreparationFailed,
        InvalidProposal,
        UnsupportedMutation,
        MemoryPreparationFailed,
        MemoryReasoningFailed,
        GamePrepareFailed,
        NarrativeEvaluationFailed,
        FinalValidationFailed,
        RevisionConflict
    };

#pragma endregion

// ============================================================
// 01. Request And Proposal
// ============================================================
#pragma region 01_Request_And_Proposal

    struct FWAINInteractionRequest
    {
        FGuid          TurnId;
        FName          CharacterId;
        FName          SceneId;
        EWAINInputKind InputKind = EWAINInputKind::Text;
        FString        Text;
    };

    /** @brief LLM을 포함한 외부 생성기의 출력은 State 명령이 아닌 검증 대상이다. */
    struct FWAINStateMutation
    {
        EWAINMutationOwner     Owner = EWAINMutationOwner::Character;
        FName                  TargetId;
        EWAINMutationOperation Operation = EWAINMutationOperation::AddInteger;
        int32                  IntegerValue = 0;
        float                  FloatValue = 0.0F;
    };

    struct FWAINResponseProposal
    {
        FString                    Dialogue;
        FName                      ActionId;
        FName                      ExpressionId;
        FName                      ExpressionVariantId;
        TArray<FWAINStateMutation> StateChanges;
    };

#pragma endregion

// ============================================================
// 02. WAIN State Snapshots
// ============================================================
#pragma region 02_WAIN_State_Snapshots

    /** @brief 감정 차원은 서로 독립적이며 각각 [0,1] 범위다. */
    struct FWAINEmotionState
    {
        float Joy = 0.0F;
        float Sadness = 0.0F;
        float Anger = 0.0F;
        float Disgust = 0.0F;
        float Anxiety = 0.0F;
        float Surprise = 0.0F;
    };

    struct FWAINCharacterState
    {
        int32             Trust = 0;
        FWAINEmotionState Emotion;
    };

    struct FWAINStateSnapshot
    {
        TMap<FName, FWAINCharacterState> Characters;
        TArray<FWAINMemoryRecord>         MemoryStream;
        TMap<FName, FGuid>                ActivePlanIds;
        FName                            ActiveBeatId;
        int32                            TurnCount = 0;
    };

#pragma endregion

// ============================================================
// 03. Interaction Result
// ============================================================
#pragma region 03_Interaction_Result

    struct FWAINInteractionResult
    {
        FGuid                 TurnId;
        EWAINInteractionError Error = EWAINInteractionError::None;
        FString               ErrorMessage;
        uint64                CommittedRevision = 0;
        FString               Dialogue;
        FName                 ExpressionId;
        FName                 ExpressionVariantId;
        bool                  bReplayed = false;

        bool Succeeded() const
        {
            return Error == EWAINInteractionError::None;
        }
    };

#pragma endregion
