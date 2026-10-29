# WAIN System 문서 볼트

이 볼트는 **WAIN System을 비주얼 노벨 Testbed에 통합하고 검증하는 작업**의 문서 입구다. 프로젝트 README에 따르면 WAIN은 LLM 기반 비주얼 노벨을 위한 World-Agent Interactive Narrative System이며, 장기 상호작용을 위해 캐릭터·관계·감정·세계·서사 상태를 LLM 외부에서 유지하는 것을 목표로 한다.

## 읽는 순서

1. [[02_Architecture/00_Overview|Architecture]]에서 WAIN 플러그인과 Testbed의 책임 경계, 현재 소스 상태를 확인한다.
2. [[03_Development/00_Overview|Development]]에서 통합 진행 상태와 미완료 작업을 확인한다.
3. [[04_Experiments/00_Overview|Experiments]]에서 검증 계획과 실제 실행 결과를 확인한다.
4. [[05_Research/00_Overview|Research]]에서 연구 질문과 평가 근거를 확인한다.
5. WAIN System의 상세 원문 문서는 [플러그인 저장소의 WAIN-Vault](../../Plugins/WAINSystem/Docs/WAIN-Vault/00_Start/0000_Vault-Guide.md)에서 확인한다.

## 문서 기준

이 저장소 README, 실제 코드·설정, 플러그인 서브모듈의 문서와 실행 증거를 구분해 기록한다. 계획을 구현된 기능으로 표현하지 않는다. 현재 Testbed C++ 모듈은 Unreal 기본 모듈 뼈대이며, WAIN 플러그인 README는 플러그인 코드와 `.uplugin` 추가 전 상태라고 설명한다. 테스트와 성능 측정은 실제 실행 결과만 기록한다.

