# WAIN Phase 2 — Persona & Character Context Patch

## 기준

이 패치는 `WAINSystem-Phase2-Plugin-and-Vault.zip` 소스에서 변경·추가된 파일만 포함합니다.
UE 5.8의 실제 컴파일과 Automation Test 실행은 현재 환경에서 수행하지 않았습니다.

## 적용

WAINTestbed 프로젝트 루트에서 ZIP을 해제해 `Plugins/WAINSystem/...`으로 덮어쓰세요.
기존 Testbed의 `Content/Data`, Widget, Unreal Target.cs 및 설정은 변경하지 않습니다.

## API

`UWAINRuntimeSubsystem::StartSession`의 마지막 인자에
`TSharedPtr<const IWAINPersonaSource>`를 선택적으로 전달할 수 있습니다.

Persona Source를 전달했다면 Proposal Provider에서 `GenerateWithContext`를 Override해야 합니다.
Source를 전달하지 않으면 기존 `Generate` 기반 Mock 동작을 유지합니다.

테스트: `WAIN.Transaction.*`, `WAIN.Character.*`, `WAIN.CharacterContext.*` (총 19개 코드 작성)

## 내용

- read-only Persona Source, Character Context, Context Builder
- proposal generation 이전 Context Preparation 경로
- Persona 조회/ID 오류 시 No Commit
- 새로운 ADR-007 + Architecture/Development/Test Plan 갱신
