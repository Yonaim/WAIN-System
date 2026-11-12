# Phase 2 — Reflection + Planning MVP

> **상태:** C++ 코드 및 Unreal Automation Test 11개 작성, **실제 UE 5.8 컴파일·테스트 실행 미검증**. 이전 38개와 합쳐 누적 49개. 최소 정책은 [ADR-009](../03_Decisions/09_ADR-009-Reflection-Planning-MVP.md)에서 `Proposed`로 기록.

## 00. 구현 범위

- `FWAINMemoryRecord`의 Reflection과 Plan 생성이 가능한 실제 Prepare 경로를 추가했다. 생성 내용을 작성하는 `IWAINMemoryReasoner`는 Host/LLM 계층에서 주입한다. 현재는 테스트에서 결정론적 Mock만 제공한다.
- `FWAINStateSnapshot::ActivePlanIds`가 캐릭터별 현재 계획을 참조한다. Plan `GoalId`는 **현재 Beat ID를 대용**한다. 별도 Narrative Goal 데이터는 아직 존재하지 않는다.
- `FWAINMemoryRetriever`는 Reflection을 다른 Observation과 함께 검색하고, 활성 Plan을 하나 더 Context에 포함한다. 비활성 Plan은 제외한다.

## 10. Top-Down 실행 흐름

```text
ProcessTurn
  BuildCharacterContext          ← 이전에 확정된 Memory/Plan 읽기 전용
  GenerateProposal
  Transaction.Execute
    ValidateProposal
    PrepareWAINState            ← Mutation + Observation
    GameAdapter.Prepare
    NarrativeEvaluator.Evaluate ← 이 턴의 Prepared Beat 확정
    MemoryReasoning.Prepare      ← Reasoner 주입 시에만 수행
      PrepareReflection         ← 중요도 임계값 Trigger
      PreparePlan               ← Plan 없음/Scene·Beat 변경 Trigger
    FinalStateValidation
    StateStore.Publish          ← Derived Memory + 다른 State 함께 확정
  ReturnVNResponse
```

- Reflection 생성은 **새 Observation이 충분히 축적된 경우에만 호출**한다. 생성된 근거 ID는 실제 입력 Memory에서 WAIN이 직접 기록한다.
- Plan 생성은 최초 등장 또는 Scene/Beat 변경 때에만 수행한다. 기존 Plan은 삭제하지 않고 활성 인덱스만 교체한다.
- 생성기가 실패하거나 잘못된 문자열/Step을 반환하면 `MemoryReasoningFailed`를 반환하고 **Memory, Relationship, Game State, Revision 중 어느 것도 확정하지 않는다**.
- `Reasoner == nullptr`이면 Reflection/Plan을 생성하지 않아 이전 Observation-only Mock 테스트를 유지한다.

## 20. 코드 경계

| 코드 | 책임 |
|---|---|
| `Public/WAINMemoryReasoning.h` | 생성기 계약과 Plan Draft. 비동기·LLM·Prompt 구현 없음 |
| `Public/WAINMemoryTypes.h` | 통합 Memory Record의 Plan Goal·Scene·Beat·Steps |
| `Public/WAINRuntimeTypes.h` | `ActivePlanIds` 및 `MemoryReasoningFailed` |
| `Private/Memory/WAINMemoryReasoningPreparer.{h,cpp}` | Trigger·생성기 호출·Prepared Memory 추가 |
| `Private/Memory/WAINMemoryRetriever.{h,cpp}` | Reflection 검색 및 활성 Plan Context 포함 |
| `Private/Core/WAINStateValidation.cpp` | Plan 형식·활성 ID·출처 방향/소유권 검증 |
| `Private/Core/WAINTransactionCoordinator.cpp` | Narrative 평가 이후 Reasoning, 최종 검증 이전 호출 |
| `Private/Core/WAINInteractionProcessor.{h,cpp}` | 선택적인 Reasoner 주입·전달 |
| `Public/WAINRuntimeSubsystem.h` | `StartSession(..., MemoryReasoner)` 선택 인자 |
| `Private/Tests/WAINMemoryReasoningTests.cpp` | 신규 자동화 테스트 11개 |

## 30. MVP 정책과 제한

- Reflection: 최신 12개 Observation (마지막 Reflection **이후**)의 중요도 합이 2.0 이상일 때만, 최소 2개 출처를 요구한다.
- Plan: 활성 ID 없음 또는 **Scene/Beat 변경** 시에만 생성. 가장 최근의 Observation/Reflection 최대 6개를 근거로 사용. 1~8개의 Step을 허용한다.
- 임계값·Source 수·Importance=0.7은 모두 임시 하이퍼파라미터다. `CurrentStepIndex`는 저장/검증만 수행하고 Step 진행 로직은 없다.
- 근거 ID의 존재, 같은 Character, 앞선 Stream 위치, 중복 금지는 검증하지만 **생성된 Reflection의 논리적 정확성과 근거 적절성은 자동 평가하지 않는다**.
- Phase 2의 Memory는 세션 메모리 전용이며 지속 저장/복원, Embedding, 비동기 LLM, 외부 상태에 따른 Replan, Plan 성공/실패 피드백은 추후 구현한다.

## 40. 검증 상태

- 신규 `WAIN.MemoryReasoning.*` 11개: Plan 최초 생성, Reflection Trigger, 다음 턴 Context, Scene/Beat 변경, 캐릭터 격리, 생성 오류 No Commit, Game Prepare 실패 시 Reasoner 미호출, Replay idempotency, 잘못된 Active Plan 거절.
- 기존 `WAIN.Transaction.*`, `WAIN.Character.*`, `WAIN.CharacterContext.*`, `WAIN.Presentation.*`, `WAIN.Memory.*` 38개도 **회귀 실행 대상**.
- ZIP/JSON/링크/Region/헤더 연결 등 **정적 점검만 수행**. UnrealBuildTool/Engine Automation을 실행하지 않았으므로 빌드 통과 또는 테스트 성공으로 표시하지 않는다.

[Memory 계약](../02_Architecture/01_Contracts/02_Memory-Reflection-Planning.md) · [ADR-008](../03_Decisions/08_ADR-008-Transactional-Observations.md) · [ADR-009](../03_Decisions/09_ADR-009-Reflection-Planning-MVP.md)
