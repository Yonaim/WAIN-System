# Current Work — WAIN Plugin

**초기 기록 기준:** 2026-11-10 제공 `testbed.zip` 스냅샷. 아래 첫 두 절은 **Runtime 구현 이전 문서 상태**다. 현재 상태는 문서 끝의 Phase 1 기록을 참조한다.

## 설계 결정 / 문서화

- [ADR-001~006](../03_Decisions/00_ADR-Index.md): State Ownership, 6축 Emotion/Host Catalog, Interaction Transaction, WAIN Snapshot, 최소 Game Adapter, Single-Agent Turn-Based 범위를 정리.
- Commit은 `Response Validation → Next State Preparation → Narrative Evaluation → Final Validation → Atomic Commit → VN Response`. Context Preparation에서 허용 Presentation 조회.
- **초기 Testbed**는 공유 Commit 경계에 참여하는 메모리 Snapshot Adapter에 범위를 제한. 독립 Actor/DB/Network의 범용 원자성은 보장하지 않음.

## 초기 문서 기준 미구현 / 미검증 (이력)

`.uplugin`, Plugin `Source`, Orchestrator, Agent, Snapshot Stores, Transaction Coordinator, Game Adapter, Shared AI, Validator, Catalog Query, 실제 PNG Renderer 모두 구현을 확인하지 못함. 기존 Testbed Mock과 혼동하지 않는다. 기존 Testbed Emotion `Type/Intensity` JSON은 **6축 계약으로 아직 변경되지 않음**.

## 최소 구현 순서

1. Plugin 모듈 골격과 Turn ID/Revision·기본 DTO.
2. WAIN Current/Prepared Snapshot 및 Testbed의 작은 Game Snapshot Adapter.
3. **두 Prepared State의 단일 Commit 경계** 구현 후 실패·중복 Turn 테스트.
4. Expression Catalog 조회/검증, Character/Memory/Narrative 규칙 점진 연결.
5. LLM과 End-to-end·평가 추가. 범용 외부 저장소 Transaction은 추후 별도 ADR.

[Roadmap](02_Implementation-Roadmap.md) · [Test Plan](../05_Experiments/00_Test-Plan.md)


## 2026-11-10 — 구현 준비를 위한 문서 인프라 동기화

- 최신 Universal Agent Vault의 Engineering Standards·Obsidian 설정·편집기 설정·포맷/빌드 템플릿·Copilot 스킬을 WAIN 문서 볼트에 통합했다.
- **프로젝트 최우선 코드 원칙:** [Top-Down Readability](03_Implementation-Principles.md). 상위 Turn 흐름을 먼저 읽고 세부 구현으로 내려가는 구조를 권장한다.
- **미실행:** Unreal UBT/Live Coding, `.clang-format`·`Tools` 실제 설치, 빌드, 자동 포맷 파이프라인. 이 ZIP은 WAIN Runtime 소스·빌드 설정을 수정하지 않는다.
- 이후 구현 목표와 ADR-001~006의 지원 범위는 변경하지 않는다.

## 2026-11-10 — Phase 1 Mock Runtime 소스 작성

- **작성:** `.uplugin`, UBT Runtime Module, `UWAINRuntimeSubsystem`, DTO/Interfaces, `WAINInteractionProcessor`, `WAINStateStore`, `WAINTransactionCoordinator`, 공통 State Validation, Map Mock Adapter, Unreal Automation Test 6개.
- **지원:** Game Thread 동기 Mock 처리, Snapshot 원자 게시(공유 Root 내부), Revision Guard, 동일 Turn ID 재전송 캐시, 초기 6축 Emotion 값 유효성 검사, 제한된 Trust·Game 정수 Mutation.
- **미지원:** 실제 Host Testbed 연동, LLM Async, Memory, Emotion Delta, Presentation 검증, 완전한 Narrative Graph, 독립 Actor 원자 Commit, Save/Load.
- **검증 상태:** Unreal 5.8 빌드/런타임·Automation Test는 사용자 개발 환경에서 수행해야 함. 정적 검사 여부는 아래 Work Log 및 새 ZIP 검증 기록 참고.
- 위 문서 상단의 **'현재 미구현' 표는 코드 작성 이전 시점의 이력**이며, 최신 상태는 이 절을 기준으로 한다.

## 2026-11-10 — Phase 1 검증본 인수 및 Phase 2 첫 기능 작성

- **입력 기준:** 사용자 제공 `WAINTestbed-Source(1).zip`을 Phase 1 검증된 소스로 취급한다(사용자 확인, 개별 빌드/테스트 로그는 이 작업에 첨부되지 않음).
- **작성:** Character 전용 `FWAINCharacterStatePreparer`, `AddFloat`/`FloatValue`, 6축 Emotion Delta/Trust Prepared State 처리, Automation Test 6개 추가.
- **보존:** Phase 1의 Shared Root Publish·Game Adapter·TurnId 중복 보호 및 기존 Automation Tests 6개.
- **아직 미지원:** Persona/Context Assembly, Memory, Presentation Catalog, LLM, 실제 Testbed UI 연동. **Phase 2 추가분의 Unreal 5.8 빌드/테스트 미검증.**
- **다음:** 사용자 Unreal 환경에서 `WAIN.Transaction.*`, `WAIN.Character.*` 전체 테스트 실행 및 Persona/Context 설계 연결.

[Character Delta 구현 문서](05_Character-Delta-Implementation.md)

## 2026-11-10 — Phase 2 Persona + Character Context Assembly

- **구현:** Host-owned `IWAINPersonaSource`, `FWAINCharacterPersona`, 읽기 전용 `FWAINCharacterContext`, `FWAINCharacterContextBuilder` 및 `GenerateWithContext` 실행 경로.
- **호환성:** Source 미주입 기존 Mock Provider 동작 유지. Source가 주입됐는데 Context 미지원 Provider를 쓰면 Persona를 무시하지 않고 `ProposalGenerationFailed` 반환.
- **구현 범위:** 현재 Trust·6차원 Emotion·Active Beat·Revision·Scene/Turn Count와 Host Persona만 합성. Memory/World/Expression Catalog/LLM/JSON Loader 없음.
- **테스트:** Persona 및 Committed State 포함, 다음 Revision 갱신, Persona 실패/ID 불일치 No Commit, 기존 Provider 호환/거절, Replay 시 재조회 없음 등 7개 테스트 코드를 작성(총 19개).
- **검증:** Python 기반 정적 검사 예정. Unreal 5.8 컴파일과 Automation Test 실행은 미검증.

[ADR-007](../03_Decisions/07_ADR-007-Read-Only-Character-Context.md) · [Implementation](06_Character-Context-Implementation.md)

## 2026-11-11 — Phase 2 Host Presentation Catalog / Expression Validation

- **구현:** Host Catalog 읽기 전용 인터페이스, Expression+Variant 허용 목록, Context Preparation 조회/검증, Provider Context 전달, Transaction 사전 검증, 성공 Result에 선택 ID 반환.
- **보존:** 기존 Snapshot/Transaction/Character/Persona 경계, Catalog 없는 Phase 1 동기 Mock 경로, 미검증 Action 거절.
- **추가 테스트:** `WAIN.Presentation.*` 11개 코드 작성, 총 30개. 실제 UE 5.8 컴파일 및 테스트 실행은 별도 필요.
- **미구현:** Host JSON Loader, PNG 연출, 비동기·LLM, Memory·World/Beat 조건식, Catalog Revision 재확인.

[ADR-002](../03_Decisions/02_ADR-002-Emotion-Expression.md) · [구현 문서](07_Presentation-Catalog-Implementation.md)

## Phase 2 Memory Observation (2026-11-11)

- 통합 Snapshot MemoryStream과 Observation Writer/Retriever, Character Context 연결을 코드로 추가.
- 실제 UE 컴파일/Automation 통과 미검증. 확정된 구현 완료로 표시하지 않는다.
- 다음 작업: Unreal 실행 검증 후 Embedding/Reflection/Planning, Host Adapter 및 Narrative 규칙 순차 확장.
- [구현 계약](08_Memory-Observation-Implementation.md) · [설계 검토 ADR](../03_Decisions/08_ADR-008-Transactional-Observations.md)
