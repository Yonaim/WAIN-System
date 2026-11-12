/**
 * @file WAINInterfaces.h
 * @brief Plugin에서 Host 상태와 제안·서사 평가를 주입받기 위한 C++ 계약을 정의한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"
#include "WAINRuntimeTypes.h"

// ============================================================
// 00. Session Root And Game Snapshot
// ============================================================
#pragma region 00_Session_Root_And_Game_Snapshot

    /**
     * @brief Host Game이 소유하는 불변 상태 Snapshot의 타입 소거 인터페이스.
     *
     * Snapshot을 가리키는 포인터를 교체하는 것만으로는 개별 Actor 속성이
     * 자동 갱신되지 않는다. Host의 화면/Actor는 Commit된 Root를 조회하는 View여야 한다.
     */
    class IWAINGameStateSnapshot
    {
        public:
        virtual ~IWAINGameStateSnapshot() = default;
        virtual FName GetKind() const = 0;
    };

    /** @brief 하나의 Revision에 묶여 함께 게시되는 두 상태의 최상위 참조 객체. */
    struct FWAINSessionRoot
    {
        uint64                                   Revision = 0;
        FWAINStateSnapshot                       WAIN;
        TSharedPtr<const IWAINGameStateSnapshot> Game;
    };

#pragma endregion

// ============================================================
// 01. Host Game Adapter Contract
// ============================================================
#pragma region 01_Host_Game_Adapter_Contract

    /**
     * @brief Host-owned Snapshot의 준비 단계만 제공하는 최소 Adapter.
     *
     * 모든 메서드는 Game Thread에서 호출된다. Prepare는 외부 부작용과
     * 권위 상태 갱신을 일으키지 않아야 한다. 준비한 Snapshot은 Publish에서
     * WAIN Snapshot과 함께 하나의 Root로 공개된다.
     */
    class IWAINGameSnapshotAdapter
    {
        public:
        virtual ~IWAINGameSnapshotAdapter() = default;

        virtual TSharedPtr<const IWAINGameStateSnapshot> CreateInitialSnapshot(FString &OutError) const = 0;

        virtual TSharedPtr<const IWAINGameStateSnapshot> Prepare(const IWAINGameStateSnapshot     &Current,
                                                                 const TArray<FWAINStateMutation> &GameMutations,
                                                                 FString                          &OutError) const = 0;

        virtual bool Validate(const IWAINGameStateSnapshot &Prepared, FString &OutError) const = 0;
    };

#pragma endregion

// ============================================================
// 02. Proposal Provider Contract
// ============================================================
#pragma region 02_Proposal_Provider_Contract

    /** @brief 실제 LLM 이전에는 결정론적 Mock Provider를 주입한다. */
    class IWAINProposalProvider
    {
        public:
        virtual ~IWAINProposalProvider() = default;

        virtual bool Generate(const FWAINInteractionRequest &Request,
                              const FWAINSessionRoot        &Current,
                              FWAINResponseProposal         &OutProposal,
                              FString                       &OutError) const = 0;
    };

#pragma endregion

// ============================================================
// 03. Narrative Evaluation Contract
// ============================================================
#pragma region 03_Narrative_Evaluation_Contract

    /** @brief Current와 준비된 상태를 비교한 뒤 다음 Beat만 결정한다. */
    class IWAINNarrativeEvaluator
    {
        public:
        virtual ~IWAINNarrativeEvaluator() = default;

        virtual bool Evaluate(const FWAINInteractionRequest &Request,
                              const FWAINSessionRoot        &Current,
                              FWAINStateSnapshot            &PreparedWAIN,
                              const IWAINGameStateSnapshot  &PreparedGame,
                              FString                       &OutError) const = 0;
    };

    /** @brief Narrative 규칙 도입 전 Mock 테스트에서 사용하는 무전이 평가기. */
    class FWAINNoOpNarrativeEvaluator final : public IWAINNarrativeEvaluator
    {
        public:
        virtual bool Evaluate(const FWAINInteractionRequest &Request,
                              const FWAINSessionRoot        &Current,
                              FWAINStateSnapshot            &PreparedWAIN,
                              const IWAINGameStateSnapshot  &PreparedGame,
                              FString                       &OutError) const override
        {
            return true;
        }
    };

#pragma endregion
