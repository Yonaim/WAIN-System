# WAIN System — Documentation Vault Changelog

## 2026-11-10 — Latest Universal Agent Vault Sync / Top-Down Implementation Readiness

- 사용자가 제공한 최신 `Universal-Agent-Vault(2).zip`과 WAIN `Docs(1).zip`을 기반으로 프로젝트 볼트를 업데이트했다.
- 공통 Engineering Standards, Obsidian theme/plugins/settings, VS Code/sortspec, Formatting & Build templates, Copilot Skills를 동기화했다.
- **WAIN 전용 Top-Down Readability 구현 지침**을 추가하고 AGENTS, README, Home, Document Map, Constraints, Current Work, Work Log, Roadmap 및 설치 가이드를 동기화했다.
- 기존 WAIN 아키텍처/계약, ADR-001~006, Research Assets, Legacy Archive는 내용과 파일 경로를 유지했다.
- 개인 Git 데이터·Copilot 모델 캐시/대화 기록·계정 식별자는 전파하지 않았다. Runtime/Build/Editor/Live Coding 검증은 아직 실행되지 않았다.

## 참고

공통 템플릿 자체의 변경 사항과 버전 이력은 [Engineering Standards](00_Engineering-Standards/00_Core/00_Code-Style.md) 및 최신 공통 볼트의 CHANGELOG에서 관리한다. 이 문서는 WAIN에 적용한 변경을 기록한다.

## 2026-11-11 — Phase 2 Presentation Catalog Implementation

- Host Presentation Catalog Public API 및 검증 코드를 도입하고, Character Context → Transaction 검증 → Result 반환 경로에 연결했다.
- ADR-002/ADR-007 구현 상태를 갱신하고, Source Map·Contracts·Test Plan·Work Log에 실제 지원·미지원 범위를 명시했다.
- WAIN `00_Engineering-Standards`, Obsidian 설정, Host Testbed 콘텐츠는 변경하지 않았다. UE 5.8 빌드/테스트 실행 미검증.

## 2026-11-13 — DEV Inspector / Turn Diagnostics

Host DEV 버튼을 탭형 Debug Inspector로 확장했다. Plugin에서 최근 48개 실제 단계·Context·Proposal을 읽기 전용으로 조회하고 Testbed Snapshot Delta·Memory·Narrative와 조합해 표시한다. [ADR-015](03_Decisions/15_ADR-015-Turn-Diagnostics-and-DEV-Inspector.md) Proposed. Automation 테스트 소스 7개 추가, 빌드 미검증.

## 2026-11-14 — LLM Private Layers Refactor

- Prompt/System Instruction을 HTTP Client에서 분리하고 Config / Prompt / Providers / Validation / Monitoring 책임별 Private 폴더를 확정했다.
- 외부 공개 LLM API를 Config / Types / TurnService에 한정했다. API/Usage/Validation 전송 구현은 Private이다.
- Testbed UI/Adapter는 공개 LLMTypes를 사용하고 기존 DEV 경고·Mock·Atomic Commit 경로를 유지한다.
- Prompt 오프라인 테스트 3개 추가(전체 테스트 코드 118개). Unreal 빌드 및 API 연동은 별도 검증이 필요하다.
- [ADR-019](03_Decisions/19_ADR-019-LLM-Internal-Layers.md).
