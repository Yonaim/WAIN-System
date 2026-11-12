# ADR-012 — Host JSON Content Bridge and Testbed Session Composition

- **상태:** Proposed (Phase 2 Host MVP 구현, 실제 UE 검증 및 콘텐츠 승인 전)
- **날짜:** 2026-11-12
- **참조:** [ADR-002](02_ADR-002-Emotion-Expression.md) · [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-007](07_ADR-007-Read-Only-Character-Context.md) · [ADR-010](10_ADR-010-Deterministic-Beat-Graph-MVP.md) · [ADR-011](11_ADR-011-World-Fact-Schema-and-Prepared-Validation.md)

## 00. 배경

기존 Plugin은 Persona·Expression·Narrative·World Reader를 주입할 수 있지만, Testbed 게임에서는 별도 Widget Mock으로 턴을 진행했다. 초기 JSON은 이전형 Emotion 타입과 Narrative boolean Flag를 사용한다. 두 구조를 무시하고 런타임 동작을 성공이라 부르면 Game-owned State 및 서사 상태의 정본이 중복된다.

## 10. 검토 대안

1. Plugin 안에 프로젝트 전용 JSON 경로를 하드코딩: Plugin 재사용성과 소유권을 훼손하므로 채택하지 않음.
2. Widget이 JSON을 읽고 매 턴 State를 직접 수정: Atomic Publish 경계를 우회하므로 채택하지 않음.
3. **Host Loader + Read-only Content Sources + Game Adapter + Session Bootstrap**를 Testbed Module에 구현(선택): Plugin Public Interface를 변경하지 않고 입력과 Commit 결과를 연결할 수 있음.

## 20. 초기 구현 선택

- `Content/Data/` 아래 정의 JSON 정본은 Testbed(Host)에 둔다. Plugin의 World/Character/Narrative Snapshot 구조와 정의 데이터를 분리한다.
- `CH01WorldFacts.json`(신규)에서 Boolean Flag를 정수 0/1로 표현한다. 공개 가능한 세 Story Progress Flag만 `bExposeToAgent=true, bWritableByProposal=true`이며 나머지 진실·시스템 관련 Flag는 비공개/쓰기 금지이다. **게임 후반의 공개 상태 변화 정책은 미구현**이다.
- `ManaEmotion.json`은 6차원 초기값 `joy,sadness,anger,disgust,anxiety,surprise`로 마이그레이션한다. 기존 EmotionDefinitions Type 목록은 용어집이고 WAIN State로 역변환하지 않는다.
- `CH01Narrative.json`의 bool Completion/허용 Transition/금지 Flag를 Host가 Graph와 Guard로 변환한다. Chapter·Sequence Goal은 기존 Beat의 의미를 바탕으로 새로 작성한 MVP 문구이므로 사용자 승인 전 정본으로 확정하지 않는다.
- Expression/Variant/Part/PNG 경로는 **기존 이름과 위치를 유지**하며 Loader가 유효성 확인 후 허용 후보를 제공한다. 실제 Texture 합성은 미래 작업이다.
- 모든 정의를 시작 시 로드·검증·캐시하고, 성공한 경우에만 WAIN Runtime의 동일 Root Session을 시작한다. Widget은 성공한 응답과 확정 Snapshot만 표시한다.
- 지금 Provider는 **결정론적 Mock**이며 실제 Prompt 생성/LLM이 아니다. Observation은 기록되지만 Reflection/Plan Reasoner는 주입하지 않는다.

## 30. 실패 정책과 제한

- 필수 JSON/참조/Schema 오류가 있으면 세션 시작을 거절하고 입력 처리를 중단한다. 초기화 실패를 임의의 Mock State 전진으로 대체하지 않는다.
- Proposal 검증·World Prepared 검증·Narrative/Memory 실패는 정상 UI 응답 전에 거절된다. Commit 이후의 UI 렌더 오류는 기존 State를 Rollback하지 않는다.
- Testbed의 금지 Flag 검사는 Game Fact **값 변경**을 거부하는 구조적 정책이다. 대사의 의미 검열, 외부 플러그인/코드에 대한 접근 권한 보안 장벽을 제공하는 것은 아니다.
- 개발 중 Editor에서 `Content/Data/*.json`을 직접 읽는 구조이며 Cooked 빌드의 Non-UFS Staging은 아직 검증되지 않았다.
- 실제 Unreal 5.8 컴파일/Automation Test 및 사용자 시연은 미검증이다.

## 40. 추후 결정

실제 LLM용 Prompt 노출 정책·World Flag 공개 시점, Story Trigger/조건 체계, Sprite 합성, Save/Load, JSON 버전 Migration, 패키징 자산 정책은 다음 ADR/구현으로 확장한다.

[Host 구현 상세](../04_Development/12_Testbed-Content-Integration.md) · [Testbed Docs](../../../../../Docs/02_Implementation/06_Content-Runtime-Integration.md).
