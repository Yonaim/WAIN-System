# ADR-010 — Deterministic Beat Graph and Hard Rule Evaluation (MVP)

- **상태:** Proposed (Phase 2 구현 선택, 게임 콘텐츠 정책 최종 승인 전)
- **날짜:** 2026-11-11
- **연관:** [ADR-001](01_ADR-001-State-Ownership.md) · [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-004](04_ADR-004-Snapshot-State.md) · [ADR-009](09_ADR-009-Reflection-Planning-MVP.md)

## 00. 배경

`IWAINNarrativeEvaluator`는 Phase 1부터 존재하지만 실제 Beat Graph는 사용하지 않았다. Character의 Plan은 현재 `ActiveBeatId`를 목표 키의 대용으로 사용하므로 서사 제약과 완료 조건을 결정론적으로 평가하는 최소 구현이 필요하다.

## 10. 검토한 대안

1. LLM에게 자유 형식으로 Beat 변경을 위임: 자연스럽지만 State 규칙의 보장과 재현성이 부족하다.
2. Game 전용 Script/Blueprint 안에서 모든 조건을 평가: Testbed에는 편하지만 범용 Plugin의 서사 규칙을 검증하기 어렵다.
3. **데이터 기반 Beat Graph + 구조화 정수 조건 + Host Fact Reader**: 현재의 Snapshot/Adapter 계약에 맞고 Mock으로 검증할 수 있다.

## 20. MVP 구현 선택

- 정의 데이터는 `Chapter > Sequence > Beat`로 구성한다. 각 노드의 Goal·Soft Constraint는 *현재 활성 부모 경로만* 읽기 전용 `FWAINNarrativeContext`로 제공한다. Host가 공개 가능한 문구만 작성해야 하며 플러그인이 비밀 텍스트를 자동 검열하지는 않는다.
- 수치 조건은 `CharacterTrust`, `TurnCount`, `GameInteger`와 `Equal/AtLeast/AtMost`의 조합이다. 조건 목록은 **AND**, 빈 Transition 조건은 `true`로 취급한다. **빈 Completion 조건은 자동 완료가 아니며 현재 Beat를 유지**한다.
- Game-owned 숫자 조회는 `IWAINNarrativeGameFactReader`가 **기존/Prepared 불변 Snapshot**에서 읽도록 한다. 누락된 Fact는 `false`로 간주하지 않고 오류를 반환해 No Commit 처리한다.
- 완료가 참이면 전이 후보를 검사하고 **높은 Priority 한 개**를 선택한다. 최고 Priority의 충족 후보가 둘 이상이면 오류/No Commit으로 처리한다. 턴당 최대 한 전이만 허용한다.
- MVP Hard Rule은 Chapter/Sequence/Beat에서 상속되는 **Guarded Game Integer Increase**다. 지정된 Game 정수 Key가 이번 턴에 증가할 경우 Prepared WAIN/Game 상태의 전제 조건이 모두 충족되어야 한다. 이를 통한 정수 State 규칙만 보장하며 자유 형식 대사·Action의 서사 준수는 아직 검증하지 않는다.
- 전이 시 `FWAINStateSnapshot.BeatHistory`에 `FromBeatId`, `ToBeatId`, `Turn`을 추가한다. 이것 또한 **Prepared Snapshot 안에서만** 변경되며 기존 Commit 경계를 유지한다.
- `IWAINNarrativeEvaluator::ValidateInitialState/BuildContext` 기본 구현은 기존 Mock을 보존한다. 실제 Graph Evaluator를 주입한 경우에만 Graph 정의·초기 Beat를 검증하고 Narrative Context를 Provider에 전달한다. Context를 무시하는 Legacy Provider는 명시적으로 거절한다.
- 실제 Graph 데이터 로더, Save/Load, 정책 언어 확장, LLM Text Guard, 비동기 처리, UI 업데이트는 범위 밖이다.

## 30. 제약 및 향후 개선

- 단일 `TArray` 순차 조회 기반 소규모 Graph 구현이다. Authoring 도구·인덱스·조건식 AST/합성·다중 캐릭터 조건·감정 임계값·Quest Flag 의미론은 후속 작업이다.
- 이 MVP의 Hard Rule은 값 **증가**만 감시한다. 미등록 Game Fact는 오류로 처리하며, 감소/유지·동시 변경 중의 제약 우회 가능성은 향후 도메인별 Rule로 보완한다.
- Soft Constraint는 Context 지침일 뿐 자동 보증이 아니다. 현재 Goal 문구 안에 공개하면 안 되는 내용을 넣을 경우 플러그인이 삭제해 주지 않는다.
- Game 조회·원자성은 초기 공유 Session Root 경계에 한한다. 독립 Actor/외부 DB State에 대한 범용 Transaction 보장은 아니다.
- `Proposed`는 *코드가 존재하지 않는다*는 뜻이 아니라 **규칙 정책에 대한 최종 사용자 승인 전**임을 의미한다.

## 40. 검증

신규 `WAIN.Narrative.*` Automation Test 10개를 소스에 작성했다. UE 5.8 컴파일·테스트 실행은 이 작업 환경에서 미검증이며 코드/문서 정적 점검과 구분한다. [구현 상세](../04_Development/10_Narrative-Graph-Implementation.md).
