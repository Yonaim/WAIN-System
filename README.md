# WAIN System — Unreal Runtime Plugin (Phase 1)

> **Status:** Minimal synchronous Mock Interaction. The Unreal 5.8 build and tests must be executed locally; they have **not** been executed as part of this archive build.

## What is implemented

- `WAINSystem.uplugin` and runtime module (`WAINSystem.Build.cs`)
- `UWAINRuntimeSubsystem` (C++ GameInstance API)
- Game Thread, single-session, single-turn-at-a-time processing
- Read-only committed `FWAINSessionRoot` with Revision
- Proposal validation → Prepared WAIN/Game snapshots → Narrative evaluator → final validation → one-root publication
- Duplicate Turn ID replay without duplicate Commit
- Mock Map Game Adapter and six Unreal Automation tests

The one-root publication is **only** atomic for state read through this in-memory Session Root on the Game Thread. It does not atomically mutate arbitrary Actor fields, network systems, databases, or external storage.

## What is not implemented

- Actual LLM, embeddings, Memory/Reflection/Planning, Emotion Delta mutations
- Full Narrative Beat Graph/conditions or Presentation Catalog validation
- Asynchronous `SubmitTurn`, Blueprint API, Save/Load, external-Actor multi-store transactions
- Testbed widget integration; the existing Testbed mock still runs separately

Unvalidated Presentation/Action identifiers are rejected, not silently accepted.

## Installation

Copy **`Plugins/WAINSystem/`** into the Unreal project. Do not replace the existing Testbed source or `Content/Data` assets.

1. Confirm the plugin appears in Edit → Plugins. If needed, enable `WAINSystem` and regenerate project files.
2. Build the project using the locally installed Unreal Engine 5.8 toolchain.
3. In Session Frontend → Automation, run `WAIN.Transaction.*` to exercise the Mock interaction.
4. Integrate the Testbed as a separate follow-up step by providing an `IWAINGameSnapshotAdapter`, `IWAINProposalProvider`, and `IWAINNarrativeEvaluator` to `UWAINRuntimeSubsystem::StartSession`.

Static validation report: `Docs/WAIN-System-Vault/05_Experiments/03_Phase1-Static-Validation.md`.

The module exposes a **synchronous C++-only** API for now. Production LLM integration will need a separate asynchronous lifecycle that preserves the Game Thread Commit boundary.

## Formatting / build hook

The plugin includes `.clang-format` and `Tools/` copied from the latest Universal Agent Vault. Run the formatter before every build from the project root:

```powershell
py -3 Plugins/WAINSystem/Tools/format_before_build.py --root Plugins/WAINSystem --source Source
```

For automatic regular UBT builds, add a `PreBuildSteps` entry to **each** actual target constructor (`WAINTestbed.Target.cs`, `WAINTestbedEditor.Target.cs` etc.), after checking existing project hooks:

```csharp
// Windows (Python Launcher installed)
PreBuildSteps.Add("py -3 \"$(ProjectDir)Plugins/WAINSystem/Tools/format_before_build.py\" --root \"$(ProjectDir)Plugins/WAINSystem\" --source Source");
```

In macOS/Linux use `python3` and `/` path separators. **These Target.cs files are not overwritten by this ZIP.** Live Coding can bypass TargetRules pre-build hooks; format manually in that path until verified. The formatter requires `clang-format` and Python on PATH.

## Docs and development record

- Obsidian Vault: `Docs/WAIN-System-Vault/00_Home.md`
- Top-Down Readability: `Docs/WAIN-System-Vault/04_Development/03_Implementation-Principles.md`
- State/Transaction: `Docs/WAIN-System-Vault/03_Decisions/`
- Implemented source map and test cases: `Docs/WAIN-System-Vault/02_Architecture/02_Source/00_Source-Map.md`

Unreal-specific Host code and character assets remain outside the reusable plugin.


## Phase 1 코드 스타일 보완 (2026-11-10)

역할별 탐색이 필요한 파일에 공통 표준의 세 줄 구분 주석과 번호 `#pragma region`을 적용했습니다. 작은 단일 기능 파일에는 강제하지 않았습니다. LLM Key/Provider 설정은 **현재 미구현**이며 `Docs/WAIN-System-Vault/04_Development/04_LLM-Configuration-Plan.md`에 구현 계획을 기록했습니다.

## Phase 2 — Character Context (2026-11-10)

`IWAINPersonaSource`를 선택적으로 `UWAINRuntimeSubsystem::StartSession`의 마지막 인자에 주입할 수 있다. Persona를 주입한 Provider는 `GenerateWithContext`를 구현해야 하며, 기존 `Generate`만 지원하는 Mock Provider는 Source 미주입 시에만 호환 호출된다. Unreal 5.8 컴파일·Automation 실행은 별도 검증이 필요하다. 자세한 사항은 `Docs/WAIN-System-Vault/04_Development/06_Character-Context-Implementation.md` 참조.
