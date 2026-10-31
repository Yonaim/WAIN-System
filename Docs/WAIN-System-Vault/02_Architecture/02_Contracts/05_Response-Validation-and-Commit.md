# 응답 검증과 State Commit

> 상태: 상호작용 처리 설계 계약. 구체적인 오류 타입과 롤백 보장은 아직 정해지지 않았다.

## 기본 원칙

LLM 응답은 **State 변경이 아니라 제안(Proposal)**이다. WAIN은 제안을 검증하고 서사 효과를 평가한 뒤 승인된 State를 Commit한다. Commit이 완료된 후에만 비주얼 노벨에 성공한 상호작용 결과를 반환한다.

## 검증 단계

1. **구조화 출력 검사**: 예상 응답 Schema로 파싱한다.
2. **결정론적 검사**: 필드 타입, 허용 Action, State 소유권, 값 범위, 변경 권한, Hard Narrative Constraint를 확인한다.
3. **선택적 의미 검사**: 결정론적으로 판단할 수 없는 위험한 모호성이나 Soft Constraint에 적용한다. 일반적인 모든 턴마다 기본 실행하지 않는다.

의미 검사는 위험을 줄일 뿐 정확성이나 비밀 유출 방지를 보장하지 않는다. 비공개 정보는 Context 구성 단계에서 제외해야 한다.

## Commit 순서

초기 Single-Agent 프로토타입은 한 번에 하나의 플레이어 상호작용을 처리한다.

1. Context를 준비하고 평가에 필요한 현재 State를 조회한다.
2. Character Agent에 응답과 변경 제안을 요청한다.
3. 응답과 제안된 변경을 검증한다.
4. 검증된 임시 변경을 기준으로 Narrative 완료와 전이를 평가한다.
5. 각 State 소유 모듈을 통해 승인된 변경을 순차 적용한다.
6. 응답과 Commit 결과를 비주얼 노벨에 반환한다.

각 소유자에 대한 쓰기 순서는 명시적으로 정하고 문서화해야 한다. 필수 Commit 작업이 실패하면 플레이어에게 성공을 표시하거나 서사를 진행해서는 안 된다.

## 실패 동작

- 파싱·검증 실패: 제안을 거부하고 유형이 구분되는 실패 또는 통제된 재시도 결과를 반환한다.
- Commit 실패: 성공으로 보고하거나 서사를 진행하지 않는다. 초기 설계에서는 중단 또는 정의된 Fallback을 우선 검토한다.
- 여러 State 소유자에 걸친 원자적 롤백을 보장하는 것처럼 표현하지 않는다. 초기 설계에는 교차 소유자 트랜잭션 보장이 없다.
- 여러 소유자의 Commit 도중 실패할 가능성이 높은 외부 작업을 가능한 한 배치하지 않는다.

## 동시성

초기 프로토타입은 직렬 처리하며 Revision 기반 Optimistic Concurrency Control(OCC)은 요구하지 않는다. 동시 상호작용을 도입할 때는 활성화 전에 오래된 조회 결과와 충돌 처리 방식을 설계해야 한다.

## 미결정 사항

- 잘못된 LLM 응답의 재시도 여부와 횟수
- 소유자별 Commit 순서와 보상 처리
- Idempotency Key와 중복 요청 처리
- Provider 또는 호스트 게임 Adapter 장애 시 사용자에게 보여줄 Fallback
- Proposal 종류별 필수 검증 항목

## 관련 문서

- [[00_Index|계약 색인]]
- [[01_Character-Agent|Character Agent]]
- [[03_World-State-Manager|World State Manager]]
- [[04_Narrative-Manager|Narrative Manager]]
- [[../01_Flows/01_Interaction-Pipeline|상호작용 파이프라인]]
