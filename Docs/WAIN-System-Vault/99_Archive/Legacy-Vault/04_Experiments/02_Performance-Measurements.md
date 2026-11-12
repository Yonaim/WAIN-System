# 성능 측정

> 상태: 측정 계획. 제공된 자료에는 지연 시간이나 비용 측정값이 없다.

## 상호작용별 측정 항목

- 상호작용 전체 처리 시간
- LLM 요청 지연과 요청 횟수
- Embedding 요청 지연과 요청 횟수
- 검증 및 State Commit 시간
- Provider가 제공하는 입력·출력 사용량 및 예상 비용
- 시간 초과, 재시도, Provider 실패, Adapter 실패 비율

표본 수와 실행 설정을 함께 기록하고 중앙값과 꼬리 지연(p95 이상)을 보고한다. 필요하면 초기 실행과 안정 상태 실행을 구분한다.

## 측정 통제

Unreal 버전, Build 설정, Provider·Model, Prompt 버전, 하드웨어, 네트워크 조건, Memory Corpus 크기, Mock 또는 실제 서비스 사용 여부를 기록한다. 동일 입력을 반복하고 의미 검증을 포함한 실행과 제외한 실행을 구분한다.

## 목표 설정

응답 지연 목표는 아직 정하지 않았다. 먼저 기준선을 측정한 뒤 비주얼 노벨의 상호작용 경험과 Provider 변동성을 고려해 목표를 설정한다.

## 관련 문서

- [[02_Architecture/00_Overview|평가 개요]]
- [[05_Experiments/01_Validation-and-Tests|검증·테스트 계획]]
- [[02_Contracts/06_Shared-AI-Services|Shared AI Services]]
