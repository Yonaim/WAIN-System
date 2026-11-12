# WAIN — Contract Register

> 상태: **설계 계약**. Unreal C++ API, 자료형, 직렬화 세부사항은 Plugin Source에서 구현·검증해야 한다.

| 계약 | 책임 / 불변조건 | 상세 문서 | 검증 |
|---|---|---|---|
| C-01 Character State | Relationship·6축 Emotion은 WAIN 정본, LLM 직접 변경 금지 | [Character Agent](01_Character-Agent.md) | State tests |
| C-02 Memory | Observation/Reflection/Plan 기록·검색 | [Memory](02_Memory-Reflection-Planning.md) | Memory tests |
| C-03 World State | WAIN/Game 소유권 분리; Game-owned 읽기·쓰기 및 Adapter 참여 | [World](03_World-State-Manager.md) | Ownership / Adapter tests |
| C-04 Narrative | Beat/Goal/Constraint, 초기 턴당 최대 한 전이 | [Narrative](04_Narrative-Manager.md) | Beat tests |
| C-05 Transaction | Validation→Prepare→Narrative→Final Validation→Atomic Commit→Response | [Commit](05_Response-Validation-Commit.md) | No-Commit / Revision / Atomic tests |
| C-06 AI Services | LLM/Embedding Provider 경계 및 오류 모델 | [AI Services](06_Shared-AI-Services.md) | Failure tests |
| C-07 Emotion/Visual | 6축 Emotion과 Host 표현 분리; Context 단계 Catalog 조회 | [Emotion/Expression](07_Emotion-Expression-Boundary.md) | Unknown ID tests |
| C-08 Data | Turn ID·Revision·Snapshot·Mutation·Expression Proposal | [Data](08_Data-Contracts.md) | Schema tests |
| C-09 Observability | Input/Proposal/Validation/Preparation/Commit 추적 | [Debug](09_Debug-Observability.md) | Trace tests |

정본: WAIN 설계 결정은 [ADR Index](../../03_Decisions/00_ADR-Index.md), 실제 동작은 향후 Plugin Source/검증 기록. 현재 Host Content JSON의 구형 Emotion 스키마는 별도 마이그레이션 대상이다.
