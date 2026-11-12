/**
 * @file WAINCharacterContextBuilder.h
 * @brief 요청과 확정 Snapshot으로부터 읽기 전용 Character Context를 구성한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINCharacterContext.h"
#include "WAINInterfaces.h"

class FWAINCharacterContextBuilder final
{
  public:
    bool Build(const FWAINInteractionRequest &Request,
               const FWAINSessionRoot &Current,
               const IWAINPersonaSource *PersonaSource,
               const IWAINPresentationCatalog *PresentationCatalog,
               FWAINCharacterContext &OutContext,
               FString &OutError) const;

  private:
    bool LoadPersona(FName CharacterId,
                     const IWAINPersonaSource *PersonaSource,
                     FWAINCharacterContext &Context,
                     FString &OutError) const;

    bool LoadPresentations(const FWAINPresentationQuery &Query,
                           const IWAINPresentationCatalog *PresentationCatalog,
                           FWAINCharacterContext &Context,
                           FString &OutError) const;

    static bool ValidatePersona(const FWAINCharacterPersona &Persona, FName ExpectedCharacterId, FString &OutError);
};
