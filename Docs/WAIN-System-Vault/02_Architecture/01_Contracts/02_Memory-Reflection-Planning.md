# Memory / Reflection / Planning

> 상태: 연구 근거에 기반한 설계. 구체 값은 미확정; 구현 없음.

## Unified Memory Stream

Observation(사건/대화 관찰), Reflection(여러 Memory 기반 해석), Plan(Goal/Steps)을 **하나의 공통 레코드 Schema**로 저장한다. 권장 필드: `Id`, `Type`, `Content`, `CreatedAt`, `LastAccessedAt`, `Importance`, `Embedding`, `SourceMemoryIds`, 선택적 Metadata. 세부 타입·보존 규칙은 향후 확정한다.

Reflection은 근거 Observation/Reflection의 ID를 참조하여 **기억 간 출처 관계를 추적**한다. 서로 다른 캐릭터의 접근 권한을 혼동하지 않는다.

## Retrieval

현재 입력을 Embedding으로 변환 → Cosine 유사도로 후보 N개 검색 → `Relevance`, 경과 턴 기반 `Recency`, 저장 당시 `Importance`를 종합한 Score로 상위 K개를 Context에 포함한다. 모델·N/K·정규화·가중치는 미정. BM25/Hybrid Search는 기본 설계에 포함하지 않는다.

## Reflection

최근 Memory의 중요도 누적값이 임계 T에 도달할 때, 최근 N개에서 핵심 질문 M개를 만들고 관련 기억을 다시 검색해 Insight를 생성한다. 생성된 해석을 Reflection Memory로 저장한다. T/N/M/K는 **설계 변수**이고 선행 연구의 값이 WAIN에 이미 채택된 것이 아니다.

## Planning

매 턴 새 계획을 만들지 않는다. Active Plan 부재, Scene/Goal 변경, 관련 State 변경, 수행 실패 등을 Trigger로 판단한다. 유효한 계획은 Current Step부터 재사용하고, 불일치하면 Goal·Memory를 바탕으로 갱신한다. 게임 VN의 **턴 기반 Goal**에 맞춘 방식이다.

## 시험 항목

기억 출처 참조 유지, 잘못된 ID 거부, 관련성/최근성/중요도 변화 시 랭킹 비교, Trigger 충족 전 Reflection 생략, State 변화 후 Plan 재평가.
