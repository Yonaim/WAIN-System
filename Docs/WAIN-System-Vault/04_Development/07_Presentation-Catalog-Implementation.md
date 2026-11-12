# Phase 2 — Host Presentation Catalog 조회와 Expression 검증

> **상태:** C++ 소스 및 Unreal Automation 테스트 11개 **작성**. 이 환경에서 Unreal 5.8 컴파일·테스트는 실행하지 않았음. 기준 코드는 사용자 제공 Phase 1 검증본 위에 이전 Phase 2 Character·Persona 변경을 누적한 것이다.

## 00. 목표와 소유권

Host Game은 캐릭터별 Expression/Variant 정의, 자연어 설명, PNG/Layer 매핑과 실제 렌더링을 소유한다. WAIN은 **그 턴에 선택 가능한 후보만** 읽어서 Provider에 전달하고, 생성된 선택이 동일한 목록에 있는지 검증한다. Emotion의 여섯 축과 표시 표정은 독립이다. [ADR-002](../03_Decisions/02_ADR-002-Emotion-Expression.md)를 따른다.

## 10. Top-Down 실행 흐름

1. `FWAINInteractionProcessor::ExecuteTurn()`에서 확정 Root를 읽는다.
2. `FWAINCharacterContextBuilder::Build()`가 Character State, 선택적 Persona, 선택적 Host Catalog 후보를 읽는다. Catalog 조회·구조 검증에 실패하면 **ContextPreparationFailed**, Provider 미호출, No Commit.
3. `IWAINProposalProvider::GenerateWithContext()`가 `FWAINCharacterContext::AvailablePresentations`를 받아 제안한다.
4. `FWAINTransactionCoordinator::ValidateProposal()`이 선택한 `(ExpressionId, ExpressionVariantId)` 쌍이 해당 턴의 후보에 있는지 **Prepare 이전**에 검사한다.
5. 기존 Prepared WAIN/Game State → Narrative Evaluation → Final Validation → Publish 순서를 변경하지 않는다.
6. 성공 Result에 Expression/Variant를 전달한다. 실제 Widget 렌더링은 **Host 책임, 미구현**이다.

## 20. DTO 및 API

| 타입 | 파일 | 계약 |
|---|---|---|
| `FWAINPresentationQuery` | `Public/WAINPresentationCatalog.h` | Character, Scene, ActiveBeat, Current Revision |
| `FWAINPresentationEntry` | 같은 파일 | 허용 Expression ID, Variant ID(None 가능), 비어 있지 않은 설명 |
| `FWAINPresentationOptions` | 같은 파일 | 해당 턴의 허용 Entry 목록. 빈 목록도 유효 |
| `IWAINPresentationCatalog` | 같은 파일 | `QueryAvailable(Query, OutOptions, OutError)` (읽기 전용) |
| `FWAINCharacterContext` | `Public/WAINCharacterContext.h` | 후보 목록과 `bHasPresentationCatalog` 포함 |
| `FWAINPresentationValidator` | `Private/Presentation/` | 목록 중복/ID/설명 검사, Proposal 선택 검증 |
| `FWAINInteractionResult` | `Public/WAINRuntimeTypes.h` | Commit 성공 시 Expression/Variant 포함 |

`StartSession(..., PersonaSource = nullptr, PresentationCatalog = nullptr)`의 마지막 두 인자는 선택적이다. 기존 Phase 1 Mock과 Persona 전용 Provider의 동작을 유지한다.

## 30. 실패 및 호환 정책

- **Catalog를 연결하지 않은 경우:** 기존 텍스트 전용 Mock은 계속 허용, Expression을 선택한 Proposal은 `InvalidProposal`로 거절.
- **Catalog를 연결한 경우:** 후보 목록이 비어 있어도 대사는 생성 가능. 없는 Expression 또는 Variant, Expression 없이 Variant만 선택한 경우는 `InvalidProposal`, No Commit.
- **Catalog 오류:** 조회 실패, Expression ID/설명이 비어 있음, 같은 Expression+Variant의 중복은 `ContextPreparationFailed`, Proposal Provider 미호출.
- **Provider 오류:** Catalog를 주입했는데 Context-aware Provider가 아닌 경우 기존 `Generate()`로 조용히 우회하지 않고 `ProposalGenerationFailed` 반환.
- **대체 정책:** WAIN은 Neutral Expression을 자동 삽입하지 않는다. Host가 기본 표시를 적용하려면 UI 정책으로 처리해야 한다.
- **Action ID:** 아직 별도의 검증 Schema가 없어 기존 정책대로 거절한다.
- **보안/불변조건:** Catalog 조회 중 Host 권위 State를 변경하지 말고, 동기 턴 처리 중 허용 목록의 의미가 변하지 않도록 Host가 보장해야 한다. Catalog Revision/재조회 검증은 비동기 LLM 단계에서 별도로 필요하다.

## 40. 아직 구현하지 않은 항목

- 실제 Testbed `Content/Data/Characters/ManaExpressions.json` 파싱과 JSON→Catalog Adapter 변환.
- Pose/Stage 기반 상세 필터, 실제 PNG 레이어 합성, Action/Event 검증.
- LLM Prompt 생성·API, 비동기 처리·Catalog Revision 충돌 처리.
- Expression 미선택 시 Host 기본 표정 정책 및 외부 UI Render 실패 복구.

## 50. 검증

`WAIN.Presentation.*`의 11개 Automation Test 코드를 추가했다. 정상 ID 선택/Result, 빈 목록에서 미선택, 미등록 Expression, 잘못된 Variant, Expression 없는 Variant, 불완전 항목, Catalog 누락, 중복 후보, Host 조회 실패, Legacy Provider 거절, Replay 시 Catalog 재조회 방지를 포함한다.

**실제 테스트 결과는 사용자 환경에서 UBT/Automation 실행 후 기록해야 한다.** 기존 `WAIN.Transaction.*`, `WAIN.Character.*`, `WAIN.CharacterContext.*`도 회귀 실행한다.
