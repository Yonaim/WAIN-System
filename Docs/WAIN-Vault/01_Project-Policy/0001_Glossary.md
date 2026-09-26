# 용어집

이 용어집은 WAIN System 제안서에서 정의한 시스템 역할과 프로토타입 개념을 정리한다. 용어의 구체적인 구현 방식은 별도 설계와 검증을 거쳐 확정한다.

| 용어 | 뜻 |
|---|---|
| **WAIN System** | 비주얼 노벨의 자유형 자연어 상호작용에서 캐릭터 경험, 게임 세계, 서사 진행을 지속적으로 반영하기 위해 제안한 World-Agent Interactive Narrative System. |
| **Character Agent** | 캐릭터 상태와 플레이어 입력을 참조하고, 서사 목표와 제약을 고려해 행동 방향 및 LLM 입력 Context를 구성하는 시스템 역할. |
| **Character State** | 캐릭터의 Persona, 플레이어와의 관계, 현재 감정 등 상호작용에 지속적으로 반영하는 캐릭터 정보. |
| **Persona** | 캐릭터의 성격, 배경, 가치관과 말투 등 기본 설정. |
| **Relationship State** | 플레이어와 캐릭터 사이에 누적되는 관계 정보. 제안서 예시에는 친밀도(Affinity)와 신뢰도(Trust)가 포함된다. |
| **Emotion State** | 상황에 따라 변하는 캐릭터의 감정 상태. |
| **Memory** | 플레이어와의 과거 상호작용 경험을 기록하고, 이후 관련 경험을 검색해 활용하는 기능. 제안서에서는 경험, Reflection, Plan을 자연어 기반으로 다루는 구상을 제시한다. |
| **Reflection** | 여러 관련 Memory를 종합해 경험에서 상위 수준의 인식이나 의미를 만드는 과정. |
| **Planning** | Memory, Reflection, 현재 Context를 바탕으로 이후 행동 방향이나 계획을 만드는 과정. |
| **World State** | 현재 장소·시간·이벤트·규칙 및 세계의 변화처럼 상호작용에 반영할 게임 세계 정보. |
| **World State Manager** | World State를 조회·갱신하고 다른 시스템 역할에 제공하는 모듈. |
| **Narrative State** | 현재 장면과 이야기의 진행 단계, 공개된 정보 등 서사 진행 상황을 나타내는 정보. |
| **Narrative Manager** | World, Narrative, Character State를 참조해 현재 장면의 Narrative Goal과 Narrative Constraint를 결정하는 시스템 역할. |
| **Narrative Goal** | 현재 장면에서 달성하려는 서사적 목표. |
| **Narrative Constraint** | 현재 서사 진행에서 응답이나 행동이 넘어서는 안 되는 제약. |
| **Context** | Character Agent가 상태, 관련 Memory, 플레이어 입력, Narrative Goal/Constraint 등을 모아 LLM에 전달하는 입력 정보. 구체적인 형식은 미정이다. |
| **Testbed** | WAIN 플러그인을 적용해 검증하기 위한 별도의 Visual Novel 테스트 프로젝트. 제안서에서는 UI, Scene/Event, Story Data와 Single-Agent 시나리오를 포함하는 구상을 제시한다. |
| **Single-Agent Prototype** | 초기 프로토타입에서 하나의 Character Agent를 사용해 핵심 기능을 검증하는 범위. |
| **Multi-Agent Extension** | 여러 Character Agent가 상호작용하는 구조로 확장하는 장기 목표. 초기 프로토타입 범위에는 포함되지 않는다. |

프로젝트 범위와 역할 설명은 [[../02_Planning/00_Project-Overview/0000_Project-Plan|Project Plan]]을, 시연 예시는 [[../02_Planning/00_Project-Overview/0002_Demo|목표 데모]]를 참고한다. 기준 자료는 [[../07_Assets/26-2_졸업프로젝트_WAIN_제안서.pdf|졸업프로젝트 제안서]]다.
