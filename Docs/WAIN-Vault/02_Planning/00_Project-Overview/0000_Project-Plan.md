# WAIN System 프로젝트 계획

상태: **제안서 기준 프로젝트 범위** · 단계별 계획은 [[02_Planning/00_Project-Overview/0001_Roadmap|Roadmap]], 목표 시연은 [[02_Planning/00_Project-Overview/0002_Demo|목표 데모]]를 참고한다.

## 배경과 문제

비주얼 노벨에서 자유형 자연어 입력과 LLM 응답 생성을 사용하면 사전 작성 선택지를 넘어서는 상호작용이 가능하다. 이를 장기적인 게임 경험으로 이어가려면 캐릭터의 성격과 경험, 플레이어와의 관계, 게임 세계의 변화, 의도된 서사 진행을 계속 반영해야 한다.

## 프로젝트 목표

플레이어와의 경험·관계와 게임 세계의 변화를 지속적으로 반영하면서, 자유형 상호작용에서도 캐릭터의 장기적 일관성과 의도된 서사 진행을 유지하는 **World-Agent Interactive Narrative System (WAIN)** 프로토타입을 구현한다.

제안 자료: [[../../07_Assets/26-2_졸업프로젝트_WAIN_제안서.pdf|졸업프로젝트 기획 제안서]], [[../../07_Assets/26-2_졸업프로젝트_WAIN_제안서_발표대본|발표 대본]].

## 프로토타입 구성

WAIN은 게임 콘텐츠와 분리된 Unreal Engine C++ Plugin으로 구현하고, 별도의 Visual Novel Testbed에 적용해 검증하는 것을 목표로 한다.

| 구성 요소 | 제안된 책임 |
|---|---|
| Character Agent | Character State와 Player Input을 참조하고, 행동 방향과 LLM Context를 구성한다. |
| Memory / Reflection / Planning | 상호작용 경험을 기록·검색하고, 경험을 해석해 이후 행동 계획에 활용한다. 생성된 Reflection과 Plan도 다음 상호작용을 위해 저장하는 구상을 포함한다. |
| World State Manager | 장소·시간·이벤트·세계 규칙 등 World State를 조회·갱신하고 다른 역할에 제공한다. |
| Narrative Manager | World, Narrative, Character State를 참조해 현재 장면의 Narrative Goal과 Constraint를 정한다. |
| LLM Integration | Character Agent가 구성한 Context를 바탕으로 Dialogue와 Action을 생성한다. 지속 상태 자체는 WAIN 시스템에서 관리한다. |
| Visual Novel Testbed | UI, Scene/Event, Story Data와 Single-Agent 시나리오로 플러그인 기능을 확인한다. |

이 표는 제안서의 역할 구분이다. 실제 API, 데이터 모델, 저장 방식과 모듈 경계는 구현 전에 설계 문서에서 확정한다.

## 프로토타입 범위

- Character Persona, Relationship State, Emotion State와 상호작용 경험의 지속 반영을 다룬다.
- Memory, Reflection, Planning으로 경험이 이후 캐릭터 반응에 반영되는 흐름을 검증한다.
- World State를 LLM 바깥에서 관리하고 상호작용 결과를 이후 응답에 반영한다.
- Narrative Manager가 현재 진행 단계에 맞는 Goal과 Constraint를 정하고 Character Agent에 전달한다.
- 초기 테스트는 하나의 Character Agent를 사용하는 Single-Agent 시나리오로 한정한다.
- 특정 게임의 콘텐츠와 WAIN 시스템의 책임을 분리하고, 별도 VN Testbed에서 통합한다.

여러 Character Agent가 상호작용하는 Multi-Agent 구조는 향후 확장 목표이며 초기 프로토타입의 완료 조건이 아니다.

## 평가 계획

같은 테스트 시나리오와 입력을 사용해 구성 요소를 단계적으로 추가하며 효과를 비교한다.

| 구성 | 비교 조건 |
|---|---|
| A | LLM Only 기준선 |
| B | Character Agent 추가 |
| C | World State Manager 추가 |
| D | Narrative Manager 추가 |

시스템 검증에서는 과거 기억의 회수 정확도, 상태 반영, Narrative Constraint 위반 여부를 살핀다. 응답 품질은 캐릭터 일관성, 맥락 적절성, 자연스러움을 평가자 기반 5점 척도로 평가하는 것이 제안된 계획이다. 구체적인 시나리오, 측정 정의, 표본과 실행 환경은 평가 전에 문서화한다.

## 주요 산출물

- WAIN System Unreal Engine Plugin 프로토타입과 소스 코드.
- 시스템 설계 문서.
- 테스트 및 비교 평가 결과.
- 기능을 보여주는 Visual Novel Testbed 시연.

## 일정 관리

실행 순서는 [[02_Planning/00_Project-Overview/0001_Roadmap|Roadmap]]에서 단계로 관리한다. 제안서의 주차 표기는 제안 당시 일정이므로 실제 달력 일정의 근거로 사용하지 않는다. 주차별 계획을 다시 수립할 때는 [[00_Start/Templates/0006_Weekly-Overview|Weekly Overview 템플릿]]과 [[00_Start/Templates/0007_Weekly-Details|Weekly Details 템플릿]]을 사용한다.

실제 구현·검증 현황과 남은 작업은 [[TODO|TODO]]에서 추적하며, 계획에 적힌 목표만으로 완료 처리하지 않는다.
