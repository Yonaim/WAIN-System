/**
 * @file WAINStateStore.cpp
 * @brief Snapshot 초기화와 원자적 Session Root 게시 경계를 구현한다.
 */

#include "Core/WAINStateStore.h"

#include "Core/WAINStateValidation.h"

bool FWAINStateStore::Initialize(
    const FWAINStateSnapshot& InitialWAIN,
    TSharedPtr<const IWAINGameStateSnapshot> InitialGame,
    FString& OutError)
{
    check(IsInGameThread());

    if (!InitialGame.IsValid() || !WAINStateValidation::Validate(InitialWAIN, OutError))
    {
        if (OutError.IsEmpty())
        {
            OutError = TEXT("초기 Game Snapshot이 없습니다.");
        }
        return false;
    }

    FWAINSessionRoot InitialRoot;
    InitialRoot.Revision = 0;
    InitialRoot.WAIN = InitialWAIN;
    InitialRoot.Game = MoveTemp(InitialGame);

    CurrentRoot = MakeShared<FWAINSessionRoot>(MoveTemp(InitialRoot));
    return true;
}

TSharedPtr<const FWAINSessionRoot> FWAINStateStore::Read() const
{
    check(IsInGameThread());
    return CurrentRoot;
}

bool FWAINStateStore::Publish(
    const uint64 ExpectedRevision,
    FWAINStateSnapshot&& PreparedWAIN,
    TSharedPtr<const IWAINGameStateSnapshot> PreparedGame)
{
    check(IsInGameThread());

    if (!CurrentRoot.IsValid() || CurrentRoot->Revision != ExpectedRevision || ExpectedRevision == MAX_uint64 || !PreparedGame.IsValid())
    {
        return false;
    }

    FWAINSessionRoot NextRoot;
    NextRoot.Revision = ExpectedRevision + 1;
    NextRoot.WAIN = MoveTemp(PreparedWAIN);
    NextRoot.Game = MoveTemp(PreparedGame);

    // Plugin과 Host의 Snapshot을 가리키는 단일 Root만 공개한다.
    CurrentRoot = MakeShared<FWAINSessionRoot>(MoveTemp(NextRoot));
    return true;
}
