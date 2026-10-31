# Shared AI Services

> 상태: 서비스 경계 설계안. Provider, Unreal 수명주기, 구체 API는 미정이다.

## 목적

Shared AI Services는 LLM과 Embedding 요청에 공통으로 필요한 인프라를 제공한다. 기능 모듈은 프롬프트, 도메인 해석, 요청이 필요한 시점을 계속 소유한다.

## 개념 인터페이스

설계에서 제안한 논리 인터페이스는 다음과 같다.

- ILLMService: 비동기 텍스트 또는 구조화 생성 요청
- IEmbeddingService: Memory 검색을 위한 비동기 Embedding 생성

이 이름은 개념을 설명하기 위한 것이며 확정된 C++ 선언이 아니다.

## 책임

- Provider별 요청·응답 세부사항을 감춘다.
- 비동기 완료 결과와 구분 가능한 오류를 제공한다.
- Provider가 제공하는 경우 사용량, 지연 시간, 비용을 기록한다.
- 평가를 위한 결정론적 Mock 또는 대체 구현을 지원한다.
- 인증 정보와 Provider 설정을 기능 프롬프트 및 도메인 State와 분리한다.

## 소유권 경계

Character Agent와 Narrative Manager는 각자의 프롬프트 구성 및 결과 해석을 담당한다. Memory는 검색 정책과 유사도 순위를 소유한다. Shared AI Services에 서사 규칙, 캐릭터 정책, 프로젝트 프롬프트 로직을 넣지 않는다.

## 실패와 취소

호출자는 성공, Provider 오류, 시간 초과, 취소, 형식 오류를 구분할 수 있어야 한다. 비동기 결과로 UObject State에 접근하기 전 적절한 Unreal 실행 문맥으로 전달해야 한다. 수명주기, 취소, 재시도, Game Thread 규칙은 구현 전에 결정해야 한다.

## 관측 정보

가능한 경우 요청 지연, Token·사용량, Provider·Model, 오류 유형을 기록한다. 기본적으로 비밀 정보나 전체 사용자 입력을 로그에 남기지 않는다. 운영 로그를 활성화하기 전 보존 기간과 비식별 처리 방식을 정한다.

## 미결정 사항

- Provider와 Model 설정
- 재시도, 시간 초과, 취소, 요청 제한 정책
- UObject, Subsystem 등 Unreal 수명주기
- Mock 인터페이스와 평가 도구 연동
- 프롬프트 및 Memory 로그의 개인정보 처리·보존·비식별 정책

## 관련 문서

- [[00_Index|계약 색인]]
- [[01_Character-Agent|Character Agent]]
- [[02_Memory-Reflection-Planning|Memory, Reflection, Planning]]
