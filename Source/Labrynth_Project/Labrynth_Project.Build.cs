using UnrealBuildTool;

public class Labrynth_Project : ModuleRules
{
	public Labrynth_Project(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
<<<<<<< Updated upstream
			"UMG",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Sockets",
			"Networking"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});
=======
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
>>>>>>> Stashed changes
	}
}
