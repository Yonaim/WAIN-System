# Requirements — UR/FR/NFR

> 기준: 2026-10-28 중간발표 Appendix(UR 5개, FR 12개, NFR 7개). **표의 상태는 현재 제공 스냅샷에서 확인한 구현 기준**으로 기록한다. 단계 설계와 실제 런타임을 혼동하지 않는다.

## User Requirements

| ID | 요구 |
|---|---|
| UR-01 | 정해진 선택지 없이 자연어로 캐릭터와 상호작용 |
| UR-02 | 경험과 관계를 기억하고 일관된 캐릭터 유지 |
| UR-03 | 플레이어 행동으로 변화한 세계를 이후 턴에 반영 |
| UR-04 | 자유 입력 중에도 서사 및 정보 공개 제약 유지 |
| UR-05 | 다른 비주얼 노벨 콘텐츠에도 재사용·검증 가능 |

## Functional Requirements

| ID | 요구 | 책임 구성 요소 | 구현 확인 |
|---|---|---|---|
| FR-01 | 자연어 Player Input 처리 | Orchestrator | Plugin 미구현; Testbed 입력 Mock만 존재 |
| FR-02 | Character Persona 유지 | Character Agent | 미구현 |
| FR-03 | Memory 저장·검색 | Memory | 미구현 |
| FR-04 | 과거 경험의 Reflection | Character Agent | 미구현 |
| FR-05 | Context·경험 기반 Planning | Character Agent | 미구현 |
| FR-06 | Relationship / Emotion State | Character Agent | 미구현; Testbed 초기값 정의 JSON 준비 |
| FR-07 | World State 조회·갱신 | World State Manager | 미구현 |
| FR-08 | 현재 서사에 따른 Goal/Constraint 결정 | Narrative Manager | 미구현 |
| FR-09 | 대사·행동 동적 생성 | LLM | 미구현; Testbed 규칙 Mock만 존재 |
| FR-10 | Dialogue/Action/Expression/Event를 VN 출력 | Plugin↔Testbed 계약 | 텍스트 출력 일부만 Mock; 표정 렌더링 미구현 |
| FR-11 | 상태 변화의 다음 턴 지속 반영 | State Manager | WAIN 미구현; Testbed Trust/Beat Mock 변경만 존재 |
| FR-12 | Schema/게임 규칙/서사 제약 검증 후 승인 변경만 적용 | Validator/State Manager | 미구현 |

## Non-Functional Requirements

| ID | 품질 항목 | 검증 방법 | 상태 |
|---|---|---|---|
| NFR-01 | 모듈 독립성 | Character/World/Narrative 책임 분리 확인 | 설계 |
| NFR-02 | 상태 일관성 | 승인된 변경만 후속 턴에 반영 | 미검증 |
| NFR-03 | 확장성 | 게임 콘텐츠/Adapter 교체 시 핵심 Plugin 재사용 | 설계 |
| NFR-04 | 테스트 가능성 | 동일 시나리오 A~D 모듈별 비교 | 계획 |
| NFR-05 | 관측 가능성 | 입력·Context·검증·Commit 로그 | 계획 |
| NFR-06 | 안정성 | 오류/잘못된 출력 시 State 보존 | 계획 |
| NFR-07 | 응답 성능 | 지연 기준은 초기 측정 후 결정 | 미정 |

자세한 수용 기준은 [Test Plan](../05_Experiments/00_Test-Plan.md)을 따른다. 본 문서는 최초 요구 기준이며 신규 **Emotion/Expression Asset ID Validation**은 FR-06·FR-10·FR-12의 세부 계약으로 연결한다.
