# Source Map — Phase 1 Verified Baseline + Phase 2 Character Delta

> **기준:** 2026-11-10 사용자 제공 `WAINTestbed-Source(1).zip`은 **Phase 1 검증본이라고 사용자가 확인**했다. Phase 2 신규 변경의 UE 5.8 빌드·테스트는 별도 미실행. 별도 Game 프로젝트의 독립 Actor State까지 원자성이 확장되는 것은 아니다.

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
| `Source/WAINSystem/Private/Character/WAINCharacterStatePreparer.{h,cpp}` | **Phase 2:** Trust 및 독립 6축 Emotion Delta를 Prepared Character State에 적용 |
| `Source/WAINSystem/Private/Core/WAINTransactionCoordinator.{h,cpp}` | Proposal Validation → Prepared State → Narrative Evaluation → Final Validation → Commit |
| `Source/WAINSystem/Private/Core/WAINStateStore.{h,cpp}` | 단일 Game Thread의 committed Session Root / Revision Publish |
| `Source/WAINSystem/Private/Core/WAINStateValidation.{h,cpp}` | 초기/최종 WAIN Snapshot의 공통 불변조건 검사 |
| `Source/WAINSystem/Private/Testing/WAINMapGameAdapter.{h,cpp}` | **테스트 전용** 메모리 Map Adapter (실제 Testbed 코드 미연동) |
| `Source/WAINSystem/Private/Tests/WAINRuntimeTests.cpp` | Phase 1 기존 6개 + Phase 2 Character 관련 6개, 총 12개 Automation Test 코드 |

## 10. 현재 적용된 경계

- `UWAINRuntimeSubsystem::StartSession`은 Host의 Adapter, Mock Proposal Provider, Narrative Evaluator를 C++로 주입받는다.
- `ProcessMockTurn`은 **Game Thread에서 동기식**으로 실행된다. LLM/Async/Blueprint API는 아직 없다.
- 상태는 `FWAINSessionRoot` 하나로 공개한다. Root 안의 `WAIN`과 `Game`은 논리적으로 서로 다른 소유권을 갖는다.
- `WAINMapGameAdapter`는 Plugin의 **Private/Testing**에 있으므로 실제 Testbed에서 직접 include하지 않는다. 실제 Host 구현은 Testbed 연결 단계에서 별도로 만든다.
- `relationship.trust`의 int32 Set/Add, 등록된 Mock Game Key의 int32 Set/Add, Character의 6축 Emotion `AddFloat`를 처리한다. 다른 Target·Action·Expression·Variant는 미지원으로 거절한다.
- Narrative Evaluator 인터페이스는 실제로 호출되며 Automation Test에서는 Trust 임계값으로 Beat가 한 번 변경되는 임시 평가기를 사용한다. 실제 Beat Graph/조건식 엔진은 미구현이다.

## 20. 다음 확장

1. 실제 Unreal 5.8 빌드/Automation Tests 통과 후 Testbed Widget 연결.
2. Character 6차원 Emotion Delta, World Fact 및 Host Presentation Catalog 검증 도입.
3. Narrative Graph/Memory와 실제 비동기 Provider 연결. 범용 저장소 간 트랜잭션은 별도 ADR 검토.

[Interaction Flow](../00_Flows/00_Main-Flow.md) · [Transaction Contract](../01_Contracts/05_Response-Validation-Commit.md) · [Implementation Roadmap](../../04_Development/02_Implementation-Roadmap.md)

## 30. Phase 2 Character Delta 확장 (신규 소스)

- `FWAINTransactionCoordinator::PrepareWAINState()`는 Current Snapshot을 복사하고, `FWAINCharacterStatePreparer::Apply()`에 Character Mutation 적용을 위임한 다음 TurnCount를 증가시킨다.
- Trust 계산을 기존 Coordinator에서 Character 전용 클래스로 옮겼다. 기존 Game Adapter·StateStore·Turn ID 처리·Root Publish 경계는 변경하지 않았다.
- `ValidateProposal()`는 유효하지 않은 Owner/Operation, Game `AddFloat`, NaN/Inf Emotion Delta를 Prepare 이전에 거절한다.
- Character별 Persona, 대화 Context 생성, Reflection, Memory는 별도 Phase 2 단계로 남겨두었다.

[Implementation Detail](../../04_Development/05_Character-Delta-Implementation.md)

## 40. Phase 2 Persona / Context Assembly 확장

- `Public/WAINCharacterContext.h`: Host Persona DTO/Source 계약과 한 턴의 읽기 전용 Character Context.
- `Private/Character/WAINCharacterContextBuilder.{h,cpp}`: Current Root 복사 → Persona 조회/검증 → Context 완성.
- `IWAINProposalProvider::GenerateWithContext`는 Host Persona Source가 없을 때만 이전 `Generate`와 호환된다.
- `WAINInteractionProcessor::ExecuteTurn`의 상위 흐름은 Current 조회 → Context Build → Proposal 생성 → Transaction Execute로 확장됐다.
- `UWAINRuntimeSubsystem::StartSession`은 마지막 인자로 선택적 Persona Source를 받는다. 별도 JSON 파싱이나 실제 LLM 연동은 없다.
- 테스트 `Private/Tests/WAINCharacterContextTests.cpp` 7개를 추가했다. 기존 12개를 유지해 **19개 테스트 코드를 작성**했다(UE 실행 미검증).

[ADR-007](../../03_Decisions/07_ADR-007-Read-Only-Character-Context.md) · [Details](../../04_Development/06_Character-Context-Implementation.md)
