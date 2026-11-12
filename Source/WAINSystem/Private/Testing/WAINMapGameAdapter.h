/**
 * @file WAINMapGameAdapter.h
 * @brief 테스트용 Host-owned 정수 State를 준비하는 메모리 Snapshot Adapter를 제공한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINInterfaces.h"

/** @brief 실제 게임 Inventory/Quest 저장소를 대신하는 테스트 전용 Snapshot. */
struct FWAINMapGameSnapshot final : public IWAINGameStateSnapshot
{
    TMap<FName, int32> Values;

    virtual FName GetKind() const override;
};

/** @brief 미리 등록된 Key에 한해서만 Set/Add 정수 Mutation을 지원한다. */
class FWAINMapGameAdapter final : public IWAINGameSnapshotAdapter
{
public:
    explicit FWAINMapGameAdapter(TMap<FName, int32> InInitialValues);

    virtual TSharedPtr<const IWAINGameStateSnapshot> CreateInitialSnapshot(FString& OutError) const override;

    virtual TSharedPtr<const IWAINGameStateSnapshot> Prepare(
        const IWAINGameStateSnapshot& Current,
        const TArray<FWAINStateMutation>& GameMutations,
        FString& OutError) const override;

    virtual bool Validate(const IWAINGameStateSnapshot& Prepared, FString& OutError) const override;

private:
    TMap<FName, int32> InitialValues;
};
