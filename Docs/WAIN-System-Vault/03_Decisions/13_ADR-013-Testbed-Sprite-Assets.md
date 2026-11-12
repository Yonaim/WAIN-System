# ADR-013 — Host Asset Paths and Layered Sprite Presentation

- **상태:** Proposed (Host Testbed MVP 파일 구조 / Unreal 검증 전)
- **날짜:** 2026-11-13
- **연관:** [ADR-002](02_ADR-002-Emotion-Expression.md) · [ADR-012](12_ADR-012-Testbed-Content-Bridge.md)

## 결정

- WAIN Plugin은 Expression/Variant 선택 검증까지만 담당하고 PNG 파일·렌더링·Part 준비 상태는 **Host Testbed**가 소유한다.
- `Content`는 `Data`, `Images`, `UI`, `Maps`로 역할을 구분한다. 미술 원본 파일(`.clip`, `.pur`, 작업 ZIP)은 `SourceArt`로 분리한다.
- 캐릭터별 Expression JSON은 `basePath = "Images/Characters/Mana/Layers"`와 파일명만 저장한다. **파츠 키와 PNG 파일명은 동일**, 기존 Expression/Variant ID 및 대소문자 보존.
- 작화 미완료 항목은 전체 캔버스의 투명 PNG를 만들고 `ready=false`, `note="아직 준비 안됨"`로 표시한다. 표준 JSON의 주석은 사용하지 않는다.
- Layered Renderer는 Host UMG에 구현하고, Commit 이후 Expression ID/Variant ID를 기준으로 선택한다. Image의 Canvas 2261×5868 좌표 및 Layer 순서 유지.
- 초기 범위는 Editor/PIE의 파일 기반 Texture Import다. Cook/Shipping·비동기 디코딩·GPU 캐시 최적화·Background Renderer는 추가 작업이다.

## 대안

모든 파츠에 독립 절대 경로를 반복하거나 `.uasset` Texture 경로로 바로 변환하는 방식을 비교했다. 현재는 원본 PNG와 기존 테스트 계약을 빠르게 유지하기 위해 단일 `basePath`를 선택한다. 상용 배포 단계에서는 재평가한다.

## 검증 상태

C++ 및 JSON/PNG 파일 변경과 신규 Testbed 테스트 6개를 작성했다. **Unreal 5.8 빌드·PIE 실행은 미검증**이다. [Testbed Visual Asset 구현](../../../../../Docs/02_Implementation/03_Visual-Asset-System.md).
