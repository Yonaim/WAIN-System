/**
 * @file WAINWorldStateManager.cpp
 * @brief Game Snapshot의 조회 권한과 정수 Mutation의 전후조건을 검증한다.
 */

#include "WAINWorldStateManager.h"
#include "WAINInterfaces.h"

// ============================================================
// 00. Initialization And World Context
// ============================================================
#pragma region 00_Initialization_And_World_Context

    // Host 정본을 복사하지 않고 공개 가능한 Key-Value만 이번 턴 Context에 제공한다.

    FWAINWorldStateManager::FWAINWorldStateManager(TArray<FWAINWorldFactDefinition>       InSchema,
                                                   TSharedPtr<const IWAINWorldFactReader> InReader)
        : Schema(MoveTemp(InSchema)),
          Reader(MoveTemp(InReader))
    {
    }

    bool FWAINWorldStateManager::ValidateInitial(const IWAINGameStateSnapshot &Current, FString &OutError) const
    {
        if (!ValidateSchema(OutError))
        {
            return false;
        }

        for (const FWAINWorldFactDefinition &Definition : Schema)
        {
            int32 IgnoredValue = 0;
            if (!ReadValidated(Current, Definition, IgnoredValue, OutError))
            {
                return false;
            }
        }
        return true;
    }

    bool FWAINWorldStateManager::BuildContext(const IWAINGameStateSnapshot &Current,
                                              FWAINWorldContext            &OutContext,
                                              FString                      &OutError) const
    {
        FWAINWorldContext NextContext;
        for (const FWAINWorldFactDefinition &Definition : Schema)
        {
            if (!Definition.bExposeToAgent)
            {
                continue;
            }

            FWAINWorldFactValue Fact;
            Fact.Key = Definition.Key;
            if (!ReadValidated(Current, Definition, Fact.Value, OutError))
            {
                return false;
            }
            NextContext.VisibleFacts.Add(Fact);
        }
        OutContext = MoveTemp(NextContext);
        return true;
    }

#pragma endregion

// ============================================================
// 01. Proposed And Prepared State Validation
// ============================================================
#pragma region 01_Proposed_And_Prepared_State_Validation

    // 제안이 허용된 Mutation인지 먼저 검사하고, Adapter의 실제 Prepared 결과까지 재확인한다.

    bool FWAINWorldStateManager::ValidateMutations(const IWAINGameStateSnapshot     &Current,
                                                   const TArray<FWAINStateMutation> &Mutations,
                                                   FString                          &OutError) const
    {
        TMap<FName, int32> Expected;
        return ComputeExpected(Current, Mutations, Expected, OutError);
    }

    bool FWAINWorldStateManager::ValidatePrepared(const IWAINGameStateSnapshot     &Current,
                                                  const IWAINGameStateSnapshot     &Prepared,
                                                  const TArray<FWAINStateMutation> &Mutations,
                                                  FString                          &OutError) const
    {
        TMap<FName, int32> Expected;
        if (!ComputeExpected(Current, Mutations, Expected, OutError))
        {
            return false;
        }

        for (const FWAINWorldFactDefinition &Definition : Schema)
        {
            int32 Actual = 0;
            if (!ReadValidated(Prepared, Definition, Actual, OutError))
            {
                return false;
            }

            const int32 *Planned = Expected.Find(Definition.Key);
            if (!Planned || Actual != *Planned)
            {
                OutError = TEXT("Prepared Game State가 승인된 World Mutation 결과와 일치하지 않습니다.");
                return false;
            }
        }
        return true;
    }

#pragma endregion

// ============================================================
// 02. Schema And Snapshot Fact Validation
// ============================================================
#pragma region 02_Schema_And_Snapshot_Fact_Validation

    // Schema 오류와 조회 실패를 모두 명시적 오류로 처리해 숨겨진 값을 추정하지 않는다.

    bool FWAINWorldStateManager::ValidateSchema(FString &OutError) const
    {
        if (!Reader.IsValid() || Schema.IsEmpty())
        {
            OutError = TEXT("World Fact Reader 및 비어 있지 않은 Schema가 필요합니다.");
            return false;
        }

        TSet<FName> Keys;
        for (const FWAINWorldFactDefinition &Definition : Schema)
        {
            if (Definition.Key.IsNone() || Keys.Contains(Definition.Key) || Definition.MinValue > Definition.MaxValue)
            {
                OutError = TEXT("World Fact Schema에 중복·누락된 Key 또는 잘못된 값 범위가 있습니다.");
                return false;
            }
            Keys.Add(Definition.Key);
        }
        return true;
    }

    bool FWAINWorldStateManager::ReadValidated(const IWAINGameStateSnapshot   &Snapshot,
                                               const FWAINWorldFactDefinition &Definition,
                                               int32                          &OutValue,
                                               FString                        &OutError) const
    {
        if (!Reader.IsValid() || !Reader->ReadInteger(Snapshot, Definition.Key, OutValue))
        {
            OutError = TEXT("World Fact를 Host Game Snapshot에서 읽지 못했습니다.");
            return false;
        }
        if (OutValue < Definition.MinValue || OutValue > Definition.MaxValue)
        {
            OutError = TEXT("World Fact 값이 Schema 범위 밖에 있습니다.");
            return false;
        }
        return true;
    }

    const FWAINWorldFactDefinition *FWAINWorldStateManager::FindDefinition(const FName Key) const
    {
        return Schema.FindByPredicate([Key](const FWAINWorldFactDefinition &Definition) { return Definition.Key == Key; });
    }

#pragma endregion

// ============================================================
// 03. Sequential Mutation Simulation
// ============================================================
#pragma region 03_Sequential_Mutation_Simulation

    // 변경을 순서대로 모의 적용하여 경계·권한 위반을 실제 Prepare 전 차단한다.

    bool FWAINWorldStateManager::ComputeExpected(const IWAINGameStateSnapshot     &Current,
                                                 const TArray<FWAINStateMutation> &Mutations,
                                                 TMap<FName, int32>               &OutExpected,
                                                 FString                          &OutError) const
    {
        OutExpected.Reset();
        for (const FWAINWorldFactDefinition &Definition : Schema)
        {
            int32 CurrentValue = 0;
            if (!ReadValidated(Current, Definition, CurrentValue, OutError))
            {
                return false;
            }
            OutExpected.Add(Definition.Key, CurrentValue);
        }

        for (const FWAINStateMutation &Mutation : Mutations)
        {
            const FWAINWorldFactDefinition *Definition = FindDefinition(Mutation.TargetId);
            if (Mutation.Owner != EWAINMutationOwner::Game || !Definition || !Definition->bWritableByProposal ||
                (Mutation.Operation != EWAINMutationOperation::AddInteger &&
                 Mutation.Operation != EWAINMutationOperation::SetInteger))
            {
                OutError = TEXT("World Fact는 등록된 쓰기 허용 정수 Key만 변경할 수 있습니다.");
                return false;
            }

            int32 *Value = OutExpected.Find(Mutation.TargetId);
            if (!Value)
            {
                OutError = TEXT("World Fact의 현재 값을 확인하지 못했습니다.");
                return false;
            }

            const int64 NextValue = Mutation.Operation == EWAINMutationOperation::AddInteger
                                        ? static_cast<int64>(*Value) + Mutation.IntegerValue
                                        : static_cast<int64>(Mutation.IntegerValue);
            if (NextValue < Definition->MinValue || NextValue > Definition->MaxValue)
            {
                OutError = TEXT("World Fact Mutation 결과가 허용 범위 밖에 있습니다.");
                return false;
            }
            *Value = static_cast<int32>(NextValue);
        }
        return true;
    }

#pragma endregion
