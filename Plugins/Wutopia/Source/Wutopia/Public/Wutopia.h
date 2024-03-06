// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once
#include "Interfaces/IPluginManager.h"
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"


class FWutopiaModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
