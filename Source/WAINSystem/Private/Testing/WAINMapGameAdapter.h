/**
 * @file WAINMapGameAdapter.h
 * @brief 테스트용 Host-owned 정수 State를 준비하는 메모리 Snapshot Adapter를 제공한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINInterfaces.h"

// ============================================================
// 00. Mock Game Snapshot
// ============================================================
#pragma region 00_Mock_Game_Snapshot

    /** @brief 실제 게임 Inventory/Quest 저장소를 대신하는 테스트 전용 Snapshot. */
    struct FWAINMapGameSnapshot final : public IWAINGameStateSnapshot
    {
        TMap<FName, int32> Values;

        virtual FName GetKind() const override;
    };

#pragma endregion

// ============================================================
// 01. Map Game Adapter
// ============================================================
#pragma region 01_Map_Game_Adapter

    /** @brief 미리 등록된 Key에 한해서만 Set/Add 정수 Mutation을 지원한다. */
    class FWAINMapGameAdapter final : public IWAINGameSnapshotAdapter, public IWAINNarrativeGameFactReader
    {
        public:
        explicit FWAINMapGameAdapter(TMap<FName, int32> InInitialValues);

        virtual TSharedPtr<const IWAINGameStateSnapshot> CreateInitialSnapshot(FString &OutError) const override;

        virtual TSharedPtr<const IWAINGameStateSnapshot> Prepare(const IWAINGameStateSnapshot     &Current,
                                                                 const TArray<FWAINStateMutation> &GameMutations,
                                                                 FString                          &OutError) const override;

        virtual bool Validate(const IWAINGameStateSnapshot &Prepared, FString &OutError) const override;

        virtual bool ReadInteger(const IWAINGameStateSnapshot &Snapshot, FName TargetId, int32 &OutValue) const override;

        private:
        TMap<FName, int32> InitialValues;
    };

#pragma endregion
