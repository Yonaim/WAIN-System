# 용어집

| 용어 | 정의 |
|---|---|
| WAIN | A World-Agent Interactive Narrative System for Visual Novels. 자유형 상호작용을 캐릭터·세계·서사 상태 관리와 결합하는 Unreal Engine C++ 플러그인 목표 |
| Interaction | 플레이어 입력 하나를 받아 응답 제안, 검증, 상태 확정과 VN 출력을 수행하는 턴 단위 작업 |
| Character Agent | 캐릭터 상태와 Memory를 참조해 Reflection·Planning을 수행하고 응답 및 변경 Proposal을 생성하는 모듈 |
| Memory Stream | Observation, Reflection, Plan을 공통 Schema와 ID 참조로 저장하는 캐릭터 경험 저장소 |
| World State | 게임 세계의 사실과 상태. 각 상태의 단일 소유자를 유지하고 필요한 Context만 제공한다 |
| Internal State Store | WAIN이 소유하는 World State를 저장·조회·변경하는 영역 |
| External State Adapter | 게임이 소유하는 State의 조회와 승인된 변경 요청을 해당 게임 시스템에 연결하는 인터페이스 |
| Narrative Manager | Chapter·Sequence·Beat 정의와 런타임 진행 상태, Goal·Constraint 및 Beat 전이를 관리하는 모듈 |
| Beat | Narrative Graph의 실행 노드. Goal, Completion Condition, Constraint와 조건부 Transition을 가진다 |
| Goal | 캐릭터 또는 서사가 달성하려는 방향을 자연어로 설명하는 값 |
| Completion Condition | Goal의 달성 여부를 구조화된 State 조건으로 판정하는 기준 |
| Constraint | 응답 허용 범위를 제한하는 규칙. Soft는 자연어 지침, Hard는 기계적으로 검증하는 조건이다 |
| Proposal | LLM이 생성한 응답·행동·상태 변경 후보. 검증과 Commit 전에는 확정 상태가 아니다 |
| State Commit | 승인된 변경을 실제 소유 시스템에 적용하는 단계 |
| Single Source of Truth | 동일 State를 둘 이상의 시스템이 독립적으로 소유·관리하지 않는 원칙 |
| 단일 에이전트 | 초기 프로토타입에서 한 Interaction에 한 Character Agent만 처리하는 범위 |
| 미확정 | 설계 방향은 논의됐지만 값·API·정책은 아직 결정되지 않은 상태 |
