# Memory, Reflection, Planning

> 하나의 Memory Stream에 관찰·해석·계획을 저장하고 필요한 기억만 검색해 응답에 사용한다.

상태: **기본 설계 기준** · 파라미터: 미확정

## Memory Stream

초기 시스템은 `Observation`, `Reflection`, `Plan`의 세 Type을 공통 Schema로 하나의 Stream에 저장한다. Memory Consolidation과 Forgetting은 초기 구현 범위에서 제외한다.

| 필드 | 의미 |
|---|---|
| `MemoryId` | 고유 식별자 |
| `Type` | Observation / Reflection / Plan |
| `Content` | 기억의 자연어 내용 |
| `CreatedAt`, `LastAccessedAt` | 생성·최근 참조 시점 |
| `Importance` | 중요도 값 |
| `Embedding` | 의미 검색용 표현 |
| `SourceMemoryIds` | Reflection 등이 근거로 쓴 기억 ID |
| `Metadata` | 추가 필터 정보 |

Content는 기억의 의미를, `SourceMemoryIds`는 근거 계보를 나타낸다.

## Retrieval

1. 현재 Query를 Embedding으로 변환한다.
2. Cosine 유사도로 의미 관련 후보 Top-N을 가져온다.
3. Relevance, Recency, Importance를 결합해 다시 순위화한다.
4. 최종 Top-K를 Context에 포함한다.

```text
Score(m) = w_r · Relevance(m, query)
         + w_c · Recency(m, current_turn)
         + w_i · Importance(m)
```

정규화 방식, 가중치, N·K와 Embedding 모델은 미확정이다. 초기 설계는 BM25/Hybrid Search 대신 의미 검색과 필요한 ID·Metadata 정확 필터링을 우선한다.

## Reflection

Reflection은 여러 Observation에서 상위 수준 해석을 만들고 근거 ID와 함께 저장한다. 매 턴 실행하지 않는다.

1. 최근 Memory 중요도 누적값이 임계값 `T`에 도달하는지 확인
2. 최근 `N`개 기억에서 질문 `M`개 생성
3. 질문별 관련 Memory 재검색
4. 질문마다 최대 `K`개 Insight를 Reflection Memory로 저장

원 연구의 예시 설정(임계값 150, 최근 기억 100개, 질문 3개, 질문당 Insight 5개)은 WAIN 값이 아니다. 실제 `T/N/M/K`는 미확정이다.

## Planning

시간표 기반 계획을 VN의 Goal·Scene 중심으로 바꾼다. Plan은 Goal, Steps, Conditions, Current Step, Status, Active Plan ID를 개념적으로 가진다. 자연어 Plan은 Stream에 저장할 수 있고 실행 상태는 별도로 유지할 수 있다.

검토 Trigger는 Active Plan 부재, Scene/Goal 변화, Plan 관련 State 변경, 실행 실패다. 기존 Plan이 유효하면 유지하고, 아니면 Goal과 관련 Memory를 써서 생성/갱신한다. Plan은 Narrative Goal·Constraint를 넘을 수 없고 행동 가능성은 World State에 근거한다.

## 미결 사항

검색 수식·값, Embedding 저장/index, 중요도 산정, Reflection/Planning Memory의 저장·Commit 시점, Plan 상태 영속화는 [[01_Current-Work|Current Work]]에서 관리한다.

