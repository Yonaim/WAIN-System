# WAIN Debugging and Dev Tools

> 상태: 개발 도구 설계 제안. 나열된 도구는 구현 완료를 뜻하지 않으며, 최종 API와 UI는 미확정이다.

## 목적과 범위

개발자가 WAIN의 State와 Narrative 진행을 확인하고, 상호작용 실패를 재현해 원인을 찾을 수 있도록 개발 전용 도구의 범위를 정한다. 진단 정보는 플레이어용 VN 화면과 분리하고 게임 진행의 Source of Truth가 되지 않게 한다.

WAIN 플러그인은 상태 조회, 데이터 검증, Interaction 진단 정보를 제공한다. Testbed는 이를 읽어 Debug 화면에 표시하고 재현용 입력을 제공한다. 플러그인에 없는 런타임 동작을 Testbed가 별도로 복제하지 않는다.

## Runtime Debug 화면

Testbed에는 플레이 중 열고 닫을 수 있는 개발 전용 Debug 화면을 둔다. 일반 VN UI와 분리하며, Shipping 빌드에는 포함하지 않거나 비활성화한다.

### Character State Inspector

- 활성 Character ID와 Persona Definition ID
- Affection, Trust, Attachment와 Emotion의 현재 값
- 이번 턴에 조회된 Memory ID와 유형(Observation, Reflection, Plan)
- 제안된 값과 확정된 값의 구분

Memory 전체 저장소를 기본 화면에 펼치지 않고, 해당 턴에 검색되어 응답 Context에 사용된 항목을 중심으로 표시한다.

### Narrative Inspector

- Chapter, Sequence, Active Beat와 Beat History
- Beat Goal 및 현재 적용되는 Soft/Hard Constraint
- Completion Condition별 평가 결과
- 가능한 Transition, 우선순위, 최종 선택 또는 Beat 유지 사유
- 현재 공개 가능 정보와 Context에서 제외된 비공개 정보의 식별자

비공개 사실의 원문은 Debug 화면에도 기본 표시하지 않고, 포함 여부와 ID만 확인할 수 있게 한다.

### World 및 화면 상태 Inspector

- 이번 Interaction에서 조회된 World Fact와 소유자
- Adapter 조회·Commit의 성공 또는 실패
- Testbed에 전달된 최종 VN 응답과 선택된 배경·표정·포즈 ID
- 콘텐츠 ID를 찾지 못한 경우의 누락 표시

## Interaction 진단 Trace

각 Interaction은 동일한 ID로 Player Input, Context Preparation, Character Agent, Response Validation, Narrative Evaluation, State Commit, VN Response의 진행을 연결한다. 단계별 요약과 오류를 남기고, 특히 Provider 원시 응답은 파싱 결과·검증 결과와 분리해 전문을 보존해 Debug 화면과 개발 로그에서 확인할 수 있게 한다. Reflection·Planning 등 내부 LLM 호출이 여러 번 발생하면 호출 목적과 시도 번호도 구분한다. 저장은 개발 중 `Saved/InteractionTraces/` 아래 JSONL로 하는 안을 권장하며, 인증 정보는 기록하지 않는다.

## 재현과 Mock 실행

실제 Provider 응답에 의존하지 않고 다음 동작을 재현할 수 있는 개발 경로를 둔다.

- 정해진 입력과 Mock 응답으로 동일 Interaction을 반복 실행
- 저장된 입력을 현재 State 또는 동일한 초기 State에서 재실행
- 새 Session 시작 또는 WAIN 상태 초기화
- 지정 Beat에서 시작해 조건과 전이를 점검
- Provider 지연·오류·잘못된 JSON 응답을 주입해 실패 경로 확인

이 기능은 개발용 버튼, 명령 또는 테스트 Harness 중 간단한 방식으로 제공한다. Mock 응답을 사용한 실행은 실제 LLM 실행과 Debug 화면에서 명확히 구분한다.

## 콘텐츠 데이터 검사

Persona, Narrative, Script, Presentation JSON을 로드할 때 Schema 버전과 필수 필드를 검증한다. 가능하면 실행 전 검사 명령도 제공해 다음 오류를 빠르게 찾는다.

- 중복 ID, 알 수 없는 Character/Beat/Scene/Flag ID
- 존재하지 않는 Transition 대상이나 참조된 Persona·Narrative 파일
- 잘못된 Completion 조건 또는 지원하지 않는 Hard Constraint 연산자
- 사용되지 않거나 누락된 배경·표정·포즈 ID
- 파싱 실패와 지원하지 않는 Schema 버전

Soft Constraint는 자연어 지침이며, Hard Constraint는 결정론적으로 검사 가능한 구조 규칙이다. 자연어로 적힌 비밀 금지 문장을 기계적으로 보장되는 규칙처럼 표시하지 않는다. 공개하면 안 되는 사실은 Context 구성에서 빠졌는지 별도로 확인한다.

## 로그, 파일과 빌드 설정

개발 로그에는 `InteractionId`, 단계, 오류 종류와 경과 시간을 포함한다. Trace 원본은 개발 산출물로 취급하고 버전 관리 대상에서 제외한다. 원시 Provider 응답과 전체 Prompt 기록은 로컬 개발 중 진단에 필요한 경우에만 저장하며 인증 토큰과 헤더는 제외한다. Trace 쓰기 실패가 Interaction 결과나 State Commit을 바꾸지는 않는다.

개발 빌드에서는 Debug 화면, Mock Provider, Trace 저장을 설정으로 켤 수 있게 한다. Shipping 빌드에서는 Debug UI와 상세 Trace를 기본 비활성화한다. 패키징된 게임에서 일반 JSON 파일을 읽으려면 해당 파일들을 스테이징하는 설정이 별도로 필요하다.

## 권장 구현 순서

1. Interaction ID와 7단계 Trace, Provider 원시 응답 보존
2. Character State와 Narrative Inspector
3. Mock 응답 및 입력 재실행
4. JSON Schema·논리 ID 검사 도구
5. Testbed VN 화면 결과와 Trace 연결

## 관련 문서

- [[../01_Flows/01_Interaction-Pipeline|Interaction Pipeline]]
- [[04_Narrative-Manager|Narrative Manager]]
- [[05_Response-Validation-and-Commit|Validation and Commit]]
- [[06_Shared-AI-Services|Shared AI Services]]
- [[07_Data-Contracts|Data Contracts]]
