# ADR-019 — Private LLM Layers and Public Contract Minimization

- **상태:** Proposed
- **일자:** 2026-11-14
- **범위:** 단일 WAINSystem Runtime Module의 LLM 내부 책임 분리

## Context

ADR-018에서 OpenAI 구현을 Plugin으로 옮겼지만 `FWAINOpenAIClient`가 HTTP, System Prompt, Context Prompt, Envelope 검증을 동시에 담당하고 OpenAI/Admin HTTP 헤더가 `Public/LLM`에 노출되어 있었다. Testbed는 구체적 Provider 구현에 직접 의존할 필요가 없다.

## Decision

1. **Config / Prompt / Providers / Validation / Monitoring**을 `Private/LLM` 하위 디렉터리로 나눈다. 추가 Unreal Module은 생성하지 않는다.
2. Public 계약은 `WAINLLMConfig.h`, `WAINLLMTypes.h`, `WAINLLMTurnService.h`로 한정한다. HTTP/Prompt/JSON Parser 클래스는 Private에 둔다.
3. `FWAINPromptBuilder`를 추출해 System Instruction과 ReadOnly Context Prompt를 독립적으로 생성한다. **기존 Prompt 필드와 길이 제한을 유지**한다.
4. Provider는 외부 OpenAI Responses HTTP와 응답 Envelope/Usage/오류를 처리하고, Validation은 Strict JSON 스키마 및 Typed Proposal을 처리한다. 실제 State 규칙과 Commit은 Runtime/Transaction이 계속 책임진다.
5. Organization Usage HTTP는 Private Monitoring에 두고, Host는 공개된 읽기 전용 `FWAINOrganizationUsageSnapshot`, `FWAINLLMUsageDiagnostics`로 경고·디버그 정보를 읽는다. **경고만 표시하고 호출은 계속 허용**한다.
6. TurnService와 Revision-bound Queue Provider는 `Private/LLM` 루트에 남긴다. 실제 Config 값은 Testbed `Config/WAIN`에 둔다.

## Consequences

- Provider 추가 시 Prompt와 Validation을 재사용할 수 있는 경계가 마련되지만 **현재 다중 Provider/키 전환은 지원하지 않는다**.
- Testbed의 `WAINOpenAIClient.h` include를 `WAINLLMTypes.h`로 변경하고 Usage Formatter API 이름이 `FWAINLLMUsageDiagnostics`로 바뀐다.
- 신규 `WAIN.LLM.Prompt.*` 테스트를 포함해 **Automation 테스트 소스 총 118개**. Unreal 5.8 컴파일·실행·PIE·네트워크 통합 검증은 별도로 수행한다.
- 기존 구체 Provider 헤더를 포함하던 외부 코드가 있다면 공개 TurnService/Types로 마이그레이션해야 한다.
- API Key와 Admin Key는 로컬 파일에서만 읽고, 공유 Source/Docs ZIP에 포함하지 않는다.

[ADR-018](18_ADR-018-Plugin-LLM-and-Host-Directories.md) · [Host Architecture](../../../../../Docs/02_Implementation/00_Testbed-Architecture.md) · [Implementation](../../../../../Docs/02_Implementation/11_LLM-Internal-Structure.md)
