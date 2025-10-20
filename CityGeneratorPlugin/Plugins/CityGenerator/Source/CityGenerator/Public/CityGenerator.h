// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "Roads.h"
#include "Buildings.h"

class FCityGeneratorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void OnWorldStarted(UWorld* World, const UWorld::InitializationValues IVS);
	FDelegateHandle WorldInitHandle;

private:
	UClass* ActorClass;
	UClass* ActorClass2;
	UStaticMesh* CylinderMesh;
};

class CityGenerator {
public:
	CityGenerator() : World(nullptr) {}

	void SetWorld(UWorld* InWorld) { World = InWorld; }

	void GenerateCity(float size);

private:
	UWorld* World;
	RoadGraph roadGraph;
	BuildingGenerator buildingGenerator;

	void GenerateRoads(float size);
	void GenerateSidewalks();
	void CreateProceduralMesh(const TArray<FVector>& Vertices,
							  const TArray<int32>& Triangles,
							  UMaterialInterface* Material,
							  const TArray<FVector2D>& InUVs,
							  const FColor InVertexColor = FColor::White,
							  bool bEnableCollision = true);

	void GenerateBuildings();

	//List of all meshes
	TArray<AActor*> meshes;
	TArray<TArray<FVector2D>> cityBlocks;

	TArray<BuildingFootprint> buildings;

	float smallRoadChange;
};