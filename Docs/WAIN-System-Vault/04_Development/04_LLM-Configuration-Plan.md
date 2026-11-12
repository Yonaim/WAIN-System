# LLM Configuration Plan — Phase 2+ (Not Implemented)

> **상태: 설계 메모 / 미구현.** Phase 1은 결정론적 Mock Proposal Provider만 지원한다. 아래 옵션 파일이 현재 동작한다고 해석하면 안 된다.

## 00. 책임과 범위

- **Plugin 설정:** LLM Provider 식별자, 모델 이름, 요청 Timeout, 재시도 횟수, API Endpoint 선택, 생성 파라미터, Debug/Trace 옵션.
- **인증 정보:** API Key 또는 토큰. 공유되는 `Default*.ini`, Git 저장소, Obsidian Vault에 **실제 Key를 기록하지 않는다.**
- **Host 통합:** Testbed 개발용 로컬 인증 설정과 배포 환경의 비밀정보 주입 정책을 분리한다.
- **비동기 실행:** 네트워크 호출은 비동기로 진행하고, 결과 검증과 Snapshot Commit은 Game Thread에서 수행한다.

## 01. 권장 구성 방향

| 구분 | 권장 경로 또는 방식 | 유의사항 |
|---|---|---|
| 공개 가능한 설정 기본값 | `Config/DefaultWAINSystem.ini` 등 Unreal 설정 계층 | 실제 Provider 연동 시 읽는 코드와 함께 작성; Secret 없음 |
| 설정 스키마 | `UWAINDeveloperSettings` 또는 동등한 Config Object | `UPROPERTY(Config)`·검증·기본값 계약 확정 필요 |
| 로컬 API Key | 개발자 환경변수 또는 OS 보안 저장소 | 소스 관리 대상에서 제외; 로그/오류 메시지에 마스킹 |
| 데모 설정 예시 | `Config/WAINSystem.example.ini` (향후) | 사용자가 채우는 항목과 필수/선택 항목만 문서화 |
| 네트워크 사용 | Provider별 Adapter / 인증 주입 | Provider 교체·테스트 모킹 지원 |

## 02. 구현 시 확정할 계약

1. 사용하는 실제 Provider 및 지원 모델을 선택한다. 특정 서비스에 결합되지 않는 `IWAINProposalProvider` 계약을 유지한다.
2. `WAINRuntimeSubsystem` 초기화 시 설정 누락/미지원 Provider/Key 누락을 **명시적 오류**로 반환한다.
3. 설정 계층의 우선순위(Host Override → Environment/Secret → Default), 변경 시점 및 멀티 환경(에디터/패키징/CI)을 정의한다.
4. 실제 Key가 포함된 로컬 파일을 사용할 경우 정확한 ignore 규칙을 별도 정의한다. 이름에 `Default`가 들어가는 공유 설정 파일에 비밀값을 저장하지 않는다.
5. 로그/Telemetry/Crash Report의 토큰 노출 가능성, 타임아웃·재시도·요청량 정책을 테스트한다.

## 03. 연동 단계

- **Phase 1 현재:** Mock Provider; 네트워크 구성 및 Config 파일 미구현.
- **Phase 2:** Config 스키마 및 선택한 Provider의 실제 비동기 구현과 함께 샘플 파일 생성.
- **Phase 3:** Testbed에서 인증 주입·오류 처리·배포/평가 환경 검증.

관련: [Current Work](00_Current-Work.md) · [Implementation Roadmap](02_Implementation-Roadmap.md) · [AI Services](../02_Architecture/01_Contracts/06_Shared-AI-Services.md)
