// Copyright (c) 2026 AvaloNero. Licensed under the MIT License.

using UnrealBuildTool;

public class BMFontExample : ModuleRules
{
	public BMFontExample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivateDependencyModuleNames.AddRange(new[] {
			"Core", "CoreUObject", "Engine", "Slate", "SlateCore", "UMG",
			"UnrealBMFont", "Json", "RHI", "RenderCore"
		});
	}
}
