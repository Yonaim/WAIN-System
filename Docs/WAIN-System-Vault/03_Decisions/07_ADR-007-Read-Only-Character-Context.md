# ADR-007 — Host Persona Source and Read-Only Character Context

- **상태:** Accepted (Phase 2 구현 계약; 신규 코드의 UE 빌드/테스트 미검증)
- **날짜:** 2026-11-10
- **관련 ADR:** [ADR-001](01_ADR-001-State-Ownership.md) · [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-004](04_ADR-004-Snapshot-State.md)

## 배경

Host가 정의한 상대적으로 고정된 Persona와 WAIN이 확정 상태로 소유한 Relationship/Emotion은 생성 시 함께 참조하되 정본 저장 위치가 달라야 한다. Persona 조회가 Transaction Commit을 건드리거나 과거 Snapshot을 수정하면 안 된다.

## 검토한 대안

- **Persona를 WAIN Character State에 복사하여 권위 저장:** Host 원본과 중복된 정본이 발생하므로 채택하지 않는다.
- **Prompt 문자열을 Plugin 내부에서 즉시 합성:** 모델 공급자별 표현 방식과 콘텐츠 노출 정책을 고정하므로 초기 단계에서 채택하지 않는다.
- **Host Persona Source 주입 + 구조화된 읽기 전용 Context 생성 (선택):** State Ownership을 보존하고 Provider가 Prompt 형식을 결정할 수 있다.

## 결정

1. Host는 `IWAINPersonaSource::GetPersona`로 `FWAINCharacterPersona`를 제공한다. 게임별 JSON 파싱·캐시와 공개 가능한 내용 선택은 Host의 책임이다.
2. Context Builder는 턴마다 현재 확정된 `FWAINSessionRoot`를 조회해 해당 Character의 `Trust` 및 독립 6축 Emotion을 **복사**하고, Persona Source가 주입되었다면 동일 `CharacterId`의 유효한 Persona를 요구한다.
3. `FWAINCharacterContext`에는 `CharacterId`, `SceneId`, `ActiveBeatId`, `StateRevision`, `TurnCount`, Character State, 선택적인 Persona만 포함한다. Memory/World/Presentation Catalog 데이터는 아직 포함하지 않는다.
4. Context Preparation에서 권위 State를 변경하지 않는다. Persona 조회/검증 실패 시 Proposal 생성 및 Commit을 수행하지 않는다.
5. 기존 Mock Provider와 시작 API의 호환을 위해 Source 주입은 선택적이다. **Source가 없는 경우에만** 기존 `Generate`로 위임하고, Persona Source가 있는데 Provider가 Context를 처리하지 못하면 명시적으로 실패시킨다.
6. 서사 비밀과 관련된 Persona 데이터는 Host가 현재 Beat에 맞춰 노출 범위를 제한해야 한다. `ActiveBeatId` 전달 자체는 비밀 누출 차단을 보장하지 않는다.

## 영향과 확장 시점

- 현재 C++ 전용 동기 경로에서 `GenerateWithContext`를 오버라이드하는 Provider가 구조화된 Context를 전달받는다. JSON Loader, 실제 LLM/Prompt, 비동기 요청, Memory/World/Presentation Context 조립은 미구현이다.
- 확장할 때 Context Assembly의 출처와 누출 정책을 명시하고 Provider 계약을 정리해야 한다. 범용 Prompt Template을 현재 단계에서 미리 고정하지 않는다.

## 검증 범위

- 성공 시 Persona·Trust·Emotion·Beat·Revision을 읽고 이전 Root의 값은 유지된다.
- 다음 턴은 이전 Commit 후 새 Revision과 Character State를 조회한다.
- 누락/ID 불일치 Persona는 No Commit; 기존 Mock Provider는 Source 미주입일 때 동작한다.
- 중복 TurnId는 Context/Persona를 다시 조회하지 않는다.

> 테스트 코드는 작성했지만 **Unreal Engine 5.8 빌드와 Automation Test 실행은 아직 수행하지 않았다.**
