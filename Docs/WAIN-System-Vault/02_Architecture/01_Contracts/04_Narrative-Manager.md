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
