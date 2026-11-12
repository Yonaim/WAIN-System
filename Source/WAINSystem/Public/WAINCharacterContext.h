/**
 * @file WAINCharacterContext.h
 * @brief Host 소유 Persona와 WAIN 확정 Character State를 함께 읽기 위한 Context 계약을 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINRuntimeTypes.h"
#include "WAINPresentationCatalog.h"

// ============================================================
// 00. Persona Definition
// ============================================================
#pragma region 00_Persona_Definition

    /** @brief Host가 제공하는 고정 캐릭터 정의. 런타임 State나 공개 불가 서사 정보는 포함하지 않는다. */
    struct FWAINCharacterPersona
    {
        FName CharacterId;
        FString DisplayName;
        TArray<FString> Traits;
        FString SpeechStyle;
        FString Background;
        TArray<FString> Preferences;
    };

    /** @brief Persona 정의의 정본은 Host에 있으며, WAIN은 조회만 수행한다. */
    class IWAINPersonaSource
    {
        public:
        virtual ~IWAINPersonaSource() = default;

        virtual bool GetPersona(FName CharacterId, FWAINCharacterPersona &OutPersona, FString &OutError) const = 0;
    };

#pragma endregion

// ============================================================
// 01. Read-Only Character Context
// ============================================================
#pragma region 01_Read_Only_Character_Context

    /** @brief 제안 생성에 사용할 현재 턴의 읽기 전용 Character 문맥. Memory·World 문맥은 추후 추가한다. Presentation 후보는 Host에서 조회한다. */
    struct FWAINCharacterContext
    {
        FName CharacterId;
        FName SceneId;
        FName ActiveBeatId;
        uint64 StateRevision = 0;
        int32 TurnCount = 0;
        FWAINCharacterState State;
        bool bHasPersona = false;
        FWAINCharacterPersona Persona;
        bool bHasPresentationCatalog = false;
        FWAINPresentationOptions AvailablePresentations;
    };

#pragma endregion
