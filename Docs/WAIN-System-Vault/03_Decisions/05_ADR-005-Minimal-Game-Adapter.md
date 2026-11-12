# ADR-005 — Minimal Transactional Game Adapter (MVP First)

- **상태:** Accepted (설계 방향, 미구현·미검증)
- **날짜:** 2026-11-10
- **관련 요구:** UR-03, UR-05, FR-07, FR-11~12, NFR-02~03, NFR-06

## 맥락과 문제

WAIN 내부 Snapshot은 원자적으로 교체할 수 있어도 Host Game의 Inventory/Quest 등은 다른 저장소에 있을 수 있다. 두 State 각각의 적용 가능성을 확인했다고 해서 **두 번의 별도 Apply**가 자동으로 원자적이지는 않다. 반면 졸업프로젝트 MVP에 범용 분산 Transaction/보상 프레임워크를 도입하는 것은 과도하다.

## 고려한 선택지 및 트레이드오프

- 개별 `Apply()` 두 번 호출: 간단하나 한쪽만 적용되는 부분 Commit 위험이 있다. **원자성 요구를 충족하지 못하므로 채택하지 않음.**
- 모든 Host에 단일 Root 저장 구조 강제: Testbed는 단순하지만 범용 Plugin 설계를 제한한다.
- 완전 범용 2단계 Commit/Compensation: 기존 Actor/외부 부작용까지 고려해야 하므로 초기 범위를 넘는다.
- **허용된 Snapshot 참여형 Adapter를 MVP에서 먼저 지원하고, 인터페이스는 확장 가능하게 유지 (선택).**

## 결정과 근거

1. Game-owned State는 계속 Host의 정본이며 WAIN이 Inventory/Quest 로직을 가져오지 않는다.
2. Host는 Game Mutation에 대해 변경 준비 가능 여부와 변경 후 **Prepared Game Snapshot**을 제공하는 최소 Adapter를 구현한다. `Prepare`는 게임의 권위 상태나 외부 부작용을 변경하지 않는다.
3. **초기 Testbed만** WAIN Prepared Snapshot과 Game Prepared Snapshot을 **하나의 공유 Commit 경계에서 공개**하도록 구성한다. 구현 후보는 두 Snapshot을 함께 가리키는 Session Root의 단일 교체다. 이 Root는 **Testbed의 Commit 구현 세부사항**이며 모든 Host에 동일한 저장 방식을 강제하는 Plugin 공개 API가 아니다.
4. 게임의 Actor/UI 등은 해당 Commit 결과를 읽기/표시하는 View로 취급하고, 별도 권위 Inventory/Quest를 즉시 변경하는 `Apply()`를 원자적 경계인 것처럼 사용하지 않는다.
5. Transaction Coordinator는 준비·검증 성공, Revision 일치 및 Adapter의 **실제 Commit 지원 능력**을 확인한 경우에만 Commit을 허용한다. 지원하지 않거나 검증되지 않은 Game Mutation은 거절한다.
6. **외부 DB·Network·다수 독립 Actor의 원자적 변경은 초기 범위에서 제외한다.** 추후 확장 시 다중 저장소 조정, 복구/보상, 실패 주입 테스트를 새 ADR로 다룬다.

## 예상 결과 및 영향

초기에는 Testbed의 작은 메모리 State에 범위를 제한하여 단순한 구현으로 실제 상태 일관성을 검증할 수 있다. 다만 임의 게임의 Adapter를 붙인다고 전체 Atomic Commit이 저절로 보장되는 것은 아니다. **기존 Unreal 시스템에 대한 범용 원자성은 미지원/미검증**으로 문서화한다.

## 수용 기준 및 재검토 조건

- 미지원 Game Mutation 거절; `Prepare` 실패 시 양쪽 State 무변경.
- 준비 성공 후 Final Validation 실패 시 양쪽 State 무변경.
- 유효한 Trust + Inventory + Beat 변경은 하나의 Revision에서 함께 관찰됨.
- Duplicate Turn/Revision 충돌 및 Commit 후 UI 실패 시 상태 정책 검증.
- 초기 Testbed 이외의 Host가 필요해질 때 Transaction Capability·복구 전략·지원 범위를 별도 ADR로 확장한다.

## 구현 연결 기록 (2026-11-10)

`IWAINGameSnapshotAdapter` 인터페이스와 `Private/Testing/WAINMapGameAdapter`를 작성했다. **Map Adapter는 테스트 전용**이며 실제 Host Testbed Adapter/Widget은 아직 연결하지 않았다. 독립된 Game Actor·저장소 간 원자적 Commit은 여전히 미지원이다. ADR의 선택과 수용 기준은 유지한다.
