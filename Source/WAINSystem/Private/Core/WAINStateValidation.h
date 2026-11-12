/**
 * @file WAINStateValidation.h
 * @brief 초기 및 준비된 WAIN Snapshot의 공통 불변조건을 검사한다.
 */

#pragma once

#include "CoreMinimal.h"
#include "WAINRuntimeTypes.h"

namespace WAINStateValidation
{
    bool Validate(const FWAINStateSnapshot &Snapshot, FString &OutError);
}
