# Interaction 흐름 색인

> WAIN의 기본 실행 단위는 한 번의 Player Input을 끝까지 처리하는 Turn-Based Interaction이다.

| 단계 | 문서 | 결과 |
|---|---|---|
| Player Input부터 VN 출력까지 | [[01_Interaction-Pipeline|Interaction Pipeline]] | 검증된 Proposal을 Commit한 뒤 확정 결과를 VN에 전달 |
| State 참조·Context 구성 | [[../02_Contracts/01_Character-Agent|Character Agent]] · [[../02_Contracts/02_Memory-Reflection-Planning|Memory]] · [[../02_Contracts/03_World-State-Manager|World State]] · [[../02_Contracts/04_Narrative-Manager|Narrative]] | 현재 턴에 필요한 Context |
| 검증·서사 판정·확정 | [[../02_Contracts/05_Response-Validation-and-Commit|Validation and Commit]] | 적용 가능한 변경과 다음 Active Beat |
| 구현 파일 연결 | [[../03_Source/00_Index|Source Index]] | 현재 구현 여부와 실제 코드 위치 |

이 흐름은 설계 기준이다. 각 단계의 실행 여부와 테스트 결과는 Source 및 Experiments에서 확인한다.
