# ADR-020 — Five-Dimension Relationship & PresentationStep Contract

- **상태:** Proposed (실제 UE5.8 빌드·PIE 검증 전)
- **일자:** 2026-11-14

## 배경

단일 Trust만으로는 연애 게임의 신뢰·호감·친숙함·편안함·연애 감정을 표현할 수 없었다. 단일 Dialogue/Expression 응답은 감정 변화에 따른 VN 페이지 전환을 표현하지 못했다. 온라인 채팅체가 캐릭터 대사에 섞이고 Persona가 짧아 마나의 말투도 불안정했다.

## 결정

1. 상태에 `Trust/Affection/Familiarity/Comfort/RomanticAttraction` 5개 정수(0~100)를 적용한다. 다른 Agent와의 양방향 저장 구조는 추후 작업이다.
2. 공통 `RelationshipDefinitions.json`은 각 파라미터의 `low/neutral/high` 의미적 기준점을 보관한다. 마나만의 반응은 `ManaPersona.json.relationship_responses`에 둔다.
3. 공통 금지 대사는 `DialogueRules.json`에 정의하고 System Instruction과 Commit Validator 양쪽에서 적용한다.
4. **단일 Dialogue/Expression 계약을 제거한다.** Proposal/Result 모두 `PresentationSteps` 배열을 사용한다. 구형 스키마 fallback/자동 변환 없음.
5. Interaction Turn당 Commit은 1회이며, Host UI는 Step마다 대사와 표정을 변경한다. Narrative Beat는 기존 개념 그대로 유지한다.
6. 기존 Testbed 시나리오의 관계 초깃값은 별도의 `CH01RelationshipState.json`에 둔다.

## 결과 및 제약

- 변경 영향: Core Proposal/Result, LLM Schema, Context/Prompt, Character Preparer, Memory Writer, Testbed Data/Adapter/UI, DEV/Tests.
- 메모리에는 여러 Step의 대사를 하나의 Observation으로 합쳐 기록한다.
- 최대 5 Steps, 800자/Step. 숫자 변화의 임계값은 hard threshold가 아닌 의미 기준점이다.
- UE5.8 빌드·PIE·실 API 호출 전까지 Proposed 상태 유지.
