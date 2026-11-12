# ADR-003 — Interaction Transaction & Commit Before Response

- **상태:** Accepted (설계, 미구현)
- **날짜:** 2026-11-10
- **관련 요구:** FR-07, FR-08, FR-10~12, NFR-02, NFR-06

## 맥락과 문제

LLM Proposal을 그대로 실행하거나 VN Response를 먼저 출력하면 성공하지 않은 상태 변경을 게임이 이미 일어난 사건으로 묘사할 수 있다. 같은 턴의 Relationship, Game Inventory, Narrative Progress가 서로 다르게 확정되면 안 된다.

## 고려한 선택지 및 트레이드오프

- Proposal 즉시 Apply: 빠르지만 검증 실패 및 부분 반영에 취약하다.
- 적용 후 백업 Rollback: 외부 부작용의 완전 복구를 보장하기 어렵다.
- **Interaction Transaction + Prepared State (선택):** 선행 검증·준비 비용이 생기나 Commit 전까지 권위 상태를 보존한다.

## 결정과 근거

하나의 Player Interaction이 하나의 **논리적 Transaction**을 구성한다.

`Player Input → Context Preparation → Response Proposal → Response Validation → Next State Preparation → Narrative Evaluation → Final Validation → Atomic Commit → State Committed Notification → VN Response`

- Proposal에는 Dialogue/Action/Expression/Event와 State Mutation이 포함되며 모두 신뢰되지 않는 입력이다.
- Response Validation은 Schema, 소유권·게임 규칙, Hard Narrative Constraint, Host Expression ID를 확인한다.
- Preparation은 Current Snapshot을 바꾸지 않고 Character/World/Game 변경 후보를 준비한다. Narrative Evaluation은 이 후보를 참조해 Beat 전이를 계산하고 Final Validation에서 전체 결과를 검증한다.
- **Commit 성공 전에 정상 성공 VN Response를 반환·표시하지 않는다.** 오류 시 커밋되지 않은 대사를 정상 응답으로 전송하지 않는다.
- Runtime은 단일 세션 순차 Turn과 Revision 검사를 기본으로 한다. Commit 완료 알림과 연출성 Action/Event는 Commit 이후에 수행한다.
- Commit 후 Display/UI 실패는 상태 Commit 실패나 자동 Rollback으로 취급하지 않는다.

## 예상 결과 및 영향

Commit 전 오류는 권위 State를 바꾸지 않는다. 여러 소유 저장소에 걸친 실제 원자성은 `Prepare()` 성공만으로 얻을 수 없으며 [ADR-005](05_ADR-005-Minimal-Game-Adapter.md)의 **허용된 Game Adapter 범위 및 공유 Commit 경계**가 있을 때에만 보장할 수 있다. 지원하지 않는 Adapter의 Game Mutation을 성공으로 취급하지 않는다.

## 재검토 조건 / 대체 ADR 링크

네트워크·외부 DB·다중 World/Actor 시스템으로 확장할 때 Commit/복구 보장 수준을 다시 설계한다. 재시도 횟수, 멱등성 자료형, 타임아웃은 세부 API 설계로 남긴다.
