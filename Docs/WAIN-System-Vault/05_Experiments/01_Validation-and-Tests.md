# Validation & Tests — Evidence Log

> **2026-11-10: 기존 Testbed JSON에 대한 이전 정적 점검 기록.** 이 기록은 구형 `Emotion Type/Intensity` 데이터를 대상으로 하며, **신규 6차원 Emotion/Transaction 구현 검증 결과가 아니다**. Plugin C++ 빌드/Unreal Editor 실행 검증은 없다.

## 과거 정적 점검 범위 (구형 JSON)

- `EmotionDefinitions.json`의 구형 Emotion Type/Intensity 후보가 정의되어 있는지 점검.
- `ManaEmotion.json` 초기 Type/Intensity가 후보 정의에 속하는지 점검.
- `ManaExpressions.json`의 `default_variant`, Variant 파츠 Key, Overlay Key 및 참조 PNG 경로·해상도/Alpha/Canvas를 확인하는 검사.
- `Presentation.json`의 Expression 목록과 Catalog 일치, `CH01Script.json` 기존 ID, 없는 파츠의 `planned_parts` 처리 점검.

## 신규 설계에 필요한 검증 (미실행)

- 6차원 Emotion State 값 범위(0~1), 동시 감정·Delta·잘못된 스키마 거절.
- Context Preparation의 허용 Presentation ID/Variant/설명 반환과 Proposal ID 검증.
- WAIN Current/Prepared Snapshot의 변경 격리, 실패 시 No Commit, Revision 경쟁 검사.
- Testbed Game Adapter의 `Prepare` 무부작용, WAIN/Game Prepared State 단일 Commit, 미지원 Game Mutation 거절.
- Trust/Inventory/Beat의 한 Revision 동시 관찰, 중복 Turn/Commit 후 UI 실패 처리.
- Unreal JSON Loader, 실제 PNG 표시 및 Packaging Stage, Memory, LLM 실패 및 A~D 비교 평가.

이 문서를 실제 테스트 성공 보고로 사용하지 않는다. 실행한 검증은 날짜·명령·입력·결과를 추후 개별 증거와 함께 기록한다.

## 2026-11-10 — Mock Runtime 검증 기록

- 자동화 테스트 소스는 `Source/WAINSystem/Private/Tests/WAINRuntimeTests.cpp`에 6개 작성하였다.
- **Unreal Engine 설치가 없는 생성 환경**에서 UBT/Unreal C++ 컴파일 및 Editor Automation 실행은 수행하지 못했다.
- 사용자 Windows Unreal 5.8 환경에서 플러그인 Load → Development Editor 빌드 → `WAIN.Transaction.*` 테스트를 실행해야 한다.
- Project `Target.cs` PreBuildSteps 자동 포맷 Hook은 Plugin Archive로 임의 수정하지 않았으므로 설치 후 별도 연결해야 한다.
- 본 문서의 '작성됨'은 실제 동작·컴파일 검증을 뜻하지 않는다.

정적 검사 세부 사항: [Phase 1 Static Validation](03_Phase1-Static-Validation.md).
