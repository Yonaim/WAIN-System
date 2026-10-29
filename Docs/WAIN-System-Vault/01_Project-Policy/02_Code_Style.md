# C++ 코드 스타일

> **한 줄 요약:** Unreal Engine C++ 명명 관례를 따르고, 형식 규칙은 저장소의 formatter 설정을 기준으로 한다.

상태: **초안** · 마지막 확인: 2026-10-29

## 적용 기준

| 항목 | 기준 |
|---|---|
| 언어 | Unreal Engine 5 C++ |
| 자동 형식 | 저장소에 formatter 설정이 있으면 그 설정을 우선한다. 별도 설정이 없으면 팀에서 합의한 formatter 설정을 추가한다. |
| 프로젝트 관례 | Unreal Engine 타입 접두사와 이 문서의 파일·API 규칙을 따른다. |
| 우선순위 | 엔진 API 관례와 저장소 설정을 우선하고, 프로젝트 자체 규칙은 이 문서에 기록한다. |

## 명명 규칙

| 대상 | 규칙 | 예 |
|---|---|---|
| UObject 파생 클래스 | `U` + PascalCase | `UWAINStateManager` |
| AActor 파생 클래스 | `A` + PascalCase | `AWAINCharacterAgent` |
| SWidget 파생 클래스 | `S` + PascalCase | `SWAINDialoguePanel` |
| 인터페이스 | `I` + PascalCase | `IWAINStateProvider` |
| 구조체 | `F` + PascalCase | `FWAINCharacterState` |
| 열거형 | `E` + PascalCase | `EWAINAgentState` |
| 템플릿 타입 | `T` + PascalCase | `TArray`, `TMap` |
| 함수·메서드·변수 | PascalCase | `BuildContext()`, `NarrativeState` |
| bool 변수 | `b` + PascalCase | `bIsInitialized` |
| 상수 | PascalCase 또는 프로젝트 내 일관된 `k` 접두사 규칙 | `MaxRetryCount` |
| 약어 | 널리 쓰이는 약어는 대문자 유지 | `LLM`, `API`, `JSON`, `HTTP` |

`UCLASS`, `USTRUCT`, `UENUM`, `UINTERFACE` 등 리플렉션 타입은 Unreal의 접두사와 매크로 관례를 따른다. 엔진·표준 라이브러리 타입의 이름은 변경하지 않는다. 로컬 변수와 매개변수는 의미가 드러나는 이름을 사용한다.

## 파일과 헤더 구성

| 항목 | 규칙 |
|---|---|
| C++ 파일명 | 주된 클래스 또는 모듈 책임을 나타내는 이름을 사용하고 `.h`, `.cpp` 확장자를 사용한다. |
| Header guard | `#pragma once`를 사용한다. |
| 대응 헤더 | `.cpp`에서는 대응하는 헤더를 첫 번째로 include한다. |
| Include 최소화 | 완전한 타입 정의가 필요하지 않으면 전방 선언을 고려한다. 불필요한 순환 의존을 만들지 않는다. |
| 모듈 경계 | 외부에 공개할 타입과 내부 구현을 Unreal 모듈의 Public/Private 디렉터리로 구분한다. |

## 함수와 API

- 읽기 전용 매개변수와 메서드는 `const`로 의도를 표시한다.
- 큰 객체는 필요에 따라 `const T&`로 전달하고, 소유권 이전은 값 전달 또는 이동으로 명확히 한다.
- 반환값을 무시하면 오류가 될 수 있는 함수에는 `[[nodiscard]]` 또는 Unreal에서 권장하는 동등한 표기를 고려한다.
- 실패할 수 있는 초기화·할당 작업에 무조건 `noexcept`를 붙이지 않는다.
- 기본값이 유효한 멤버는 선언 위치에서 초기화한다.
- UObject 수명 관리와 참조는 Unreal의 GC 및 `UPROPERTY` 규칙을 따른다. UObject를 일반 스마트 포인터로 임의 소유하지 않는다.
- 공개 API는 필요한 범위만 노출하고, 내부 구현은 모듈의 Private 영역에 둔다.
- `auto`는 우변만으로 타입이 명확하거나 반복 표기를 줄일 때 사용한다. 일반적인 상태·수치 변수는 의도가 읽히도록 타입을 드러낸다.

## 코드 구성과 주석 구분선

구현 파일의 함수는 기능 또는 책임 단위로 모은다. 파일이 커져 탐색이 어려워지면 편집기 접기 region을 사용할 수 있지만, region은 클래스나 파일의 책임 분리를 대신하지 않는다. 책임이 너무 많아지면 타입이나 파일을 나눌지 검토한다.

```cpp
#pragma region Lifecycle

void UWAINStateManager::Initialize()
{
}

void UWAINStateManager::Shutdown()
{
}

#pragma endregion
```

긴 영역 안에 하위 묶음이 실제로 필요할 때만 구분선 주석을 추가한다. 함수마다 구분선을 붙이거나 바깥 영역 제목을 반복하지 않는다. 헤더 선언 순서와 구현 정의 순서는 대응시켜 찾기 쉽게 유지한다.

## 오류와 로그

- 오류를 조용히 무시하지 않는다. 호출자가 복구할 수 있도록 실패 원인과 관련 맥락을 전달한다.
- Unreal 로그 카테고리를 사용하고, 메시지에 기능·요청·에이전트 식별 정보 등 필요한 맥락을 포함한다.
- 매 프레임이나 고빈도 경로에서 반복 로그를 남기지 않는다.
- 외부 API 응답과 JSON 입력은 경계에서 검증하고, 민감한 프롬프트·토큰·개인정보를 로그에 기록하지 않는다.

## 형식 규칙

저장소 formatter 설정이 아직 확인되지 않았다. 설정이 추가되면 아래 표 대신 실제 설정을 정본으로 연결한다. 형식 기준은 합의 없이 파일별로 다르게 적용하지 않는다.

| 항목 | 기준 |
|---|---|
| 들여쓰기 | 저장소 formatter 설정을 따르며 Tab/공백을 혼용하지 않는다. |
| 중괄호 | 프로젝트 전체에서 일관되게 적용한다. |
| 줄 길이 | 읽기 쉬운 범위에서 일관되게 줄바꿈한다. |
| Include | formatter 및 모듈 의존성 규칙을 따른다. |

## 적용과 확인

C++ 변경 전후에 저장소에서 지정한 formatter와 Unreal 빌드 명령을 사용한다. formatter는 명명, UObject 수명, API 계약, 비동기 동작이나 알고리즘 정확성을 검증하지 않으므로 관련 검토와 테스트를 별도로 수행한다.
