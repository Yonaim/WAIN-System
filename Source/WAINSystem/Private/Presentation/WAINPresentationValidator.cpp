/**
 * @file WAINPresentationValidator.cpp
 * @brief 후보 ID·설명을 확인하고 선택된 표정이 정확히 허용 목록에 포함되는지 검사한다.
 */

#include "Presentation/WAINPresentationValidator.h"

// ============================================================
// 00. Host Catalog Validation
// ============================================================
#pragma region 00_Host_Catalog_Validation

    bool FWAINPresentationValidator::ValidateCatalog(const FWAINPresentationOptions &Options, FString &OutError)
    {
        for (int32 Index = 0; Index < Options.Entries.Num(); ++Index)
        {
            const FWAINPresentationEntry &Entry = Options.Entries[Index];
            if (Entry.ExpressionId.IsNone() || Entry.Description.TrimStartAndEnd().IsEmpty())
            {
                OutError = TEXT("Host Catalog에 ExpressionId가 없거나 설명이 빈 후보가 있습니다.");
                return false;
            }

            for (int32 Previous = 0; Previous < Index; ++Previous)
            {
                const FWAINPresentationEntry &Other = Options.Entries[Previous];
                if (Other.ExpressionId == Entry.ExpressionId && Other.VariantId == Entry.VariantId)
                {
                    OutError = TEXT("Host Catalog에 중복된 Expression/Variant 조합이 있습니다.");
                    return false;
                }
            }
        }

        return true;
    }

#pragma endregion

// ============================================================
// 01. Proposal Selection Validation
// ============================================================
#pragma region 01_Proposal_Selection_Validation

    bool FWAINPresentationValidator::ValidateSelection(const FWAINResponseProposal    &Proposal,
                                                       const FWAINPresentationOptions *Options,
                                                       FString                        &OutError)
    {
        for (const FWAINPresentationStep &Step : Proposal.PresentationSteps)
        {
            if (Step.ExpressionId.IsNone())
            {
                if (!Step.ExpressionVariantId.IsNone())
                {
                    OutError = TEXT("ExpressionId 없이 VariantId를 사용할 수 없습니다.");
                    return false;
                }
                continue;
            }
            if (!Options)
            {
                OutError = TEXT("Presentation Catalog가 없습니다.");
                return false;
            }
            bool bFound = false;
            for (const FWAINPresentationEntry &Entry : Options->Entries)
            {
                if (Entry.ExpressionId == Step.ExpressionId && Entry.VariantId == Step.ExpressionVariantId)
                {
                    bFound = true;
                    break;
                }
            }
            if (!bFound)
            {
                OutError = TEXT("PresentationStep의 Expression/Variant 조합이 Catalog에 없습니다.");
                return false;
            }
        }
        return true;
    }

#pragma endregion
