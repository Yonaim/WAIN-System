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

    /** @brief 현재 확정된 Character State·Host 정의·검색된 Memory를 포함한 읽기 전용 Context. */
    struct FWAINCharacterContext
    {
        FName CharacterId;
        FName SceneId;
        FName ActiveBeatId;
        uint64 StateRevision = 0;
        int32 TurnCount = 0;
        FWAINCharacterState State;
        TArray<FWAINMemoryRecord> RelevantMemories;
        bool bHasPersona = false;
        FWAINCharacterPersona Persona;
        bool bHasPresentationCatalog = false;
        FWAINPresentationOptions AvailablePresentations;
    };

#pragma endregion
