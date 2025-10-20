#pragma once

#include "CoreMinimal.h"

struct BuildingFootprint
{
	TArray<FVector2D> vertices; // 2D vertices defining the building footprint
	float height;               // Height of the building
};

class BuildingGenerator {
public:
	BuildingGenerator() {}
	TArray<BuildingFootprint> GenerateBuildings(const TArray<FVector2D> cityBlock, float height);

private:
	BuildingFootprint CreateBuilding(FVector2D start, FVector2D dir, TArray<bool> corner = {false, false}, TArray<FVector2D> cornerDir = {});
	float minWidth = 300.0f;
	float maxWidth = 1000.0f;

	float buildingSize = 1000.0f;
};