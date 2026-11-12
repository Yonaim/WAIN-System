/**
 * @file WAINWorldStateManager.h
 * @brief Host World Fact의 공개 Context 및 Prepared 상태 변경 검증을 조율한다.
 */

#pragma once

#include "WAINRuntimeTypes.h"
#include "WAINWorldTypes.h"

// ============================================================
// 00. World State Manager Contract
// ============================================================
#pragma region 00_World_State_Manager_Contract

    /** @brief 읽기 전용 Host Fact Reader와 값 없는 Schema만 보유한다. */
    class WAINSYSTEM_API FWAINWorldStateManager final
    {
        public:
        FWAINWorldStateManager(TArray<FWAINWorldFactDefinition> InSchema, TSharedPtr<const IWAINWorldFactReader> InReader);

        bool ValidateInitial(const IWAINGameStateSnapshot &Current, FString &OutError) const;

        bool BuildContext(const IWAINGameStateSnapshot &Current, FWAINWorldContext &OutContext, FString &OutError) const;

        bool ValidateMutations(const IWAINGameStateSnapshot     &Current,
                               const TArray<FWAINStateMutation> &Mutations,
                               FString                          &OutError) const;

        bool ValidatePrepared(const IWAINGameStateSnapshot     &Current,
                              const IWAINGameStateSnapshot     &Prepared,
                              const TArray<FWAINStateMutation> &Mutations,
                              FString                          &OutError) const;

        private:
        bool                            ValidateSchema(FString &OutError) const;
        bool                            ReadValidated(const IWAINGameStateSnapshot   &Snapshot,
                                                      const FWAINWorldFactDefinition &Definition,
                                                      int32                          &OutValue,
                                                      FString                        &OutError) const;
        bool                            ComputeExpected(const IWAINGameStateSnapshot     &Current,
                                                        const TArray<FWAINStateMutation> &Mutations,
                                                        TMap<FName, int32>               &OutExpected,
                                                        FString                          &OutError) const;
        const FWAINWorldFactDefinition *FindDefinition(FName Key) const;

        TArray<FWAINWorldFactDefinition>       Schema;
        TSharedPtr<const IWAINWorldFactReader> Reader;
    };

#pragma endregion
