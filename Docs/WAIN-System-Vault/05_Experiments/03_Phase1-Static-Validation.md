# Phase 1 Static Validation — 2026-11-10

> 이 문서의 '통과'는 **생성 환경의 정적 검사만** 의미한다. Unreal Editor 실행이나 C++ 컴파일 성공이 아니다.

## 점검 결과

| 항목 | 결과 |
|---|---|
| Plugin Descriptor `.uplugin` JSON 파싱 | 통과 |
| Vault 내 JSON / Plugin Descriptor 24개 파싱 | 통과 |
| WAIN-owned C/C++ 파일 16개 최상단 `@file` 책임 주석 | 통과 |
| C/C++ 브레이스 수량 기본 검사 | 통과 (파서·컴파일러 검사는 아님) |
| Private Header include 경로 확인 | 통과 |
| 최신 Vault의 Markdown 상대 링크 253개 | 통과, 깨진 링크 0개 |
| Python 포맷 도구 두 개 `py_compile` | 통과 |
| `format_before_build.py --check` | **미완료: 생성 환경에 `clang-format` 없음** |
| Unreal Build Tool / UE 5.8 C++ 컴파일 | **미실행: 엔진 없음** |
| Unreal Automation Tests 6개 | **코드 작성, 미실행** |
| Testbed Widget 및 실제 Game Adapter 통합 | **미구현** |
| Editor Live Coding / `Target.cs` PreBuildSteps | **미설치·미검증** |

## 실제 Unreal 개발 환경에서 검증할 순서

1. `clang-format`과 Python 3를 설치하고 Plugin `Tools/format_before_build.py`를 실행한다. 같은 명령을 연속 2회 실행하여 포맷 결과가 안정적인지 확인한다.
2. `WAINTestbed.Target.cs` 및 `WAINTestbedEditor.Target.cs`에 `PreBuildSteps`를 프로젝트 실제 빌드 정책과 충돌 없이 추가한다. Live Coding 경로는 별도로 확인한다.
3. UE 5.8에서 Plugin 모듈을 Development Editor로 컴파일한다.
4. Session Frontend의 Automation 필터로 `WAIN.Transaction.*` 6개를 실행하고 실행 기록을 남긴다.
5. 각 Testbed UI/Actor가 Session Root를 읽도록 연동한 뒤 State 변경 알림·표시 실패 정책을 검증한다.

## 계약상 유의사항

현재 Root Publication은 **Game Thread에서 협조하는 Root 읽기 경로**에 한정된다. 독립적인 Actor/DB/외부 서비스가 동시에 변경되는 보편적인 다중 저장소 Transaction은 지원하지 않는다. 테스트 Adapter는 `Private/Testing` 내부 예시이며 Host 제품 코드의 권위 상태 관리 방식과 동일하지 않다.
