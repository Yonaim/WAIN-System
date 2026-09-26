# 커밋 메시지 작성 규칙

커밋 메시지는 변경 내용을 짧고 일관되게 찾을 수 있도록 아래 형식을 따른다.

## 형식

```text
<Type>: <한국어 설명>
```

- `Type`은 아래 표에 있는 키워드만 사용한다. 임의의 키워드는 추가하지 않는다.
- 키워드는 첫 글자만 대문자로 쓴다. 예: `Feat`, `Docs`, `Chore`.
- 콜론 뒤 설명은 한국어로 작성한다. 클래스명, 파일명, API, 외부 라이브러리처럼 번역하면 의미가 달라지는 기술 명칭은 원래 표기를 유지한다.
- 설명은 변경 사항을 구체적이고 간결하게 나타내며, 끝에 마침표를 붙이지 않는다.
- 제목은 한 줄로 작성한다. 추가 맥락이 꼭 필요할 때만 빈 줄 뒤에 본문을 쓴다.

## 허용 키워드

| 키워드 | 사용 목적 | 예시 |
|---|---|---|
| `Feat` | WAIN 기능 또는 플러그인 기능 추가 | `Feat: Character Memory 검색 연결` |
| `Fix` | 잘못된 동작이나 결함 수정 | `Fix: Narrative Constraint 누락 처리` |
| `Docs` | 문서 추가·수정 | `Docs: Single-Agent 평가 시나리오 정리` |
| `Style` | 동작 변경 없는 코드 형식·표기 정리 | `Style: Character Agent 코드 형식 정리` |
| `Refactor` | 외부 동작을 바꾸지 않는 코드 구조 개선 | `Refactor: World State 갱신 책임 분리` |
| `Perf` | 측정 또는 명확한 근거가 있는 성능 개선 | `Perf: Surface Buffer 업로드 비용 절감` |
| `Test` | 테스트 또는 테스트 데이터 변경 | `Test: Memory 회수 일관성 사례 추가` |
| `Build` | 빌드 시스템·컴파일 설정 변경 | `Build: WAIN Unreal plugin module 설정` |
| `CI` | 지속적 통합 및 자동화 파이프라인 변경 | `CI: Pull Request에서 WAIN 테스트 실행` |
| `Chore` | 기능·문서·테스트 변경에 속하지 않는 유지보수 | `Chore: Third-party 의존성 갱신` |
| `Revert` | 이전 커밋 되돌리기 | `Revert: Narrative Goal 표시 추가` |

한 커밋에 여러 종류의 변경이 섞이면 핵심 변경을 기준으로 키워드 하나를 고른다. 서로 독립적인 변경은 별도 커밋으로 나눈다.

## 예시

```text
Feat: Relationship State 갱신 기능 추가
Docs: 주차별 프로토타입 목표 정리
Refactor: Narrative State 처리 책임 분리
```

새 커밋은 허용 키워드와 위 형식을 사용한다.
