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

## Phase 2 Character Automation Tests — 작성, Unreal 실행 전

| Test Name | 검증 목적 | 현재 |
|---|---|---|
| `WAIN.Character.EmotionDeltaAndRelationshipCommit` | 6차원 동시 변화 + Trust + Game + Beat 단일 Commit, 이전 Snapshot 불변 | 코드 작성 / 미실행 |
| `WAIN.Character.EmotionDeltaIsClamped` | 각 Emotion 차원 개별 0~1 Clamp | 코드 작성 / 미실행 |
| `WAIN.Character.UnknownEmotionTargetKeepsState` | 미등록 Emotion Target은 전체 No Commit | 코드 작성 / 미실행 |
| `WAIN.Character.IntegerEmotionMutationKeepsState` | Emotion에 잘못된 정수 연산 거절 | 코드 작성 / 미실행 |
| `WAIN.Character.NonFiniteDeltaKeepsState` | NaN Mutation 거절, Revision 및 Game 값 유지 | 코드 작성 / 미실행 |
| `WAIN.Character.DuplicateTurnDoesNotReapplyEmotion` | 동일 TurnId 재전송 시 Emotion 중복 반영 방지 | 코드 작성 / 미실행 |

**회귀 검증:** 기존 `WAIN.Transaction.*` 6개도 함께 실행. 완료 수치는 총 12개 테스트 *코드 작성*이며, **Phase 2 테스트 실행 성공을 의미하지 않는다**.

## Phase 2 Character Context Tests — 작성, Unreal 실행 전

| Test Name | 기대 동작 | 현재 |
|---|---|---|
| `WAIN.CharacterContext.UsesCommittedState` | Persona·이전 Trust/Emotion/Beat/Revision 조회, 이전 Root 불변 | 코드 작성 / 미실행 |
| `WAIN.CharacterContext.RefreshAfterCommit` | 새 Turn은 직전 Commit 후 State/Revision 확인 | 코드 작성 / 미실행 |
| `WAIN.CharacterContext.MissingPersonaNoCommit` | Host 조회 실패 시 Proposal 미호출·No Commit | 코드 작성 / 미실행 |
| `WAIN.CharacterContext.WrongIdentityNoCommit` | Character ID 불일치 시 No Commit | 코드 작성 / 미실행 |
| `WAIN.CharacterContext.LegacyProviderFallback` | Source 미주입 기존 Mock Provider와의 호환 | 코드 작성 / 미실행 |
| `WAIN.CharacterContext.LegacyProviderRejectsPersona` | Persona 제공되는데 미지원 Provider는 명시적 실패 | 코드 작성 / 미실행 |
| `WAIN.CharacterContext.ReplaySkipsAssembly` | 같은 Turn ID 재전송 시 Persona/Context 재조회 없음 | 코드 작성 / 미실행 |

현재 등록된 Unreal Automation Test 코드는 **기존 12 + 신규 7 = 총 19개**다. `WAIN.Transaction.*`, `WAIN.Character.*`, `WAIN.CharacterContext.*`를 함께 실행하고 결과 로그를 기록한다.

## Phase 2 Presentation Automation Tests — 작성, UE 실행 전 (2026-11-11)

| Test Name | 검증 목적 |
|---|---|
| `WAIN.Presentation.AllowedExpression` | 허용된 ID·Variant가 Proposal을 통과하고 Result에 포함, 기존 Snapshot 불변 |
| `WAIN.Presentation.NoExpressionAllowed` | 빈 Catalog에서 Expression을 자동 대체하지 않고 대사만 성공 |
| `WAIN.Presentation.UnknownExpressionNoCommit` | 미등록 Expression에서 No Commit |
| `WAIN.Presentation.UnknownVariantNoCommit` | 허용되지 않은 Variant에서 No Commit |
| `WAIN.Presentation.MissingCatalogNoCommit` | Catalog 미주입 상태의 표현 ID 거절 |
| `WAIN.Presentation.VariantWithoutExpressionNoCommit` | Expression 없는 Variant 제안 거절 |
| `WAIN.Presentation.InvalidCatalogEntryNoCommit` | 설명 없는 Host 후보 거절 |
| `WAIN.Presentation.DuplicateEntriesNoCommit` | 중복 `(ExpressionId,VariantId)` 후보 거절, Provider 미호출 |
| `WAIN.Presentation.LookupFailureNoCommit` | Host 조회 실패 시 Provider 미호출·No Commit |
| `WAIN.Presentation.LegacyProviderRejectsCatalog` | Catalog를 무시할 수 있는 Legacy Provider를 실패 처리 |
| `WAIN.Presentation.ReplaySkipsCatalog` | 동일 TurnId 재처리 시 조회·생성·Commit 중복 방지 |

총 등록 코드: **기존 19개 + 신규 11개 = 30개**. 테스트 코드는 작성됐지만 UE 컴파일·실행 통과는 주장하지 않는다. 회귀: `WAIN.Transaction.*`, `WAIN.Character.*`, `WAIN.CharacterContext.*`, `WAIN.Presentation.*`.

## Phase 2 Observation Memory Automation Tests — 작성, UE 실행 전 (2026-11-11)

| 테스트 | 검증 범위 |
|---|---|
| `WAIN.Memory.ObservationCommit` | Player/Character Observation을 성공 턴에만 기록, 이전 Snapshot 불변 |
| `WAIN.Memory.NextTurnContext` | 다음 턴에서 직전 확정 Memory Context 검색 |
| `WAIN.Memory.CharacterIsolation` | MemoryStream은 통합, 조회는 Character별 격리 |
| `WAIN.Memory.QueryRanking` | 임시 어휘 관련성 기반 우선순위 확인 |
| `WAIN.Memory.PrepareFailureNoCommit` | Game Prepare 실패 시 Observation No Commit |
| `WAIN.Memory.InvalidProposalNoCommit` | Proposal Validation 실패 시 Observation No Commit |
| `WAIN.Memory.DuplicateTurn` | Replay에서 Observation 중복 작성·재조회 없음 |
| `WAIN.Memory.SilenceTurn` | Silence 턴은 빈 Player 입력을 저장하지 않음 |

**총 38개 Automation 테스트가 소스에 작성된 상태**로, UE 5.8 빌드/실행 합격을 의미하지 않는다. 반드시 이전 30개 테스트와 함께 회귀 실행한다. Reflection SourceMemoryIds, Embedding Retrieval, Save/Load는 후속 테스트 대상이다.

## Phase 2 Reflection / Planning MVP Automation Tests — 코드 작성, UE 실행 전 (2026-11-11)

신규 11개 테스트: `WAIN.MemoryReasoning.PlanCreated`, `ReflectionTrigger`, `ContextIncludesDerivedMemory`, `SceneChangeReplans`, `BeatChangeReplans`, `CharacterIsolation`, `InvalidPlanNoCommit`, `InvalidReflectionNoCommit`, `GameFailureSkipsReasoning`, `ReplayDoesNotRegenerate`, `InvalidActivePlanRejected`.

**누적 테스트 코드 49개(기존 38 + 신규 11).** Reflection 출처·Plan Trigger·현재 Snapshot 불변·Provider/Adapter 실패를 검증하도록 작성. 실제 Unreal Automation 실행 및 결과는 기록되지 않았다. 이후 의미적 Reflection 품질, LLM 결과 구조/보안, 과도한 생성·장기 Memory 부하 테스트를 추가한다.

## Phase 2 Narrative MVP Automation Tests — 코드 작성, Unreal 실행 전 (2026-11-11)

10개: `WAIN.Narrative.ContextGoals`, `PreparedTransition`, `IncompleteBeat`, `TurnProgress`, `InvalidGraph`, `HardRuleReject`, `HardRulePreparedTrust`, `PriorityTieReject`, `UnknownFactReject`, `ReplayNoDoubleTransition`.

검증 대상: 읽기 전용 Chapter/Sequence/Beat Context, Prepared WAIN/Game 값으로 조건 검사, 높은 Priority 선택, 턴당 최대 한 Beat 전이, 정의 오류·누락 Fact·Hard Rule 거절 시 **No Commit**, 중복 TurnId 무중복.

**현재 누적 59개 Automation Test 코드 작성 상태. Unreal 5.8 컴파일/실행 결과는 아직 없다.** 회귀 실행은 기존 49개를 포함해야 한다. Soft Constraint 자연어 위반 탐지·실제 Scenario JSON Loader와 Host 연동 테스트는 아직 작성하지 않았다.

## Phase 2 World State Manager — 테스트 작성, UE 실행 전 (2026-11-12)

신규 9개: `WAIN.World.VisibleContext`, `MutationCommit`, `ReadOnlyReject`, `BoundsReject`, `UnregisteredKeyReject`, `DuplicateSchemaReject`, `InvalidInitialReject`, `PreparedDriftReject`, `ReplayNoDoubleCommit`.

의도된 검증: 비공개 Key의 Context 제외, Schema 권한 및 범위, 연속 Mutation, 초기 Schema 오류, Game Adapter가 의도치 않게 변경한 Prepared 값 차단, 원자적 No Commit, 재전송 중복 방지. **누적 68개 테스트 코드 등록이 실제 UE 컴파일·테스트 성공을 의미하지 않는다.**

## 2026-11-12 Host 실제 콘텐츠 테스트 — 코드 작성, UE 미실행

`WAIN.Testbed.Content.LoadAndValidate`, `Content.WorldFactVisibility`, `Runtime.IntroTransition`, `Runtime.ThreeBeatProgress`, `Runtime.ReplayDoesNotAdvance`, `Runtime.UnknownCharacterNoCommit`, `Runtime.ReadonlyFactRejected` 총 7개 Integration Test 코드를 추가했다. 실제 Testbed 프로젝트의 `Content/Data`가 필요하다. Plugin 기존 68개 테스트 코드와 함께 Editor에서 회귀 실행하되 실행 이전에 통과로 기록하지 않는다.

## Host Testbed Sprite Tests — 2026-11-13

`WAIN.Testbed.Sprite.*` 6개: Catalog Path/Count, Default Layer Order, Multiple Overlays, Invalid Variant No Fallback, All PNG Dimensions, Placeholder Metadata. 기존 Plugin 68 + Host Integration 7 + Sprite 6 = **81개 Automation 코드 등록**. UE 5.8 컴파일·실행 미검증. 실제 UMG alpha 합성/렌더 화면은 별도 PIE 수동 검증해야 한다.

## 2026-11-13 — UE 5.8 Editor Build Verification

`WAINTestbedEditor Win64 DebugGame` 빌드 성공. Plugin/Host 모듈과 81개 Automation 테스트 소스가 컴파일·링크됐다. Automation 테스트 실행 및 PIE 검증은 수행하지 않았다.

## Phase 2 DEV Inspector / Debug Trace (2026-11-13, 코드 작성·미실행)

신규 Plugin: `WAIN.DebugTrace.SuccessfulTurn`, `FailedProposalNoCommit`, `ReplayPreservesOriginal` 3개. 신규 Testbed: `WAIN.Testbed.Debug.StateDelta`, `NarrativeTransition`, `MemoryProvenance`, `ExecutedStages` 4개.

검증: 단계의 실제 수행 여부, Proposal와 Revision/No Commit, State Diff, BeatHistory, Memory Retrieval·Source, Replay 불변. 기존 86개 회귀 포함 총 93개 Automation Test 소스 등록. **실제 UE 5.8 Editor 실행/테스트 결과는 없다.**

## 2026-11-13 — Structured Proposal and Host LLM Handoff Tests

- `WAIN.StructuredProposal.*` 8개: Schema JSON, 정상 출력, Unknown/Missing/Wrong Type, Expression/Variant, 정수 Mutation 및 Fraction 거절.
- `WAIN.Testbed.LLM.*` 5개: Responses Envelope, incomplete/refusal, Queue Revision 불일치 및 1회 소비.
- 기존 93개와 함께 총 106개 Automation Test 코드 등록. **실제 Unreal 실행 미검증**. 외부 API를 네트워크로 호출하는 테스트는 의도적으로 추가하지 않았다.

## 2026-11-14 — Plugin LLM Private Layers Tests

- `WAIN.LLM.Prompt.*` 오프라인 테스트 3개로 System Instruction의 고정 계약, ReadOnly Context Prompt의 필드/누락 처리, 길이 제한을 확인한다.
- Organization Usage 및 OpenAI/Structured Proposal 테스트를 Plugin의 Private 구현 경로로 이동하고 Host는 공개 `WAINLLMTypes` 계약만 사용한다.
- 전체 Plugin/Host Automation 등록 코드는 **118개**다. UE 5.8 컴파일과 Automation 실행은 아직 수행하지 않았다.
- [ADR-019](../03_Decisions/19_ADR-019-LLM-Internal-Layers.md) · [LLM Internal Structure](../../../../../Docs/02_Implementation/11_LLM-Internal-Structure.md).
