# WAIN System / Testbed 아키텍처

> WAIN 공통 시스템은 플러그인으로, 비주얼 노벨 표시·입력·게임 상태와 어댑터는 Testbed에 둔다는 책임 분리를 따른다.

상태: **초기 구조 확인됨 · 통합 구현 전** · 마지막 확인: 2026-10-29

## 무엇을 어디서 읽는가

| 목적 | 문서 |
|---|---|
| 초기화·실행·통합 흐름 | [[01_Flows/00_Index|Flows 단계별 지도]] |
| 현재 모듈 경계와 확인된 설정 | [[02_Contracts/00_Index|Contracts 색인]] |
| 실제 파일과 모듈 | [[03_Source/00_Index|Source 색인]] |
| 미완료 통합 작업 | [[../03_Development/01_Current-Work|Current Work]] |
| WAIN System 프로젝트 정의와 계획 | [플러그인 WAIN-Vault](../../../Plugins/WAINSystem/Docs/WAIN-Vault/02_Planning/00_Project-Overview/0000_Project-Plan.md) |

## 현재 구성

- **WAINTestbed Unreal 프로젝트:** 단일 에이전트 시나리오를 우선 검증하려는 통합 Testbed다. 정적 배경·캐릭터 이미지는 우선 UMG 위젯에 표시하도록 프로젝트 README가 설명한다.
- **WAINSystem 플러그인 서브모듈:** 공통 Character Agent, State Manager, Narrative Manager, Orchestrator, Validator와 외부 연동 인터페이스를 둘 목표다. 플러그인 README는 현재 저장소에 문서와 빈 플러그인 폴더만 있으며 `.uplugin`과 C++ 모듈이 아직 추가되지 않았다고 명시한다.
- **게임별 Testbed:** VN 화면·입력·실제 게임 상태·시나리오 데이터·게임별 Adapter 구현을 둘 책임이다. 현재 이 작업 공간에서 확인한 Testbed C++는 Unreal 기본 게임 모듈 초기화 코드 수준이다.

## 현재 한계와 확인 필요

- `Config/DefaultEngine.ini`의 시작 맵은 `/Engine/Maps/Entry`다. README는 `/Game/Maps/L_Testbed`를 만든 뒤 맵 설정을 변경하도록 안내한다.
- WAIN 플러그인 런타임 기능, LLM 호출, JSON 상태 흐름, UI 시나리오는 이 저장소에서 아직 구현된 것으로 확인되지 않았다.
- 위 내용은 저장소 README와 현재 파일을 기준으로 한 시점 기록이다. 변경 시 코드와 설정을 재확인한다.


