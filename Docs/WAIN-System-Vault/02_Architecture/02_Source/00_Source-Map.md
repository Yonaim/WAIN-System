# Source Map — Phase 1 Runtime (Draft, Build Not Verified)

> **기준:** Phase 1 패키지에 실제 C++ 소스를 생성했다. Unreal 5.8 프로젝트에서 **아직 컴파일·실행하지 않았으므로 구현 검증 완료로 표시하지 않는다.** 별도 Game 프로젝트의 독립 Actor State까지 원자성이 확장되는 것은 아니다.

## 00. Runtime Module

플러그인 루트는 `Plugins/WAINSystem/`이다.

| 경로 (Plugin 기준) | 책임 |
|---|---|
| `WAINSystem.uplugin` | Runtime Plugin Module 등록 |
| `Source/WAINSystem/WAINSystem.Build.cs` | UnrealBuildTool 모듈 의존성 정의 |
| `Source/WAINSystem/Private/WAINSystemModule.cpp` | Runtime Module 등록 |
| `Source/WAINSystem/Public/WAINRuntimeTypes.h` | Mock Turn / Proposal / Mutation / Character·Emotion DTO |
| `Source/WAINSystem/Public/WAINInterfaces.h` | Host Snapshot Adapter, Proposal Provider, Narrative Evaluator 인터페이스와 Session Root |
| `Source/WAINSystem/Public/WAINRuntimeSubsystem.h` | GameInstance의 C++ 전용 Runtime API |
| `Source/WAINSystem/Private/WAINRuntimeSubsystem.cpp` | Session 수명과 Commit 후 State 변경 알림 |
| `Source/WAINSystem/Private/Core/WAINInteractionProcessor.{h,cpp}` | 입력 검증 → Proposal → Transaction → 결과 반환, TurnId 중복 보호 |
| `Source/WAINSystem/Private/Core/WAINTransactionCoordinator.{h,cpp}` | Proposal Validation → Prepared State → Narrative Evaluation → Final Validation → Commit |
| `Source/WAINSystem/Private/Core/WAINStateStore.{h,cpp}` | 단일 Game Thread의 committed Session Root / Revision Publish |
| `Source/WAINSystem/Private/Core/WAINStateValidation.{h,cpp}` | 초기/최종 WAIN Snapshot의 공통 불변조건 검사 |
| `Source/WAINSystem/Private/Testing/WAINMapGameAdapter.{h,cpp}` | **테스트 전용** 메모리 Map Adapter (실제 Testbed 코드 미연동) |
| `Source/WAINSystem/Private/Tests/WAINRuntimeTests.cpp` | Unreal Automation Tests 6개 |

## 10. 현재 적용된 경계

- `UWAINRuntimeSubsystem::StartSession`은 Host의 Adapter, Mock Proposal Provider, Narrative Evaluator를 C++로 주입받는다.
- `ProcessMockTurn`은 **Game Thread에서 동기식**으로 실행된다. LLM/Async/Blueprint API는 아직 없다.
- 상태는 `FWAINSessionRoot` 하나로 공개한다. Root 안의 `WAIN`과 `Game`은 논리적으로 서로 다른 소유권을 갖는다.
- `WAINMapGameAdapter`는 Plugin의 **Private/Testing**에 있으므로 실제 Testbed에서 직접 include하지 않는다. 실제 Host 구현은 Testbed 연결 단계에서 별도로 만든다.
- `relationship.trust`의 int32 Set/Add와 등록된 Mock Game Key의 int32 Set/Add만 처리한다. 다른 Target·Action·Expression·Variant는 미지원으로 거절한다.
- Narrative Evaluator 인터페이스는 실제로 호출되며 Automation Test에서는 Trust 임계값으로 Beat가 한 번 변경되는 임시 평가기를 사용한다. 실제 Beat Graph/조건식 엔진은 미구현이다.

## 20. 다음 확장

1. 실제 Unreal 5.8 빌드/Automation Tests 통과 후 Testbed Widget 연결.
2. Character 6차원 Emotion Delta, World Fact 및 Host Presentation Catalog 검증 도입.
3. Narrative Graph/Memory와 실제 비동기 Provider 연결. 범용 저장소 간 트랜잭션은 별도 ADR 검토.

[Interaction Flow](../00_Flows/00_Main-Flow.md) · [Transaction Contract](../01_Contracts/05_Response-Validation-Commit.md) · [Implementation Roadmap](../../04_Development/02_Implementation-Roadmap.md)
