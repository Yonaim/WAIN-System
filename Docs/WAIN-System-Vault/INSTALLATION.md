# WAIN Plugin Vault — 업데이트 적용 및 초기 설정

## 00. 이번 ZIP의 대상

- 압축 해제하면 `Docs/WAIN-System-Vault/`가 나온다. 대상은 **WAIN 플러그인 저장소**의 `Plugins/WAINSystem/Docs/WAIN-System-Vault/`에 해당하는 위치다.
- `WAIN-System-Vault`를 별도의 Obsidian 볼트로 열면 `.obsidian/`의 테마·플러그인·설정이 사용된다.
- **현재 저장소의 기존 Vault를 먼저 백업한 후 폴더 단위로 교체**한다. 단순 덮어쓰기는 오래된 파일을 남길 수 있다.
- WAIN Plugin이 별도 Git 저장소/서브모듈이면 해당 Repo에서 변경을 검토·커밋하고 필요한 경우 Host의 서브모듈 포인터를 갱신한다.

## 10. 동기화 기준과 예외

- 최신 공통 볼트와 **내용 동일:** `00_Engineering-Standards/`, `.obsidian/` (아래 예외 제외), `.vscode/`, `sortspec.md`, `Assets/`의 Formatting/Build 재사용 원본, `copilot/skills/`.
- **보존:** WAIN 아키텍처·계약·ADR·개발/평가 문서·Research/PDF·Legacy Archive. `AGENTS.md`는 공통 진입 규칙 + WAIN 전용 규칙을 결합했다.
- **의도적 제외/개인화:** `.git/`, `.copilot/model-catalog-cache.json`, `copilot/copilot-conversations/`, Copilot의 계정·기기 식별자/키체인 토큰. 개인 상태는 다른 프로젝트 볼트에 전파하지 않는다.
- Copilot의 기본 설정·모델 선택 정보는 가져오지만, 계정 연결이나 인증이 필요한 기능은 새 볼트에서 다시 설정할 수 있다.

## 20. 빌드 전 포맷 파이프라인은 별도로 설치

**이번 ZIP은 C++ 코드를 생성하거나 실제 빌드 훅을 변경하지 않는다.** 이후 실제 Plugin 소스 저장소를 확인하고 다음 설치 작업을 수행한다.

1. [Project Setup](00_Engineering-Standards/03_Workflow/04_Project-Setup.md)의 기존 프로젝트 절차에 따라 `.uproject`, `Target.cs`, `Build.cs`, Engine 버전·Source 실제 경로를 확인한다.
2. 볼트 [`Assets/Formatting`](Assets/Formatting/format_before_build.py)의 원본에서 `.clang-format`을 프로젝트 루트에, `indent_regions.py`, `format_before_build.py`를 프로젝트의 `Tools/`에 배치한다. 기존 설정이 있으면 병합하고 원본을 무단 덮어쓰지 않는다.
3. [UBT Integration](Assets/Build/Unreal/Build-Integration.md)에 따라 관련 실제 `Target.cs`에 **컴파일 이전 사전 작업**을 연결한다. 일반 UBT 빌드·Live Coding에서의 실제 동작은 각각 확인한다.
4. **`clang-format → indent_regions.py` 순서**, 연속 실행 안정성, 실패 시 빌드 중단 여부를 확인하고 Work Log/검증 문서에 남긴다. 설치 전에 완료로 표시하지 않는다.

## 30. VS Code와 Obsidian 설정 적용 범위

- 볼트 자체를 VS Code 작업 폴더로 열면 볼트의 `.vscode/settings.json`이 적용된다. **플러그인 저장소/Unreal 프로젝트 루트를 열 경우에는 상위 작업 폴더 설정과 병합해야 한다.** 현재 공통 최신 설정의 `explorer.sortOrder`는 `filesFirst`다.
- Obsidian은 `WAIN-System-Vault` 폴더를 직접 볼트로 열고 Blue Topaz·Style Settings·Custom Sort·기타 플러그인 및 `sortspec.md` 적용을 확인한다.
- Obsidian Sync·외부 계정 연결 상태는 사용자/장치별 기능이므로 이번 복사로 자동 로그인 또는 동기화 활성화를 보장하지 않는다.

## 40. 구현 시작 규칙

`AGENTS.md` → `01_Document-Map.md` → [Implementation Principles](04_Development/03_Implementation-Principles.md) → 해당 Contract/ADR 순서로 필요한 문서를 확인한다. **상위 함수의 처리 흐름이 먼저 읽히는지**를 우선 검토한다.
