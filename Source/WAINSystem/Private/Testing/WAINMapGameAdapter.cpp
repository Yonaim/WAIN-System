/**
 * @file WAINMapGameAdapter.cpp
 * @brief 권위 State를 수정하지 않는 Testbed 후보용 Map Snapshot 준비를 구현한다.
 */

#include "Testing/WAINMapGameAdapter.h"

// ============================================================
// 00. Snapshot Identity
// ============================================================
#pragma region 00_Snapshot_Identity

    FName FWAINMapGameSnapshot::GetKind() const
    {
        static const FName Kind(TEXT("WAIN.MapGameSnapshot"));
        return Kind;
    }

#pragma endregion

// ============================================================
// 01. Snapshot Preparation
// ============================================================
#pragma region 01_Snapshot_Preparation

    FWAINMapGameAdapter::FWAINMapGameAdapter(TMap<FName, int32> InInitialValues)
        : InitialValues(MoveTemp(InInitialValues))
    {
    }

    TSharedPtr<const IWAINGameStateSnapshot> FWAINMapGameAdapter::CreateInitialSnapshot(FString &OutError) const
    {
        TSharedRef<FWAINMapGameSnapshot> Snapshot = MakeShared<FWAINMapGameSnapshot>();
        Snapshot->Values = InitialValues;
        return Snapshot;
    }

    TSharedPtr<const IWAINGameStateSnapshot> FWAINMapGameAdapter::Prepare(const IWAINGameStateSnapshot     &Current,
                                                                          const TArray<FWAINStateMutation> &GameMutations,
                                                                          FString                          &OutError) const
    {
        if (Current.GetKind() != FWAINMapGameSnapshot().GetKind())
        {
            OutError = TEXT("현재 Game Snapshot의 타입을 지원하지 않습니다.");
            return nullptr;
        }

        const FWAINMapGameSnapshot      &Original = static_cast<const FWAINMapGameSnapshot &>(Current);
        TSharedRef<FWAINMapGameSnapshot> Prepared = MakeShared<FWAINMapGameSnapshot>();
        Prepared->Values = Original.Values;

        for (const FWAINStateMutation &Mutation : GameMutations)
        {
            if (Mutation.Owner != EWAINMutationOwner::Game || (Mutation.Operation != EWAINMutationOperation::AddInteger &&
                                                               Mutation.Operation != EWAINMutationOperation::SetInteger))
            {
                OutError = TEXT("지원하지 않는 Game Mutation입니다.");
                return nullptr;
            }

            int32 *CurrentValue = Prepared->Values.Find(Mutation.TargetId);
            if (!CurrentValue)
            {
                OutError = TEXT("미등록 Game State Target은 변경할 수 없습니다.");
                return nullptr;
            }

            const int64 NextValue = Mutation.Operation == EWAINMutationOperation::AddInteger
                                        ? static_cast<int64>(*CurrentValue) + Mutation.IntegerValue
                                        : static_cast<int64>(Mutation.IntegerValue);
            if (NextValue < 0 || NextValue > MAX_int32)
            {
                OutError = TEXT("Game State 값은 음수 또는 int32 범위 밖일 수 없습니다.");
                return nullptr;
            }

            *CurrentValue = static_cast<int32>(NextValue);
        }

        return Prepared;
    }

#pragma endregion

// ============================================================
// 02. Prepared State Validation
// ============================================================
#pragma region 02_Prepared_State_Validation

    bool FWAINMapGameAdapter::Validate(const IWAINGameStateSnapshot &Prepared, FString &OutError) const
    {
        if (Prepared.GetKind() != FWAINMapGameSnapshot().GetKind())
        {
            OutError = TEXT("Game Snapshot 타입이 일치하지 않습니다.");
            return false;
        }

        const FWAINMapGameSnapshot &GameState = static_cast<const FWAINMapGameSnapshot &>(Prepared);
        for (const TPair<FName, int32> &Pair : GameState.Values)
        {
            if (Pair.Value < 0)
            {
                OutError = TEXT("Game State에 음수 값이 있습니다.");
                return false;
            }
        }

        return true;
    }

#pragma endregion
