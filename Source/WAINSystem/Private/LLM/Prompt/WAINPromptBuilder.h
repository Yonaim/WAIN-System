/**
 * @file WAINPromptBuilder.h
 * @brief 읽기 전용 WAIN Context와 캐릭터 지침을 Provider 독립 프롬프트로 구성한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINInterfaces.h"

// ============================================================
// 00. Provider-Independent Prompt Construction
// ============================================================
#pragma region 00_Provider_Independent_Prompt_Construction
    // Prompt는 상태를 변경하지 않으며, 외부 입력을 권위 있는 명령으로 취급하지 않는다.

    class FWAINPromptBuilder final
    {
        public:
        static FString BuildSystemInstruction();
        static FString BuildUserPrompt(const FWAINInteractionRequest &Request, const FWAINCharacterContext &Context);
    };

#pragma endregion
