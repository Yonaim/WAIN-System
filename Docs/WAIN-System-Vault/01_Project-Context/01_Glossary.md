# WAIN Glossary

| 용어 | 의미 / 소유권 |
|---|---|
| Interaction | 플레이어 입력 하나를 처리하는 Turn 단위 작업 |
| Transaction | 한 Interaction에서 승인된 WAIN/Game State 변경을 하나의 논리적 확정 대상으로 다룸 |
| Current Snapshot | 현재 확정된 WAIN-owned State의 읽기 가능한 버전 |
| Prepared Snapshot | Current를 직접 수정하지 않고 생성한 변경 후보 State |
| Revision | State 버전을 식별해 오래된 준비 결과의 Commit을 방지하기 위한 값 |
| State | 턴 간 유지되는 권위 있는 데이터. Prompt/Proposal 그 자체가 아님 |
| Character Agent | Persona·Relationship·Emotion·Memory/Plan에 근거해 응답 제안 |
| Persona | Host가 제공하는 상대적으로 고정적인 성격·말투 정의 |
| Relationship | WAIN이 소유하는 Affection, Trust, Attachment 등 누적 상태 |
| Emotion State | `joy/sadness/anger/disgust/anxiety/surprise` 0~1 독립 여섯 값; 표정과 별개 |
| EmotionDelta | 감정 축별 증감에 대한 승인 전 변경 Proposal |
| Expression ID | Host 소유 화면 표시용 의미 ID (`EXP_MANA_*`) |
| Expression Variant | Expression ID에서 선택 가능한 게임별 눈·입·오버레이 구성 |
| Presentation Catalog | Host의 허용 Expression/Variant 및 설명, 파츠/PNG 데이터 정의 |
| Memory Stream | Observation / Reflection / Plan 기록과 검색 기반 |
| World Fact | World State Manager 또는 Host Adapter 경계로 접근하는 세계 관련 값 |
| Beat | Chapter/Sequence 안에서 Goal·Constraint·Completion 조건을 갖는 서사 단위 |
| Proposal | 생성 모델이 제안한 대사·행동·표현·상태 변경의 미검증 후보 |
| Commit | 준비·검증된 변경을 권위 State로 확정하는 단계 |
| External State Adapter | Game-owned State 조회, 준비 및 지원되는 Transaction Commit 참여 경계 |
| Session State Root | 초기 Testbed에서 WAIN/Game Prepared State를 하나의 확정 참조로 묶는 구현 후보; 범용 Host 필수 데이터 형식 아님 |
| Testbed | WAIN을 검증하는 Unreal Host 게임. 게임 콘텐츠/에셋 소유 |
| Mock | 제공된 C++의 규칙·턴수 기반 응답이며 실제 WAIN 런타임 아님 |
