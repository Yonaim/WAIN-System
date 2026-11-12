# 현재 결정

이 페이지는 설계 기준에서 결정된 내용과 미결정 질문을 구분한다. ‘결정’은 현재 설계의 기준이라는 뜻이며 코드에 구현됐다는 뜻은 아니다.

## 설계 기준으로 정한 내용

| 주제 | 결정 | 근거와 범위 |
|---|---|---|
| 프로토타입 상호작용 | Single-Agent, Turn-Based, 한 번에 하나씩 처리 | State 평가와 Commit을 순차화 |
| State 권위 | 영속 State를 LLM 외부에서 관리 | 생성 결과는 권위 있는 State가 아닌 제안 |
| 기본 Relationship 항목 | Affection(호감도), Trust(신뢰도), Attachment(애착도) | 세 항목을 독립적으로 관리. 기존 인계 문서의 Affinity/Trust는 초기 설계 기록 |
| 파이프라인 | Context 준비 → 생성 → 검증 → 서사 평가 → Commit → 응답 반환 | 필수 변경이 승인되기 전 성공 표시 금지 |
| 소유권 | Character Agent는 캐릭터 State, World State Manager는 World Fact, Narrative Manager는 진행 State 소유 | 중복 진실 공급원 방지 |
| 서사 계층 | Chapter → Sequence → Beat | WAIN에서 목표·제약·진행을 구성하는 방식 |
| Beat 전이 | 초기 프로토타입에서는 상호작용당 최대 한 Beat 전이 | 진행 동작 범위 제한 |
| 검증 | 결정론적 검사를 먼저 하고 필요할 때 의미 검사 | 일반 턴 비용을 억제하고 규칙을 검사 가능하게 유지 |
| World 연동 | 프로토타입은 내부 Store 사용, 호스트 게임 State용 Adapter 경계 마련 | 게임 State를 WAIN이 중복 소유하지 않고 초기 평가 가능 |
| Memory 검색 | Embedding 기반 의미 검색 후 관련성·최근성·중요도 점수화 | BM25 또는 Hybrid Retrieval은 초기 설계에 포함하지 않음 |
| Planning | 일정 기반이 아닌 Goal 기반 계획 | Goal과 State 변화에 따라 계획 진행 |
| 동시성 | 순차 처리, 초기에는 OCC 미적용 | 동시 Commit은 프로토타입 범위 밖 |
| 평가 | 같은 시나리오·입력을 A~D 단계에서 비교 | 누적 시스템 비교이며 개별 요소의 인과 효과를 분리하지 못함 |

## 미결정 사항

- Unreal 모듈 구조, UObject·Subsystem 수명주기, 구체 C++ 타입
- Provider, Model, 프롬프트 버전 관리, 재시도·시간 초과 동작
- Relationship과 Emotion의 숫자 범위와 변화 정책
- Embedding Model, 유사도 파라미터, 후보 수 N, 최종 수 K, 점수 가중치와 정규화
- Reflection 실행 임계값, 질문 생성, Memory 저장·Commit 시점
- World Fact Schema, Adapter API, Save·Load 형식, 여러 소유자에 걸친 실패 복구
- Narrative 조건 표현 언어, Transition 우선순위, 작성 도구
- 품질 평가 운영 방법, 평가자, 표본 수, 응답 시간 목표
- 프롬프트·Memory·로그의 보존 및 개인정보 처리

## WAIN 설정으로 채택하지 않은 문헌 예시 값

Generative Agents 논문의 예시(T=150, 최근 Memory 100개, 질문 Q=3개, 통찰 최대 5개)는 연구의 예시 값이지 WAIN의 설정이 아니다. 명시적 결정 전까지 프로젝트 파라미터처럼 기록하지 않는다.

## 관련 문서

- [[02_Architecture/00_Overview|개발 개요]]
- [[01_Current-Work|현재 작업]]
- [[02_Related-Work|선행 연구]]
- [[02_Contracts/00_Index|아키텍처 계약]]
