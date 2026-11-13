# ADR-015 — Bounded Turn Diagnostics and Host DEV Inspector

- **상태:** Proposed (Host 디버그 구현 계약, 공개/보안 및 장기 저장 정책 최종 승인 전)
- **날짜:** 2026-11-13
- **관계:** [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-004](04_ADR-004-Snapshot-State.md) · [ADR-012](12_ADR-012-Testbed-Content-Bridge.md) · [ADR-014](14_ADR-014-Testbed-VN-Presentation.md)

## 00. 문제

기존 Testbed `DEV` 버튼은 `Revision / Beat / Turn / Trust / Expression` 한 줄만 제공했다. 실패한 턴의 실제 실패 단계와 Proposal을 보여주지 않았으므로 개발자가 No Commit 원인을 확인하기 어려웠다. 또한 플레이어용 Backlog와 개발용 Trace를 혼합해서는 안 된다.

## 10. 대안

1. 매 턴 전체 World/Game Snapshot을 영구 보관: 사용이 간편하지만 메모리/개인정보 비용이 커진다.
2. 로그 문자열만 출력: 지연이 적지만 Snapshot/Proposal/검색 Context의 진실성을 검증하기 어렵다.
3. **최근 48개 턴의 최소 진단 DTO + Host Snapshot 전후 요약본**: 세션에 한정되고 현재 모듈 경계를 보존한다.

## 20. MVP 결정

- Plugin `FWAINInteractionProcessor`에서 실제 실행된 **Request/Context/Proposal/Validation/Preparation/Evaluate/Publish** 단계만 `FWAINInteractionDebugRecord::Steps`에 PASS/FAIL/SKIP으로 보존한다. 실패 시 그 뒤의 미실행 단계는 기록하지 않는다. 전체 턴 실행 지연은 별도 필드로 저장하며 **단계별 시간은 기록하지 않는다**.
- Processor/Runtime Subsystem에서 `GetDebugRecord(TurnId)` 읽기 전용 API 제공. 이 API는 상태를 변경하거나 Commit을 유발하지 않는다. 성공/실패 기록은 최근 **48개 TurnId**에 제한한다. Replay는 원본 Trace를 덮어쓰지 않으며 UI에서 새 Pipeline 실행으로 표시하지 않는다.
- Host `FWAINTestbedDebugInspector`는 Commit 전후 Root에서 **Trust/Emotion/World Fact/Beat/새 Memory**만 복사하며, 원본 Root를 과거 턴마다 보관하지 않는다. 현재 활성 Plan과 선택된 Expression/Variant/PNG Layer 경로도 표시한다.
- Host `UWAINTestbedVNWidget`는 기존 DEV 버튼의 UI만 교체하여 `State / Narrative / Memory / Trace`를 제공하고 이전/다음/최신 턴 탐색을 허용한다. 기존 `LOG`는 사용자 대사 히스토리로 분리 유지한다.
- 서사 조건의 개별 평가 결과를 Coordinator에서 따로 전송하지 않았으므로 **Narrative 탭의 PASS/WAIT는 확정 Snapshot 값을 이용한 사후 재평가**이며, 실제 Evaluate 호출에서 사용한 Prepared 조건별 기록을 뜻하지 않는다. 실패 턴의 Prepared 값은 수집하지 않았으므로 재구성하지 않는다.
- Memory Retriever의 개별 수치 점수는 아직 노출하지 않는다. 순위·Content·Importance·SourceMemoryIds만 표시하고 정확한 Score는 `not captured`로 명시한다.
- DEV에는 **Host의 비공개 World Fact도 표시**될 수 있다. 현재 게임 내부 개발용이며 릴리스 빌드에서 DEV 표시 및 값 노출을 제거할 정책이 필요하다. 이 구현은 사용자가 권한으로 구분되는 생산환경 진단 시스템이 아니다.

## 30. 미지원 범위

- 영구 로그/Save·Load, JSON/CSV Export, 추적 필터/검색, LL​​M API Context/원문 보안 저장, 스테이지별 시간, 전체 Prepared Snapshot, 후보 전이별 실제 심사 메트릭, 최적화된 장기 History 등은 구현되지 않았다.
- Plugin의 Snapshot 원자성과 WAIN/Host State Ownership을 변경하지 않는다. 새로운 관측 기능만 추가한다.
- UE 5.8 빌드·PIE/Automation 실행은 현재 환경에서 미확인. 실제 프로덕션에 적용하기 전 메모리 프로파일 및 비공개 정보 노출 정책 점검 필요.

[구현 상세](../04_Development/11_Turn-Debug-Trace-Implementation.md)
