# ADR-006 — Turn-Based Single-Agent MVP Scope

- **상태:** Accepted (설계, 미구현)
- **날짜:** 2026-11-10
- **관련 요구:** UR-01~05, FR-01, NFR-01, NFR-04

## 맥락과 문제

다중 Agent/자율 World Event/실시간 이벤트를 동시에 도입하면 Memory·State 일관성 및 서사 제어의 검증 범위가 지나치게 넓어진다.

## 고려한 선택지 및 트레이드오프

- 자유 실행형 다중 Agent: 상호작용 다양성이 높으나 상태 충돌/평가가 복잡하다.
- **Single-Agent, Player Interaction 중심 Turn-Based (선택):** 긴 호흡의 캐릭터/관계/서사 일관성을 먼저 검증할 수 있다.

## 결정과 근거

- 한 번의 Player Interaction(Text 또는 명시적 Silence)을 턴의 실행 단위로 삼는다.
- Character Agent 한 명의 응답 생성 파이프라인으로 시작한다. 다른 캐릭터/씬 콘텐츠의 추가 가능성은 유지하되 다중 Agent 간 자율 조정은 초기 범위가 아니다.
- 독립적으로 자주 실행되는 NPC AI, 자유 실시간 World Event, 다중 Agent 간 자발적 상호작용은 MVP 범위에서 제외한다.
- 게임 내 Goal/Constraint를 가진 Narrative Manager와 Persistence 가능한 Memory/Relationship 유지에 집중한다.

## 예상 결과 및 영향

Turn Transaction의 순차 처리와 비교 평가(A~D)를 간단하게 할 수 있다. 시스템이 복수의 자율 Character Agent를 지원한다고 홍보하거나 문서화하지 않는다.

## 재검토 조건 / 대체 ADR 링크

MVP의 기능·평가 결과가 확보된 이후 다중 캐릭터 간 대화 및 비동기 World Event 지원을 개별 ADR로 검토한다.
