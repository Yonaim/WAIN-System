# World State Manager

> 상태: 프로토타입 설계 계약. 책임과 경계는 설계 문서에 정의되어 있으며, 구체적인 Unreal 타입과 구현은 미정이다.

## 목적

World State Manager는 WAIN이 사용하는 World Facts의 권위 있는 접근 경계다. WAIN 소유 내부 State와 Adapter를 통해 접근하는 게임 소유 State를 다룬다. Character State나 Narrative Progress는 소유하지 않는다.

## 책임

- 현재 상호작용에 필요한 World Context 조회
- 타입이 지정된 World Fact의 조회와 검증
- 제안된 World 변경의 준비와 승인된 변경의 Commit
- Schema 검증과 게임 규칙 검증의 분리
- LLM에 전달하기 전 관련 사실과 공개 가능 정보 필터링

## State 소유권

| State | 소유자 | 접근 경로 |
|---|---|---|
| WAIN 프로토타입용 사실 | World State Manager | Internal Store |
| 게임 고유 사실 | 호스트 게임 | External State Adapter |
| Persona, Relationship, Emotion, Memory | Character Agent | Character Agent 계약 |
| Active Beat, Beat History | Narrative Manager | Narrative Manager 계약 |

관리자는 자신이 소유한 사실의 단일 진실 공급원이어야 한다. Character State나 Narrative State의 복사본을 만들지 않는다.

## Fact Schema

등록할 Fact에는 다음 정보가 필요하다.

- Key: 안정적인 식별자
- ValueType: 기대하는 타입
- Default: 해당하는 경우 초기값
- Constraints: 구조와 값의 제한
- Mutability: 변경 가능 여부와 변경 권한
- Owner: 내부 관리자 또는 외부 게임 시스템

Unreal Struct, Gameplay Tag, Variant Type 등 구체 표현은 아직 결정되지 않았다.

## External State Adapter

프로토타입부터 호스트 게임의 읽기·쓰기 경계를 Adapter로 둔다. 첫 구현은 내부 Store를 사용할 수 있지만, 이후 게임 State와 연결할 수 있도록 Adapter 계약을 유지한다. Adapter는 소유권과 실패를 명시적으로 반환해야 하며, 게임 State 조회 실패를 권위 있는 기본값으로 조용히 대체해서는 안 된다.

## Context와 정보 공개

Orchestrator는 현재 상호작용에 필요한 사실만 요청한다. 프롬프트를 구성하기 전에 관련성과 공개 정책을 적용한다. 비공개 사실은 LLM 입력에서 먼저 제외해야 한다. 응답 이후 검사는 보조 방어일 뿐, 비밀을 지키는 주된 경계가 아니다.

## 변경 처리

1. 소유 모듈에서 타입이 지정된 변경 제안을 받는다.
2. Schema, 변경 권한, 소유자, 결정론적 제약을 확인한다.
3. 필요하면 Adapter를 통해 게임 규칙 검증을 수행한다.
4. 정해진 상호작용 순서에 따라 승인된 변경을 Commit한다.
5. Orchestrator에 승인·거부·오류 결과를 명확히 반환한다.

이 검증은 여러 소유자에 걸친 원자적 트랜잭션을 보장하지 않는다. 초기 프로토타입은 Commit을 순차 처리하고, 앞선 State가 바뀐 뒤 실패할 가능성이 높은 외부 작업을 중간에 배치하지 않는 방향으로 설계한다.

## 미결정 사항

- 위치, 월드 시간 등 핵심 World Fact의 범위
- Fact 등록·직렬화 형식
- Adapter API와 호스트 State를 읽을 수 없을 때의 동작
- 여러 State 소유자에 걸친 실패 복구와 트랜잭션 처리
- 영속 기간과 Save Game 연동

## 관련 문서

- [[01_Flows/00_Index|계약 색인]]
- [[01_Interaction-Pipeline|상호작용 파이프라인]]
- [[01_Contracts/04_Narrative-Manager|Narrative Manager]]
- [[05_Response-Validation-and-Commit|응답 검증과 Commit]]
