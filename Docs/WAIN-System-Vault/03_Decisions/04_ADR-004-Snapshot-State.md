# ADR-004 — Prepared Snapshot State Management

- **상태:** Accepted (설계, 미구현)
- **날짜:** 2026-11-10
- **관련 요구:** FR-06, FR-07, FR-08, FR-11~12, NFR-02, NFR-06

## 맥락과 문제

검증 전에 실제 State를 수정하고 실패 시 백업으로 복구하면 중간 실패 및 부작용에 취약하다. Narrative Evaluation도 변경 후 상태를 미리 조회해야 한다.

## 고려한 선택지 및 트레이드오프

- Live State 직접 수정 + 이전 값 백업: 구현은 간단하나 Rollback이 필요하다.
- 모든 버전 장기 보관: Undo/Replay에는 유용하지만 현재 MVP의 필수 요구가 아니다.
- **Current + Prepared Snapshot (선택):** 상태 확정 전 변경 없이 평가할 수 있고, 이전 버전의 장기 보존을 강제하지 않는다.

## 결정과 근거

1. WAIN-owned의 권위 State는 현재 확정된 **Current Snapshot**으로 조회한다.
2. 턴 처리는 현재 Revision으로부터 독립적인 **Prepared Snapshot**을 생성하고 승인 후보 Mutation을 적용한다.
3. 유효성/서사 조건을 확인한 후 성공한 Prepared Snapshot만 새로운 Current로 공개한다. 실패한 Prepared Snapshot은 폐기한다.
4. Commit 전 Current를 수정하지 않으므로 Commit 전 실패에 **복구용 Rollback을 요구하지 않는다.**
5. Snapshot은 **직전 백업 보관** 자체가 목적이 아니다. 구현은 전체 복사 또는 안전한 구조 공유가 가능하며 과거 Snapshot 영구 보관은 필수가 아니다.
6. 동일 세션의 읽기·쓰기 동기화, Revision 경쟁 검사, Snapshot 불변성이 충족되어야 한다.

## 예상 결과 및 영향

WAIN-owned State 내부의 준비/검증 경계가 명료해진다. Snapshot만으로 Game-owned 저장소까지 원자성이 자동 확보되지는 않는다. Game 연동에는 [ADR-005](05_ADR-005-Minimal-Game-Adapter.md)가 필요하다.

## 재검토 조건 / 대체 ADR 링크

영속 Save/Load, 큰 Memory Store, 구조 공유의 비용, Replay/Undo 필요성이 커지면 Snapshot 저장 단위와 버전 보존 전략을 별도 결정한다.

## 구현 연결 기록 (2026-11-10)

Phase 1 코드에 `FWAINStateStore::Publish` 및 `FWAINSessionRoot`를 작성했다. 단일 Game Thread 및 협조적인 Root 조회에 한정되며 Unreal Build/실행은 미검증이다. 이 기록은 ADR의 설계 결정을 변경하지 않는다.
