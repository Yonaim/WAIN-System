# 소스 구조 색인

## 현재 저장소 상태

확인한 WAINTestbed 작업 폴더에는 호스트 Unreal 프로젝트와 WAINSystem 플러그인 폴더가 있다. 확인 시점에 플러그인 폴더에는 문서만 있고 플러그인 설명자(.uplugin)와 Source 디렉터리는 없었다. 따라서 이 페이지는 실제 코드 색인이 아니라 계획된 소스 책임을 기록한다.

아래 모듈을 이미 컴파일되거나 테스트된 런타임 기능으로 해석하지 않는다.

## 계획된 모듈 경계

| 모듈 | 계획된 책임 | 관련 계약 |
|---|---|---|
| Interaction Orchestrator | 상호작용 절차 조율. Character·World State를 소유하지 않음 | [[../02_Contracts/05_Response-Validation-and-Commit|Commit 흐름]] |
| Character Agent | Persona, Relationship, Emotion, Memory를 바탕으로 응답과 상태 변경 제안 | [[../02_Contracts/01_Character-Agent|Character Agent]] |
| World State Manager | 내부 World Facts와 호스트 게임 Adapter 경계 | [[../02_Contracts/03_World-State-Manager|World State Manager]] |
| Narrative Manager | Beat 진행과 서사 제약 관리 | [[../02_Contracts/04_Narrative-Manager|Narrative Manager]] |
| Response Validator | 구조화된 응답과 제안 변경 검증 | [[../02_Contracts/05_Response-Validation-and-Commit|검증 계약]] |
| Shared AI Services | Provider용 LLM·Embedding 서비스 추상화 | [[../02_Contracts/06_Shared-AI-Services|Shared AI Services]] |

## 계획된 의존 방향

Orchestrator는 도메인 모듈의 계약을 호출하며 상호작용 순서를 조정한다. 각 도메인 모듈은 자신의 State와 정책을 소유한다. Shared AI Services는 요청 전달 인프라를 제공하고, 프롬프트와 결과 해석은 호출한 도메인 모듈이 담당한다. 게임 고유 사실의 소유자는 호스트 게임이다.

## 구현 착수 후 갱신 항목

소스 파일이 추가되면 계획을 실제 코드에 근거한 색인으로 바꾸고 다음을 기록한다.

- 플러그인 설명자와 모듈 이름
- Public·Private 소스 디렉터리
- 주요 타입과 State 소유권
- 런타임 의존성과 Unreal Subsystem·수명주기 선택
- 테스트 또는 샘플 프로젝트 위치

## 관련 문서

- [[../00_Overview|아키텍처 개요]]
- [[../02_Contracts/00_Index|계약 색인]]
- [[../../03_Development/01_Current-Work|현재 작업]]
