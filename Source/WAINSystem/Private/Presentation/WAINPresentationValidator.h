/**
 * @file WAINPresentationValidator.h
 * @brief Host Expression 후보 목록과 Proposal의 Expression/Variant 조합을 검증한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINPresentationCatalog.h"
#include "WAINRuntimeTypes.h"

class FWAINPresentationValidator final
{
  public:
    static bool ValidateCatalog(const FWAINPresentationOptions &Options, FString &OutError);
    static bool ValidateSelection(const FWAINResponseProposal    &Proposal,
                                  const FWAINPresentationOptions *Options,
                                  FString                        &OutError);
};
