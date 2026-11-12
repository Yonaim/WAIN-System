# WAIN System — Plugin Documentation Vault

이 폴더는 Unreal Engine 기반 **WAIN System Plugin**의 정본 개발 문서 볼트다. 범용 개발 규칙은 **2026-11-10 제공 최신 Universal Agent Vault**와 동기화하고, 프로젝트 고유의 결정·계약·평가 기준을 이 볼트에 기록한다.

- 시작: [00_Home](00_Home.md) → [Project Brief](01_Project-Context/00_Project-Brief.md) → [Document Map](01_Document-Map.md)
- **현재 상태(2026-11-10 제공 스냅샷): 설계 문서만 있음.** `Plugins/WAINSystem/` 내 `.uplugin`과 `Source/`가 없어 플러그인 런타임 구현을 확인하지 못했다.
- WAIN은 **상태·검증·서사 처리**를 정의한다. 마나의 이미지·표정 파츠·배경과 대본은 **호스트 Testbed 소유**이며 플러그인에 복제하지 않는다.
- 기존 볼트의 상세 문서와 인계 자료는 [Legacy Archive](99_Archive/00_Legacy-Docs-Index.md)에 보존했다. **정본이 아니라 참고 근거**다.
- 중간발표·제안서 PDF는 [Research Assets](06_Research/00_Design-Basis.md)에 연결했다.

## 저장소 배치 및 작업 환경

이 볼트의 실제 위치는 `Plugins/WAINSystem/Docs/WAIN-System-Vault/`다. **Plugin 저장소의 루트를 VS Code로 열면 하위 볼트의 `.vscode/settings.json`은 자동 적용되지 않는다.** 에디터 설정이 필요하면 플러그인 저장소 루트 또는 VS Code 사용자 설정에 병합한다. Obsidian은 이 폴더 자체를 볼트로 열 수 있다.

## 문서 책임

- `00_Engineering-Standards`: 제공된 최신 범용 코딩·문서화·설치·포맷 규칙의 원본과 동일한 사본
- `01_Project-Context`: 프로젝트 목표·요구·제약·용어의 정본
- `02_Architecture`: 턴 흐름·모듈·데이터·표현 경계의 상세 계약
- `03_Decisions`: 현재 채택한 선택과 근거
- `04_Development`: 개발 범위·현황·이력
- `05_Experiments`: 검증 계획 및 측정 결과(실측 없으면 미실행 표시)
- `06_Research`: 프로젝트 근거와 발표 자료
- `99_Archive`: 교체한 이전 볼트 문서의 스냅샷


## 최신 공통 볼트 동기화 · WAIN 적용 범위

- **Obsidian:** `.obsidian`의 Blue Topaz, Style Settings, Custom Sort, Smart Fold, Editor Width Slider, Table Width, Automatic Plugin Installer, Copilot 및 관련 설정/스타일/플러그인 파일을 공통 볼트 기준으로 동기화했다. `sortspec.md`도 같은 내용을 적용한다. 계정·기기 식별자와 캐시·대화 기록은 복제하지 않는다.
- **에이전트와 도구:** 범용 표준 `00_Engineering-Standards`, `Assets/Formatting`, `Assets/Build`, `copilot/skills`를 복사했다. WAIN 고유 규칙은 [AGENTS](AGENTS.md) 및 [Implementation Principles](04_Development/03_Implementation-Principles.md)에서 정의한다.
- **최우선 코드 품질 기준:** Top-Down Readability. Runtime 진입점은 순서가 보이는 조율 함수로, 세부 계산은 의미 있는 하위 함수로 분리한다.
- **실행 전 설정:** 이 ZIP은 문서 볼트를 업데이트한다. 프로젝트 루트의 `.clang-format`·`Tools/` 파일, 실제 `Target.cs`·빌드 훅·Source 코드는 자동 변경하지 않는다. [설치 및 빌드 연결](INSTALLATION.md)을 먼저 확인한다.
- **설계 보존:** 기존 [ADR-001~006](03_Decisions/00_ADR-Index.md), WAIN 아키텍처와 Testbed 간 콘텐츠 소유권은 변경하지 않았다.
