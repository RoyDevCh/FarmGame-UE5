using UnrealBuildTool;
using System.Collections.Generic;

public class FarmGameUETarget : TargetRules
{
	public FarmGameUETarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("FarmGameUE");
	}
}
