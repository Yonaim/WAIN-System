# Constraints — Architecture & Prototype Boundaries

1. **Game Content 독립성:** Plugin에 `MANA`, 특정 PNG 이름, 호스트 배경·대본 경로를 하드코딩하지 않는다. 예시 ID는 테스트 문맥에서만 사용한다.
2. **State Ownership:** Character/Memory/Emotion, Narrative State, WAIN World Facts는 WAIN 정본. Inventory/Quest/Game Flags는 Host 소유이며 Game Adapter로 읽기·쓰기 제안에 참여한다.
3. **Interaction-Driven Update:** Single-Agent Turn-Based 프로토타입. 독립 실시간 이벤트와 NPC 자율 행동 제외.
4. **검증 전 변경 금지:** LLM Proposal은 신뢰되지 않으며, 승인 후보는 Current를 수정하지 않고 Prepared Snapshot에서 계산한다.
5. **Transaction 순서:** Response Validation → Next State Preparation → Narrative Evaluation → Final Validation → Atomic Commit → State Notification → VN Response. Beat 전이는 턴당 최대 1회.
6. **원자적 적용의 지원 범위:** 초기 Testbed는 하나의 Commit 경계로 WAIN/Game Prepared Snapshot을 함께 확정하는 방식만 지원한다. 독립 Actor/DB/Network의 다중 Store Transaction은 아직 미지원. 구현·검증 전에는 원자성을 주장하지 않는다.
7. **정보 공개:** 현재 Beat에 허용되지 않은 비밀을 LLM Context에 노출하지 않는다. Hard Constraint는 모델 지시만이 아니라 결정론적 Validator로 검사한다.
8. **Emotion ≠ Expression:** 여섯 독립 0~1 Emotion(`joy/sadness/anger/disgust/anxiety/surprise`)과 Host Expression/Variant는 분리한다. Context Preparation에서 허용 Catalog를 조회한다.
9. **개발·코딩 규칙:** 최신 범용 Code Style의 **Top-Down Readability를 최우선**으로 삼고 [WAIN Implementation Principles](../04_Development/03_Implementation-Principles.md)를 따른다. 프로젝트 소유 C/C++는 한국어 `@file`·`@brief` 파일 책임 설명을 유지한다. Region·빈 줄은 의미 있는 구획에 적용하고, 불필요한 추상화와 무의미한 주석은 피한다. 실제 구현·테스트는 별도 기록한다.
10. **빌드·포맷 연결:** UBT는 유지하며, `.clang-format → indent_regions.py`를 컴파일 전에 실행하는 도구를 실제 빌드 타깃에 연결·검증해야 한다. 현재 볼트에는 원본 템플릿만 포함되어 있고 Plugin 소스/타깃 설정은 변경하지 않았다.

[ADR Index](../03_Decisions/00_ADR-Index.md) · [Main Flow](../02_Architecture/00_Flows/00_Main-Flow.md)
