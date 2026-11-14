/**
 * @file WAINDialogueValidator.h
 * @brief 모든 PresentationStep에 적용할 공통 대사 금지 표현 검증기를 선언한다.
 */
#pragma once
#include "WAINRelationshipTypes.h"
#include "WAINRuntimeTypes.h"

class FWAINDialogueValidator final
{
  public:
    static bool
    Validate(const FWAINResponseProposal &Proposal, const FWAINGlobalDialogueRules &Rules, FString &OutError);
};
