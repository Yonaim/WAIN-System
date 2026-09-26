# WAIN System 개발 로드맵

상태: **제안서의 개발 순서를 단계로 재구성** · 목표와 프로토타입 범위는 [[02_Planning/00_Project-Overview/0000_Project-Plan|Project Plan]]을 기준으로 한다.

| 단계 | 작업 | 주요 결과 |
|---:|---|---|
| 1. 기반 구성 | Unreal Engine C++ 개발 환경과 Plugin 구조를 마련하고, LLM API 연동 및 Character / World / Narrative State 데이터 구조를 설계한다. | 실행 가능한 플러그인 기반과 핵심 데이터 모델 |
| 2. Character Agent | Character State 관리, Memory 저장·검색, Reflection과 Planning 흐름을 구현한다. Relationship / Emotion State가 이후 반응에 반영되는지 확인한다. | 경험이 축적되고 캐릭터 반응에 이어지는 단일 Agent |
| 3. 세계·서사 관리 | World State Manager와 Narrative Manager를 구현하고, Goal / Constraint를 Character Agent에 전달한다. | 게임 세계 및 서사 진행을 반영하는 상호작용 흐름 |
| 4. Testbed 통합 | 플러그인을 별도 Visual Novel Testbed에 연결하고, UI·Scene/Event·Story Data가 있는 Single-Agent 시나리오에서 동작시킨다. | 재현 가능한 통합 데모 경로 |
| 5. 검증·평가·제출 | 같은 시나리오의 A–D 구성 비교, 자동 검증과 응답 품질 평가를 수행하고 문서·소스·시연 결과를 정리한다. | 테스트 및 평가 결과와 최종 프로토타입 산출물 |

## 단계 간 흐름

```text
Plugin / State 기반
  → Character Agent와 경험 처리
  → World / Narrative Manager
  → VN Testbed 통합
  → A–D 비교 평가와 결과 정리
```

## 일정과 진척

제안서에는 9–16주차 계획이 들어 있지만, 해당 주차는 제안 당시 일정이다. 이 로드맵은 구현 의존성을 보여주는 단계 기준이며 현재 달력 일정이나 완료 상태를 뜻하지 않는다. 실제 주차별 목표는 [[00_Start/Templates/0006_Weekly-Overview|Weekly Overview 템플릿]]으로, 상세 작업은 [[00_Start/Templates/0007_Weekly-Details|Weekly Details 템플릿]]으로 새로 작성한다.

각 단계의 완료 여부는 코드와 재현 가능한 검증으로 확인하고 [[TODO|TODO]]에 반영한다. 범위나 순서가 바뀌면 이 문서와 영향을 받는 주차 계획을 함께 갱신한다.
