# Phase 2 — Runtime Turn Trace / Host DEV Inspector

> **상태:** C++ 구현 및 신규 Unreal Automation 테스트 코드 작성. **UE 5.8 실제 컴파일/PIE/테스트는 미검증**. 기능 이름에 `Trace`가 포함되어도 모든 내부 판단 근거를 수집하는 것은 아니다.

## 00. 추적 계약

```text
ProcessTurn
  Request Validation            → Step
  Current Snapshot Read         → Step
  Character Context             → Step
  Narrative Context             → Step
  World Context                 → Step / Skip
  Proposal Generation           → Step + Proposal copy
  Coordinator.Execute
    Proposal Validation         → Step
    Snapshot Read               → Step
    WAIN Preparation            → Step
    World Mutation Validation   → Step / Skip
    Game Preparation            → Step
    Prepared World Validation   → Step / Skip
    Narrative Evaluation        → Step
    Memory Reasoning            → Step / Skip
    Final Validation            → Step
    Atomic Publish              → Step
  Read Committed Revision
  Store Bounded Debug Record    → (최대 48개)
```

실패한 단계의 오류 문자열은 Trace에 남지만, 이후 단계는 실행되지 않은 것이므로 자동으로 PASS 처리하지 않는다. 실패한 Turn은 Replay 캐시처럼 다시 실행하지 않고, 같은 ID의 Replay는 기존 Debug Record를 덮어쓰지 않는다.

## 10. 주요 코드

| 경로 (Source/WAINSystem 기준) | 역할 |
|---|---|
| `Public/WAINInteractionDebug.h` | `EWAINTraceStatus`, `FWAINTraceStep`, `FWAINInteractionDebugRecord` |
| `Private/Core/WAINInteractionProcessor.{h,cpp}` | Context/Proposal/실패 기록, 최근 48건 보관, `GetDebugRecord` |
| `Private/Core/WAINTransactionCoordinator.{h,cpp}` | 실제 실행 단계·성공/실패/스킵 기록 |
| `Public/WAINRuntimeSubsystem.h`, `Private/WAINRuntimeSubsystem.cpp` | Host의 읽기 전용 `GetDebugRecord` API |
| `Private/Tests/WAINInteractionDebugTests.cpp` | 정상/실패/Replay 진단 테스트 소스 |

## 20. Host Inspector

Host는 `Source/WAINTestbed/WAINTestbedDebugInspector.{h,cpp}`에서 권위 Root를 참조해 **값 복사본**을 만들고, `WAINTestbedVNWidget`은 기존 DEV 버튼에 탭/내비게이션을 붙인다. 이전/다음 턴에서 Context와 State Delta를 확인할 수 있고 LOG Backlog는 변경하지 않는다.

- State: Trust/Emotion 전후, 실제 Game Fact 전후(숨김 표시), Expression/Variant와 PNG Parts.
- Narrative: 현재 Goal/Soft Constraints, 성공 턴의 BeatHistory, 후보 전이·Priority·조건식의 확정값 기반 재평가.
- Memory: 해당 턴 Context의 실제 검색 결과·Importance·ID·출처, 새로운 Commit 기록, 활성 Plan/Step.
- Trace: 요청/오류/Proposal, 실제 수행 단계만 PASS/FAIL/SKIP, Revision 전후, 전체 Turn 시간.

**중요 경계:** 의미 기반 LLM 판단 근거는 저장하지 않는다. Retrieval 수치 점수, Evaluate의 개별 실제 판단 로그 및 실패 시 Prepared State는 아직 기록하지 않는다.

## 30. 테스트 및 검증 상태

신규 `WAIN.DebugTrace.*` 3개, `WAIN.Testbed.Debug.*` 4개, 총 7개 작성. 기존 86개가 변경 없이 등록돼 있을 경우 **전체 93개 소스 등록**이다. 실제 Unreal Editor Automation 결과가 아니다.

향후 필요: Unreal 5.8 컴파일·PIE UI 확인, 48턴 메모리 사용량 확인, Debug만 접근할 수 있도록 Development 빌드 분리, 정량 검색 Score/자세한 Rule Trace 수집 여부 결정.

[ADR-015](../03_Decisions/15_ADR-015-Turn-Diagnostics-and-DEV-Inspector.md)
