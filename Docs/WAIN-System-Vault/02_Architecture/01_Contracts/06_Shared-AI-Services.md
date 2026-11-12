# Shared AI Services

> 상태: LLM·Embedding 인프라를 추상화하는 설계 계약. Provider, 모델, 실제 API는 미선정.

## 서비스 경계

- **LLM Generation:** Character Agent의 Context/Goal/Constraint를 입력받아 구조화 Response Proposal 생성.
- **Embedding:** Query/Memory 텍스트를 동일 Embedding 공간의 Vector로 변환해 검색에 사용.
- 공통 Timeout, Retry, Rate Limit, Provider 오류 분류, 사용량/지연 기록을 제공하되 **도메인 State의 소유권은 갖지 않는다**.

## 제약

Provider의 텍스트를 신뢰할 수 없는 입력으로 취급하고 Schema Validator에 통과시켜야 한다. API Key를 Prompt/로그/Repository에 넣지 않는다. 실험 A~D의 모델·Seed·Version을 고정하거나 결과에 함께 기록해야 비교할 수 있다.

## 미결정

Provider, Serializer/JSON Schema 검증 방식, 재시도 횟수·비용·Timeout, 서비스 소유 객체/Unreal 수명주기, 프로파일과 응답 지연 예산.
