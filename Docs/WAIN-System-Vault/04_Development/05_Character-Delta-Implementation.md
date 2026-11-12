# Phase 2 — Character Delta Implementation

> 상태: **구현 소스 및 Unreal Automation Test 작성, 엔진 빌드·실행 별도 검증 필요**  
> 기준: 사용자 제공 Phase 1 검증본 `WAINTestbed-Source(1).zip` (2026-11-10)

## 00. 목적과 범위

Character Agent의 상태 관리 기능을 작은 단위로 구현한다. 범위는 현재 Character의 `Trust`와 여섯 독립 Emotion 값의 Mutation 적용이다. **Persona/LLM Context 생성·메모리/Reflection/Planning·감정 자연 감쇠는 아직 구현하지 않는다.**

[ADR-002](../03_Decisions/02_ADR-002-Emotion-Expression.md) · [Character Contract](../02_Architecture/01_Contracts/01_Character-Agent.md)

## 10. 상위 처리 흐름

```text
InteractionProcessor::ExecuteTurn
  → TransactionCoordinator::Execute
      → ValidateProposal
      → PrepareWAINState
          → Copy Current Snapshot
          → CharacterStatePreparer::Apply
              → ApplyTrust / ApplyEmotionDelta
          → Increment TurnCount
      → GameAdapter::Prepare
      → NarrativeEvaluator::Evaluate
      → Final Validation
      → StateStore::Publish
```

**권위 상태 변경:** 오직 `StateStore::Publish`에서 Shared Root를 교체한다. Character Preparer는 `Prepared` 참조만 수정하며 기존 Root를 수정하지 않는다.

## 20. 지원 계약

| Target ID | Operation | Payload | 규칙 |
|---|---|---|---|
| `relationship.trust` | `AddInteger`, `SetInteger` | `IntegerValue:int32` | 적용 결과가 `[0,100]` 밖이면 거절 |
| `emotion.joy` | `AddFloat` | `FloatValue:float` | 각 축 `Clamp(Current+Delta,0,1)` |
| `emotion.sadness` | `AddFloat` | `FloatValue:float` | 동일 |
| `emotion.anger` | `AddFloat` | `FloatValue:float` | 동일 |
| `emotion.disgust` | `AddFloat` | `FloatValue:float` | 동일 |
| `emotion.anxiety` | `AddFloat` | `FloatValue:float` | 동일 |
| `emotion.surprise` | `AddFloat` | `FloatValue:float` | 동일 |

- Emotion Delta는 유한 float만 허용한다. Emotion 값은 각 축이 독립적이며 합계 1로 정규화하지 않는다.
- Unknown Target / 정수 Emotion Operation / Game `AddFloat`는 거절한다.
- 여러 변경 중 하나라도 검증 실패하면 Commit을 수행하지 않는다.
- `FWAINStateMutation`에 `FloatValue`가 추가되었지만 기존 정수 Target·Game Adapter API는 유지한다.

## 30. 테스트와 회귀

`WAIN.Transaction.*` 기존 여섯 테스트와 `WAIN.Character.*` 여섯 테스트를 모두 실행한다. Unreal Editor Session Frontend → Automation에서 필터링할 수 있다.

**현재:** 새 C++ 코드와 테스트만 생성했다. 이 환경에서는 Unreal 5.8/UBT를 실행하지 못했으므로 실제 Pass/Fail을 기록하지 않는다.

## 40. 다음 단계

1. 사용자 환경에서 전체 12개 테스트 컴파일·실행 결과 확인.
2. Host Persona Data 계약과 Character Context 조립 API를 설계 및 구현.
3. Host Presentation Catalog Query와 제한된 Context 표현 목록을 연결.

[Source Map](../02_Architecture/02_Source/00_Source-Map.md) · [Test Plan](../05_Experiments/00_Test-Plan.md) · [Roadmap](02_Implementation-Roadmap.md)
