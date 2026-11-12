# 현재 작업

## 확인 시점

설계 기준은 문서화되어 있다. 제공된 작업 폴더 스냅샷에서 WAINSystem 플러그인 설명자나 플러그인 Source 디렉터리를 확인하지 못해 구현 진행 상황은 파일로 검증할 수 없다.

## 구현 시작 시 진행 순서 제안

1. Unreal 플러그인 설명자와 모듈 골격 생성 또는 복구
2. 버전이 지정된 요청·결과 자료형과 오류 결과 정의
3. 순차 처리하는 Interaction Orchestrator 구현
4. 결정론적 검증과 내부 World State Store 구현
5. 실제 Provider 연동 전 Mock을 지원하는 서비스 인터페이스 추가
6. 최초 평가 시나리오를 만들고 기준선 결과 기록

## 구현 시 해결할 의존 사항

- Unreal Engine 버전과 플러그인 모듈 구성 확인
- Provider·Model과 인증 정보 설정 방식 선택
- 영속 State Schema와 Save·Load 소유권 정의
- 비동기 Callback과 UObject·Game Thread 수명주기 결정
- Affection, Trust, Attachment, Emotion의 초기 숫자 정책 정의
- 재시도, 시간 초과, 사용자용 실패 처리 정의

## 상태 갱신 원칙

이 페이지는 현황 기록이지 작업 추적표가 아니다. 저장소 근거가 달라지면 갱신한다. 완료로 표시할 때는 코드나 Commit을 연결한다. 제안서나 일정표만으로 완료를 추정하지 않는다.

## 관련 문서

- [[02_Architecture/00_Overview|개발 개요]]
- [[02_Current-Choices|현재 결정]]
- [[03_Source/00_Index|소스 구조 색인]]
