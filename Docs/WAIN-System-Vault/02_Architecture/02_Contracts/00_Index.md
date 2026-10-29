# 시스템 계약 색인

> **한 줄 요약:** 플러그인 공통 기능과 Testbed 고유 기능의 경계를 추적한다.

상태: **책임 경계는 README 기준 · 런타임 계약 미작성** · 마지막 확인: 2026-10-29

| 계약 | 확인된 내용 | 상태 |
|---|---|---|
| 공통 시스템 vs 게임별 책임 | 플러그인에 공통 Agent/Manager/Orchestrator/Validator, Testbed에 화면·입력·게임 상태·시나리오·Adapter | README의 의도, 코드 계약은 미확인 |
| 상태·컨텍스트 데이터 | JSON 기반 상태/컨텍스트가 Tech Stack으로 명시됨 | 스키마·직렬화 규약 미작성 |
| LLM 외부 연동 | LLM API integration이 목표 기술 스택에 기재됨 | provider·요청·응답·오류 계약 미작성 |
| 프로젝트 시작 맵 | 현재 `Entry`; README는 Testbed 맵 생성 후 설정 변경 안내 | ini 확인됨 |
| 에셋 관리 | `.uasset`, `.umap`은 Git LFS 사용 안내 | 저장소 설정과 실제 추적은 별도 확인 필요 |

기능 계약은 플러그인 소스와 승인된 요구사항을 확인해 작성한다. README의 설계 의도와 구현 사실을 같은 상태로 표시하지 않는다.
