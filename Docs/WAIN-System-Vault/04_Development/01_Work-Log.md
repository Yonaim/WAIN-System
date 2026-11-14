# Work Log

## 2026-11-10 — Vault & Data Contract Initialization

- **입력:** `testbed.zip`, `Universal-Agent-Vault.zip`(v5.9), 기존 프로젝트 문서·중간발표 자료.
- **변경:** 플러그인 상세 설계·State/Emotion/Expression 경계 문서를 v5.9 구조로 재작성. 기존 문서·발표 PDF는 역사 자료로 보존.
- **Testbed 연계:** JSON 3개 추가(`EmotionDefinitions`, `ManaEmotion`, `ManaExpressions`), `Presentation.json` 메타데이터 확장, 상위 Docs 추가. PNG 파일명/코드 변경 없음.
- **검증:** 오프라인 JSON Parse, ID·Variant·PNG 참조 및 문서 링크 검사 예정/실행 결과는 산출물 검증 로그에 기재. Unreal Build/런타임/LLM 실험 미실행.
- **주의:** 설계 문서 및 런타임 구현 현황은 별개. 새로운 플러그인 Source 생성하지 않음.


## 2026-11-10 — 최신 Universal Vault 동기화 (문서/설정만)

- **입력:** 사용자 제공 `Universal-Agent-Vault(2).zip`, 현재 WAIN `Docs(1).zip`.
- **변경:** 범용 Engineering Standards·Obsidian(테마/플러그인/설정)·VS Code·Sortspec·Formatting/Build Templates·Copilot Skills를 최신본 기준으로 갱신. WAIN의 프로젝트별 AGENTS/README/Document Map/Constraints/Implementation Principles와 설치 가이드 정비.
- **유지:** 기존 Architecture/Contracts, ADR-001~006, Research/PDF, Legacy Archive 및 구현 현황은 그대로 보존.
- **검증:** ZIP 구성·동기화 대상 파일 일치·JSON 파싱·기존 ADR 바이트 보존·Markdown 상대 링크 검사. 코드 변경, Unreal 빌드·플러그인 실행·포맷 훅 실행은 수행하지 않음.
- **개인 상태 제외:** Git 내부 데이터, Copilot 캐시·과거 대화, 계정/장치 식별자 및 저장된 인증 정보는 복제하지 않음.

## 2026-11-10 — Phase 1 C++ Runtime 코드 작성 (최신 작업)

- **읽은 기준:** WAIN Vault `AGENTS.md`, `01_Document-Map.md`, Top-Down Readability, ADR-001~006, Main Flow, Data/Commit Contracts, C++/Source Comments, UBT/Formatting Guide 및 현재 Testbed 모듈 소스.
- **코드 변경:** 새 WAIN Runtime Plugin Module, Public DTO/Interfaces/Subsystem, Private Snapshot Store/Validation/Transaction/Interaction Processor, 테스트용 Map Game Adapter, Automation Test 6개.
- **Top-Down Readability:** `ProcessTurn` 및 `TransactionCoordinator::Execute`를 순서가 보이는 상위 함수로 유지. 세부 Mutation·검증·Root 공개는 하위 책임에 위임.
- **문서 갱신:** Source Map, Data/Transaction Contracts, Main Flow, Current Work/Roadmap, Test Plan/Validation, Work Log. ADR-001~006의 결정은 변경하지 않음.
- **안전 경계:** Host/게임 고유 JSON과 Testbed Widget 변경 없음. 미지원 Action·Presentation/State Mutation 거절, 테스트 Adapter는 비공개 Mock 전용.
- **검증:** Python 스크립트 구문 검사, C++ 헤더 경로·기초 브레이스 검사, 24개 JSON 파싱, Markdown 253개 상대 링크 무결성을 확인함. `clang-format` 부재로 포맷 검사는 완료하지 못함. **Unreal 엔진 5.8, UBT, C++ 컴파일, Automation Tests, Live Coding는 미실행.** 실제 포맷터 실행 성공 여부는 산출물 검증 기록 참고.

## 2026-11-10 — Phase 1 코드 Region 구성 보완

- `00_Engineering-Standards/00_Core/01_Local-Numbering-and-Navigation.md`를 근거로 여러 책임을 포함한 C++ 소스에 **3줄 구분 주석 + 지역 번호가 있는 `#pragma region`**을 추가했다.
- 주로 Interaction Processor, Transaction Coordinator, 공통 데이터 타입·인터페이스, Mock Adapter, Automation Tests의 기능 구획을 만들었다. 작은 단일 책임 파일에는 무의미한 Region을 강제하지 않았다.
- 변경 범위는 탐색성 및 문서 보완. 의도적으로 실행 로직, API 및 ADR을 수정하지 않았다.
- **남은 검증:** 실제 UE 5.8 컴파일/테스트, MSVC 및 에디터의 Region 처리, `clang-format → indent_regions.py` 2회 반복 안전성 검증, UBT PreBuildSteps 연결. 이 항목을 수행했다고 주장하지 않는다.
- LLM API 키를 포함한 보안 설정은 [LLM Configuration Plan](04_LLM-Configuration-Plan.md)에 별도 정의했으며 Phase 1에서는 사용하지 않는다.

## 2026-11-10 — Phase 2 Character State Delta (Phase 1 검증본 기반)

- 기준 소스: `WAINTestbed-Source(1).zip` (사용자가 Phase 1 검증본이라고 확인).
- 변경 파일: `WAINRuntimeTypes.h`, `WAINTransactionCoordinator.cpp`, `WAINRuntimeTests.cpp`, 신규 `Character/WAINCharacterStatePreparer.{h,cpp}`.
- 설계 준수: ADR-001·ADR-002·ADR-003·ADR-004의 기존 결정 유지. 구현 범위 확장이므로 새 ADR은 발행하지 않음.
- 구현: six-dimensional independent Emotion Delta(유한 float, 0~1 Clamp), Trust 범위 유지, 불량 변경 No Commit, Region 구획 및 Top-Down 처리.
- 회귀 시험: Phase 1 테스트 6개 보존, Character Automation Tests 6개 추가(코드만 작성).
- 검증: 업로드 소스와 신규 변경분의 파일·헤더 경로·Region 짝·테스트 등록을 정적으로 확인. Unreal 5.8 Build/Automation 테스트는 **이 환경에서 수행하지 않음**.

## 2026-11-10 — Phase 2 Character Context 확장

- 입력: 직전 Phase 2 검증 전 코드 패키지(`WAINSystem-Phase2-Plugin-and-Vault.zip`)를 변경 기준본으로 사용. Phase 1 검증 기반을 보존.
- 코드: Host Persona Source 및 Context DTO/Builder 추가, `WAINInteractionProcessor`에 Context Build를 Proposal 이전에 삽입, 기존 `StartSession`에 Source 선택 인자 추가, `IWAINProposalProvider`의 Context-aware 생성 경로 확장.
- 결정: Host Persona 정본과 WAIN State 정본의 분리 및 Persona 주입 시 무시 금지 정책을 [ADR-007](../03_Decisions/07_ADR-007-Read-Only-Character-Context.md)로 기록.
- 테스트: Context 관련 7개 Automation Tests 작성. 기존 12개는 소스 변경 없이 유지.
- 불변 범위: WAIN Transaction Coordinator/State Store/Character Mutation, Host Testbed 소스·JSON, Obsidian 설정 전부 변경하지 않음.
- 미검증: Unreal Engine 5.8 Build, 실제 테스트 실행, JSON Loader/Prompt/LLM 연동.

## 2026-11-11 — Phase 2 Presentation Catalog 조회 및 Expression 검증

- 입력: 직전 `WAINSystem-Phase2-Persona-Context-Plugin-and-Vault.zip`의 Plugin 전체 소스를 기준으로 변경.
- 코드: Host Catalog Public DTO/Interface 및 Private Validator를 추가하고 Character Context Builder → Proposal Provider → Transaction Coordinator → Interaction Result 흐름에 연결.
- ADR: [ADR-002](../03_Decisions/02_ADR-002-Emotion-Expression.md)의 구현 상태만 갱신. 원래 승인된 Emotion/Expression 소유권은 변경하지 않음. [ADR-007](../03_Decisions/07_ADR-007-Read-Only-Character-Context.md)에는 Context 확장 이력 추가.
- 테스트: `WAIN.Presentation.*` 11개 신규 등록 코드 작성, 기존 19개 테스트 소스 유지.
- **검증 범위:** Python 기반 구조·참조·JSON·Markdown 링크·Region 짝 검사. UnrealBuildTool/UE 5.8 실제 C++ 컴파일·테스트·Live Coding은 실행하지 않음.
- 변경하지 않음: Host Testbed UI·JSON·PNG, Unreal Build Hook, Obsidian 테마/플러그인 설정.

[Implementation](07_Presentation-Catalog-Implementation.md)

## 2026-11-11 — Phase 2 Observation Memory (저장·검색·Context)

- 기준: 직전 `WAINSystem-Phase2-Presentation-Catalog-Plugin-and-Vault.zip` 누적 Plugin 및 Vault.
- 추가: 통합 `FWAINMemoryRecord`, `MemoryStream` Snapshot, `FWAINMemoryWriter`, `FWAINMemoryRetriever`, Context `RelevantMemories`.
- 처리: 성공 Transaction에만 Player/Character 발화를 기록하고, 다음 Turn은 확정 Memory에서 캐릭터별 Observation을 검색한다. 실패/Replay/다른 Character가 Stream을 오염시키지 않도록 처리.
- 데이터: Observation만 구현. 임시 키워드·최근성·Importance 점수 및 상위 6개 설정을 구현 세부사항으로 명시, Embedding/Reflection/Plan은 미구현.
- 테스트: `WAIN.Memory.*` 8개 신규 소스 작성, 기존 30개 유지 (총 38개). 신규 구현분은 **Unreal 5.8에서 컴파일·실행 미검증**.
- ADR: [ADR-008](../03_Decisions/08_ADR-008-Transactional-Observations.md) **Proposed** 추가; 기존 Accepted ADR-001~007은 변경하지 않음.
- Obsidian/VS Code 설정, Host Testbed 코드/JSON/PNG 및 기존 Runtime API 호출 방식은 유지.

[Implementation](08_Memory-Observation-Implementation.md)

## 2026-11-11 — Phase 2 Memory Reflection / Planning MVP

- 기준: `WAINSystem-Phase2-Memory-Plugin-and-Vault.zip`.
- 변경: `IWAINMemoryReasoner` 계약, `FWAINMemoryReasoningPreparer`, Memory Plan 필드와 ActivePlanIds, Search/Context, Final State Validation, Transaction 주입 경로.
- 정책: 최초/Scene/Beat 변경 시 계획 갱신, Reflection 임계값, 근거 ID와 순서 검증. **생성기 주입 없으면 이전 기능 그대로**.
- 신규 자동화 테스트 11개 작성(누적 49개). 현재 환경에서 UE 5.8 컴파일/테스트를 실행하지 못했으므로 정적 검사 결과만 기록한다.
- 문서: [ADR-009](../03_Decisions/09_ADR-009-Reflection-Planning-MVP.md) Proposed. 표준 Obsidian/VS Code/테마 설정과 Host Testbed 코드는 변경 없음.

[구현 상세](09_Memory-Reasoning-Implementation.md).

## 2026-11-11 — Beat Graph MVP 작성

- 직전 Reflection/Planning Plugin Source/Vault를 기준으로 신규 Narrative DTO·Evaluator·Fact Reader·준비 중 BeatHistory 및 Context 연결.
- Graph 정의 오류, Prepared Trust/Game Fact에 의한 전이, Hard Rule 실패 No Commit, 동일 우선순위 충돌, 재전송 무중복 등 `WAIN.Narrative.*` 테스트 10개 작성(총 59개 코드 등록).
- 기존 StateStore Publish·Transaction Coordinator의 단계 순서는 보존. 코드 포맷 체인은 개발 환경에서 `clang-format → indent_regions.py` 순서로 실행하는 것으로 가정하며 여기서는 UE 컴파일/Automation 실행 미검증.
- 새 MVP 정책은 [ADR-010](../03_Decisions/10_ADR-010-Deterministic-Beat-Graph-MVP.md)에 `Proposed`로 작성.

## 2026-11-12 — World State Manager & Game Adapter 검증 경계

- 직전 Narrative Graph 전체 Plugin Source/Vault의 코드에 World Fact 관리 계층 추가.
- Reader 공통화(`IWAINNarrativeGameFactReader` 별칭), World Context 필터링, Schema 기반 Mutation/Prepared State 검증, 선택적 Manager 주입으로 기존 Mock 호환.
- 테스트 `WAIN.World.*` 9개 작성(누적 68개 등록). Region·코드 주석·문서·ZIP 검증, UE 빌드/Automation 실행 미검증.
- ADR-011 Proposed 기록, 기존 ADR·Engineering Standards·Obsidian 설정 유지. [구현 상세](11_World-State-Manager-Implementation.md).

## 2026-11-12 — Host JSON/Session Integration

- 기준: World State Manager Plugin 누적본 + `WAINTestbed-Source(1).zip`. Plugin C++ 코드는 수정하지 않음.
- 신규 Host JSON Loader, Game Snapshot/World Reader, Persona/Expression Sources, Narrative Guard, Mock Proposal, Session Bootstrap, Widget Commit View 연결.
- ManaEmotion을 6D 초기값으로 변경, CH01WorldFacts 신규 추가, CH01Narrative Goal 필드 추가. PNG 원본/경로 보존.
- `WAIN.Testbed.*` 7개 Editor Automation Tests 코드 작성. Plugin 기존 68개 유지, 실제 UE 5.8 컴파일/실행 미검증.
- [ADR-012](../03_Decisions/12_ADR-012-Testbed-Content-Bridge.md) Proposed. 실제 Host 문서 [Integration](12_Testbed-Content-Integration.md).

## 2026-11-13 — Host Layered Sprite Renderer and Content Cleanup

- 기존 Testbed 통합 소스를 기준으로 `Content` 폴더, 파츠 PNG 파일명을 구조화했다. 완성 PNG 27개와 배경 10개는 바이트 불변 이동, 미완성 6개는 동일 RGBA Canvas의 완전 투명 PNG.
- JSON: `basePath = Images/Characters/Mana/Layers`, `path = <part_id>.png`, `ready/note` 메타데이터. Expression/Variant ID는 유지.
- C++: Host Content Loader에서 파츠·Variant DTO 구성, Renderer에서 UMG 이미지 슬롯과 Texture2D 동기 로드, Commit Response 표정 연동.
- 테스트: `WAIN.Testbed.Sprite.*` 6개 추가, 총 81개 등록. UBT·PIE 미실행.
- ADR-013 Proposed. 기존 Plugin/Engineering Standards/Obsidian 설정 불변. [Host 구현 문서](../../../../../Docs/02_Implementation/03_Visual-Asset-System.md).

## 2026-11-13 — Cross-Module Runtime Exports and Editor Build

- `FWAINWorldStateManager`와 `FWAINBeatGraphNarrativeEvaluator`를 Host 모듈에서 사용할 수 있도록 `WAINSYSTEM_API` export를 추가했다. 공개 함수 서명과 동작 계약은 변경하지 않았다.
- UE 5.8 `WAINTestbedEditor Win64 DebugGame` 빌드 성공. Plugin/Host C++ 컴파일 및 링크 완료.
- Pre-build formatter의 44개 C/C++ 파일 결과를 반복 실행 해시로 비교해 내용 변경이 없음을 확인했다. Automation/PIE는 실행하지 않았다.

## 2026-11-13 — DEV Inspector / Bounded Turn Trace

- 기준: 사용자가 업로드한 `WAINTestbed-Source(2).zip` (현재 소스 기준).
- Runtime `WAINInteractionDebug` 추가, Processor/Coordinator 실행 단계와 실제 Proposal/Context 수집.
- Testbed `WAINTestbedDebugInspector` 및 기존 DEV 버튼에 State/Narrative/Memory/Trace + 턴 선택 UI 바인딩. 실패한 턴의 미확정 State나 미실행 단계를 추정하지 않음.
- 7개 Automation 테스트 소스 작성(기존 86개 + 신규 7개, 누적 93개). 현재 환경에서 Unreal 컴파일/PIE/Automation **미실행**.
- Obsidian 설정·Engineering Standards 및 Host Content 에셋 미변경.

[ADR-015](../03_Decisions/15_ADR-015-Turn-Diagnostics-and-DEV-Inspector.md) · [구현](11_Turn-Debug-Trace-Implementation.md).

## 2026-11-13 — Async LLM Provider + Structured Output

- 현재 DEV Inspector 소스 기준 `TurnNavigation` 지역 변수명을 수정.
- WAIN Context Preview/JSON Schema/Strict Parser, Testbed 비동기 HTTP Provider + Revision Queue, Widget 요청 대기 상태/오류 처리를 연결했다.
- `OpenAI Responses API` 호출은 명시적인 환경변수 모드로만 허용. DEV Trace는 기존 Pipeline Commit의 실제 Stage를 기록한다.
- 13개 자동화 테스트 코드 작성(누적 106개). Unreal Build/PIE/API 실증은 미검증.
- [ADR-016](../03_Decisions/16_ADR-016-Asynchronous-LLM-Proposal-Handoff.md).


## 2026-11-14 — indent_regions.py 사용자 제공본 교체

- **읽은 기준:** Root README, Vault AGENTS·Document Map·Code Style·Pre-Build Formatting 및 기존 스크립트.
- **변경:** `Tools/indent_regions.py`와 `Assets/Formatting/indent_regions.py`를 사용자 첨부 원문으로 교체.
- **문서 검토:** CLI·포맷 계약·소스 경로 변경이 없어 관련 설계·소스 맵·테스트 계획은 유지.
- **검증:** 두 파일의 첨부 원문 바이트 일치 확인 및 Python AST 구문 검사 통과. 파일 교체 범위이므로 Unreal 빌드와 전체 소스 포맷 실행은 생략.

## 2026-11-14 — Plugin LLM Ownership and Host Source Layout

- `WAINTestbedLLM*` 및 Host Usage Monitor/Config 구현을 Plugin `Public/LLM`, `Private/LLM`으로 이동하고 `FWAINLLMTurnService`를 추가했다.
- Testbed C++를 `Gameplay/Data/Integration/Presentation/UI/Tests`로 재배치하고 Host HTTP 의존성을 Plugin으로 옮겼다.
- `Content/` 데이터·이미지와 로컬 비밀키는 보존했다. ADR-018, Architecture, LLM/Usage 문서를 갱신했다.
- 정적 변경 검토만 수행했다. UE 5.8 빌드, Automation, PIE, API 실호출은 실행하지 않았다.
