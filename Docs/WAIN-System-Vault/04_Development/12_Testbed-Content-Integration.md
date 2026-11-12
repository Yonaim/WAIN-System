# Phase 2 — WAINTestbed Host JSON Runtime Integration

> 상태: **Host C++/JSON/자동화 테스트 7개 코드 작성, UE 5.8 빌드·실행 미검증**. 기준: World State Manager 누적 Plugin·Vault 및 `WAINTestbed-Source(1).zip`.

## 00. 작업 범위

이번 작업은 Plugin의 Public API/Runtime/Transaction 코드를 변경하지 않고 **Host `Source/WAINTestbed/`와 `Content/Data/`, Testbed `Docs/`**만 변경한다. `Plugins/WAINSystem/Docs/WAIN-System-Vault`에서는 ADR/Work Log/Source Map 상태를 기록한다.

## 10. 호스트 구현 흐름

```text
Widget.NativeOnInitialized
  ContentLoader.Load              ← Config, Persona, 6D Emotion, World Facts,
                                    Expressions, Narrative, Opening Script
  SessionBootstrap.Start
    HostAdapter / PersonaSource / CatalogSource
    NarrativeEvaluator + WorldStateManager
    UWAINRuntimeSubsystem.StartSession
  ShowOpeningScript

SubmitTurn
  CreateRequest(Text | Silence)
  Runtime.ProcessMockTurn
    Context → Mock Proposal → Validation → Prepared WAIN/Game → Publish
  OnSuccess: show committed dialogue and snapshot fields
  OnFailure: show error, no local Trust/Beat mutation
```

## 20. 연결 지점

| Host 경로 | 역할 |
|---|---|
| `WAINTestbedContentLoader.{h,cpp}` | JSON 필수 필드/Schema, 참조, PNG Part 경로 검사 및 DTO 조립 |
| `WAINTestbedRuntimeAdapters.{h,cpp}` | Host Snapshot Adapter, World Fact Reader, Persona/Presentation Sources, Narrative Guard, Mock Provider, Bootstrap |
| `WAINTestbedVNWidget.{h,cpp}` | 직접 State 변경 제거, Runtime Commit 결과만 UI에 반영 |
| `WAINTestbed.Build.cs` | `WAINSystem` 모듈 의존성 추가 |
| `Tests/WAINTestbedIntegrationTests.cpp` | Editor Content 경로를 이용한 Integration Tests 7개 |
| `Content/Data/Scenarios/CH01WorldFacts.json` | Story Progress Flag의 0/1 정의·권한·초기값 |
| `Content/Data/Characters/ManaEmotion.json` | 독립 6차원 초기값 |
| `Content/Data/Scenarios/CH01Narrative.json` | 변환을 위한 Chapter/Sequence Goals·Flag 정의 보완 |

## 30. 구현 완료와 미지원 구분

**코드 작성:** JSON 캐싱·Persona/Expression/World Fact/Narrative 연결, 초기 Story Flag를 이용한 Intro → Transfer Hint → Memory Doubt 전이, Host Game Snapshot Commit 조회, 실패 No Commit.

**아직 미지원:** 실 LLM 응답, Reflection/Plan Reasoner 주입, PNG Layered Sprite, Blueprint/UMG Widget 자산, Save/Load, 콘텐츠 Hot Reload, Cooked 파일 Staging/패키징 보장, 텍스트 의미 Hard Constraint 검열.

`WAIN.Testbed.*`는 **총 7개 테스트 코드**가 작성됐고, 이전 Plugin의 `WAIN.*` 68개 Automation Test 코드도 회귀 대상이다. **테스트 등록 코드 숫자는 PASS 숫자가 아니다.**

[ADR-012](../03_Decisions/12_ADR-012-Testbed-Content-Bridge.md).
