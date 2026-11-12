# ADR-002 — Continuous Emotion & Host Presentation Catalog

- **상태:** Accepted (설계, 미구현)
- **날짜:** 2026-11-10
- **관련 요구:** FR-06, FR-10, FR-12, UR-02, UR-05

## 맥락과 문제

`Emotion(Type, Intensity)` 단일 상태로는 동시·혼합 감정을 충분히 표현하기 어렵다. 또한 게임별 PNG와 Expression을 Plugin에 고정하면 재사용성이 떨어진다.

## 고려한 선택지 및 트레이드오프

- 단일 Enum + 강도: 단순하지만 복합 감정·연출의 독립성이 약하다.
- 복합 감정별 Enum 추가: 조합과 게임별 용어가 무한히 증가한다.
- **독립 6차원 연속값 + Host Expression Catalog (선택):** 검증·해석 규칙이 필요하지만 표현력과 분리도가 높다.

## 결정과 근거

- Emotion 정본은 Character Agent가 소유하는 `joy`, `sadness`, `anger`, `disgust`, `anxiety`, `surprise` **여섯 독립 float(각 0~1)**이다. 합계가 1일 필요는 없다.
- 변화는 `EmotionDelta` Proposal로 받되 Validation/Preparation에서 범위·규칙 검사 후 Commit한다. 복합 감정의 이름은 Context를 고려한 **해석**이며 별도의 권위 Enum으로 저장하지 않는다.
- Emotion 값과 Expression/Variant는 별개다. 예: Anxiety가 높아도 Soft Smile을 표시할 수 있다.
- Host가 Expression ID, Variant, 설명, PNG 파츠를 소유한다. Plugin은 Context Preparation에서 `IWAINPresentationCatalog` 개념 계약으로 **현재 허용된 ID/Variant/설명**을 조회해 LLM Context에 전달한다.
- Proposal의 Expression은 조회한 허용 목록 및 Host 정책으로 검증한다. 없는 ID를 임의의 Neutral로 대체하지 않으며, Host가 명시한 대체 정책만 사용한다.
- `embarrassed` 같은 이름은 게임별 Expression Preset으로 사용할 수 있으나, WAIN의 6차원 Emotion State에 새 축을 추가하는 것은 아니다.

## 예상 결과 및 영향

기존 Testbed `EmotionDefinitions.json`, `ManaEmotion.json`의 `Type/Intensity` 초안은 **신규 계약과 불일치**한다. 실제 마이그레이션은 별도의 Content 패치/검증 작업이다. 현재 JSON이 수정되거나 로더가 구현됐다고 기록하지 않는다. PNG 파일 이름과 Display Layer는 Host 책임이다.

## 재검토 조건 / 대체 ADR 링크

Emotion 범위 및 시간 감쇠 수식, Context 목록 필터링 정책, Host 기본 Expression 선택 정책은 구현·실험에서 별도 결정한다.
