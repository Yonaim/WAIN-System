# WAIN Data Contracts — Conceptual MVP

> 상태: **Phase 1 C++ DTO 초안 작성, 컴파일 미검증.** `USTRUCT`/Blueprint·Serializer·전체 State Type Registry는 후속 구현이다.

## Interaction Request

```json
{"turn_id":"TURN-0001","character_id":"MANA","scene_id":"SC_CH01_CLASSROOM_01","interaction":{"kind":"text","text":"안녕"}}
```

`text`와 명시적 `silence`를 구분한다. Turn ID는 향후 중복 실행 방지에 사용하며 실제 타입은 미정이다.

## Response Proposal

```json
{
  "character_id":"MANA",
  "dialogue":"오늘 날씨 정말 좋지?",
  "action_id":"LOOK_WINDOW",
  "presentation":{"expression_id":"EXP_MANA_SOFT_SMILE","variant_id":"default"},
  "proposed_state_changes":[
    {"owner":"character","target_id":"MANA.relationship.trust","op":"add","value":1},
    {"owner":"character","target_id":"MANA.emotion.joy","op":"add","value":0.1}
  ]
}
```

모델 출력은 **적용 명령이 아니라 검증 대상 Proposal**이다. Mutation은 Owner/Target/Operation/Value를 구조화하고 임의 파일/State 경로 쓰기는 금지한다. 값의 실제 타입·허용 Target Registry는 추후 확정.

## Emotion State

```json
{"joy":0.2,"sadness":0.0,"anger":0.0,"disgust":0.0,"anxiety":0.6,"surprise":0.0}
```

각 값은 독립된 0~1; 합계 1 제한 없음. 복합 Emotion Label은 정본으로 저장하지 않는다. **이 JSON은 신규 계약의 예시이며 현재 Host 파일 형식과 다르다.**

## State / Transaction

- Current WAIN Snapshot, Prepared WAIN Snapshot, Host Prepared Game Snapshot, Revision 개념을 구분.
- Game-owned 변경은 Adapter가 검증/준비할 수 있고 공유 Commit 경계에 참여할 수 있을 때만 승인.
- `Commit` 성공 뒤 VN Response 생성/전달. Save/Load, 버전 업그레이드, Memory Persistence 스키마는 별도 설계.

## Catalog / Presentation

Context Preparation에서 Host로부터 허용된 Expression ID·Variant·설명을 조회. Proposal ID는 해당 목록을 기준으로 검증한다. 원본 PNG 경로 및 렌더링 세부사항은 Game 소유.

## 버전 및 현실 구현

Testbed Content JSON의 기존 `schema_version: 1`은 신규 Schema를 자동 보증하지 않는다. 현재 Plugin Parser, Serializer, Catalog Query, Snapshot Commit은 미구현이고 패키징된 JSON의 Stage 정책도 별도 검증 필요.

## Phase 1 소스와의 대응 (2026-11-10)

- `FWAINInteractionRequest`: `FGuid TurnId`, `FName CharacterId/SceneId`, `EWAINInputKind`, `FString Text`. 한 `TurnId`에 동일 입력 재전송 시 완료 결과를 재사용하고 변경된 입력 재사용은 거절한다. 실패 재시도는 새 TurnId를 사용한다.
- `FWAINResponseProposal`: 대사/Action·Expression 식별자/Mutation 목록. 초기 구현에서는 검증할 Catalog가 없으므로 Action·Expression·Variant가 채워지면 **거절**한다.
- `FWAINStateMutation`: `Owner=Character|Game`, `Operation=AddInteger|SetInteger`, `TargetId`, `int32 IntegerValue`. Character는 `relationship.trust`만 지원(0~100), Mock Game은 사전 등록된 key만 지원(음수 불가). Enum 및 Target 확장 전까지는 미지원으로 처리한다.
- `FWAINEmotionState`: 승인된 6개 실수 차원, 초기/최종 Snapshot 범위 검증만 구현. Delta Mutation과 복합 감정 해석은 미구현.
- `FWAINSessionRoot`: `uint64 Revision`, `FWAINStateSnapshot WAIN`, `TSharedPtr<const IWAINGameStateSnapshot> Game`. 단일 Game Thread에서 게시하며 Root를 통하지 않는 독립 Host 상태 변경에는 원자성을 보장하지 않는다.
- `IWAINGameSnapshotAdapter`: `CreateInitialSnapshot`, `Prepare`, `Validate`. `Prepare`에는 권위 상태 변경 및 외부 부작용이 없어야 한다. Mock 구현은 `Private/Testing/WAINMapGameAdapter`이다.
- **API 공개 범위:** C++ 인터페이스와 `UGameInstanceSubsystem`은 초안이며, Public DTO는 Unreal Reflection(`USTRUCT`/`UENUM`)으로 아직 공개하지 않는다. LLM Async/Blueprint API도 확정하지 않았다.

[Source Map](../02_Source/00_Source-Map.md) · [ADR-005](../../03_Decisions/05_ADR-005-Minimal-Game-Adapter.md)
