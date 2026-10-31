# Interaction Pipeline

> **한 줄 요약:** 검증 전 LLM 출력을 Proposal로 취급하고, 승인된 변경만 확정한 뒤 VN에 표시한다.

상태: **설계 기준** · 마지막 확인: 2026-10-30

| # | 단계 | 입력과 작업 | 출력·경계 |
|---:|---|---|---|
| 1 | Player Input | 자유형 자연어 입력을 Interaction 단위로 수신 | `InteractionId`, `CharacterId`, 입력 텍스트 등 개념 요청 값 |
| 2 | Context Preparation | Character State·관련 Memory, 필요한 World Context와 Narrative Context 조회 | 턴에 필요한 제한된 Context. 전체 World State/비공개 정보를 무조건 넣지 않음 |
| 3 | Character Agent | Persona·관계·감정·Memory·Plan을 사용해 응답 생성과 변경 제안 | Dialogue, 표현/행동, State Change Proposal. 실행/Commit 전 상태 아님 |
| 4 | Response Validation | 출력 Schema, owner·World State·Action 조건, Hard Narrative Constraint 검사 | 통과/실패와 오류 내역. 고위험 의미 위반에만 Semantic Validation 고려 |
| 5 | Narrative Evaluation | 검증된 변경을 반영한 임시 평가 State에서 Completion 및 Transition 평가 | Beat 유지 또는 우선순위가 정해진 다음 Beat 후보. Narrative State는 아직 미변경 |
| 6 | State Commit | Character·WAIN-owned World·Narrative 변경을 실제 owner에 순차 적용 | 확정 상태와 Commit 결과. 초기 설계는 다중 owner 원자성 보장 안 함 |
| 7 | VN Response | 확정된 Dialogue·Action·Expression·Event를 Testbed에 전달 | Commit 결과와 일치하는 표시 |

## 필수 순서

`Player Input → Context Preparation → Character Agent → Response Validation → Narrative Evaluation → State Commit → VN Response`

Commit은 응답 표시보다 앞선다. 플레이어가 확인하는 결과와 다음 턴이 참조하는 상태가 같은 확정 결과를 가리켜야 한다.

## Interaction 직렬화

초기 범위에서는 한 캐릭터에 한 번에 하나의 Interaction만 처리한다. LLM 요청 자체는 비동기일 수 있으나 응답 대기 중 추가 턴이나 독립 실시간 이벤트가 State를 변경하는 구조는 지원 범위 밖이다. Revision 기반 Optimistic Concurrency Control은 초기 구현에서 제외한다.

## 실패 처리 기준

- Validation 실패 시 Proposal을 Commit하지 않는다.
- Commit 전 유효성을 검사하고 Game Thread에서 순차 적용하는 방향이다.
- 외부 시스템 적용이 실패했는데 Narrative만 전이하거나 실패한 행동을 성공으로 출력하지 않는다.
- 확정 응답 중단 또는 Fallback이 우선 후보지만 재생성 횟수·Fallback 내용·복구 정책은 미확정이다.
- 여러 owner에 걸친 완전한 원자적 트랜잭션과 롤백은 초기 범위에서 보장하지 않는다.

근거: 설계 인계 문서 §4, 중간발표 슬라이드 7·14·15·27·28 및 발표 대본. 요청/결과 API는 [[../02_Contracts/07_Data-Contracts|Data Contracts]]에 정리한 개념안이며 실제 구현 계약은 아니다.

개발 중 상태 검사, 재현 도구, 데이터 검사 및 단계별 Interaction Trace는 [[../02_Contracts/08_Debugging-and-Dev-Tools|WAIN Debugging and Dev Tools]]를 따른다. 진단 정보는 상호작용의 상태나 결과를 변경하지 않는다.
