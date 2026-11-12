# 아키텍처 계약 색인

| 계약 | 문서 | 결정 상태 |
|---|---|---|
| Character Agent와 State | [[01_Character-Agent|Character Agent]] | 책임·상태 개념 설계됨, 수치 정책 미확정 |
| Memory, Reflection, Planning | [[02_Memory-Reflection-Planning|Memory / Reasoning]] | 구조·흐름 설계됨, 임계값·검색 설정 미확정 |
| World State와 외부 Adapter | [[03_World-State-Manager|World State Manager]] | 소유권·Hybrid Model 설계됨, UE API 미확정 |
| Narrative Definition과 Beat Graph | [[04_Narrative-Manager|Narrative Manager]] | 계층·조건부 전이 설계됨, 데이터 형식 미확정 |
| Proposal, Validation, Commit | [[05_Response-Validation-and-Commit|Validation and Commit]] | 순서·원칙 설계됨, Schema·실패 정책 미확정 |
| LLM / Embedding 서비스 | [[06_Shared-AI-Services|Shared AI Services]] | 역할 설계됨, Provider·구현 미확정 |
| 요청·결과·상태 데이터 구분 | [[07_Data-Contracts|Data Contracts]] | 개념 필드만 정의, 최종 C++ 계약 아님 |
| 디버그와 개발 도구 | [[08_Debugging-and-Dev-Tools|WAIN Debugging and Dev Tools]] | 상태 검사·재현·데이터 검사·7단계 Trace 제안, 미구현 |

> 이 문서들은 설계 기준을 설명한다. 구현 여부는 [[03_Source/00_Index|Source Index]]에서 별도로 확인한다.
