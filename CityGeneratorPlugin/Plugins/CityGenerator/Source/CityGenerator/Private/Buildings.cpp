#include "Buildings.h"

TArray<BuildingFootprint> BuildingGenerator::GenerateBuildings(const TArray<FVector2D> cityBlock, float height){
	TArray<BuildingFootprint> buildings;
	
	FVector2D position = cityBlock[0];
	FVector2D nextVert = cityBlock[1];
	//distance
	float distance = (nextVert - position).Size();
	FVector2D direction = (nextVert - position).GetSafeNormal();

	TArray<FVector2D> cornerDir;
	TArray<bool> isCorner = {false, false};
	
	int lastIndex = cityBlock.Num() - 1;
	FVector2D prevVert = cityBlock[lastIndex];
	// To prev
	FVector2D prevDirection = (prevVert - position).GetSafeNormal();
	cornerDir.Add(prevDirection);

	buildings.Add(CreateBuilding(position, direction, isCorner, cornerDir));
	buildings.Add(CreateBuilding(position + direction * (buildingSize + 200.0f), direction, isCorner, cornerDir));

	return buildings;
}

BuildingFootprint BuildingGenerator::CreateBuilding(FVector2D start, FVector2D dir, TArray<bool> isCorner, TArray<FVector2D> cornerDir) {
	BuildingFootprint building;
	FVector2D rightDir(-dir.Y, dir.X);
	building.vertices.Add(start);
	building.vertices.Add(start + dir * buildingSize);
	if (!isCorner[0] && !isCorner[1]) {
		building.vertices.Add(building.vertices[1] + rightDir * buildingSize);
		building.vertices.Add(building.vertices[0] + rightDir * buildingSize);
	}


	/*
	if (isCorner[1]) {
		FVector2D cornerPoint = building.vertices[1] + cornerDir[1] * buildingSize;
		building.vertices.Add(cornerPoint);
	}
	else {
		building.vertices.Add(building.vertices[1] + rightDir * buildingSize);
	}

	if (isCorner[0]) {
		FVector2D cornerPoint = building.vertices[0] + cornerDir[0] * buildingSize;
		building.vertices.Add(cornerPoint);
	}
	else {
		building.vertices.Add(building.vertices[0] + rightDir * buildingSize);
	}
	*/
	return building;
}