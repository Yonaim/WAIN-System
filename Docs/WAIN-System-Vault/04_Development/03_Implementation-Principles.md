# WAIN Implementation Principles — Top-Down Readability

> **우선순위 1: Top-Down Readability.** 이 문서는 최신 [Common Code Style](../00_Engineering-Standards/00_Core/00_Code-Style.md)을 변경하지 않고, WAIN Runtime에 적용할 프로젝트 전용 판단 기준을 정의한다. 설계 가이드이며 현재 C++ 구현 완료를 의미하지 않는다.

## 00. 상위 함수는 실행 단계의 목차

- Interaction의 최상위 흐름에서 **무엇을 어떤 순서로 하는지** 이해할 수 있어야 한다. 핵심 단계의 이름·실패 분기·Commit 시점이 한눈에 보여야 한다.
- 세부 알고리즘, JSON 역직렬화, Snapshot 복사·Mutation 계산은 적절한 하위 함수에 배치한다. 단순히 한 줄을 감추기 위한 래퍼는 만들지 않는다.
- **모든 단계가 하나의 함수로 구현되어야 하는 것은 아니다.** 실제 책임과 모듈 API에 맞춰 분리하고, 상위 흐름의 독해가 끊기지 않도록 한다.
- 같은 추상화 수준의 작업을 나란히 놓고, 입력 검증·정책 판단·상태 변경·출력 같은 서로 다른 수준의 세부 구현을 무분별하게 섞지 않는다.

## 10. Transaction 구현 예시 (의사코드)

다음은 메서드 이름의 계약 예시가 아니라 **읽기 순서에 관한 의사코드**다. 실제 Unreal C++ 선언이나 반환 타입은 구현 시 검증·확정한다.

```cpp
FInteractionResult ProcessTurn(const FInteractionRequest& Request)
{
    const auto Context = PrepareContext(Request);
    if (!Context)
    {
        return MakeContextError();
    }

    const auto Proposal = GenerateProposal(*Context);
    if (!Proposal)
    {
        return MakeProposalError();
    }

    const auto CommitResult = CommitInteraction(*Proposal, *Context);
    if (!CommitResult)
    {
        return MakeTransactionError(CommitResult);
    }

    NotifyStateCommitted(*CommitResult);
    return BuildVNResponse(*Proposal, *CommitResult);
}
```

- **비동기 LLM**을 실제로 붙일 때는 위 동기식 의사코드를 그대로 복사하지 않고, `SubmitTurn`/비동기 완료 시점의 상태 전이와 Game Thread Commit을 분리한다.
- `CommitInteraction`은 승인된 Proposal에 대해 **Response Validation → Next State Preparation → Narrative Evaluation → Final Validation → Atomic Commit** 순서를 갖는다. 구체 계약은 [Interaction Transaction](../02_Architecture/01_Contracts/05_Response-Validation-Commit.md)과 [ADR-003](../03_Decisions/03_ADR-003-Commit-Before-Response.md)을 따른다.
- **서사 Constraint와 Expression 허용 목록 검증을 생략하지 않는다.** 최종 검증 전 WAIN/Host 권위 상태를 수정해서는 안 된다.

## 20. 파일과 함수 구성

1. Public Runtime API에서 의도가 드러나는 `SubmitTurn`, `GetStateSnapshot` 등 진입점을 제공한다. 실제 계약 확정 전 예시 타입을 그대로 사용하지 않는다.
2. Orchestrator/Coordinator는 **단계 조율**에 집중하고 Memory·Narrative·Snapshot 내부 로직을 한 파일에 쌓아두지 않는다.
3. 조건 분기가 복잡하면 조기 반환과 명시적 실패값을 사용한다. **Revision 충돌/Prepare 실패/Commit 성공**을 숨기지 않는다.
4. 선언 → 유효성 확인 → 계산 → 상태 변경·반환처럼 *논리적 의미가 다른 그룹* 사이에 빈 줄을 둔다. 한 줄마다 빈 줄을 넣거나 모든 분기에 Region을 강제하지 않는다.
5. Region과 번호 섹션은 **규모가 큰 파일에서 실제 탐색 가치가 있을 때만** 사용한다. 함수 추출·책임 분리가 필요한 코드를 접기 마커로 숨기지 않는다.
6. WAIN이 관리하는 C++ 소스/헤더의 최상단에는 프로젝트 합의에 따라 한국어 `@file`, `@brief`로 파일의 단일 책임을 설명한다. Doxygen 본문은 Why/불변조건/소유권/스레드 경계에 집중한다.

## 30. Snapshot과 부작용 경계

- Current Snapshot은 검증 중 수정하지 않고 Prepared Snapshot에서 변경 후보를 구성한다.
- MVP Testbed에서 **실제 지원·검증한 Game Adapter**에만 공유 Commit 경계를 허용한다. 두 `Apply()`를 순서대로 호출하는 방식은 원자적 Commit으로 주장하지 않는다.
- State Commit 후 VN 표시 실패는 State Rollback을 뜻하지 않는다. Display/사운드/뷰 갱신은 Commit 이후로 분리한다.
- 플러그인에 호스트 게임 고유 인벤토리 구현이나 특정 캐릭터 Asset 이름을 하드코딩하지 않는다.

## 40. 검증 및 문서화

- **코드 리뷰 우선 질문:** 진입점의 호출 순서만 읽어도 처리 흐름과 Commit 경계를 설명할 수 있는가?
- 도입 전 검증: Plugin Load, TurnId 중복, Revision 충돌, Prepared State 폐기, WAIN+Game 동시 관찰, Commit 후 UI 실패.
- [Pre-Build Formatting](../00_Engineering-Standards/03_Workflow/03_Pre-Build-Formatting.md)은 실제 UBT 빌드 타깃에 연결할 때까지 **미설치/미검증**으로 기록한다. Live Coding 경로는 별도 검증한다.
- 코드/API/불변조건 변경 시 [Contract Register](../02_Architecture/01_Contracts/00_Contract-Register.md), [Source Map](../02_Architecture/02_Source/00_Source-Map.md), [Test Plan](../05_Experiments/00_Test-Plan.md)을 확인하고 [Work Log](01_Work-Log.md)를 갱신한다. 새로운 아키텍처 선택이나 기존 합의 변경은 ADR에 남긴다.
