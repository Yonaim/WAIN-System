# Narrative Manager — Beat Graph

> 상태: Chapter→Sequence→Beat 설계 계약. Trigger/조건식 구현은 없음.

## 서사 데이터

- `Chapter`: 장기 Goal과 전역 Constraint의 부모
- `Sequence`: 중간 단계 Goal과 Constraint
- `Beat`: 현재 실행 단위, Goal과 필수 Completion Condition, 선택적인 Constraint/Transition 정보
- `Narrative State`: Active Beat, 완료 이력·전이 이력 등 **지속 State**

`Goal`은 생성 모델이 참고할 자연어 방향이다. `Completion Condition`은 플래그/숫자 등 **구조화된 값**을 평가하여 현재 Beat 종료 가능 여부를 판정한다. Soft Constraint는 자연어 지침, Hard Constraint는 결정론적 검사 가능한 Rule만 포함한다.

## Narrative Context Assembly

현재 Beat에서 Sequence/Chapter를 거슬러 Goal·Constraint를 수집하고 충돌 정책을 적용한 뒤 Character Agent의 Context로 전달한다. 허용되지 않은 진실은 Prompt 구성 전부터 제외한다.

## Beat Transition

이번 턴 검증된 변경 제안을 바탕으로 Completion 평가 → 현재 Beat의 허용 Transition 조건 검사 → 가능한 후보를 Priority로 결정 → **State Commit 단계에서** Active Beat 변경. 초기에는 턴당 최대 한 번, 조건 불충족 시 현재 Beat 유지.

## Testbed 샘플

`Content/Data/Scenarios/CH01Narrative.json`은 `INTRO`, `TRANSFER_HINT`, `MEMORY_DOUBT`와 기밀 Flag 제한을 예시로 둔다. 이는 플러그인 스키마의 확정된 구현이 아니며, Testbed Mock이 그 조건을 해석하고 있지도 않다.

## 미결정

Transition Priority 정렬, 구조화 조건 언어, 실패 시 재평가, Parent 제약 합성 및 Narrative Authoring 도구.

## Phase 2 Beat Graph MVP — 현재 작성된 코드 (2026-11-11)

최소 결정론적 Evaluator: `Public/WAINNarrativeTypes.h`, `Public/WAINBeatGraphNarrativeEvaluator.h`, `Private/Narrative/WAINBeatGraphNarrativeEvaluator.cpp`. Chapter/Sequence/Beat 정의와 ID 참조 검증, 현재 활성 부모 Goal/Soft Constraint의 읽기 전용 Context, `Trust/TurnCount/GameInteger` 조건 AND, 단일 최고 Priority Transition, Prepared `BeatHistory` 기록을 지원한다.

Game Fact는 `IWAINNarrativeGameFactReader`가 읽는다. MVP Hard Rule은 **지정된 Game Integer 증가 시** 준비된 State의 선행 조건을 요구한다. 자연어 텍스트 검열·행동의 의미 검증은 **없으므로**, 비밀 누설 방지 전체를 보장한다고 해석해서는 안 된다. 기존 `NoOp` 구현은 그대로 지원한다. JSON Loader·저장/복원·감정 조건은 미지원. [ADR-010](../../03_Decisions/10_ADR-010-Deterministic-Beat-Graph-MVP.md) · [Implementation](../../04_Development/10_Narrative-Graph-Implementation.md).
