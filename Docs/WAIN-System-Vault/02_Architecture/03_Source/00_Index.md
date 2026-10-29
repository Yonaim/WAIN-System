# 소스 코드 색인

> **한 줄 요약:** WAIN Testbed의 현재 코드·설정과 플러그인 서브모듈 경로를 찾는다.

상태: **초기 구조 확인됨** · 마지막 확인: 2026-10-29

| 구성요소 | 경로 | 현재 책임·상태 |
|---|---|---|
| Unreal 게임 모듈 등록 | `Source/WAINTestbed/WAINTestbed.cpp` | `FDefaultGameModuleImpl` 등록 |
| 게임 모듈 헤더 | `Source/WAINTestbed/WAINTestbed.h` | 기본 헤더 확인 |
| 빌드 의존성 | `Source/WAINTestbed/WAINTestbed.Build.cs` | Core, CoreUObject, Engine, InputCore, EnhancedInput |
| 타깃 설정 | `Source/WAINTestbed/WAINTestbed.Target.cs`, `WAINTestbedEditor.Target.cs` | 게임·에디터 타깃 |
| 맵 설정 | `Config/DefaultEngine.ini` | EditorStartupMap/GameDefaultMap 모두 `/Engine/Maps/Entry` |
| 입력 설정 | `Config/DefaultInput.ini` | 입력 설정 확인 위치 |
| 플러그인 서브모듈 | `Plugins/WAINSystem/` | README·문서·빈 플러그인 폴더; 런타임 모듈 미확인 |

콘텐츠, UMG, 시나리오, Adapter 구현이 추가되면 파일 경로와 실행 흐름을 연결한다.
