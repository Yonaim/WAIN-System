# ADR-021 — Beat Turn Budget, Authored Hooks, Safe Save

- Status: Accepted / Implemented in Testbed source; UE runtime verification pending
- Related: ADR-003 Commit Before Response, ADR-010 Deterministic Beat Graph, ADR-014 Testbed VN Presentation, ADR-020 Relationship & Presentation Steps

## Context

WAIN Testbed에서는 미연시 장면의 자유 대화가 무한히 지속되어 수업 시작 등의 자연스러운 시점을 표현하기 어렵다. 또한 필수 사건과 프리젠테이션 연출을 LLM에만 위임하면 재현성이 낮아진다.

## Decision

1. Scene은 Narrative Chapter/Sequence/Beat와 분리된 Host-owned Presentation Context다.
2. Narrative Beat는 Goal, Soft Constraint, Hard Rules, Completion, Transitions를 유지한다. **Turn Budget은 Constraint에 섞지 않는다.**
3. Beat별 `MaxInteractionTurns`를 사용하며 Sequence 공동 예산은 없다. `bHoldAfterGoal`로 Goal 만족과 장면 이탈을 분리한다.
4. `OnEnterScriptId`와 `OnBudgetEndScriptId`를 BeatContent JSON으로 연결한다. 한 Beat에 고정 Hook 1개, 고정 Script 진행 중 새 Turn은 생성하지 않는다.
5. Scripted World Mutation은 `bWritableByScript`인 Fact만 허용한다. LLM Proposal 변경 권한은 `bWritableByProposal`로 별도 검증한다.
6. Dream Interlude는 마나가 접근 가능한 실제 Memory와 작가 고정 메시지로 구성한다.
7. Testbed는 `USaveGame` 슬롯에서 안전한 턴 경계의 WAIN+Host Root를 저장하고, 복원 시 새 Revision을 발행한다.
8. DEV에는 순수 Scene Preview와 Editor 전용 Beat Preset Jump를 구분한다.

## Consequences

- 작가가 주요 사건의 발생을 보장하면서 자유 대화의 표현 방식과 관계 변화는 LLM에 위임할 수 있다.
- 단일 Narrative Beat 안에 Script Event, Presentation Step, Interaction Turn이라는 서로 다른 시간 단위가 생기므로 DEV 로그와 UI에서 구분해야 한다.
- Unreal Asset 파일 자체는 배포 ZIP에 포함되지 않는다. 리소스 Catalog는 현재 JSON 하나에서 관리한다.
- 실제 UE 5.8 컴파일과 PIE/LLM 장면 검증을 거치기 전까지는 구현 완료와 실행 검증을 혼동하지 않는다.
