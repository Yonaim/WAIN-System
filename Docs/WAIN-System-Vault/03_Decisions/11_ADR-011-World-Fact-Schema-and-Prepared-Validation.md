# ADR-011 — World Fact Schema and Prepared Game State Validation (MVP)

- **상태:** Proposed (2026-11-12 코드 기반 구현안, 상세 정책 최종 승인 전)
- **관련:** [ADR-001](01_ADR-001-State-Ownership.md) · [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-005](05_ADR-005-Minimal-Game-Adapter.md) · [ADR-010](10_ADR-010-Deterministic-Beat-Graph-MVP.md)

## 00. 배경

Host Game이 Inventory, Story Flag, Quest 등을 소유한다는 원칙은 수립됐지만, 현재 `FWAINMapGameAdapter`는 임의의 등록된 정수 Key의 변경을 직접 처리한다. World State를 Character Context에 전달할 때 공개 범위와 Mutation 허용 범위가 분리되어 있지 않으면 캐릭터가 숨겨진 정보에 접근하거나 LLM 제안이 게임의 불변조건을 위반할 수 있다.

## 10. 검토 대안

1. 모든 Game-owned State를 WAIN 내부에 복제: 정본 이중화와 데이터 불일치 위험이 커 **배제**.
2. Host Adapter `Prepare()`에만 모든 검증 위임: 게임별 검증 코드를 중복 구현해야 하고 Context 공개 정책을 재사용하기 어려워 **보완 필요**.
3. **Host Schema + Snapshot Reader + World State Manager + Prepared 결과 재검증:** 기존 원자적 Publish 경계를 유지하면서 공통 검증 기능을 제공할 수 있어 **MVP 선택**.

## 20. 구현 선택

- `FWAINWorldFactDefinition`은 `Key`, `MinValue`, `MaxValue`, `bExposeToAgent`, `bWritableByProposal`을 가진다. 값 타입은 초기에는 `int32` 하나만 지원한다. Schema는 Host에서 전달한 정의, **실제 값은 Host의 Current/Prepared Game Snapshot만이 정본**이다.
- `IWAINWorldFactReader::ReadInteger()`를 World 및 Narrative 공통의 읽기 전용 계약으로 사용한다. 기존 `IWAINNarrativeGameFactReader`는 별칭으로 남겨 소스 호환성을 유지한다.
- `BuildContext()`는 공개 허용 Key만 값을 읽고 `FWAINCharacterContext::World.VisibleFacts`에 담는다. 숨겨진 Key는 이름도 담지 않는다. **이것은 LLM Context 데이터 필터이지 보안 경계가 아니다**. C++ Provider는 아직 `Current.Game` 객체도 인자로 받으므로, 무신뢰 Provider를 위한 권한 격리는 추후 필요하다.
- Transaction은 Game Adapter의 `Prepare()` **이전**에 Mutation을 Schema에 대조한다. 등록·쓰기권한·정수 연산·순차 변경·범위 조건을 모두 확인한다. 실패는 `InvalidProposal`, No Commit.
- Prepare 이후에는 **Schema 전체 Key**의 실제 Prepared 값을 읽어 예상값과 대조한다. 승인 Mutation이 없는 Key가 바뀌면 `GamePrepareFailed`, No Commit. `GameAdapter.Validate` 및 기존 WAIN/게임 공유 Root Publish는 변경하지 않는다.
- Host가 제공하는 Schema는 첫 `StartSession`에서 검사한다. Reader 부재, 빈 Schema, 중복 Key, Min > Max, 값 누락/범위 위반은 초기화 실패로 처리한다.
- Manager를 주입하지 않은 기존 Phase 1/2 Mock 경로는 종전 동작을 유지한다. 실제 World Context 제공 시 Provider는 `GenerateWithContext()`를 구현해야 한다.

## 30. 한계와 추가 결정

- 이번 MVP의 공통 Schema는 **게임 소유 정수 Fact**만 포함한다. WAIN-owned World Fact의 별도 저장·수명·Commit 정책 및 구조체/열거형/문자열 타입은 후속 설계 대상이다.
- World Manager는 Reader가 반환하는 **등록된 Key의 값만** 확인할 수 있다. Adapter가 미등록 내부 필드를 변경하거나 외부 DB/Actor를 실제 수정하는지까지는 검출하지 못하므로 Host의 부작용 없는 `Prepare` 의무와 공유 Root 계약은 계속 필요하다.
- `bExposeToAgent`는 Context 직렬화 범위이지 Narrative Rule에서 Fact를 참조할 수 있는지의 정책은 아니다. Narrative는 Author가 정의한 구조화 규칙으로 Game Fact를 읽는다.
- 권한별 사용자/캐릭터 분리, 조건부 공개, 별도 Mutation Reason, Schema Revision/SaveLoad, 실제 Testbed JSON 및 Blueprint API는 미구현이다.
- ADR이 `Proposed`인 이유는 게임 콘텐츠 및 운영 정책을 최종 승인하지 않았기 때문이며 **소스 작성과 UE 실제 컴파일·테스트 통과는 별개의 상태**이다.

## 40. 검증

신규 World State Automation 테스트 9개 작성. UE 5.8 실제 빌드 및 테스트는 실행하지 않았다. [구현 상세](../04_Development/11_World-State-Manager-Implementation.md).
