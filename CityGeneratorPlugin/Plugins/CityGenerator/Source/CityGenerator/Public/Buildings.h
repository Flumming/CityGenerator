#pragma once

#include "CoreMinimal.h"

struct BuildingFootprint
{
	TArray<FVector2D> vertices;
	float height;              
};

class BuildingGenerator {
public:
	BuildingGenerator() {}
	TArray<BuildingFootprint> GenerateBuildings(const TArray<FVector2D> cityBlock);

private:
	BuildingFootprint CreateBuilding(FVector2D start, FVector2D dir, float width, TArray<bool> corner = {false, false}, TArray<FVector2D> cornerDir = {});
	FVector2D  CalculateCornerPoint(bool isFirstCorner, FVector2D cornerDir, TArray<FVector2D> building, TArray<FVector2D> insidePoints);


	FVector2D CalculateSafeStart(const BuildingFootprint& building, const FVector2D& nextVert, const FVector2D& nextDirection);
	void AddBuilding(BuildingFootprint building, TArray<BuildingFootprint>& buildings, const TArray<FVector2D> cityBlock);

	float minWidth = 750.0f;
	float maxWidth = 1500.0f;

	float minDepth = 1000.0f;
	float maxDepth = 2000.0f;

	float minHeight = 3000.0f;
	float maxHeight = 6000.0f;
	
	float minGap = 150.0f;
	float maxGap = 300.0f;
};