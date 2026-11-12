# WAIN System — Home

> 설계 목적: 자유형 자연어 입력을 비주얼 노벨의 지속적인 캐릭터·세계·서사 경험으로 연결한다. **LLM의 제안은 권위 있는 State가 아니며 검증 후 Commit한다.**

## 탐색

1. [Project Brief](01_Project-Context/00_Project-Brief.md) · [Glossary](01_Project-Context/01_Glossary.md) · [Requirements](01_Project-Context/02_Requirements.md) · [Constraints](01_Project-Context/03_Constraints.md)
2. [Turn Interaction Flow](02_Architecture/00_Flows/00_Main-Flow.md) · [Emotion/Expression Flow](02_Architecture/00_Flows/01_Emotion-Expression-Flow.md)
3. [Contract Register](02_Architecture/01_Contracts/00_Contract-Register.md) · [Source Map](02_Architecture/02_Source/00_Source-Map.md)
4. [ADR Index](03_Decisions/00_ADR-Index.md) · [Current Work](04_Development/00_Current-Work.md) · [Test Plan](05_Experiments/00_Test-Plan.md)
5. [Design Basis](06_Research/00_Design-Basis.md) · [Legacy Archive](99_Archive/00_Legacy-Docs-Index.md)

## Agent 실행 규칙

- 작업 시작 시 [AGENTS](AGENTS.md) → [Document Map](01_Document-Map.md)을 읽고 과제에 필요한 계약만 추가 확인한다.
- 실제 구현과 설계 제안을 구분한다. 게임별 JSON·PNG의 소유자는 Testbed다.
- 2026-11-10 제공 소스에는 WAIN 플러그인 런타임이 없다. 설계 기능을 구현 완료로 기재하지 않는다.


## 구현 시작 — 필수 규칙과 설치

- **코드 최우선 기준:** [Top-Down Readability / WAIN Implementation Principles](04_Development/03_Implementation-Principles.md).
- **최신 공통 표준:** [Code Style](00_Engineering-Standards/00_Core/00_Code-Style.md), [C++](00_Engineering-Standards/01_Languages/01_CPP.md), [Pre-Build Formatting](00_Engineering-Standards/03_Workflow/03_Pre-Build-Formatting.md).
- **Unreal Plugin Setup:** [Project Setup](00_Engineering-Standards/03_Workflow/04_Project-Setup.md), [Unreal Build Integration](Assets/Build/Unreal/Build-Integration.md), [INSTALLATION](INSTALLATION.md).
- 설계 채택과 코드 구현·테스트 성공은 별개이며 기존 ADR의 상태는 그대로 유지한다.

## Phase 1 C++ Runtime (2026-11-10)

새 소스 및 구현 범위는 [Source Map](02_Architecture/02_Source/00_Source-Map.md), 최신 구현 상태는 [Current Work](04_Development/00_Current-Work.md), 테스트 계획은 [WAIN Test Plan](05_Experiments/00_Test-Plan.md)을 참조한다. Plugin ZIP에 소스가 추가되었지만 Unreal Build/Automation 실행 결과는 아직 없다.
