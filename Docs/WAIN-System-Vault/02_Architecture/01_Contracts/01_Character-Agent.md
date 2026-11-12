# Character Agent Contract

> 상태: **채택된 설계, Plugin 구현 없음**. [ADR-001](../../03_Decisions/01_ADR-001-State-Ownership.md) · [ADR-002](../../03_Decisions/02_ADR-002-Emotion-Expression.md).

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
