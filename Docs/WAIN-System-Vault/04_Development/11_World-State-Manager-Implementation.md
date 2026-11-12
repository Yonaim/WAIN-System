# Phase 2 — World State Manager / Schema-Driven Game Adapter Validation

> **상태:** 코드 및 Unreal Automation Test 9개 추가, **UE 5.8 컴파일·실행 미검증**. 직전 Narrative Graph 버전의 테스트 59개에 신규 9개를 더해 **누적 68개 테스트 등록**. 관련 결정은 [ADR-011](../03_Decisions/11_ADR-011-World-Fact-Schema-and-Prepared-Validation.md) Proposed.

## 00. Top-Down 처리 순서

```text
StartSession
  Adapter.CreateInitialSnapshot
  Adapter.Validate
  WorldManager.ValidateInitial                 ← Schema, Reader, 초기 Host 값 검사
  NarrativeEvaluator.ValidateInitialState
  StateStore.Initialize

ProcessTurn
  Read Committed Root
  Build Character/Memory/Presentation Context
  Build Narrative Context
  WorldManager.BuildContext                   ← 공개 가능한 Host Fact만 전달
  ProposalProvider.GenerateWithContext
  Transaction.Execute
    ValidateProposal
    Prepare WAIN Mutations / Observations
    WorldManager.ValidateMutations            ← Key·권한·범위·순차 적용 검사
    Adapter.Prepare                            ← Host Game 복사본 생성
    WorldManager.ValidatePrepared             ← 실제 Prepared 값 재대조
    NarrativeEvaluator.Evaluate               ← 공통 World Reader 기반 Fact 검사
    MemoryReasoning.Prepare
    Final WAIN/Game Validation
    Publish Root                              ← WAIN+Game 동시 확정
  Return VN Response
```

`Manager == nullptr`이면 기존 Mock 경로 그대로 작동한다. 기존 Narrative Graph는 `IWAINNarrativeGameFactReader` 별칭을 통해 공통 World Reader를 이용한다. **Context에는 비공개 Fact의 ID와 값 모두 포함되지 않는다**.

## 10. 소스와 API

| 경로 (`Source/WAINSystem` 기준) | 책임 |
|---|---|
| `Public/WAINWorldTypes.h` | World Fact Reader, Schema 정의와 공개 Context 값 |
| `Public/WAINWorldStateManager.h` | Schema 검증, Context, Mutation 및 Prepared Snapshot 검증 계약 |
| `Private/World/WAINWorldStateManager.cpp` | 읽기·연속 Mutation 계산·범위·실제 Prepare 결과 검증 |
| `Public/WAINNarrativeTypes.h` | Narrative Reader를 공통 World Reader 별칭으로 제공 |
| `Public/WAINCharacterContext.h` | `bHasWorldContext`, `World.VisibleFacts` |
| `Public/WAINInterfaces.h` | World Context를 무시하는 Legacy Provider의 조용한 우회 방지 |
| `Private/Core/WAINInteractionProcessor.{h,cpp}` | 선택적 Manager 주입·초기 검사·Context 준비 |
| `Private/Core/WAINTransactionCoordinator.{h,cpp}` | Adapter Prepare 전후의 World 검사 |
| `Public/WAINRuntimeSubsystem.h`, `Private/WAINRuntimeSubsystem.cpp` | `StartSession(..., WorldManager = nullptr)` 선택 인자 |
| `Private/Tests/WAINWorldStateTests.cpp` | 세계 상태 테스트 9개 |

## 20. 공개·쓰기권한과 실패 정책

- `bExposeToAgent`: `World.VisibleFacts` 포함 허용. False이면 명칭도 전달하지 않음. C++ Provider는 `Current.Game`에 여전히 접근할 수 있으므로 **LLM 전용 구조화 Context로 직렬화할 때만 공개 제한을 의미**한다.
- `bWritableByProposal`: Proposal의 Game Mutation 허용 여부. false이면 `InvalidProposal`.
- `MinValue`/`MaxValue`: 초기값·매 단계 연산 후 값·최종 Prepared를 모두 검사. 기본 범위 [0, INT32_MAX].
- 한 키의 Add/Set가 여러 번 포함되면 **순차적**으로 계산해 최종 예상값과 비교한다.
- Schema 내 다른 Key가 Adapter의 Prepare로 바뀌면 `GamePrepareFailed`. 권위 Root는 변경하지 않는다.
- Schema 미등록 Key/Read 실패/Reader 부재: Session 초기 실패 또는 Turn No Commit. 실제 Game Adapter 자체의 내재적인 규칙은 `Adapter.Validate()`가 담당.

## 30. 검증 현황과 다음 확장

신규 `WAIN.World.*` 자동화 테스트: VisibleContext, MutationCommit, ReadOnlyReject, BoundsReject, UnregisteredKeyReject, DuplicateSchemaReject, InvalidInitialReject, PreparedDriftReject, ReplayNoDoubleCommit.

**2026-11-13 검증:** `WAINTestbedEditor Win64 DebugGame` UE 5.8 빌드 성공. `FWAINWorldStateManager`를 Host 모듈에서 링크할 수 있도록 모듈 export를 표시했다. World 테스트 9개와 기존 테스트의 Automation 실행은 별도로 필요하다.

다음 작업은 실제 Testbed의 Game Content Schema/Fact 값 JSON을 로딩하는 Host Adapter, 다중 타입 World Facts, World Context용 LLM 직렬화, 관측/로그, 권한별 정보 공개 규칙 등이다.
