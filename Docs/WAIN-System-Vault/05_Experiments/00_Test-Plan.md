# WAIN Test Plan

> **계획**: 자동/Unreal/LLM 실험은 실행하지 않았다. 각 항목은 기능 구현 후 결과를 기록한다.

## 구성별 비교

A: LLM Only → B: A+Character Agent → C: B+World State Manager → D: C+Narrative Manager. 동일 시나리오·입력·모델/파라미터를 기록해 비교한다. A~D 누적 비교로 개별 요소의 인과 효과를 독립적으로 단정하지 않는다.

## 계약별 테스트

| ID | 테스트 | 기대 결과 | 현재 |
|---|---|---|---|
| T-01 | 올바른 구조화 Proposal | Validator 통과 후 Commit | 미실행 |
| T-02 | 잘못된 Enum/누락 필드 | No Commit | 미실행 |
| T-03 | 금지된 Story Flag/Hard Constraint | 거절, 비밀 노출 없음 | 미실행 |
| T-04 | Game-owned World Fact 변경 | Adapter 검증 경로만 적용 | 미실행 |
| T-05 | Memory Reflection SourceMemoryIds | 참조 무결성 유지 | 미실행 |
| T-06 | Plan Reuse/Goal 변화 | Trigger에 따라 재사용·갱신 | 미실행 |
| T-07 | Expression ID 유효/미유효 | Host Catalog 검사·오류 정책 적용 | 미실행 |
| T-08 | Emotion≠Expression | 불안+미소 등 허용, 강제 1:1 없음 | 미실행 |
| T-09 | `EXCITED` default/eyes_closed | 두 Variant 모두 선택 가능 | JSON 정적검증만 완료 |
| T-10 | LF/CRLF Region 폴딩 | v5.9 정규식 대응, 편집기 실제 검증 필요 | 미실행 |
| T-11 | 외부 서비스 오류/부분 Commit | 실패 후 일관된 State 유지 | 미실행 |

## 측정

기억 정확도, 관계 지속 반영률, Hard Constraint 위반율, 응답 자연스러움·인물 일관성·맥락 적절성 평가(5점척도), 입력→응답 지연. 표본/반복/임계값은 구현 후 정한다.

## Phase 1 Unreal Automation Tests — 작성됨, 실행 전

명령 예시: Unreal Editor Session Frontend → Automation에서 `WAIN.Transaction.*` 필터링. 실제 결과를 확인한 후에만 **Pass/Fail**로 표시한다.

| Test Name | 기대 동작 | 현재 |
|---|---|---|
| `WAIN.Transaction.AtomicCommit` | Trust +5, Game Inventory -1, Beat 전이, 기존 Snapshot 유지 | 코드 작성 / **미실행** |
| `WAIN.Transaction.PrepareFailureKeepsState` | 미등록 Game Key 거절, Root 무변경 | 코드 작성 / **미실행** |
| `WAIN.Transaction.DuplicateTurnIsIdempotent` | 재전송 시 중복 Commit 없음, 다른 입력 동일 ID 거절 | 코드 작성 / **미실행** |
| `WAIN.Transaction.UnvalidatedPresentationIsRejected` | Catalog 없는 Expression ID 거절 | 코드 작성 / **미실행** |
| `WAIN.Transaction.StaleRevisionCannotPublish` | 오래된 Revision으로 게시 거절 | 코드 작성 / **미실행** |
| `WAIN.Transaction.OutOfRangeTrustKeepsState` | Trust 범위 밖 변경 거절, 양측 State 무변경 | 코드 작성 / **미실행** |

**미포함 테스트:** WAIN Subsystem의 이벤트 방송 정확히 1회, 실제 Host Game Actor/Widget 연동, 세션 종료 후/멀티스레드 Commit, 실제 LLM 오류/Timeout, Save/Load, 비동기 재진입. 연결 단계에서 추가한다.
