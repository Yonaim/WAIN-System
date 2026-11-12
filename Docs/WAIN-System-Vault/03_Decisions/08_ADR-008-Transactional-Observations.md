# ADR-008 — Transactional Observation Memory in WAIN Snapshot

- **상태:** Proposed (Phase 2 구현 선택; 아키텍처 최종 승인은 별도)
- **날짜:** 2026-11-11
- **관련 ADR:** [ADR-001](01_ADR-001-State-Ownership.md) · [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-004](04_ADR-004-Snapshot-State.md)

## 배경

성공한 대화 턴의 Memory와 관계·Emotion·Game 상태가 서로 다른 시점에 확정되면, 실패한 턴의 발화가 장기 기억으로 남거나 응답 Context와 실제 상태가 어긋날 수 있다.

## 검토 대안

- **Commit 성공 후 별도 Memory.Append:** 구현이 간단하지만 Memory 저장 실패 시 대화 상태와 기억의 원자적 일관성이 깨진다.
- **Host가 Memory를 보유:** WAIN의 장기 Character State 정본이 Host로 이동하여 모듈 책임을 위반한다.
- **준비된 WAIN Snapshot에 Observation을 포함하여 함께 Publish (MVP 선택):** 기존 Transaction 경계를 재사용하고 실패한 턴의 기록을 막는다.

## 제안 결정 / 구현 선택

1. Observation은 WAIN-owned `FWAINStateSnapshot::MemoryStream`에 포함한다. Player 입력과 Character 대사는 검증된 Proposal을 토대로 Prepared Snapshot에서 생성하고, Revision Publish 성공 시 함께 확정한다.
2. 검색은 현재 **Committed Snapshot의 CharacterId 범위**로 제한하며 Context Preparation 중 메모리를 변경하지 않는다. Replay는 검색·추가 수행 없이 이전 결과를 반환한다.
3. `Type`은 Observation/Reflection/Plan 통합 레코드이며 현재 실제 작성은 Observation만 한다. Reflection/Plan 생성 및 Embedding 검색은 후속 단계다.
4. 현재 키워드·최근성·중요도 임시 점수와 K=6, 기록 상한은 구현 파라미터일 뿐, 범용적 권장/확정 하이퍼파라미터로 채택하지 않는다.
5. Session 메모리만 제공하며 Persistence·GC·LLM Prompt 안전 정책은 범위 밖이다.

## 영향

Memory가 Snapshot의 일부가 되므로 해당 턴의 Mutation/Game Prepare/Narrative 실패 후 권위 Stream은 변하지 않는다. 반대로 기록 용량 초과는 턴 Commit 전체를 실패시킨다. 저장 용량/성능/Save Schema 확장 정책이 필요해지는 시점에 재평가한다.

## 검증 상태

C++ 및 Automation Test 코드는 작성했지만 UE 5.8 빌드·테스트 실행 결과는 없다. [구현 상세](../04_Development/08_Memory-Observation-Implementation.md).
