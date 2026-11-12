/**
 * @file WAINRuntimeTests.cpp
 * @brief Mock Interaction의 원자적 Commit·실패 시 무변경·중복 턴 동작을 검증한다.
 */

#include "Core/WAINInteractionProcessor.h"
#include "Core/WAINStateStore.h"
#include "Testing/WAINMapGameAdapter.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

namespace WAINTests
{
class FFixedProposalProvider final : public IWAINProposalProvider
{
public:
    explicit FFixedProposalProvider(FWAINResponseProposal InProposal)
        : Proposal(MoveTemp(InProposal))
    {
    }

    virtual bool Generate(
        const FWAINInteractionRequest& Request,
        const FWAINSessionRoot& Current,
        FWAINResponseProposal& OutProposal,
        FString& OutError) const override
    {
        OutProposal = Proposal;
        return true;
    }

private:
    FWAINResponseProposal Proposal;
};

class FThresholdNarrativeEvaluator final : public IWAINNarrativeEvaluator
{
public:
    virtual bool Evaluate(
        const FWAINInteractionRequest& Request,
        const FWAINSessionRoot& Current,
        FWAINStateSnapshot& PreparedWAIN,
        const IWAINGameStateSnapshot& PreparedGame,
        FString& OutError) const override
    {
        const FWAINCharacterState* Character = PreparedWAIN.Characters.Find(Request.CharacterId);
        if (Character && Character->Trust >= 50 && PreparedWAIN.ActiveBeatId == TEXT("INTRO"))
        {
            PreparedWAIN.ActiveBeatId = TEXT("NEXT");
        }

        return true;
    }
};

FWAINStateSnapshot MakeInitialWAIN(const int32 Trust = 45)
{
    FWAINStateSnapshot State;
    FWAINCharacterState Character;
    Character.Trust = Trust;
    State.Characters.Add(TEXT("TEST_CHARACTER"), Character);
    State.ActiveBeatId = TEXT("INTRO");
    return State;
}

FWAINInteractionRequest MakeRequest()
{
    FWAINInteractionRequest Request;
    Request.TurnId = FGuid::NewGuid();
    Request.CharacterId = TEXT("TEST_CHARACTER");
    Request.SceneId = TEXT("TEST_SCENE");
    Request.InputKind = EWAINInputKind::Text;
    Request.Text = TEXT("안녕");
    return Request;
}

FWAINResponseProposal MakeProposal(const bool bIncludeGameChange = true)
{
    FWAINResponseProposal Proposal;
    Proposal.Dialogue = TEXT("반가워!");

    FWAINStateMutation Relationship;
    Relationship.Owner = EWAINMutationOwner::Character;
    Relationship.TargetId = TEXT("relationship.trust");
    Relationship.IntegerValue = 5;
    Proposal.StateChanges.Add(Relationship);

    if (bIncludeGameChange)
    {
        FWAINStateMutation Inventory;
        Inventory.Owner = EWAINMutationOwner::Game;
        Inventory.TargetId = TEXT("inventory.gift");
        Inventory.IntegerValue = -1;
        Proposal.StateChanges.Add(Inventory);
    }

    return Proposal;
}

TSharedRef<FWAINMapGameAdapter> MakeAdapter()
{
    TMap<FName, int32> Values;
    Values.Add(TEXT("inventory.gift"), 2);
    return MakeShared<FWAINMapGameAdapter>(MoveTemp(Values));
}

TUniquePtr<FWAINInteractionProcessor> MakeProcessor(
    const FWAINResponseProposal& Proposal,
    FString& OutError,
    const int32 InitialTrust = 45)
{
    TUniquePtr<FWAINInteractionProcessor> Processor = MakeUnique<FWAINInteractionProcessor>(
        MakeAdapter(),
        MakeShared<FFixedProposalProvider>(Proposal),
        MakeShared<FThresholdNarrativeEvaluator>());

    if (!Processor->StartSession(MakeInitialWAIN(InitialTrust), OutError))
    {
        return nullptr;
    }

    return Processor;
}

int32 GetGiftCount(const TSharedPtr<const FWAINSessionRoot>& Root)
{
    if (!Root.IsValid() || !Root->Game.IsValid() || Root->Game->GetKind() != FWAINMapGameSnapshot().GetKind())
    {
        return INDEX_NONE;
    }

    const auto& GameState = static_cast<const FWAINMapGameSnapshot&>(*Root->Game);
    return GameState.Values.FindRef(TEXT("inventory.gift"));
}
} // namespace WAINTests

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FWAINCommitSuccessTest,
    "WAIN.Transaction.AtomicCommit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAINCommitSuccessTest::RunTest(const FString& Parameters)
{
    FString Error;
    auto Processor = WAINTests::MakeProcessor(WAINTests::MakeProposal(), Error);
    if (!TestTrue(TEXT("세션 생성"), Processor != nullptr))
    {
        return false;
    }

    const auto Previous = Processor->GetSnapshot();
    const auto Result = Processor->ProcessTurn(WAINTests::MakeRequest());
    const auto Current = Processor->GetSnapshot();

    TestTrue(TEXT("트랜잭션 성공"), Result.Succeeded());
    TestEqual(TEXT("Revision 증가"), Current->Revision, uint64(1));
    TestEqual(TEXT("Character Trust 증가"), Current->WAIN.Characters.FindChecked(TEXT("TEST_CHARACTER")).Trust, 50);
    TestEqual(TEXT("Game Inventory 감소"), WAINTests::GetGiftCount(Current), 1);
    TestEqual(TEXT("Narrative 전이"), Current->WAIN.ActiveBeatId, FName(TEXT("NEXT")));
    TestEqual(TEXT("기존 Snapshot 유지"), Previous->WAIN.Characters.FindChecked(TEXT("TEST_CHARACTER")).Trust, 45);
    TestEqual(TEXT("기존 Game Snapshot 유지"), WAINTests::GetGiftCount(Previous), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FWAINPrepareFailureTest,
    "WAIN.Transaction.PrepareFailureKeepsState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAINPrepareFailureTest::RunTest(const FString& Parameters)
{
    FWAINResponseProposal Proposal = WAINTests::MakeProposal();
    Proposal.StateChanges.Last().TargetId = TEXT("unregistered.gift");

    FString Error;
    auto Processor = WAINTests::MakeProcessor(Proposal, Error);
    if (!TestTrue(TEXT("세션 생성"), Processor != nullptr))
    {
        return false;
    }

    const auto Result = Processor->ProcessTurn(WAINTests::MakeRequest());
    const auto Current = Processor->GetSnapshot();

    TestTrue(TEXT("Game Prepare 실패"), Result.Error == EWAINInteractionError::GamePrepareFailed);
    TestEqual(TEXT("Revision 유지"), Current->Revision, uint64(0));
    TestEqual(TEXT("Trust 유지"), Current->WAIN.Characters.FindChecked(TEXT("TEST_CHARACTER")).Trust, 45);
    TestEqual(TEXT("Inventory 유지"), WAINTests::GetGiftCount(Current), 2);
    TestEqual(TEXT("Beat 유지"), Current->WAIN.ActiveBeatId, FName(TEXT("INTRO")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FWAINDuplicateTurnTest,
    "WAIN.Transaction.DuplicateTurnIsIdempotent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAINDuplicateTurnTest::RunTest(const FString& Parameters)
{
    FString Error;
    auto Processor = WAINTests::MakeProcessor(WAINTests::MakeProposal(), Error);
    if (!TestTrue(TEXT("세션 생성"), Processor != nullptr))
    {
        return false;
    }

    const auto Request = WAINTests::MakeRequest();
    const auto First = Processor->ProcessTurn(Request);
    const auto Replay = Processor->ProcessTurn(Request);

    TestTrue(TEXT("첫 턴 성공"), First.Succeeded());
    TestTrue(TEXT("재전송 결과 재사용"), Replay.bReplayed);
    TestEqual(TEXT("재전송 시 Revision 유지"), Processor->GetSnapshot()->Revision, uint64(1));
    TestEqual(TEXT("재전송 시 Inventory 중복 차감 없음"), WAINTests::GetGiftCount(Processor->GetSnapshot()), 1);

    FWAINInteractionRequest Altered = Request;
    Altered.Text = TEXT("다른 입력");
    const auto Conflict = Processor->ProcessTurn(Altered);
    TestTrue(TEXT("TurnId 재사용 충돌"), Conflict.Error == EWAINInteractionError::DuplicateTurnConflict);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FWAINInvalidProposalTest,
    "WAIN.Transaction.UnvalidatedPresentationIsRejected",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAINInvalidProposalTest::RunTest(const FString& Parameters)
{
    FWAINResponseProposal Proposal = WAINTests::MakeProposal();
    Proposal.ExpressionId = TEXT("EXP_NOT_REGISTERED");

    FString Error;
    auto Processor = WAINTests::MakeProcessor(Proposal, Error);
    if (!TestTrue(TEXT("세션 생성"), Processor != nullptr))
    {
        return false;
    }

    const auto Result = Processor->ProcessTurn(WAINTests::MakeRequest());
    TestTrue(TEXT("미검증 Presentation 거절"), Result.Error == EWAINInteractionError::InvalidProposal);
    TestEqual(TEXT("State 변경 없음"), Processor->GetSnapshot()->Revision, uint64(0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FWAINRevisionConflictTest,
    "WAIN.Transaction.StaleRevisionCannotPublish",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAINRevisionConflictTest::RunTest(const FString& Parameters)
{
    auto Adapter = WAINTests::MakeAdapter();
    FString Error;
    FWAINStateStore Store;
    if (!TestTrue(TEXT("초기 Root 구성"), Store.Initialize(WAINTests::MakeInitialWAIN(), Adapter->CreateInitialSnapshot(Error), Error)))
    {
        return false;
    }

    const auto Old = Store.Read();
    TArray<FWAINStateMutation> NoChanges;
    auto FirstGame = Adapter->Prepare(*Old->Game, NoChanges, Error);
    FWAINStateSnapshot FirstWAIN = Old->WAIN;
    TestTrue(TEXT("정상 게시"), Store.Publish(Old->Revision, MoveTemp(FirstWAIN), MoveTemp(FirstGame)));

    auto StaleGame = Adapter->Prepare(*Old->Game, NoChanges, Error);
    FWAINStateSnapshot StaleWAIN = Old->WAIN;
    TestFalse(TEXT("과거 Revision은 게시 거절"), Store.Publish(Old->Revision, MoveTemp(StaleWAIN), MoveTemp(StaleGame)));
    TestEqual(TEXT("확정 Revision 1 유지"), Store.Read()->Revision, uint64(1));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FWAINTrustLimitTest,
    "WAIN.Transaction.OutOfRangeTrustKeepsState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FWAINTrustLimitTest::RunTest(const FString& Parameters)
{
    FString Error;
    auto Processor = WAINTests::MakeProcessor(WAINTests::MakeProposal(), Error, 99);
    if (!TestTrue(TEXT("세션 생성"), Processor != nullptr))
    {
        return false;
    }

    const auto Result = Processor->ProcessTurn(WAINTests::MakeRequest());
    TestTrue(TEXT("Trust 초과 거절"), Result.Error == EWAINInteractionError::UnsupportedMutation);
    TestEqual(TEXT("Revision 무변경"), Processor->GetSnapshot()->Revision, uint64(0));
    TestEqual(TEXT("Game 무변경"), WAINTests::GetGiftCount(Processor->GetSnapshot()), 2);
    return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
