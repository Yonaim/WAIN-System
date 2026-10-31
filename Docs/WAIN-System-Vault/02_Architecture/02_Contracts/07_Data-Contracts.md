# 모듈 간 데이터 계약

> 상태: 개념 수준의 계약 목록. 구체적인 C++ 타입과 직렬화 형식은 아직 결정되지 않았다.

## 상호작용 요청

Orchestrator는 한 번에 하나의 플레이어 상호작용을 조율한다. 요청에는 플레이어 입력, 활성 캐릭터, 관련 서사 Context, 요청 추적용 식별자가 필요하다. 정확한 필드와 소유권은 미정이다.

## Character State

Character Agent State는 다음 영역으로 구성한다.

- Persona: 캐릭터의 정체성과 안정적인 정의
- Relationship: 기본 항목은 Affection(호감도), Trust(신뢰도), Attachment(애착도). 각 항목은 독립적인 장기 관계 상태
- Emotion: 감정 Type과 Intensity
- Memory: Character Agent의 영속 Memory Stream

LLM은 의미상 변화 방향과 강도를 제안할 수 있다. 결정론적 프로젝트 정책이 이를 실제 숫자 변경으로 변환한다. 척도, 범위, 변환 규칙은 미정이다.

## Memory Record

제안된 공통 Memory Record 필드는 다음과 같다.

| 필드 | 의미 |
|---|---|
| MemoryId | 안정적인 식별자 |
| Type | Observation, Reflection, Plan 중 하나 |
| Content | 저장할 의미 내용 |
| CreatedAt | 생성 시각 |
| LastAccessedAt | 마지막 조회 시각 |
| Importance | 중요도 값 |
| Embedding | 의미 검색에 사용하는 벡터 |
| SourceMemoryIds | 근거가 된 원본 Memory 식별자 |
| Metadata | 선택적 타입 지정·색인 정보 |

각 필드의 자료형, 버전 관리, 보존 정책, 직렬화 형식은 아직 정하지 않았다.

## World Fact

World Fact는 Key, ValueType, 필요하면 Default, Constraints, Mutability, Owner를 설명한다. WAIN 내부 프로토타입 Fact는 World State Manager가 소유한다. 호스트 게임 Fact의 소유권은 게임에 두고 Adapter로 접근한다.

## Narrative State

최소 진행 State 제안은 ActiveBeatId, CompletedBeatIds, BeatHistory다. Narrative Content에는 Chapter, Sequence, Beat 식별자와 Goal, Constraint, Completion Condition, 조건부 Transition이 포함된다.

## LLM Proposal 봉투

Model이 생성한 모든 State 변경은 검증 전까지 신뢰할 수 없는 제안으로 취급한다. 제안은 소유 모듈과 상호작용을 식별하는 데 필요한 정보를 포함해야 한다. 구체 Schema, 신뢰도 표현, 버전 전략은 미결정이다.

## 공통 규칙

- 각 State 영역에 단일 소유자를 둔다.
- 도메인에서 사용하기 전에 구조를 검증한다.
- Model 출력은 신뢰할 수 없는 입력으로 취급한다.
- Schema 검증과 게임 규칙 검증을 구분한다.
- 장기간 유지되는 Save에 의존하기 전에 데이터 버전을 정한다.
- 설계·구현 전에는 여러 소유자에 걸친 원자적 트랜잭션을 보장한다고 표현하지 않는다.

## 관련 문서

- [[00_Index|계약 색인]]
- [[01_Character-Agent|Character Agent]]
- [[02_Memory-Reflection-Planning|Memory, Reflection, Planning]]
- [[03_World-State-Manager|World State Manager]]
- [[04_Narrative-Manager|Narrative Manager]]
- [[05_Response-Validation-and-Commit|응답 검증과 Commit]]
- [[06_Shared-AI-Services|Shared AI Services]]
