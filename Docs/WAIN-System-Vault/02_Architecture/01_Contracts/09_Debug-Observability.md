# Debug & Observability Contract

> 상태: 개발·평가 도구 제안. 사용 가능한 Plugin Debug UI/Trace 시스템은 확인되지 않았다.

## Turn Trace에 기록할 최소 사실

`trace_id`, input kind/text (허용된 범위), 사용한 Character/World/Narrative State Revision, 검색 Memory IDs, LLM Provider/Model/Latency, Proposal ID, Schema·Rules·Constraint Validation 결과, Narrative Eval 결과, 적용 State 목록, Commit 결과, 반환 Expression ID/Variant, 사용자에게 표시한 Dialogue/Action.

## 뷰 제안

- **Character:** Persona/Relationship/Emotion 값, 활성 Memory/Plan
- **World:** Fact Owner, Adapter 읽기/적용 성공·실패
- **Narrative:** Active Chapter/Sequence/Beat, Goal, Constraint, 충족 조건
- **Presentation:** Proposal Expression ID와 Host Catalog Validation/Variant 선택 및 PNG Layer 확인
- **Timing:** LLM, Embedding, Validation, Commit 소요 시간 구분

## 보안·검증 경계

미공개 Story Facts, 사용자 자연어, LLM Prompt를 저장할 때 접근 범위·마스킹·보존 기간이 필요하다. 현재 Testbed에는 단순 `DEV MOCK` Status Label만 존재하며 이를 WAIN Trace 구현으로 보지 않는다.
