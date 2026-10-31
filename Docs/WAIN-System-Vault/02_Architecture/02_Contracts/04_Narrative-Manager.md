# Narrative Manager

> 상태: 프로젝트 설계와 중간발표 자료를 바탕으로 한 프로토타입 설계 계약.

## 목적

Narrative Manager는 구조화된 서사 진행 상태를 관리하고, 상호작용으로 현재 Beat를 완료하거나 다음 Beat로 전이할 수 있는지 판단한다. 서사 계층은 **Chapter → Sequence → Beat**다. Beat는 런타임 진행의 노드이며, Scene과 같은 개념이 아니다.

## 서사 단위

- **Chapter**: 목표와 제약을 가진 상위 서사 단위
- **Sequence**: 관련 Beat를 묶으며 자체 목표와 제약을 가질 수 있는 단위
- **Beat**: 가장 작은 진행 단위. Narrative Graph는 Beat와 조건부 전이로 구성한다.

Chapter와 Sequence의 식별자를 하위 단위가 참조한다. Goal은 의도한 서사 결과를 자연어로 설명한다. Completion Condition은 명시적인 구조 규칙이며 Goal과 구분한다.

## 제약

제약은 다음과 같이 나눈다.

- **Hard Constraint**: 구조화되어 결정론적으로 검사할 수 있는 규칙
- **Soft Constraint**: 자연어 지침으로, 의미 검사가 필요할 수 있는 규칙

Beat는 소속 Chapter와 Sequence의 제약을 상속한다. 상속 제약을 해제하려면 그 사실을 명시해야 한다. 공개할 수 없는 정보는 LLM 입력 전에 필터링한다. 응답 의미 검사는 보조 수단이며 비밀 유출을 완전히 보장하지 않는다.

## 진행 계약

각 상호작용에서 Narrative Manager는 현재 Narrative State와 검증된 임시 변경을 받아 다음을 수행한다.

1. 현재 Beat의 완료 조건을 평가한다.
2. 가능한 다음 전이를 찾는다.
3. 여러 전이가 가능하면 명시된 우선순위로 하나를 선택한다.
4. 초기 프로토타입에서는 상호작용당 최대 한 Beat만 전이한다.
5. 가능한 전이가 없으면 현재 Beat를 유지한다.

여러 전이가 가능하지만 우선순위로 해결할 수 없다면 임의로 선택하지 않는다. 콘텐츠 작성 또는 런타임 처리를 위해 미해결 전이 결과를 반환한다.

완료 여부는 검증된 임시 변경을 기준으로 평가하며, 검증 전 LLM 출력만으로 판단하지 않는다.

## State

최소 런타임 필드 제안:

- ActiveBeatId
- CompletedBeatIds
- BeatHistory

직렬화 구조와 영속화 방식은 미정이다.

## 초기 범위의 경계

프로토타입은 제작자가 작성한 조건과 전이를 평가한다. 대규모 Narrative Graph에서 다음 Beat를 최적화해 찾는 동적 Façade는 초기 범위에서 제외한다. 독립적인 자율 World Event와 동시 Multi-Agent 진행도 범위 밖이다.

## 미결정 사항

- Condition 표현 형식과 지원 연산자
- 전이 우선순위와 동률 처리 방식
- 전이에 추가 State 변경을 연결할 수 있는지 여부
- Narrative Graph 작성·검증 절차
- Save·Load와 데이터 마이그레이션

## 관련 문서

- [[00_Index|계약 색인]]
- [[03_World-State-Manager|World State Manager]]
- [[05_Response-Validation-and-Commit|응답 검증과 Commit]]
- [[../01_Flows/01_Interaction-Pipeline|상호작용 파이프라인]]
