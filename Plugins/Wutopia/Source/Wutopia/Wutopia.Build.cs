// Some copyright should be here...

using UnrealBuildTool;
using System.IO;

public class Wutopia : ModuleRules
{
	public Wutopia(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// "Wutopia/Public",
				// "../ThirdParty"
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"Wutopia/Private"
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "CoreUObject", "Engine", "InputCore", "Sockets", "Networking", "Messaging", "RenderCore", "RHI","WebBrowserWidget"//, Path.Combine(ModuleDirectory, "ThirdParty/include")
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"InputCore",
				"Json",
				"JsonUtilities",
				"Networking",
				"Sockets",
				"WebBrowserWidget",
				"Projects"
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        // add libraries
        string DllDir = Path.Combine(ModuleDirectory, "ThirdParty/bin");
        // Dlls
        foreach (string file in Directory.GetFiles(DllDir))
        {
            RuntimeDependencies.Add(file);
            // delay load
            string filename = Path.GetFileName(file);
            PublicDelayLoadDLLs.Add(filename);
        }
    }
}
