using UnrealBuildTool;

public class Labrynth_ProjectTarget : TargetRules
{
	public Labrynth_ProjectTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Labrynth_Project");
	}
}
