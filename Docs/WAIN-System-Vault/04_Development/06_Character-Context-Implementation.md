# Phase 2 — Persona and Character Context Assembly

- **상태:** C++/Automation Test 코드 작성, Unreal 5.8 실행 미검증
- **범위:** Persona 정의를 Host로부터 조회하고 현재 WAIN Snapshot의 Character State를 Proposal Provider에 제공
- **정책 근거:** [ADR-007](../03_Decisions/07_ADR-007-Read-Only-Character-Context.md) · [Character Contract](../02_Architecture/01_Contracts/01_Character-Agent.md)

## 목표 및 Top-Down 처리

```text
ProcessTurn
  → ValidateRequest
  → ExecuteTurn
    → Read Committed Session Root
    → CharacterContextBuilder.Build (Character / Persona 조회·검증)
    → ProposalProvider.GenerateWithContext
    → TransactionCoordinator.Execute
    → Return Result
```

서사 진행·감정·관계를 Persona 파일에 저장하지 않는다. `Build`는 State 복사·Persona 검증만 수행하며 Commit을 시작하지 않는다. 기존 `ProcessTurn`/`TransactionCoordinator`의 Revision/Idempotency 처리 순서는 유지한다.

## 새·변경 코드 계약

| 경로 (모듈 기준) | 책임 |
|---|---|
| `Public/WAINCharacterContext.h` | `FWAINCharacterPersona`, `IWAINPersonaSource`, `FWAINCharacterContext` |
| `Private/Character/WAINCharacterContextBuilder.{h,cpp}` | Current Root에서 현재 Character를 읽고 Host Persona 조회·검증 |
| `Public/WAINInterfaces.h` | `IWAINProposalProvider::GenerateWithContext` 호환 오버로드 |
| `Private/Core/WAINInteractionProcessor.{h,cpp}` | Context Prepare → Proposal Generate 순서 연결 |
| `Public/WAINRuntimeSubsystem.h`, 대응 `.cpp` | `StartSession`에서 선택적으로 Persona Source 주입 |
| `Public/WAINRuntimeTypes.h` | `ContextPreparationFailed` 오류 |
| `Private/Tests/WAINCharacterContextTests.cpp` | 신규 테스트 7개 |

`StartSession(..., FString& OutError, TSharedPtr<const IWAINPersonaSource> PersonaSource = nullptr)`는 기존 C++ 호출을 유지한다. 실제 Persona Source 구현은 Host Testbed의 `Content/Data/Characters/ManaPersona.json`을 캐시하고 해당 Beat에서 안전한 내용만 제공하도록 이후 연결한다.

## 검증 규칙과 범위

- `CharacterId`가 현재 Snapshot에 있어야 한다.
- Source가 제공된 경우 조회에 성공하고 반환 Persona의 `CharacterId`가 요청과 같아야 하며 `DisplayName`이 비어 있으면 안 된다. Trait/Preference 항목도 빈 문자열을 허용하지 않는다.
- Source가 없으면 `bHasPersona=false`로 Context를 만들고 기존 Provider `Generate`로 위임한다.
- Source가 있으면 Provider는 `GenerateWithContext`를 구현해야 한다. 구현하지 않았다면 **Generation 실패 / No Commit**을 반환한다. Persona가 조용히 무시되게 하지 않는다.
- Persona 조회 실패에는 `ContextPreparationFailed`를 반환하며 Proposal/Transaction을 호출하지 않는다.
- Context는 읽기 전용 데이터 복사본이다. WAIN State 소유권과 Host Persona 정본은 변하지 않는다.

## 아직 하지 않은 작업

실제 Host JSON Loader/Schema 검증, Persona의 서사 단계별 안전 필터링, Memory Retrieval, World/Narrative Constraints 생성, Presentation Catalog 조회, Prompt Serialization, LLM API Config/Async Provider, Blueprint 연동. 기존 Provider 기본 메서드는 실제 Persona 내용을 사용하지 않으므로, **Persona Source와 Context-aware Provider를 함께 연결한 경우에만 Persona가 생성 경로에 반영된다.**

## 테스트

`WAIN.CharacterContext.*` 7개를 새로 작성했다. 기존 `WAIN.Transaction.*` 6개 및 `WAIN.Character.*` 6개도 함께 실행해야 한다. 문서상 총 **19개 테스트 코드 작성**이며, 이 작업 환경에서는 Unreal 빌드를 실행하지 않았다.
