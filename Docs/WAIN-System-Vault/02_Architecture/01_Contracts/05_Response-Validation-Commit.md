# Response Validation & Interaction Transaction

> 핵심 불변조건: **Proposal ≠ Committed State**. 지원 범위에서 한 턴의 WAIN/Game 변경은 모두 확정 또는 모두 무변경이어야 한다. **Phase 1의 Mock C++ 경로는 작성되었으나 Unreal 빌드·실행은 미검증**이다.

## 처리 순서

`Response Proposal → Response Validation → Next State Preparation → Narrative Evaluation → Final Validation → Atomic Commit → State Committed Notification → VN Response`

## Validation

1. Schema: 필수 타입·ID·Proposal 형식 및 범위 검사.
2. Game Rules/Ownership: Mutation Owner/Target/Operation/Value 적법성 및 Adapter 지원 여부 검사.
3. Narrative: 현재 Hard Constraint와 정보 공개 범위 검사.
4. Presentation: Context Preparation에서 획득한 허용 Host Catalog의 Expression/Variant 검사. 없는 ID를 임의로 Neutral로 대체하지 않는다.

## Preparation 및 Commit

- WAIN-owned State는 **Current Snapshot을 수정하지 않고** Prepared Snapshot 생성.
- Game-owned State는 지원 가능한 Host Adapter의 `Prepare`에서 변경 후보 생성. Prepare는 권위 State를 수정하거나 효과를 실행하지 않아야 한다.
- Narrative Manager는 **Prepared State**를 기준으로 Completion/Transition을 평가하고 결과를 후속 검증에 반영한다.
- Final Validation 및 Revision 검사에 실패하면 Prepared State를 폐기한다.
- 초기 Testbed에서는 **공유된 하나의 Commit 경계**에서 Prepared WAIN/Game 상태를 함께 확정한다. 각 저장소의 `Apply()`를 순서대로 두 번 호출하는 것과 원자적 Commit은 다르다.
- 미지원/미검증 Game Adapter에는 Game Mutation 성공을 반환하지 않는다. 외부 DB·Network·독립 Actor 적용은 후속 설계 범위다.
- Commit 후 Display/UI/사운드 등의 연출 실패는 State를 Rollback하지 않고 표현을 재시도·복구한다.

## Error / Trace

Validation/Prepare/Revision 충돌은 No Commit. Commit 실패 가능성과 복구가 필요한 외부 저장소 연동은 현재 미지원. Turn ID, Snapshot Revision, Validator/Prepare/Commit 단계, 거절 사유, Display 결과를 추적한다. 모델 원문 로그/개인정보 보존 정책은 별도 결정.

[ADR-003](../../03_Decisions/03_ADR-003-Commit-Before-Response.md) · [ADR-004](../../03_Decisions/04_ADR-004-Snapshot-State.md) · [ADR-005](../../03_Decisions/05_ADR-005-Minimal-Game-Adapter.md)

## Phase 1 구현 경계

현재 `WAINTransactionCoordinator::Execute`는 상단 순서대로 단계를 실행한다. 실패 시 Root 참조는 변경되지 않는다. `WAINStateStore::Publish`는 기대 Revision 일치 시 Root 하나를 교체하며, 다중 Thread, 독립 Actor·외부 저장소, 영속성 원자성까지 확장하지 않는다. `IWAINGameSnapshotAdapter`의 계약 위반(Prepare 중 권위 State 직접 변경)까지 Plugin이 자동으로 차단하지는 못한다.

MVP에서 아직 지원하지 않는 State Target과 Presentation/Action ID는 **검증 완료로 간주하지 않고 거절**한다. 실제 LLM 응답, Hard Constraint Engine 및 Host Presentation Catalog 조회는 후속 단계다.

Commit 성공 뒤 `UWAINRuntimeSubsystem`이 `OnStateCommitted(TurnId, Revision)`를 방송한다. **같은 TurnId의 재전송에서는 중복 방송하지 않는다.** Testbed UI 알림/표시 재시도는 아직 구현되지 않았다.

## Phase 2 Presentation 검증 확장 (2026-11-11)

기존 문서의 'Presentation ID는 거절'은 Phase 1 당시의 상태다. 현재는 Host Catalog가 제공한 `(ExpressionId, VariantId)` 조합이 정확히 일치하는 경우에 한해 Presentation을 허용한다. Catalog 조회 실패·불완전한 후보 목록은 Context Preparation에서 실패하고, 미등록 ID·Variant는 Transaction의 첫 Validation에서 거절하여 **WAIN/Game Prepared State를 만들기 전에** No Commit한다. Action ID와 완전한 Narrative Hard Constraint 엔진은 아직 미지원이다.

[Presentation 구현](../../04_Development/07_Presentation-Catalog-Implementation.md)
