# World State Manager & External State Adapter

> 상태: **설계 채택, C++ Adapter 미구현.** [ADR-001](../../03_Decisions/01_ADR-001-State-Ownership.md) · [ADR-005](../../03_Decisions/05_ADR-005-Minimal-Game-Adapter.md).

## State Ownership

- WAIN-owned World Facts: Plugin 내부 Snapshot의 일부로 관리.
- Game-owned Inventory/Quest/Game Flags 등: Host 정본. Plugin이 중복 정본을 만들지 않는다.
- Game Adapter: Game State 읽기 **및** 승인 후보 변경의 준비/확정을 위한 경계. ReadOnly 전용으로 제한하지 않는다.

## 최소 MVP Adapter 계약 (개념, C++ API 미확정)

- `ReadSnapshot()`: 현재 Game-owned State에 대한 일관된 읽기 결과 제공.
- `PrepareChanges(Current, Mutations)`: **권위 State를 바꾸지 않고** Game Prepared Snapshot 및 오류 반환.
- `CanCommitWithinBoundary()`: Game Adapter가 현재 Session의 공유 Commit 경계에 실제로 참여할 수 있는지 확인.
- `PublishPrepared(...)`: 테스트베드에서 **WAIN과 Game을 함께** 공개하는 단일 Commit 경계에 참여; 단순 개별 순차 `Apply()`를 원자적 호출로 포장하지 않는다.

초기 Testbed는 공유 메모리 Snapshot 방식의 Adapter만 구현하고, 독립적인 Actor/외부 DB/Network 상태를 즉시 변경하는 Adapter는 지원하지 않는다. Plugin은 Game의 데이터 형태를 소유하지 않고, 다른 Host의 저장 구조를 단일 Root로 고정하지 않는다.

## World Fact 개념 스키마

`Key`, `ValueType`, `Owner`, `Default`, `Constraints`, `Mutability`, `Visibility`. 실제 USTRUCT 및 직렬화는 후속 설계.

[Commit Contract](05_Response-Validation-Commit.md) · [Data Contracts](08_Data-Contracts.md)
