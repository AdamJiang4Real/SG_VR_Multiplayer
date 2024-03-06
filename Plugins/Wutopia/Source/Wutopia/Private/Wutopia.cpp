// Copyright Epic Games, Inc. All Rights Reserved.

#include "Wutopia.h"

#define LOCTEXT_NAMESPACE "FWutopiaModule"

//bool LoadRequiredLibraries()
//{
//	IFileManager& FileManager = IFileManager::Get();
//
//	FString DllPath = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("Wutopia"))->GetBaseDir(), TEXT("/Source/Wutopia/ThirdParty/bin"));
//	FPlatformProcess::PushDllDirectory(*DllPath);
//
//	TArray<FString> DllNames;
//	if (FileManager.DirectoryExists(*DllPath))
//	{
//		FileManager.FindFiles(DllNames, *DllPath);
//	}
//
//	for (auto& dllname : DllNames)
//	{
//		if (FPaths::GetExtension(dllname) != "dll")
//			continue;
//		// GetDllHandle()必须传入绝对路径，不然会加载失败！
//		FPlatformProcess::GetDllHandle(*FPaths::Combine(DllPath, dllname));
//	}
//
//	return true;
//}


void FWutopiaModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

 //   FString path = IPluginManager::Get().FindPlugin("Wutopia")->GetBaseDir();
 //   FString dllpath = path + "/ThirdParty/bin/viumsgsdk.dll";
 //   FPlatformProcess::GetDllHandle(*dllpath);
	//UE_LOG(LogTemp, Warning, TEXT("%s"),*dllpath);
}

void FWutopiaModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWutopiaModule, Wutopia)