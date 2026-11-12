# Implementation Roadmap (Proposed, MVP First)

> 개발 시 복잡한 범용 Adapter 프레임워크를 먼저 만들지 않는다. 지원 범위를 명시하고 작은 Testbed Snapshot 계약부터 검증한다.

| 단계 | 구현 목표 | 수용 / 관찰 기준 |
|---|---|---|
| 0 | Plugin 모듈 골격 및 Turn/Revision 기본 구조. [Top-Down Readability](03_Implementation-Principles.md) 준수, 포맷터 도구와 실제 UBT 사전 작업 연결 검토 | UE Editor Plugin 로드 및 포맷 실행 경로 별도 확인 |
| 1 | WAIN-owned Current / Prepared Snapshot Store | 검증 전 권위 State 무변경 |
| 2 | **Testbed 전용 최소 Game Snapshot Adapter** | Game Mutation 준비 중 Live State 무변경 |
| 3 | Transaction Coordinator 및 공유 Commit 경계 | WAIN/Game 동시 관찰; Prepare 실패·Revision 충돌·미지원 Mutation No Commit |
| 4 | Character Relationship / 6축 Emotion, Persona | 감정 범위·연속값·동시 감정 유지 |
| 5 | Host Presentation Catalog Query/Validation | Context에서 허용 ID만 생성 후보; 미지정/없는 Variant 정책 검사 |
| 6 | Memory Retrieval/Reflection/Planning | 장기 상태 지속성과 검색 검증 |
| 7 | Narrative Beat Graph/Goals/Constraints | Prepared State 평가·턴당 최대 한 전이 |
| 8 | Shared AI 및 End-to-end, Turn Trace | LLM/Schema 실패 시 No Commit; Commit 후 UI 실패 분리 |
| 9 | A~D 비교 평가 및 보고 | 동일 시나리오 기반 결과 기록 |

**추후 확장:** 독립 Unreal Actor/DB/Network State를 가로지르는 Transaction, 영속 Save/Load·Replay, 다중 Agent는 MVP 완료 후 별도 ADR과 테스트 근거로 추진. 이 로드맵은 구현 완료 기록이 아니다.

## 2026-11-10 구현 착수 표시 (설계가 아닌 소스 존재 기준)

| 대상 | 상태 |
|---|---|
| 0: `.uplugin` + `WAINSystem` Runtime Module | **소스 작성 / UBT 빌드 미검증** |
| 1: WAIN Current / Prepared Snapshot | **소스 작성 / Unreal 실행 미검증** |
| 2: 최소 Game Adapter | **Private Mock Map Adapter 작성 / 실제 Testbed 연동 전** |
| 3: Transaction / One-Root Commit | **소스 작성 / 실패 주입 Automation Tests 미실행** |
| 4~9: Character Emotion Delta, Catalog, Memory, Narrative Graph, LLM, 평가 | **미구현** |

매 빌드 전 포맷팅 도구는 Plugin `Tools`에 배치되어 있으나 실제 Testbed `Target.cs` hook 연결은 아직 이루어지지 않았다. `README.md`의 예시를 참조해 각 타깃에 추가하고 Live Coding 경로를 별도 검증한다.

## Phase 2 착수 업데이트 (2026-11-10)

- **Phase 1:** 사용자 제공 검증 소스를 Baseline으로 채택. 과거 표의 ‘빌드 미검증’은 당시 문서 이력임.
- **단계 4 일부:** Character Relationship `Trust`와 6축 Emotion `AddFloat` 적용 로직 및 검증 테스트 코드 작성 완료. Persona·Context Assembly는 미구현, Phase 2 부분 완료.
- **단계 5 이후:** 기존 로드맵 유지. Game-owned 상태에 일반 목적 float Mutation은 아직 허용하지 않음.
- **개발 환경 확인 필요:** Phase 2 신규 코드의 Unreal 컴파일 및 `WAIN.Transaction.*`, `WAIN.Character.*` Automation Tests 실행.

## Phase 2 Persona/Context 업데이트 (2026-11-10)

- 단계 4 확장: Host Persona 읽기 전용 Source·Context Builder·Context-aware Proposal Provider 계약과 Automation Tests 작성. Character Context의 **일부만** 구현, Unreal 실행 미검증.
- 단계 5 Presentation Catalog, 단계 6 Memory, 단계 7 Narrative, 단계 8 LLM/Prompt는 아직 구현하지 않았다.
- 초기 Context는 현재 확정 State 기반이며, Game-owned World State 및 장기 Memory는 포함하지 않는다.

## 2026-11-11 Phase 2 Presentation 진행 상태

- **단계 5 (Presentation):** Host Catalog 조회 및 Expression·Variant 검증의 C++ 최소 구현과 11개 Automation Test 코드를 작성했다(UE 실행 미검증).
- 아직 연결되지 않은 실제 게임 콘텐츠: `ManaExpressions.json` 로더, Pose·Stage/PNG Variant 선택, Host Widget 렌더러.
- **다음 단계 후보:** Host Game의 World State 조회 계약 또는 Memory Stream 최소 구현. LLM 연결은 Context의 기본 입력 출처를 충분히 확보한 뒤 진행.
- 이전 Roadmap 표에서 Catalog를 미구현으로 표시한 것은 작성 당시의 이력이며 현재 현황은 본 절을 따른다.

## 2026-11-11 Phase 2 Memory 진행 상태

- **단계 6 일부:** Snapshot 내부 Observation append·keyword fallback retrieval·Character Context 전달과 8개 Automation Test **코드 작성**.
- **아직 미구현:** Embedding Provider/Cosine, Reflection·Planning, 중요도 평가, 지속 저장·복원·압축, 실제 LLM Prompt 연결.
- **검증 필요:** 현재 Phase 2 변경분 전부 UE 5.8 컴파일 및 `WAIN.Memory.*` + 기존 30개 테스트의 실제 실행 결과.
- 이번 코드의 임시 검색 가중치는 Architecture의 최종 결정이 아니라 실험 파라미터다. [Memory 구현](08_Memory-Observation-Implementation.md).

## 2026-11-11 Phase 2 Reflection / Planning 진행 상태

- 단계 6 **MVP 일부 확장:** Reflection/Plan에 대한 주입형 결정론적 Reasoner 경로와 Scene/Beat 기반 Plan 유효성 검사가 소스에 포함됐다. 실 LLM 문장 생성, Embedding, Goal/World 변화 감지, Step 진행, 지속 저장은 **미구현**.
- Reflection/Plan 생성은 Prepared Snapshot에 포함되어 Commit 전 실패 시 원상태가 유지된다. [ADR-009](../03_Decisions/09_ADR-009-Reflection-Planning-MVP.md) Proposed.
- 신규 Automation Test 11개 작성, 전체 49개. UE 5.8 빌드·실행 검증 필요. 이후 단계 7의 Narrative Graph/Goal·Constraint 엔진으로 진행한다.

## 2026-11-11 Phase 2 Narrative Graph 진행 상태

- 단계 7: 구조화 정수 조건 기반 Beat Graph, 현재 Goal Context, Prepared 전이/Hard Rule을 **C++로 작성**했다. 기존 한 턴 한 Publish 정책 유지.
- 신규 테스트 10개 작성(누적 59개). 모든 Phase 2 변경분의 실제 Unreal 빌드·Automation 실행은 사용자 환경에서 확인해야 한다.
- **다음:** World State Manager의 Game Fact Query/Schema와 실제 Testbed 콘텐츠 로딩을 연결한 뒤 LLM/Response Validation을 확장. 완전한 Narrative Authoring 및 자유 형식 텍스트 Hard Constraint 검증은 차후 단계.
- [Narrative 구현](10_Narrative-Graph-Implementation.md) · [ADR-010](../03_Decisions/10_ADR-010-Deterministic-Beat-Graph-MVP.md).
