# 선행 연구

아래 문헌은 프로젝트 설계 문서와 중간발표 자료에서 식별한 참고 자료다. 링크는 논문 원문 또는 출판사·연구 기관 저장소를 가리킨다.

## 지속형 Agent와 Memory

- Park et al. (2023), [Generative Agents: Interactive Simulacra of Human Behavior](https://arxiv.org/abs/2304.03442). Memory Stream, 최근성·관련성·중요도 기반 검색, Reflection, Planning의 참고 자료다. WAIN은 이러한 아이디어를 비주얼 노벨 캐릭터 Agent에 맞게 변형하며 논문의 전체 시뮬레이션을 재현한다고 주장하지 않는다. 논문에 나온 예시 파라미터는 WAIN 설정이 아니다.

## 서사 설계와 진행

- [Story Designer: Towards a Mixed-Initiative Tool to Create Narrative Structures](https://arxiv.org/abs/2210.09294) (2022). 프로젝트 설계는 Trope 중심 Narrative Graph와 사람·AI 협업형 서사 작성을 참고한다. 해당 저작 도구의 Graph와 WAIN의 런타임 Beat Graph는 동일한 구조가 아니다.
- [GENEVA: GENErating and Visualizing Branching Narratives Using LLMs](https://www.microsoft.com/en-us/research/publication/geneva-generating-and-visualizing-branching-narratives-using-llms/) (2024). Beat 기반 Graph로 분기 서사를 생성·시각화하는 연구다. WAIN 설계에서 Narrative Beat 표현의 참고 자료로 사용했으며, WAIN의 State 관리 구조 전체를 뒷받침하는 근거는 아니다.
- Mateas와 Stern (2005), [Structuring Content in the Façade Interactive Drama Architecture](https://doi.org/10.1609/aiide.v1i1.18722). Drama Manager, Beat 진행, Preconditions, 우선순위, 제작된 Interactive Drama 구성을 참고할 수 있다. Façade는 제작자가 작성한 대사와 행동을 선택·조합한다. WAIN 초기 설계는 LLM 대사를 생성하고 명시적 State 검증을 거친다. 동적으로 최적 Beat를 탐색하는 기능은 첫 프로토타입 범위에서 제외한다.
- Riedl과 Bulitko (2012), [Interactive Narrative: A Novel Application of Artificial Intelligence for Computer Games](https://doi.org/10.1609/AAAI.V26I1.8447). Interactive Narrative를 AI 연구 문제로 설명하는 배경 문헌이다.
- Han et al. (2024), [IBSEN: Director-Actor Agent Collaboration for Controllable and Interactive Drama Script Generation](https://aclanthology.org/2024.acl-long.88/). 감독 역할의 Plot 제어와 배우 역할의 캐릭터 연기를 나누는 연구다. IBSEN은 Multi-Agent 드라마 대본 생성 구조이며 WAIN 초기 런타임은 Single-Agent, Turn-Based다.

## Unreal 연동과 소프트웨어 경계

- Epic Games, [Unreal Engine Gameplay Framework 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-framework-in-unreal-engine). 호스트 게임의 수명주기와 연동 경계를 이해하기 위한 자료다. WAIN 플러그인의 구체 모듈과 UObject·Subsystem 수명주기는 미결정이다.
- Fowler (2021), Gateway Pattern. 설계 문서에서 WAIN과 호스트 게임 State 사이의 Adapter 경계를 설명하는 참고로 인용했다. 학위 논문에 사용하기 전에 정확히 어떤 Fowler 문헌을 가리키는지 확인해야 한다.

## 설계 문서에서 언급한 기타 사례

설계 문서는 Voyage, AI Dungeon, Inworld AI, Vaudeville, Uncover the Smoking Gun도 기술·제품 사례로 언급한다. 이 사례들은 맥락을 제공하기 위한 것이며 WAIN이 같은 기능을 구현했다는 증거는 아니다. 논문에서 비교에 사용할 경우 제품별 링크와 기준 날짜가 있는 설명을 추가한다.

## WAIN 설계의 자체 종합

Chapter → Sequence → Beat 계층과 영속 캐릭터 State, 필터링된 World Context, 검증된 State Commit, 명시적 서사 진행의 결합은 WAIN 요구에 맞춘 프로젝트 자체 종합이다. 이 전체 설계를 특정 논문 하나의 제안으로 돌리지 않는다.

## 참고 문헌 관리

학위 논문이나 공개 배포 전 저자 순서, 연도, 학회·저널, DOI 또는 대표 URL, 인용한 주장을 확인한다. 각 연구의 주장 범위를 넘어선 설명을 하지 않는다.

## 관련 문서

- [[02_Architecture/00_Overview|연구 개요]]
- [[01_Research-Problem|연구 문제]]
- [[02_Current-Choices|현재 결정]]
