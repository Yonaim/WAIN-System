# WAIN System 문서 볼트

이 볼트는 **비주얼 노벨을 위한 World-Agent Interactive Narrative System**의 정본 문서 입구다. WAIN은 자유형 자연어 상호작용을 지원하면서 캐릭터 경험, 게임 세계 상태와 제작자가 의도한 서사 진행을 LLM 바깥의 명시적 시스템에서 관리한다.

## 읽는 순서

1. [[02_Architecture/00_Overview|Architecture]]에서 모듈 책임과 시스템 전체 구조를 확인한다.
2. [[02_Architecture/01_Flows/00_Index|Flows]]에서 턴 처리 순서를 읽고, [[02_Architecture/02_Contracts/00_Index|Contracts]]에서 설계 계약을 확인한다.
3. [[03_Development/01_Current-Work|Current Work]]와 [[03_Development/02_Current-Choices|Current Choices]]에서 구현 현황, 결정과 미결 사항을 확인한다.
4. [[04_Experiments/01_Validation-and-Tests|Validation and Tests]]에서 계획과 실행 증거를 구분한다.
5. [[05_Research/00_Overview|Research]]에서 문제 정의, 선행 연구와 평가 계획을 확인한다.
6. 원본 자료는 [설계 인계 문서](../../../../설계.md) 및 [[06_Assets/README|06_Assets]]에서 확인한다.

## 문서 상태 기준

- **설계 기준:** 아키텍처 의도와 규칙이 합의됐지만 구현 여부는 별도 확인이 필요하다.
- **구현 확인:** 소스 코드에서 동작이 확인된 상태다.
- **검증됨:** 재현 가능한 테스트나 실험 결과가 연결된 상태다.
- **미확정:** 설계 변수가 열려 있다. 예시 값이나 원 연구의 값으로 확정 설정처럼 표시하지 않는다.

현재 플러그인 저장소에서는 README와 문서/자료만 확인되며 `.uplugin` 및 런타임 C++ 소스는 아직 발견되지 않았다. 이 볼트의 아키텍처는 설계 기준이며 구현 완료를 뜻하지 않는다.
