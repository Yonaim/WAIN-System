/**
 * @file WAINCharacterContextBuilder.cpp
 * @brief Host Persona를 검증하고 현재 Character Snapshot을 복사하여 제안 생성 Context를 만든다.
 */

#include "Character/WAINCharacterContextBuilder.h"

// ============================================================
// 00. Context Assembly
// ============================================================
#pragma region 00_Context_Assembly

    bool FWAINCharacterContextBuilder::Build(const FWAINInteractionRequest &Request,
                                             const FWAINSessionRoot &Current,
                                             const IWAINPersonaSource *PersonaSource,
                                             FWAINCharacterContext &OutContext,
                                             FString &OutError) const
    {
        const FWAINCharacterState *Character = Current.WAIN.Characters.Find(Request.CharacterId);
        if (!Character)
        {
            OutError = TEXT("현재 Snapshot에 CharacterId가 없습니다.");
            return false;
        }

        FWAINCharacterContext NextContext;
        NextContext.CharacterId = Request.CharacterId;
        NextContext.SceneId = Request.SceneId;
        NextContext.ActiveBeatId = Current.WAIN.ActiveBeatId;
        NextContext.StateRevision = Current.Revision;
        NextContext.TurnCount = Current.WAIN.TurnCount;
        NextContext.State = *Character;

        if (!LoadPersona(Request.CharacterId, PersonaSource, NextContext, OutError))
        {
            return false;
        }

        OutContext = MoveTemp(NextContext);
        return true;
    }

#pragma endregion

// ============================================================
// 01. Host Persona Lookup And Validation
// ============================================================
#pragma region 01_Host_Persona_Lookup_And_Validation

    bool FWAINCharacterContextBuilder::LoadPersona(FName CharacterId,
                                                   const IWAINPersonaSource *PersonaSource,
                                                   FWAINCharacterContext &Context,
                                                   FString &OutError) const
    {
        // Phase 1의 Persona 미주입 Mock 경로를 유지한다. 주입된 Source는 실패 시 조용히 무시하지 않는다.
        if (!PersonaSource)
        {
            return true;
        }

        FWAINCharacterPersona Persona;
        if (!PersonaSource->GetPersona(CharacterId, Persona, OutError))
        {
            if (OutError.IsEmpty())
            {
                OutError = TEXT("Host Persona를 조회하지 못했습니다.");
            }
            return false;
        }

        if (!ValidatePersona(Persona, CharacterId, OutError))
        {
            return false;
        }

        Context.Persona = MoveTemp(Persona);
        Context.bHasPersona = true;
        return true;
    }

    bool FWAINCharacterContextBuilder::ValidatePersona(const FWAINCharacterPersona &Persona,
                                                        const FName ExpectedCharacterId,
                                                        FString &OutError)
    {
        if (Persona.CharacterId != ExpectedCharacterId || Persona.DisplayName.TrimStartAndEnd().IsEmpty())
        {
            OutError = TEXT("Persona CharacterId 또는 DisplayName이 올바르지 않습니다.");
            return false;
        }

        for (const FString &Trait : Persona.Traits)
        {
            if (Trait.TrimStartAndEnd().IsEmpty())
            {
                OutError = TEXT("Persona에 빈 Trait가 있습니다.");
                return false;
            }
        }

        for (const FString &Preference : Persona.Preferences)
        {
            if (Preference.TrimStartAndEnd().IsEmpty())
            {
                OutError = TEXT("Persona에 빈 Preference가 있습니다.");
                return false;
            }
        }

        return true;
    }

#pragma endregion
