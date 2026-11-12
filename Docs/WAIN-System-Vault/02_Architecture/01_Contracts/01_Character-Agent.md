# Character Agent Contract

> 상태: **부분 구현** — Phase 2 첫 단계에서 Trust 및 6축 Emotion Delta의 Prepared Snapshot 적용을 추가했다. Persona·Context Assembly·Memory는 미구현. [ADR-001](../../03_Decisions/01_ADR-001-State-Ownership.md) · [ADR-002](../../03_Decisions/02_ADR-002-Emotion-Expression.md).

## 책임

- **Persona:** 상대적으로 고정적인 성격·가치관·말투를 참조한다. 정의 콘텐츠는 Host 제공.
- **Relationship:** Affection, Trust, Attachment 등 지속값은 WAIN Character Agent가 정본을 소유한다. 스케일 및 변화량 제한은 미정.
- **Emotion:** `joy`, `sadness`, `anger`, `disgust`, `anxiety`, `surprise`의 0~1 독립 State를 관리한다. 복합 감정은 Context에 따른 해석이고 별도 State Enum으로 저장하지 않는다.
- **Memory:** Observation, Reflection, Plan을 기록·검색하고 다음 Context에 반영한다.
- **Context Assembly:** Persona/Relationship/Emotion/Memory와 World/Narrative, Host가 조회해 준 허용 Presentation 항목을 종합한다.

## 입력 / 출력

입력은 Player Interaction + Current Snapshot + World/Narrative Context + Memory Retrieval + Host Presentation Options. 출력은 Dialogue/Action/Expression/Event와 **EmotionDelta/Relationship/World 등 State Change Proposal**이다.

**모든 Proposal은 미검증 상태이며 Character Agent/LLM이 정본을 직접 수정하지 않는다.** Transaction Coordinator가 승인 후보를 Prepared Snapshot에서 평가하고 Commit한다.

## 표현 예시

`anxiety = 0.6`, `joy = 0.2`인 상태에서도 `EXP_MANA_SOFT_SMILE`을 제안할 수 있다. 단 Expression ID와 Variant는 이번 Context에서 허용된 Host Catalog 항목으로 검증해야 한다.

## 미결정

감정 시간 감쇠·스케일 조정, Relationship 연산 규칙, Memory Retrieval 점수/Trigger, 실제 C++ API·Persistence 자료형.

[Memory](02_Memory-Reflection-Planning.md) · [Emotion Boundary](07_Emotion-Expression-Boundary.md)

## Phase 2 — Character State Preparation (2026-11-10)

- `FWAINCharacterStatePreparer::Apply(CharacterId, Mutations, Prepared, Error)`가 **Prepared WAIN Snapshot**의 해당 Character만 수정한다. Committed Root는 변경하지 않는다.
- `relationship.trust`는 기존 `AddInteger`/`SetInteger`, `[0,100]` 범위 검사를 유지한다.
- `emotion.joy`, `emotion.sadness`, `emotion.anger`, `emotion.disgust`, `emotion.anxiety`, `emotion.surprise`는 **`AddFloat` 전용**이다. 각 Delta는 유한한 `float`여야 하며, 적용값은 `Clamp(Current + Delta, 0, 1)`로 계산한다.
- 다른 감정 차원을 비례 조정하거나 합계 1로 정규화하지 않는다. 미등록 Target, 잘못된 Operation, NaN/무한대는 Commit 이전에 거절한다.
- Character 변경을 준비한 뒤에도 Host Game Prepare·Narrative Evaluation·Final Validation이 끝나지 않으면 어떤 변경도 Publish하지 않는다.
- 이 단계는 **Agent의 상태 처리 기반**이며 Persona, 실제 LLM Context 생성, 메모리·감정 자연 감쇠는 여전히 구현 범위 밖이다.

[Character Delta Implementation](../../04_Development/05_Character-Delta-Implementation.md) · [ADR-002](../../03_Decisions/02_ADR-002-Emotion-Expression.md)

## Phase 2 — Persona & Character Context Assembly (신규)

[ADR-007](../../03_Decisions/07_ADR-007-Read-Only-Character-Context.md)에 따라 Host가 `IWAINPersonaSource`로 Persona를 읽기 전용 제공한다. `FWAINCharacterContextBuilder`는 Current Session Root의 해당 Character에서 Trust/6축 Emotion 및 Active Beat/Revision을 조회해 복사한 Context를 구성한다. Context Preparation 실패는 Proposal/Commit 전 거절된다. 기존 문서 상단의 Persona·Context 미구현 문장은 **이 단계 이전 상태**다.

새 `IWAINProposalProvider::GenerateWithContext`는 Source가 주입된 경우 Context를 사용하는 Provider의 구현이 필요하다. Source가 없으면 기존 Mock `Generate`를 유지한다. Persona 문자열을 곧바로 LLM Prompt로 직렬화하는 단계는 아직 구현되지 않았다. 서사상의 비밀을 포함하는 Persona 필드의 노출은 Host가 Active Beat/정책에 맞춰 제한해야 한다.

[Phase 2 Context Implementation](../../04_Development/06_Character-Context-Implementation.md)
