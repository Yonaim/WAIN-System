# Project Brief — WAIN System

## 목적

**WAIN (World-Agent Interactive Narrative System)**은 자유형 자연어 비주얼 노벨에서 캐릭터의 지속적인 경험, 변화한 세계 상태, 제작자가 의도한 서사 진행을 **LLM 외부의 명시적인 State 관리**로 일관되게 유지하는 재사용 가능한 Unreal Engine C++ Plugin을 목표로 한다.

## 프로토타입 범위

- **Single-Agent / Turn-Based**: 플레이어 입력 한 번에 처리 턴 하나를 수행한다.
- Character Agent: Persona, Relationship, Emotion, Memory/Reflection/Planning의 결정에 필요한 Context를 구성한다.
- World State Manager: WAIN-owned 내부 Fact와 Game-owned Fact의 Adapter 접근을 구분한다.
- Narrative Manager: Chapter→Sequence→Beat, Goal/Constraint, 조건부 전이를 관리한다.
- Orchestrator: 생성→Schema/Rules/Narrative 검증→Beat 평가→승인된 State Commit→VN Response 순으로 조율한다.
- 런타임 `Expression ID`는 **게임 소유 Catalog에서 해석**하며 WAIN은 PNG·표정 파츠 이름을 보유하지 않는다.

## 범위 밖

Multi-Agent 동시 실행, 독립적인 실시간 World Event 기반 State 변경, NPC 자율 행동, 상용 게임 통합 검증, Save/Load 완성, 성능 보장치는 현 프로토타입의 범위 밖이다.

## 현재 소스 기반 확인 (2026-11-10, 첨부 testbed.zip)

- `Plugins/WAINSystem/`에는 `.uplugin` 및 `Source/`가 **없고 문서 볼트만 존재**한다.
- Testbed `Source/WAINTestbed/WAINTestbedVNWidget.cpp`는 JSON 일부를 읽어 **텍스트 UI 및 규칙 기반 Mock 대화**를 실행한다. WAIN Plugin 요청·검증·Commit·표정 렌더링과 연결되지 않았다.
- Testbed `Content/Data/`의 설정·Persona·Narrative·Script·Presentation은 실험용 JSON이며 대다수 필드를 런타임에서 소비하지 않는다.
- 이번 작업에서는 문서와 게임 소유 감정/표정 JSON만 작성했으며 **코드는 수정하지 않았다**.

## 개발 프로필

| 항목 | 기준 |
|---|---|
| 저장소 성격 | WAINTestbed 안의 `Plugins/WAINSystem/` — 별도 플러그인 저장소/서브모듈 |
| 활성 언어 | Unreal Engine 5.8 C++ (계획), JSON (Testbed 데이터) |
| C++ 명명·스타일 | Universal Agent Vault v5.9 **Unreal** 프로필 / 실제 코드 도입 시 formatter 결정 |
| 모듈 엔트리 | WAIN 플러그인 소스 미존재 — 미결정 |
| 빌드·테스트 | 실제 플러그인 실행/빌드 명령 미확정, 환경 미실행 |
| 게임별 결합 | Testbed는 자체 UI·에셋·Script·Content/State Adapter를 관리 |

## 정본 연결

[Requirements](02_Requirements.md) · [Constraints](03_Constraints.md) · [Architecture Flow](../02_Architecture/00_Flows/00_Main-Flow.md) · [Decisions](../03_Decisions/00_ADR-Index.md)
