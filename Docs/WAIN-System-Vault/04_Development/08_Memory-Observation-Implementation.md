# Phase 2 — Memory Stream: Observation / Retrieval / Context

> **상태:** C++ 최소 구현 및 Unreal Automation 테스트 8개 **작성** (기존 30개 + 신규 8개 = 총 38개). 이 환경에서는 UE 5.8 컴파일·테스트를 실행하지 않았다. Phase 1만 사용자가 검증한 기준본이다.

## 00. 범위와 정본

- WAIN이 `FWAINStateSnapshot::MemoryStream`을 소유한다. 게임은 별도 Memory 정본을 만들지 않는다.
- 통합 `FWAINMemoryRecord`의 `Type`은 Observation/Reflection/Plan을 표현할 수 있지만, **이번 단계에서 실제 생성·검색은 Observation만** 지원한다.
- CharacterId로 검색 범위를 분리하고, 다른 Character의 Observation을 Context에 포함하지 않는다.
- 데이터는 **메모리 Session Snapshot 한정**이다. 저장·불러오기, 임베딩 캐시, 토큰 예산, 프롬프트 노출 필터는 아직 없다.

## 10. Top-Down 처리

```text
ProcessTurn
  ReadCommittedSnapshot
  BuildCharacterContext
    CopyCharacterState
    RetrieveRelevantObservations   ← Current MemoryStream만 조회
    LoadPersona
    QueryAvailablePresentations
  GenerateProposal
  Transaction.Execute
    ValidateProposal
    PrepareWAINState
      ApplyCharacterMutations
      IncrementTurnCount
      PrepareObservations          ← Prepared Snapshot에만 기록
    PrepareGameState
    EvaluateNarrative
    FinalValidate
    PublishRoot                   ← Memory와 WAIN/Game 변경을 함께 확정
  ReturnVNResponse
```

- 정상 Text 턴: Player 입력 1개 + Character 대사 1개를 Observation으로 기록한다.
- Silence 턴: 입력 문자열 Observation을 만들지 않고 Character 대사 1개만 기록한다.
- Proposal/Prepare/Final Validation/Publish 실패 시 **Current Snapshot에 새 Memory가 존재하지 않는다**.
- TurnId 재생은 기존 결과만 반환하며 관찰을 중복 저장하거나 Context를 다시 검색하지 않는다.

## 20. DTO 및 주요 코드

| 파일 | 책임 |
|---|---|
| `Public/WAINMemoryTypes.h` | 공통 Memory 타입, Record/Owner/Speaker, 임의 타입 추가의 확장 지점 |
| `Public/WAINRuntimeTypes.h` | WAIN Snapshot의 단일 `MemoryStream` 포함 |
| `Private/Memory/WAINMemoryWriter.{h,cpp}` | 이번 턴을 Prepared Observation으로 변환, 한도 검증 |
| `Private/Memory/WAINMemoryRetriever.{h,cpp}` | Current Snapshot을 대상으로 읽기 전용 검색 |
| `Private/Character/WAINCharacterContextBuilder.cpp` | `RelevantMemories` 구성 |
| `Private/Core/WAINTransactionCoordinator.cpp` | Mutation·TurnCount와 함께 Observation 준비, 최종 Commit |
| `Private/Core/WAINStateValidation.cpp` | 기록 ID 중복·소유자·내용·범위 검증 |
| `Private/Tests/WAINMemoryTests.cpp` | Observation 저장, 다음 턴 조회, 격리, 검색 순서, No Commit, Replay, Silence |

`FWAINMemoryRecord`는 `Id`, `CharacterId`, `Type`, `Speaker`, `Content`, `CreatedTurn`, `Importance`, `SourceMemoryIds`를 포함한다. `Embedding`, `LastAccessedTurn`은 확장용 선택 필드로 존재하지만 현재 쓰기/검색 알고리즘은 갱신하지 않는다. 시각(UTC), Metadata/Save serializer는 아직 계약화하지 않았다.

## 30. 임시 검색 알고리즘 — 확정된 Embedding Retrieval이 아님

현재 Request.Text를 공백 기반 토큰으로 분할하고, 포함 문자열 비율을 Relevance로 사용한다. 임시 점수는 다음과 같다.

`Score = 0.60 * LexicalRelevance + 0.25 * TurnRecency + 0.15 * Importance`

- `TurnRecency = 1 / (1 + AgeTurns / 8)`, Importance 기본값 `0.5`, 상위 6개 사용.
- 점수 동률이면 Stream에서 더 나중에 저장된 Observation을 먼저 선택한다.
- Silence 입력은 관련성 점수 0으로 최근성/중요도를 활용한다.
- **한국어 형태소·의미 유사도 검색을 제공하지 않는다.** 이 가중치/상위 K/반감 기준은 MVP 실험 변수이며 선행 연구 또는 사용자 승인 하이퍼파라미터가 아니다.
- 향후 `Embedding → Cosine Similarity` 구현 시 `FWAINMemoryRetriever` 내부를 교체하되, `FWAINCharacterContext::RelevantMemories`의 외부 계약은 유지하는 것을 목표로 한다.

## 40. 제한과 실패 정책

- Observation Content는 비어 있으면 안 되고 길이 4,096자 이하여야 한다. 메모리 레코드 수 2,000개 한도이며 초과 시 **MemoryPreparationFailed, No Commit**. 자동 삭제·압축은 하지 않는다.
- 메모리 검색은 Current의 사본만 반환하고 `LastAccessedTurn`을 변경하지 않는다.
- Legacy Mock Provider는 Persona·Catalog 미주입 시 기존 `Generate()` 경로로 계속 진행한다. 이 경로에서는 Retrieval 결과가 응답 생성에 사용되지 않는다. **실제 LLM Provider는 `GenerateWithContext()`에서 Memory 내용을 반영하도록 구현해야 한다.**
- 현재 키워드 검색은 프롬프트 인젝션/비공개 정보 구분을 수행하지 않는다. Host가 승인한 대화만 기록하고, LLM Context 직렬화 단계에서 접근 정책과 비신뢰 데이터 경계를 구현해야 한다.

## 50. 다음 확장

1. UE 5.8 컴파일 및 `WAIN.Memory.*` 자동화 테스트 실행, 기존 30개 회귀 테스트.
2. 메모리 검색 서비스/Embedding 연동 및 중요도 추정 규칙 실험.
3. Reflection(근거 Memory ID 참조) / Planning(Trigger 기반 재사용) 및 검증.
4. 메모리 GC/압축과 Save/Load·Migration·보안/접근 정책은 별도 결정과 ADR 필요.

[Memory 설계](../02_Architecture/01_Contracts/02_Memory-Reflection-Planning.md) · [ADR-008](../03_Decisions/08_ADR-008-Transactional-Observations.md)
