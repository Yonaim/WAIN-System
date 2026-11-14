# Architecture Decision Records

> **2026-11-10 설계 결정 기록.** Accepted는 **설계 채택**을 뜻하며 실제 Plugin 구현·원자성·테스트 성공을 뜻하지 않는다. 결정과 구현 현황은 별도로 기록한다.

| ADR | 결정 | 상태 |
|---|---|---|
| [ADR-001](01_ADR-001-State-Ownership.md) | WAIN / Host State의 권위 소유권과 Game Adapter | Accepted (설계) |
| [ADR-002](02_ADR-002-Emotion-Expression.md) | 독립 6차원 Emotion 및 Host Presentation Catalog | Accepted (설계) |
| [ADR-003](03_ADR-003-Commit-Before-Response.md) | Turn Transaction의 검증·준비·서사 평가·확정·출력 순서 | Accepted (설계) |
| [ADR-004](04_ADR-004-Snapshot-State.md) | WAIN 내부 State의 Prepared Snapshot 방식 | Accepted (설계) |
| [ADR-005](05_ADR-005-Minimal-Game-Adapter.md) | 초기 Testbed의 최소 Transaction Adapter, 범용 확장 경계 | Accepted (설계) |
| [ADR-006](06_ADR-006-Turn-Based-Single-Agent.md) | 단일 Agent / Turn 기반 MVP 범위 | Accepted (설계) |
| [ADR-007](07_ADR-007-Read-Only-Character-Context.md) | Host Persona Source 및 읽기 전용 Character Context | Accepted (Phase 2 구현 계약) |
| [ADR-008](08_ADR-008-Transactional-Observations.md) | Observation을 WAIN Prepared Snapshot과 함께 Commit, 조회는 ReadOnly | **Proposed (Phase 2 구현 선택)** |
| [ADR-009](09_ADR-009-Reflection-Planning-MVP.md) | 근거 연결 Reflection·Scene/Beat 기반 활성 Plan을 Prepared Snapshot에 포함하는 MVP 정책 | **Proposed (Phase 2 구현 선택)** |
| [ADR-010](10_ADR-010-Deterministic-Beat-Graph-MVP.md) | Chapter/Sequence/Beat Graph, Prepared State 기반 구조화 조건과 최소 Hard Rule | **Proposed (Phase 2 구현 선택)** |
| [ADR-011](11_ADR-011-World-Fact-Schema-and-Prepared-Validation.md) | Host Fact Schema와 공통 World Reader, 제안/Prepared 검사 | **Proposed (Phase 2 구현 선택)** |

| [ADR-012](12_ADR-012-Testbed-Content-Bridge.md) | Host JSON 정의를 WAIN Session으로 연결하고 Game-owned Flag/6D Emotion을 초기화 | **Proposed (Host MVP)** |

**문서화 원칙:** 새로운 핵심 설계가 승인되면 관련 ADR을 생성·갱신하고 Architecture Contract, Flow, Development 현황을 함께 동기화한다. 아직 결정되지 않은 구현 최적화·타입·예외 복구는 Accepted로 기록하지 않는다.

| [ADR-013](13_ADR-013-Testbed-Sprite-Assets.md) | Host Sprite 경로/Ready 메타데이터/UMG Layered Renderer | **Proposed (Testbed 구현 선택)** |
| [ADR-014](14_ADR-014-Testbed-VN-Presentation.md) | Host 소유 VN 배경·날짜·Backlog Presentation | **Proposed (Testbed 구현 선택)** |
| [ADR-015](15_ADR-015-Turn-Diagnostics-and-DEV-Inspector.md) | 최근 48개 Turn Trace와 Host DEV Inspector, LOG 분리 | **Proposed (Host MVP)** |
| [ADR-016](16_ADR-016-Asynchronous-LLM-Proposal-Handoff.md) | Async OpenAI 호스트 응답을 TurnId/Revision Bound Proposal로 기존 동기 Transaction에 인계 | **Proposed (Host MVP)** |

- [ADR-017 — Host Usage Preflight and Warning-only Policy](17_ADR-017-Host-Usage-Preflight-and-Warnings.md) — **Proposed**, Host 조직 사용량 추정·경고/키 보호

- [ADR-018 — Plugin LLM Ownership and Host Directories](18_ADR-018-Plugin-LLM-and-Host-Directories.md) — **Proposed**, LLM 구현 Plugin 이동 / Host Data 명칭 확정
