# ADR-018 — Plugin LLM Ownership and WAINTestbed Source Layout

- **상태:** Proposed
- **일자:** 2026-11-14
- **대상:** WAINSystem Plugin / WAINTestbed C++ Source 책임 경계 및 호스트 Data 디렉터리

## Context

OpenAI HTTP, Prompt Builder, JSON Schema 처리, Config Loader, 조직 Usage Monitor가 WAINTestbed 루트에 있어 재사용 가능한 WAINSystem Plugin과 게임별 Host의 책임이 혼합돼 있었다. 호스트 소스 루트도 게임플레이, 콘텐츠, UI 등이 평탄하게 나열되어 확장/탐색이 어렵다.

## Decision

1. Provider의 비동기 HTTP, Prompt 조립, Structured Proposal, Config 읽기·검증, 조직 사용량 조회, 비동기 요청 연결은 **WAINSystem Plugin** 모듈 `Public/LLM`, `Private/LLM`이 소유한다.
2. **WAINTestbed는 게임별 설정값 및 콘텐츠를 소유**한다. 실제 파일 `Config/WAIN/LLMConfig.json`, Git 제외 `LLMConfig.local.json`은 Testbed 루트에 둔다.
3. `Source/WAINTestbed/`의 모듈 진입점/Build.cs를 제외한 게임별 소스는 `Gameplay / Data / Integration / Presentation / UI / Tests`로 그룹화한다.
4. **`Data`**는 **C++ JSON Loader 코드** 저장소이고, 최상위 **`Content/Data`**는 게임 데이터 파일 저장소다. Unreal의 `Content` 자산 디렉터리와 `Source`를 혼동하지 않는다.
5. Host는 Plugin의 공개 LLM API만 호출한다. Plugin은 WAINTestbed 전용 클래스/헤더/에셋에 의존하지 않는다.
6. 경고 정책/Mock 기본값/Transaction 검증 및 No Commit 규칙은 그대로 유지한다. 네트워크 실호출/플레이테스트는 별도로 검증한다.

## Consequences

- Plugin `Build.cs`에 `HTTP` 의존성을 두고 Testbed 모듈에는 필요하지 않다.
- 기존 Plugin LLM/Usage 오프라인 테스트는 Plugin `Private/Tests`로 이동한다. 테스트 등록 내용은 유지한다.
- 호스트 헤더의 `#include` 및 UHT/UE 컴파일을 모두 재확인해야 한다. 현재는 정적 검사까지만 진행했다.
- 새 게임에서 Plugin 사용 시 LLM 구현을 복제하지 않아도 되지만, API 키를 배포 프로그램에 직접 탑재하지 않는 별도 보안 설계는 여전히 필요하다.

관련: [ADR-016](16_ADR-016-Asynchronous-LLM-Proposal-Handoff.md) · [ADR-017](17_ADR-017-Host-Usage-Preflight-and-Warnings.md) · [Host Architecture](../../../../../Docs/02_Implementation/00_Testbed-Architecture.md)
