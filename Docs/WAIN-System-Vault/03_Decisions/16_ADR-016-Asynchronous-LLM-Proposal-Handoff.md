# ADR-016 — Async LLM Proposal Handoff and Strict Output Validation

- **상태:** Proposed (Local Testbed 구현 정책)
- **날짜:** 2026-11-13
- **관련:** [ADR-003](03_ADR-003-Commit-Before-Response.md) · [ADR-006](06_ADR-006-Turn-Based-Single-Agent.md) · [ADR-015](15_ADR-015-Turn-Diagnostics-and-DEV-Inspector.md)

## 배경

현재 WAIN Transaction은 Game Thread에서 동기적으로 처리되고, `IWAINProposalProvider`도 동기 인터페이스다. HTTP LLM 요청을 이 인터페이스 내부에서 동기적으로 기다리면 Game Thread가 정지하거나 HTTP 콜백이 지연되는 문제가 발생할 수 있다.

## 대안

- 동기 Provider에서 HTTP 응답을 폴링·대기: **거절**. Game Thread 블로킹 위험.
- Transaction을 즉시 멀티스레드화: **보류**. 기존 Snapshot/Game Adapter 계약을 광범위하게 수정해야 한다.
- **채택한 MVP 설계:** HTTP 비동기 요청을 먼저 수행한 뒤 기존 Game Thread Transaction에 결과를 단 한 번 전달한다.

## 구현 선택

1. Host Widget에서 `Runtime.PrepareTurnContext()`로 읽기 전용 Context를 준비한다. `BuildContext`는 Runtime Transaction 단계와 공통 코드를 사용한다.
2. `WAIN_LLM_MODE=openai`일 때만 Host `FWAINTestbedOpenAIClient`가 OpenAI Responses API에 비동기 요청을 전송한다. 기본 Mock은 그대로 유지한다.
3. `FWAINStructuredProposal`이 JSON Schema + 로컬 타입/범위 검증을 제공한다. 실제 World/Narrative 권위 검증은 Transaction에서 유지한다.
4. `FWAINTestbedQueuedProposalProvider`는 `TurnId`와 초기 `Revision`이 모두 맞는 Proposal을 **한 번만** 동기 생성 인터페이스에 반환한다. 불일치·오류는 실패로 반환한다.
5. 완성된 Proposal은 기존 `ProcessTurn`에 전달하여 **Commit 전 검증 → State Publish → VN 출력** 순서를 유지한다. 실패한 API/JSON/권한·서사 규칙은 No Commit이다.
6. API 키는 프로세스 환경변수 `WAIN_OPENAI_API_KEY`로만 받는다. 응답 본문·인증 헤더는 디버그 출력에 기록하지 않는다. 직접 클라이언트 접근은 **로컬 개발 전용**이다.
7. Store=false, 수동 입력, 단일 in-flight request, 30초 Timeout. 자동 Retry나 Mock Fallback은 수행하지 않는다.

## 한계

- 외부 연동이 중간에 Game Revision을 바꾸면 해당 LLM 결과를 거절한다. 중간 결과를 새 State에 자동 Merge하지 않는다.
- 실제 API의 refusal/incomplete/네트워크 오류를 처리하도록 작성했지만 해당 오류별 라이브 실행 결과는 아직 없다.
- Persona·Memory·현재 공개 World Fact 및 Narrative Goal은 외부 클라우드 API로 전송될 수 있다. 배포 서비스에서는 별도 서버 프록시·접근 정책이 필요하다.
- 생성기 출력의 JSON Schema 준수와 서사적 사실성은 다르다. Generation 자체와 Game/Narrative Constraint 검증을 분리한다.
- HTTP 처리 시간은 기존 `TurnTrace.ElapsedMs`와 별도이며, LLM 토큰 사용량·비용 메트릭은 다음 단계에 추가한다.

## 검증 상태

신규 오프라인 테스트 13개 코드를 추가했다. 기존 93개와 함께 소스에는 총 106개 자동화 테스트가 등록된다. 여기서 UE 5.8 컴파일·Automation·실제 OpenAI API 호출은 수행하지 못했다.

[Host 구현 문서](../../../../../Docs/02_Implementation/09_LLM-Structured-Output-Implementation.md)
