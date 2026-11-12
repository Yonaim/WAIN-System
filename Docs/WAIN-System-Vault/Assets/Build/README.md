# Build Templates — Reusable Source Assets

이 폴더는 **설치된 프로젝트의 빌드 설정이 아니라 템플릿 원본**이다. Setup 에이전트는 프로젝트 실물을 조사한 후 필요한 한 계열만 선택한다.

| 프로젝트 유형 | 원본 템플릿 | 배치 대상 |
|---|---|---|
| 소규모 C / Make | [Makefile.template](Make/Makefile.template) | `<Project>/Makefile` |
| 일반 C++ / CMake | [CMakeLists.txt.template](CMake/CMakeLists.txt.template), [CMakePresets.json.template](CMake/CMakePresets.json.template) | 프로젝트 루트의 `CMakeLists.txt`, 선택적으로 `CMakePresets.json` |
| Unreal Engine | [Build-Integration.md](Unreal/Build-Integration.md) | 기존 `Source/<Project>*.Target.cs`의 관련 타깃 설정에 **병합** |

- 프로젝트 이름, 표준, 엔트리 소스, 타깃, SDK 경로 등의 자리표시자를 실제 조사 결과로 대체한다. 미확정 값을 사실로 만들지 않는다.
- **기존 프로젝트**에서는 Makefile, CMakeLists.txt, Target.cs를 통째로 덮어쓰거나 다른 빌드 시스템으로 자동 전환하지 않는다.
- 기본 포맷 도구의 원본은 [Formatting](../Formatting/format_before_build.py)에 있으며 Setup 과정에서 `.clang-format`(프로젝트 루트), `Tools/indent_regions.py` 및 `Tools/format_before_build.py`로 복사한다.
- 정상 빌드 경로마다 포맷 단계가 실제 컴파일에 선행하는지 실행으로 확인한다. CI·IDE·직접 빌드 등 별도의 진입 경로도 점검한다.
- 선택 정책은 [Build-System Selection](../../00_Engineering-Standards/03_Workflow/05_Build-System-Selection.md), 초기 질문은 [Setup Questionnaire](../../00_Engineering-Standards/03_Workflow/06_Setup-Questionnaire.md)를 따른다.
