# ADR-017 — Host Organization Usage Preflight and Warning-only Policy

- **상태:** Proposed
- **일자:** 2026-11-13
- **범위:** WAIN Testbed Host 설정/Usage UI. WAINSystem Plugin의 권위 State나 Transaction 설계는 변경하지 않음.

## Context

WAIN Testbed가 아닌 다른 애플리케이션도 같은 OpenAI 조직의 API를 호출할 수 있다. 각 Responses API의 `usage`를 로컬에 합산하는 것만으로는 조직 전체 소비를 확인할 수 없다. 사용자는 현재 단일 `gpt-5.4-mini` Provider를 유지하고, 무료 사용량 근접/초과 시 모델 교체·요청 차단 없이 **화면 중앙 상단 경고만** 원한다.

## Decision

1. Host 프로젝트의 `Config/WAIN/LLMConfig.json`에 모델·Reasoning·요청 Timeout·모니터 경고 기준값을 둔다.
2. 일반 키와 Admin 키는 Host `Config/WAIN/LLMConfig.local.json` 또는 환경변수로 주입한다. Git, 배포 패키지, 로그, DEV UI에는 비밀값을 포함하지 않는다.
3. OpenAI 모드의 플레이어 턴 시작마다 Admin API Key로 조직 Usage GET을 요청한 후 기존 Responses POST에 연결한다. GET 실패는 개발자 경고일 뿐 LLM 호출을 막지 않는다.
4. Usage는 모델별 `input_tokens + output_tokens`를 바탕으로 **알려진 소형 인센티브 그룹 집계 추정**을 표시한다. 실제 무료 혜택 잔액/현재 과금 상태를 검증한 것으로 주장하지 않는다.
5. `>=240만` 주의, `>=249만` 위험 경고. 기본 250만 설정은 조직의 실제 무료 혜택 적용을 확인한 이후에만 의미가 있다. API 응답 지연·다른 프로그램 동시 호출로 과금 방지가 보장되지 않는다.
6. 경고는 비모달로 현재 화면의 상단 중앙에 표시하며 `닫기`가 가능하다. 수동 승인 과정 없이 LLM 대화 요청은 계속 진행한다.
7. DEV Inspector는 읽기 전용으로 모델, Reasoning, Usage 조회 시각/진단, 응답 토큰, 오류 코드만 남긴다. 원본 API Key, HTTP Body는 남기지 않는다.

## Consequences

- 정상 연결 시 플레이어 대화 한 턴의 HTTP는 GET+POST **2회**가 된다. Admin 키 미설정/모니터 비활성일 때는 POST만 보낸다.
- Admin 키는 조직 권한이 큰 비밀 정보다. **로컬 개발/PIE 전용**이고 배포 클라이언트에는 탑재하지 않는다.
- 새 C++ 파일은 Testbed에만 추가하며 WAIN Plugin에는 별도 의존성을 만들지 않는다.
- 다중 Provider/Key 자동 교체, 실제 결제 차단, 조직 정책 자동 확인, Admin API 전용 서버 Proxy는 추후 별도 작업이다.
- UE5.8 실제 빌드/PIE/네트워크 실호출은 사용자 환경에서 확인해야 하므로 본 결정은 Proposed다.

관련: [ADR-016](16_ADR-016-Asynchronous-LLM-Proposal-Handoff.md), [Testbed Usage Monitor](../../../../../Docs/02_Implementation/10_Organization-Usage-Monitor.md).
