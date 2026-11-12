# ADR-009 — Transactional Reflection and Trigger-Based Planning (MVP)

- **상태:** Proposed (Phase 2 구현 선택; 임계값·트리거·장기 정책 최종 승인 전)
- **날짜:** 2026-11-11
- **연관:** [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-004](04_ADR-004-Snapshot-State.md) · [ADR-008](08_ADR-008-Transactional-Observations.md)

## 00. 배경

Observation Stream은 이미 성공 Transaction에만 포함된다. Reflection/Plan까지 도입할 때 생성 중 오류나 Narrative 실패 후 일부 Memory만 확정되면 일관성을 잃는다. 반면 완전한 LLM/Embedding Provider와 장기 Planning을 먼저 구현하면 MVP의 범위를 넘는다.

## 10. 검토한 대안

1. **Commit 후 Background Reflection:** 부하를 격리할 수 있지만 Memory와 Turn State가 다른 Revision에 확정될 수 있다. 추후 별도 작업 유형으로 검토한다.
2. **매 턴 Reflection과 Plan 생성:** 구현은 단순하지만 비용·중복·계획 불안정성이 커진다.
3. **Trigger 기반 Prepared Snapshot 내 생성 (MVP):** 기존 Transaction No Commit 원칙을 지키면서 작업 호출을 억제할 수 있다.

## 20. 구현 선택

- 생성기는 선택적 `IWAINMemoryReasoner` C++ 인터페이스로 주입한다. **실제 LLM 호출 구현이 아니며** 단일 Game Thread의 동기 Mock 경로에서만 호출한다. 미주입 시 기존 Observation-only 동작을 유지한다.
- `NarrativeEvaluator`가 Prepared Beat를 결정한 **이후**, `FinalValidation` **이전**에 `PrepareReflection → PreparePlan`을 수행한다. 생성 실패/무효 Content/State 위반 시 전체 Turn은 No Commit이다.
- Reflection은 최근 Reflection 이후의 현재 캐릭터 Observation 중 최신 12개를 수집한다. **개수 2개 이상, Importance 합계 2.0 이상**일 때 호출한다. 근거 ID는 WAIN이 실제 Source에서 직접 기록한다. 임계값과 개수는 **임시 실험값**이며 확정된 연구 수치가 아니다.
- Plan은 캐릭터별 `ActivePlanIds`로 활성 ID를 관리한다. 없거나 기존 Plan의 Scene 또는 현재 Prepared Beat가 달라지면 재생성하고, 같으면 재사용한다. 신규 Plan의 `GoalId`는 MVP에서 Active Beat ID를 사용한다. 관계 변화·계획 수행 실패·Step 완료 등의 갱신 규칙은 **아직 없다**.
- 계획은 `Summary`, 최대 8개 `Steps`, `CurrentStepIndex`(초기 0), `SceneId/BeatId/GoalId`, 실제 근거 Memory ID를 가진다. 이전 Plan은 History로 Stream에 남기고 활성 Plan ID만 새 것으로 교체한다.
- Reflection은 Observation/Reflection에서, 활성 Plan은 ID로 별도로 읽어 Character Context에 포함한다. 비활성 Plan은 기본 검색 대상에서 제외한다.
- Memory ID는 동일 Character의 **앞에 저장된 레코드**만 참조할 수 있다. 그래프 순환·교차 Character 접근·중복 근거는 거절한다.

## 30. 제약과 영향

- **생성된 Insight가 사실임을 검증하지 않는다.** 출처 연결은 Provenance만 보장하며 의미적 타당성은 추후 LLM 품질 및 인용 평가가 필요하다.
- Replanning은 Scene/Beat 변경만 지원하며 Goal/World/Trust 변화 감지는 추후 Narrative/World 통합 후 확장한다.
- 메모리 2,000개 제한과 세션 메모리 전용 정책 유지. 저장·불러오기, GC, Embedding, 토큰 예산, 비동기 LLM, 타임아웃은 범위 밖이다.
- 생성기 실패는 이번 MVP에서 **전체 Turn 실패** 정책이다. 선택적 후처리나 Deferred 작업 방식은 후속 ADR에서 검토한다.
- 이 ADR의 상태 `Proposed`는 코드가 없다는 뜻이 아니라 **MVP 구현 정책이 아직 최종 확정되지 않았다는 의미**다.

## 40. 검증

[구현 문서](../04_Development/09_Memory-Reasoning-Implementation.md)에 자동화 테스트 작성 내용과 미검증 범위를 기록한다. UE 5.8 컴파일 및 Automation Test 실행은 이 환경에서 수행하지 않았다.
