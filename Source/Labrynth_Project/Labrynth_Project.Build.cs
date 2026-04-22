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
<<<<<<< Updated upstream
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
=======
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
			"OnlineSubsystem",
			"OnlineSubsystemUtils"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
<<<<<<< Updated upstream
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
	}
}
