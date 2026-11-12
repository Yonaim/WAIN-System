# Phase 2 — Narrative Beat Graph MVP

> **상태:** C++ 코드와 Automation Test **10개 작성**, UE 5.8 빌드·실행 미검증. 기존 49개 + 신규 10개 = **총 59개 테스트 코드**. 정책은 [ADR-010](../03_Decisions/10_ADR-010-Deterministic-Beat-Graph-MVP.md)의 `Proposed` 선택이다.

## 00. 구현 목표

Current Context의 Goal을 Provider에 제공하고, 이번 턴의 Prepared WAIN/Game 상태를 기준으로 완료 조건·Hard Rule·Beat Transition을 평가한다. Game Host의 Narrative JSON을 Plugin이 직접 파싱하거나 World Actor를 직접 수정하지 않는다.

## 10. Top-Down 처리 흐름

```text
StartSession
  GameAdapter.CreateInitialSnapshot
  GameAdapter.Validate
  NarrativeEvaluator.ValidateInitialState       ← Graph·Active Beat 정의 확인
  StateStore.Initialize

ProcessTurn
  Read Current Root
  CharacterContextBuilder.Build
  NarrativeEvaluator.BuildContext               ← Chapter / Sequence / Beat Goal + Soft Constraints
  ProposalProvider.GenerateWithContext
  TransactionCoordinator.Execute
    ValidateProposal
    PrepareWAINState                             ← Trust, Emotion, Observation, TurnCount
    GameAdapter.Prepare                          ← Game Fact 후보
    NarrativeEvaluator.Evaluate
      EnforceHardRules                           ← 부모 3단계 Rules + Prepared 값
      EvaluateCompletion                         ← 조건 AND
      SelectNextBeat                             ← 후보 Priority / 동점 오류
      AppendBeatHistory                          ← Prepared WAIN에만 기록
    MemoryReasoning.Prepare                      ← 전이된 Beat로 Plan 갱신
    FinalStateValidation
    StateStore.Publish                          ← WAIN + Game Snapshot 함께 확정
  ReturnVNResponse
```

## 20. 신규·변경 파일

| 경로 (Source/WAINSystem 기준) | 책임 |
|---|---|
| `Public/WAINNarrativeTypes.h` | Chapter/Sequence/Beat/Condition/Hard Rule/Context/Fact Reader DTO |
| `Public/WAINBeatGraphNarrativeEvaluator.h` | `IWAINNarrativeEvaluator` 구현 계약 |
| `Private/Narrative/WAINBeatGraphNarrativeEvaluator.cpp` | Graph 정의 검사, Goal Context, Prepared Fact 평가·전이 선택 |
| `Public/WAINInterfaces.h` | 기본 `ValidateInitialState`·`BuildContext` 훅 및 Context 미지원 Provider 방어 |
| `Public/WAINCharacterContext.h` | 읽기 전용 Narrative Context 필드 |
| `Public/WAINRuntimeTypes.h` | `FWAINBeatTransitionRecord`, `BeatHistory` |
| `Private/Core/WAINInteractionProcessor.cpp` | 세션 Graph 사전 검사 및 턴별 Narrative Context 빌드 |
| `Private/Core/WAINStateValidation.cpp` | BeatHistory 기본 범위·구조 검사 |
| `Private/Testing/WAINMapGameAdapter.{h,cpp}` | Mock Game Snapshot Fact Reader; 별도 Live State 변경 없음 |
| `Private/Tests/WAINNarrativeTests.cpp` | 신규 Automation Test 10개 |

## 30. 지원 계약 및 제한

- `FWAINNarrativeGraph`는 Host가 생성하여 Evaluator에 전달하는 **데이터 구조**이며 JSON Loader는 미구현이다.
- `CharacterTrust`, `TurnCount`, 사전 등록된 `GameInteger`만 조건에 쓸 수 있다. 수치 조건 목록은 AND. 한 턴의 변경 제안이 Prepare에 성공한 **다음 상태**를 평가한다.
- 완료 조건을 주지 않은 Beat는 자동 전이하지 않는다. 완료 조건이 충족되었더라도 조건에 맞는 Transition이 없다면 현재 Beat에 머문다.
- 상위 3단계의 Hard Rule은 지정된 Game Integer **증가**에만 적용한다. 위반 시 NarrativeEvaluationFailed 및 전체 No Commit.
- Graph 내 ID 중복·누락 Parent·잘못된 Transition 참조는 Session 시작 때 거절한다. 동시에 충족되는 최고 Priority 전이 둘은 No Commit.
- 이전 No-Op Mock Narrative 경로는 유지한다. 실제 Evaluator 주입 시 Provider가 `GenerateWithContext`를 구현해야 Goal을 무시하지 않는다.
- Script 조건, 텍스트 자체의 비밀 공개 감지, End Event, Graph Save/Load, 영구적인 Transition History 등은 차후 개발한다.

## 40. 테스트와 검증

`WAIN.Narrative.ContextGoals`, `PreparedTransition`, `IncompleteBeat`, `TurnProgress`, `InvalidGraph`, `HardRuleReject`, `HardRulePreparedTrust`, `PriorityTieReject`, `UnknownFactReject`, `ReplayNoDoubleTransition`을 새로 작성했다.

전체 회귀 실행 대상은 `WAIN.Transaction.*`, `WAIN.Character.*`, `WAIN.CharacterContext.*`, `WAIN.Presentation.*`, `WAIN.Memory.*`, `WAIN.MemoryReasoning.*`, `WAIN.Narrative.*`다.

**2026-11-13 검증:** `WAINTestbedEditor Win64 DebugGame` UE 5.8 빌드 성공. `FWAINBeatGraphNarrativeEvaluator`를 Host 모듈에서 링크할 수 있도록 모듈 export를 표시했다. Automation 테스트는 실행하지 않았다.
