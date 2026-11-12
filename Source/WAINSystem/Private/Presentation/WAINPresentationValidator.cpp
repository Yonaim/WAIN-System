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
        if (Proposal.ExpressionId.IsNone())
        {
            if (!Proposal.ExpressionVariantId.IsNone())
            {
                OutError = TEXT("ExpressionId 없이 VariantId만 선택할 수 없습니다.");
                return false;
            }

            return true; // 표정 선택은 선택 사항이며, Host의 기본 표정 정책을 임의로 가정하지 않는다.
        }

        if (!Options)
        {
            OutError = TEXT("Host Presentation Catalog가 없어 Expression ID를 검증할 수 없습니다.");
            return false;
        }

        for (const FWAINPresentationEntry &Entry : Options->Entries)
        {
            if (Entry.ExpressionId == Proposal.ExpressionId && Entry.VariantId == Proposal.ExpressionVariantId)
            {
                return true;
            }
        }

        OutError = TEXT("Proposal의 Expression/Variant 조합이 이번 턴의 허용 목록에 없습니다.");
        return false;
    }

#pragma endregion
