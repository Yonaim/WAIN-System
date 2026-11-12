# ADR-014 — Host-Owned VN Scene Presentation and Backlog

- **상태:** Proposed (Host MVP UI 선택, PIE 시각 검증 전)
- **날짜:** 2026-11-13
- **관련:** [ADR-001](01_ADR-001-State-Ownership.md) · [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-012](12_ADR-012-Testbed-Content-Bridge.md) · [ADR-013](13_ADR-013-Testbed-Sprite-Assets.md)

## 배경

이전 Testbed는 플레이어 대화 전체 ScrollBox와 별도 Sprite Panel을 화면에 나열했다. 원본 비주얼 노벨의 게임 화면에 맞춰 배경·스탠딩·대사·날짜를 분리하되 WAIN Runtime State의 단일 Source of Truth를 유지할 필요가 있다.

## 검토안

- **Plugin 내부 UI 전담:** WAIN의 게임 비종속성을 해친다.
- **Host의 UMG Canvas와 Content JSON 정의 (MVP 채택):** Presentation을 게임 자산 책임으로 유지하며 플러그인 Transaction 변경이 없다.

## 결정 / 구현 정책

1. WAIN은 ExpressionId/VariantId, Commit된 대사/Beat/State를 제공하며, Host Widget이 화면을 갱신한다. Background 및 Story Date의 정본은 Host 콘텐츠 정의다.
2. `TestbedConfig.currentDate = "2015-04-07"`, `timeSlot = "after_school"`은 UI 시작 데이터다. 현재 UI는 `2015.04.07 Tue`와 `방과 후`를 출력한다. **일단 고정**하고, 날짜 진행은 별도 Game-owned World Clock/Event로 이후 모델링한다.
3. Scene ID→Background ID 매핑과 Background ID→PNG 경로는 서로 구분한다. 외부 PNG는 `Images/Backgrounds` 제한과 파일 존재 검증을 거친다.
4. Background/Sprite는 게임 전체 공간에 합성하고 입력 위에 가독성 높은 Dialogue Panel을 표시한다. 화면의 현재 대사는 하나이며, 모든 이전 대사는 Host UMG ScrollBox의 **임시 Backlog**에 기록한다.
5. LOG 버튼은 Backlog Modal을 열고 닫는다. Debug/Revision/Trust/Beat 등 내부 상태는 일반 HUD에서 숨기고 DEV 버튼으로만 노출한다. Backlog는 영구 저장/Memory Stream 대체물이 아니다.
6. Commit 실패는 State에 반영하지 않는다. Content Loader/Renderer/Widget 코드는 Game/Testbed Source에 위치하며 Plugin Core·Transaction 코드는 건드리지 않는다.

## 제약 / 위험

- 실제 Unreal Engine 5.8 컴파일, 한글 폰트·입력 필드 대비·크롭 및 레이어 합성의 육안 검증이 필요하다.
- 현재 CH01은 동일한 교실 Scene을 사용하므로 실제 Scene 변경 배경 테스트는 추가 시나리오가 필요하다.
- 외부 PNG/JSON의 Cooked Staging, Save/Load, Story Date 증가, Voice/Animation·Backlog 지속 저장은 이번 범위가 아니다.

## 검증 상태

Testbed `WAIN.Testbed.VN.*` 신규 5개 Automation Test 소스를 추가했다. 전체 C++ 테스트 등록은 86개 목표이며 실제 UE 실행 결과는 없다. [Host 구현 문서](../../../../../Docs/02_Implementation/07_VN-Screen-Implementation.md).
