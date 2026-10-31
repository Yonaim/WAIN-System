# 개발 개요

## 현재 기준

이 볼트는 프로젝트 설계 문서와 2026-10-28 중간발표 자료에 기록된 WAIN System 설계를 정리한다. 확인한 작업 폴더에는 WAINSystem 플러그인 설명자나 Source 디렉터리가 없어 런타임 구현을 확인할 수 없었다. 따라서 이 문서는 구현 완료 보고가 아니라 설계 기준이다.

## 프로토타입 목표

비주얼 노벨용 Single-Agent, Turn-Based 상호작용 파이프라인을 만든다. 캐릭터·월드·서사 State는 LLM 밖에서 영속적으로 관리한다. 생성된 대사와 변경 사항은 검증 전까지 제안으로 다룬다.

단계별 계획은 다음과 같다.

1. A: LLM 단독 기준선
2. B: Character Agent와 영속 캐릭터 State 추가
3. C: World State Manager와 World Context 추가
4. D: Narrative Manager와 Beat 진행 추가

이는 평가 단계 제안이며 완료된 마일스톤을 의미하지 않는다.

## 초기 프로토타입 범위

- 플레이어 상호작용을 한 번에 하나씩 처리
- 캐릭터 응답과 State 변경 제안 생성
- 명시적인 검증과 State Commit
- 영속 Memory 검색, Reflection, Goal 기반 Planning
- 필요한 World Context와 제작된 Narrative Beat 진행
- 호스트 게임 Adapter 경계. 첫 구현은 내부 State를 사용할 수 있음

## 초기 범위에서 제외

- 동시 자율 Agent
- 독립적인 실시간 World Event
- 동시 상호작용 Commit 및 Revision 기반 OCC
- 대규모 Narrative Graph의 동적 Façade 탐색
- 상용 게임 전체 통합

## 개발 문서 작성 원칙

- 확정 결정과 미결정 질문을 구분한다.
- 소스 근거가 없으면 아키텍처를 구현 완료로 표시하지 않는다.
- 실제 구현 상황이 바뀌면 Current Work를 갱신한다.
- 결정과 근거, 상태를 Current Choices에 기록한다.
- 새 설계·구현·실험·버그·검증 계획에는 Project Policy의 Template을 사용한다.

## 관련 문서

- [[01_Current-Work|현재 작업]]
- [[02_Current-Choices|현재 결정]]
- [[../02_Architecture/00_Overview|아키텍처 개요]]
- [[../04_Experiments/00_Overview|평가 개요]]
