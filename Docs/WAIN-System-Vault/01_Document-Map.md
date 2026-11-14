# WAIN — Task-Based Document Map

> 각 코드 작업에서 [AGENTS](AGENTS.md)와 본 맵을 먼저 읽는다. **필요한 문서만** 추가로 읽는다. 범용 표준은 [Engineering Standards](00_Engineering-Standards/00_Core/00_Code-Style.md)를 정본으로 한다.

| 작업 | 반드시 확인할 문서 | 변경 후 함께 검토할 문서 |
|---|---|---|
| **모든 WAIN C++ 구현: Top-Down Readability** | [Implementation Principles](04_Development/03_Implementation-Principles.md), [Code Style](00_Engineering-Standards/00_Core/00_Code-Style.md), [AGENTS](AGENTS.md) | 관련 Contract/Flow·Source Map·Work Log |
| **Plugin 초기 설정 / 빌드 전 포맷** | [Project Setup](00_Engineering-Standards/03_Workflow/04_Project-Setup.md), [Pre-Build Formatting](00_Engineering-Standards/03_Workflow/03_Pre-Build-Formatting.md), [UBT Integration](Assets/Build/Unreal/Build-Integration.md) | Constraints·Source Map·실제 Hook 검증·Work Log |
| Unreal C++ 구조/타입/명명 | [CPP](00_Engineering-Standards/01_Languages/01_CPP.md), [Code Style](00_Engineering-Standards/00_Core/00_Code-Style.md), [Source Map](02_Architecture/02_Source/00_Source-Map.md) | 관련 Contract·Work Log |
| 입력→Commit→VN 출력 순서 | [Main Flow](02_Architecture/00_Flows/00_Main-Flow.md), [Commit Contract](02_Architecture/01_Contracts/05_Response-Validation-Commit.md) | ADR-003·Validation Tests |
| Host Persona 및 Character Context | [ADR-007](03_Decisions/07_ADR-007-Read-Only-Character-Context.md), [Character Context Implementation](04_Development/06_Character-Context-Implementation.md) | Character Contract·Source Map·Context 테스트 |
| Persona/Relationship/Emotion | [Character Agent](02_Architecture/01_Contracts/01_Character-Agent.md), [Emotion Boundary](02_Architecture/01_Contracts/07_Emotion-Expression-Boundary.md) | Testbed 표현 계약·State 테스트 |
| Memory/Reflection/Planning | [Memory](02_Architecture/01_Contracts/02_Memory-Reflection-Planning.md) | Test Plan·개발 기록 |
| Snapshot / Game Transaction 확장 | [Snapshot ADR](03_Decisions/04_ADR-004-Snapshot-State.md), [MVP Adapter ADR](03_Decisions/05_ADR-005-Minimal-Game-Adapter.md), [Commit](02_Architecture/01_Contracts/05_Response-Validation-Commit.md) | 실패 주입·Atomic State 테스트 |
| Game-owned World State | [World State](02_Architecture/01_Contracts/03_World-State-Manager.md), [Ownership ADR](03_Decisions/01_ADR-001-State-Ownership.md) | Adapter·Commit 계약 |
| Beat/Goal/Constraint | [Narrative](02_Architecture/01_Contracts/04_Narrative-Manager.md) | Response Validation·Test Plan |
| 외부 LLM/Embedding | [AI Services](02_Architecture/01_Contracts/06_Shared-AI-Services.md), [LLM Internal Structure](../../../../Docs/02_Implementation/11_LLM-Internal-Structure.md) | [ADR-019](03_Decisions/19_ADR-019-LLM-Internal-Layers.md)·Data Contracts·Observability |
| `EXP_*` 검증, 표정 variant | [Emotion/Expression Contract](02_Architecture/01_Contracts/07_Emotion-Expression-Boundary.md) | 호스트 `Docs/02_Visual-Asset-System.md`, `Content/Data/Characters/ManaExpressions.json` |
| 데이터 Schema/Save | [Data Contracts](02_Architecture/01_Contracts/08_Data-Contracts.md) | 관련 모듈·Test Plan |
| 평가·성능 | [Test Plan](05_Experiments/00_Test-Plan.md), [Validation](05_Experiments/01_Validation-and-Tests.md) | 실측이 있을 때만 Performance 문서 갱신 |
| 파일 헤더·주석·Region | [Source Comments](00_Engineering-Standards/02_Documentation/01_Source-Comments.md), [Local Numbering](00_Engineering-Standards/00_Core/01_Local-Numbering-and-Navigation.md) | Source Map·Work Log |

## 문서 우선순위

1. 제공된 **실제 소스 및 Content JSON**은 현재 동작의 근거다.
2. `01_Project-Context`/`02_Architecture`/ADR은 앞으로 구현할 **의도된 계약**이다.
3. `99_Archive`와 발표 자료는 설계 결정의 **출처와 역사**이지 런타임 구현 근거가 아니다.


## 문서 동기화 기준

- 범용 규칙 변경은 [Common Engineering Standards](00_Engineering-Standards/00_Core/00_Code-Style.md)를 최신 원본과 동기화하고, 프로젝트별 추가 규칙은 [Implementation Principles](04_Development/03_Implementation-Principles.md)에 기록한다.
- `copilot/skills`와 `.obsidian`은 도구의 사용 편의를 위한 **복제 설정**이며 WAIN Runtime의 구현 계약이 아니다.

## 2026-11-11 추가 구현 문서

- `Host Presentation Catalog / Expression Validation` 관련 코드는 [ADR-002](03_Decisions/02_ADR-002-Emotion-Expression.md), [Emotion/Expression Boundary](02_Architecture/01_Contracts/07_Emotion-Expression-Boundary.md), [Presentation 구현 상세](04_Development/07_Presentation-Catalog-Implementation.md)를 함께 확인한다.
- 새 구현의 상태는 [Current Work](04_Development/00_Current-Work.md)와 [Test Plan](05_Experiments/00_Test-Plan.md)에서 관리한다.
