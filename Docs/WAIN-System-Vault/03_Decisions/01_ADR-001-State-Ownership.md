# ADR-001 — State Ownership & Host Adapter

- **상태:** Accepted (설계, 미구현)
- **날짜:** 2026-11-10
- **관련 요구:** UR-03, UR-05, FR-06, FR-07, FR-08, FR-11, NFR-01~03

## 맥락과 문제

Plugin이 게임의 Inventory/Quest/Game Flags를 별도 정본으로 보관하면 Host와 WAIN 상태가 불일치할 수 있다. 반대로 Game-owned State를 읽기 전용으로 제한하면 대화에 따라 아이템을 소모하거나 퀘스트를 갱신할 수 없다.

## 고려한 선택지 및 트레이드오프

- WAIN이 모든 State를 소유: Commit 단순하나 Plugin 재사용성과 게임 소유권을 해친다.
- Host가 모든 State를 소유: WAIN의 Character/Memory/Narrative 일관성을 Host 구현에 종속시킨다.
- **소유권 분리 + Adapter (선택):** 정본은 각각의 도메인에 남기되, Transaction을 통한 Game-owned 읽기·쓰기 요청을 지원한다.

## 결정과 근거

- **WAIN-owned:** Character Relationship / Emotion / Memory / Plan, Narrative Active Beat / Progress, WAIN 내부 World Facts.
- **Host-owned:** Inventory, Quest, Game Flags 및 게임 고유 World State; Persona/Narrative Graph/Presentation Catalog/PNG 등 콘텐츠 정의.
- Host는 WAIN의 Relationship 및 Narrative 상태를 공개 조회 API·Commit 완료 알림으로 읽는다. 정본을 중복 소유하지 않는다.
- Game-owned State는 Adapter를 통해 조회하고 승인된 Mutation의 준비 및 확정에 참여할 수 있다. **ReadOnly로 제한하지 않는다.**
- LLM 응답은 어디까지나 Proposal이며 직접 WAIN/Host State를 변경하지 못한다.

## 예상 결과 및 영향

게임 콘텐츠 교체가 쉬워진다. 대신 Adapter가 제공하는 읽기·쓰기·Transaction 능력에 따라 지원 가능한 Game Mutation이 달라진다. 단순히 개별 `Apply` API를 호출하는 것으로 전체 Atomic Commit을 보장하지 않는다.

## 재검토 조건 / 대체 ADR 링크

Game-owned State 변경을 실제로 구현할 때 [ADR-005](05_ADR-005-Minimal-Game-Adapter.md)의 초기 지원 범위를 따른다. 외부 DB/다수 Actor에 대한 범용 트랜잭션이 필요해지면 별도 ADR을 작성한다.
