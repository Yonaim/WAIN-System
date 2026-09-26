# 코드 주석 작성 기준

상태: **초안** · 제안서는 특정 주석 형식을 요구하지 않는다. 실제 Unreal 플러그인 코드의 API와 저장소 관례가 정해지기 전까지는 아래 원칙을 적용한다.

## 기본 원칙

- 주석은 코드만으로 알기 어려운 의도, 계약, 제약을 설명한다.
- 구현 내용을 줄마다 다시 말하거나 이름을 문장으로 반복하지 않는다.
- 상태의 소유권, 갱신 시점, LLM에 전달되는 정보처럼 모듈 사이에서 지켜야 할 계약은 필요한 경우 문서와 코드 주석에 함께 남긴다.
- API 문서 형식은 Unreal Engine과 실제 코드베이스의 관례에 맞춘다. 현재는 Doxygen 형식을 일괄 강제하지 않는다.
- 코드나 계약이 바뀌면 주석도 함께 확인하고, 근거가 없는 TODO는 두지 않는다.

```cpp
// Reflection and the resulting plan are retained for later interactions.
StoreExperience(Reflection);
StoreExperience(Plan);
```

위 코드는 제안서의 Memory → Reflection → Planning 흐름을 설명하는 예시다. 실제 함수 선언이나 저장 순서를 뜻하지 않는다.

코드 명명 및 Unreal 모듈 관련 기준은 [[01_Project-Policy/0002_Code_Style|Unreal Engine C++ 코드 작성 기준]]을 따른다.
