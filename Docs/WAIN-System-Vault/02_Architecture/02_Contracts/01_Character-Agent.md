# Character Agent

> 캐릭터의 장기 설정과 경험을 사용해 이번 턴의 응답 및 상태 변경 Proposal을 만든다.

상태: **기본 설계 기준** · 구현 확인: 미완료

## 책임

1. Persona, Relationship, Emotion 등 Character State 조회
2. Query와 관련된 Memory Retrieval
3. 필요할 때 Reflection과 Planning 수행
4. World·Narrative Context를 포함한 Response Context 구성
5. Shared LLM Service로 Dialogue, 표현/행동, State Change Proposal 생성
6. 결과 반환. 실제 State 확정은 Validator와 Commit이 담당

Character Agent가 항상 LLM을 한 번만 호출하지는 않는다. Reflection, Planning, Response Generation은 필요에 따라 공유 서비스를 호출한다.

## Character State

| 상태 | 의미 | 초기 설계 |
|---|---|---|
| Persona | 성격, 가치관, 말투, 배경, 선호 등 비교적 고정된 정체성 | 일반 대화에서 임의 변경하지 않음. 의도적 성장 이벤트에는 명시 정책 필요 |
| Relationship | 캐릭터와 플레이어 사이의 장기 관계 | 기본 항목은 Affection(호감도), Trust(신뢰도), Attachment(애착도). 각각 독립적으로 변하며 후속 응답과 행동에 영향 |
| Emotion | 비교적 단기적인 정서 상태 | Emotion Type과 Intensity. 관계보다 빨리 변하거나 감쇠 가능 |

정확한 타입, 범위, Clamp·Decay, Relationship 업데이트 계수는 확정되지 않았다.

Affection은 상대를 좋아하는 정도, Trust는 상대의 말과 행동을 믿는 정도, Attachment는 관계를 잃는 데 대한 심리적 저항을 뜻한다. 세 값은 같은 방향으로 움직일 필요가 없다. 기존 인계 문서의 `Affinity / Trust` 표기는 초기 설계 기록이며, 현재 기본 항목은 위 세 값으로 정한다.

## State Change Proposal

LLM이 관계 점수를 임의 숫자로 확정하지 않고 의미적 방향·강도(예: Increase/Decrease/Neutral, Small/Medium/Large)를 제안하는 방식을 우선한다. 정책 계층이 그 의미를 검증 가능한 수치 변경으로 변환한다. 적용량과 정책은 미확정 설계 변수다.

## 개념적 비동기 인터페이스

```text
ProcessAsync(Request, OnComplete)
Request: InteractionId, CharacterId, PlayerInput, WorldContext, NarrativeContext
Result: ResponseProposal, StateChanges, MemoryOperations, Status
```

이는 설계 문서의 개념 인터페이스이며 실제 함수명·타입·Callback 계약이 아니다. 한 캐릭터당 동시 Interaction은 한 건으로 제한한다. Reflection·Planning Memory의 최종 Commit 경계도 미확정이다.

## 관련 문서

- [[02_Memory-Reflection-Planning|Memory, Reflection, Planning]]
- [[05_Response-Validation-and-Commit|Proposal 검증과 Commit]]
- [[../01_Flows/01_Interaction-Pipeline|Interaction Pipeline]]

