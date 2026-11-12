# Emotion State & Host Presentation Contract

> 상태: **Plugin의 C++ Host Catalog 조회/검증 코드 작성 / 실제 Testbed JSON Loader 및 Content 마이그레이션 미구현.** [ADR-002](../../03_Decisions/02_ADR-002-Emotion-Expression.md).

## State와 Presentation 경계

| 구분 | WAIN Character Agent | Host / Testbed |
|---|---|---|
| Emotion | `joy/sadness/anger/disgust/anxiety/surprise` 0~1 연속 State 정본; Delta Proposal/Commit | 초기 설정 콘텐츠 제공 가능; 런타임 정본 아님 |
| Complex Emotion | 6차원 값 + Persona/Context에 따른 의미 해석 | 별도 Expression Preset은 자유롭게 정의 |
| Expression | 허용 목록 조회, 의미 ID/Variant Proposal 및 Validation | Catalog/Variant/PNG 경로 정의 및 실제 렌더링 |
| Physical Assets | PNG 파일·레이어 합성 구현을 알지 못함 | `Content/Drawing/` 및 Presentation JSON 소유 |

## Host Interface — 개념 계약과 Phase 2 C++ 구현

- `QueryAvailablePresentation(CharacterId, SceneId, PoseId, Context) -> AllowedExpressionOptions`.
- 각 Option은 `expression_id`, 허용 `variant_id`, 자연어 설명 및 필요한 제한 조건 제공.
- Game 시작 시 Catalog 자체의 파일/ID/PNG 존재 확인. Context Preparation에서 **그 턴에 사용 가능한 후보**를 필터링해서 모델에 제공.
- 생성된 ID/Variant는 반드시 허용 목록과 Host 정책으로 검증. 기본 대체는 **Host에서 명시한 경우에만** 허용.

## 테스트베드 기존 데이터와의 차이

- `Content/Data/EmotionDefinitions.json`과 `Characters/ManaEmotion.json`은 기존 **Emotion Type/Intensity 기반 초안**이며 새 6차원 State를 구현한 데이터가 아니다.
- `Content/Data/Characters/ManaExpressions.json`, `Presentation.json`은 게임 측 표현 Preset과 파츠 ID 참조를 유지한다.
- 현재 Testbed의 `eye_0.png` 등 원본 파일명, 동일 캔버스 레이어 합성 계약을 유지. `embarrassed`는 Host Expression Preset이지만 WAIN의 추가 Emotion 차원은 아니다.
- 추가 예정 파츠는 파일이 실제로 있을 때만 활성 Variant에서 참조한다. 기존 Host JSON 및 PNG는 **이번 문서 작업에서 수정하지 않았다.**

[Emotion Flow](../00_Flows/01_Emotion-Expression-Flow.md) · [Data Contracts](08_Data-Contracts.md)

## Phase 2 C++ 실제 지원 범위 (2026-11-11)

- `IWAINPresentationCatalog::QueryAvailable(FWAINPresentationQuery, OutOptions, OutError)`는 Character/Scene/ActiveBeat/Revision을 입력받는다. 초기 버전에서는 Pose 필터 및 Host 파일 존재 검사가 인터페이스 자체에 포함되지 않는다.
- `FWAINPresentationEntry`는 정확한 Expression/Variant 조합과 자연어 Description을 포함한다. Catalog 구조·중복 검사 후 Context로 복사하고, 동일 후보 목록으로 Proposal을 검증한다.
- `FWAINInteractionResult`에 허용된 Expression/Variant 선택을 반환한다. 실제 Host PNG 합성은 미구현이다.
- 미등록 ID, 잘못된 Variant, 조회 실패는 Commit 이전에 거절한다. Context-aware Provider가 아닌 기존 Mock은 Catalog 주입 시 실패한다.

[Presentation 구현 문서](../../04_Development/07_Presentation-Catalog-Implementation.md)
