# UnrealBuildTool — Existing Target Integration

**Unreal 프로젝트에서 일반 Make/CMake 템플릿을 엔진 빌드용으로 생성하지 않는다.** `.uproject`, `Source/*.Target.cs`, `Source/*/*.Build.cs`의 현재 구성과 사용 중인 엔진 버전을 먼저 확인한다.

공식 `TargetRules.PreBuildSteps`는 **UBT가 해당 타깃을 빌드할 때** 호스트 셸에서 사전 명령을 실행하는 목록이다. **기존** `Source/<Project>.Target.cs`, `<Project>Editor.Target.cs` 등의 TargetRules 생성자에 필요한 타깃마다 다음 **한 줄만 선택해 추가**한다. 전체 Target.cs를 재생성하지 않는다.

**Windows (Python Launcher `py`가 설치된 경우)**

```csharp
PreBuildSteps.Add("py -3 \"$(ProjectDir)Tools\\format_before_build.py\" --root \"$(ProjectDir)\" --source Source");
```

**macOS/Linux (python3 실행 가능)**

```csharp
PreBuildSteps.Add("python3 \"$(ProjectDir)Tools/format_before_build.py\" --root \"$(ProjectDir)\" --source Source");
```

- `$(ProjectDir)` 등 UBT가 지원하는 매크로는 타깃 빌드 전에 확장한다. 호스트 셸과 Python 런처는 **실제 프로젝트 기기에서 시험**해야 한다.
- `Build.cs`는 모듈 의존성·컴파일 설정에 사용하고, 전체 빌드 전 포맷 같은 명령은 `Target.cs`의 빌드 훅으로 관리한다.
- **Game·Editor·Client·Server·Program 등 실제 사용하는 타깃마다** 별도로 연결 여부를 확인한다.
- Unreal 생성 코드, `Intermediate/`, `Binaries/`, Engine, Plugin의 외부 소스, 타사 코드는 포맷 대상에서 제외한다. `Tools/format_before_build.py`의 기본 소스 경로가 프로젝트 소유 코드에만 해당하는지 확인한다.
- **Live Coding, 일부 에디터 내부 컴파일·핫 리로드 경로는 일반 UBT 타깃 빌드와 같은 훅이 실행된다고 가정하지 않는다.** 직접 확인되지 않으면 해당 경로의 전용 사전 실행 또는 명시적 수동 포맷 절차를 추가하고, 미보장 경로를 기록한다.
- 일반 Unreal 빌드 전체가 이 훅을 항상 호출하는지, 훅 실패 시 빌드가 중단되는지는 엔진 버전·빌드 경로마다 실측한다. 실행하지 않았다면 검증 완료라고 쓰지 않는다.

검증 순서: (1) 실제 타깃 명시 (2) Python/clang-format 실행 가능 확인 (3) `PreBuildSteps` 로그 확인 (4) 포맷 오류 시 실패 확인 (5) 다른 타깃·Editor 빌드 경로 반복.

참조: [Epic UBT Target Reference](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/unreal-engine-build-tool-target-reference), [Epic UnrealBuildTool](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-build-tool-in-unreal-engine).
