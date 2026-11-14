/**
 * @file WAINSystem.Build.cs
 * @brief WAIN 런타임 플러그인의 UnrealBuildTool 모듈 의존성을 정의한다.
 */

using UnrealBuildTool;

public class WAINSystem : ModuleRules
{
    public WAINSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "HTTP"
        });
    }
}
