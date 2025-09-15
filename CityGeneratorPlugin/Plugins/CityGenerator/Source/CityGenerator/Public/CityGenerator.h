// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Roads.h"

class FCityGeneratorModule : public IModuleInterface
{
public:

	RoadGraph roadGraph;

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnWorldStarted(UWorld* World, const UWorld::InitializationValues IVS);
	FDelegateHandle WorldInitHandle;

private:
	UClass* ActorClass;
	UClass* ActorClass2;
	UStaticMesh* CylinderMesh;
	void SetupRoad();
};
