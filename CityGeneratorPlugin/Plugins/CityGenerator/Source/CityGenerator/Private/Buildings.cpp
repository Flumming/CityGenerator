#include "Buildings.h"
#include "GeomTools.h"

TArray<BuildingFootprint> BuildingGenerator::GenerateBuildings(const TArray<FVector2D> cityBlock){
	TArray<BuildingFootprint> buildings;
	
	int numVerts = cityBlock.Num();
	if (numVerts < 3) {
		return buildings;
	}

	float smallOffset = 0.1f;

	for (int i = 0; i < numVerts; i++) {
		FVector2D prevVert = cityBlock[(i - 1 + numVerts) % numVerts];
		FVector2D position = cityBlock[i];
		FVector2D nextVert = cityBlock[(i + 1) % numVerts];
		FVector2D nextNextVert = cityBlock[(i + 2) % numVerts];

		float distance = (nextVert - position).Size();
		//Maybe skip when distance it too small, or only do one building
		FVector2D direction = (nextVert - position).GetSafeNormal();

		TArray<FVector2D> cornerDir;
		// To prev
		FVector2D prevDirection = (prevVert - position).GetSafeNormal();
		cornerDir.Add(prevDirection);
		// To next
		FVector2D nextDirection = (nextNextVert - nextVert).GetSafeNormal();
		cornerDir.Add(nextDirection);

		while (distance > maxWidth) {
			float buildingWidth = FMath::FRandRange(minWidth, maxWidth);

			TArray<bool> isCorner = {false, false};
			if (position == cityBlock[i]) {
				isCorner[0] = true;
				float offset = buildingWidth + smallOffset;
				position += direction * offset;
				distance -= offset;
				continue;
			}

			BuildingFootprint building = CreateBuilding(position, direction, buildingWidth, isCorner, cornerDir);
			if (building.vertices.Num() > 0) {
				AddBuilding(building, buildings, cityBlock);
			}

			float gapSize;
			if (FMath::FRand() < 0.1f) {
				gapSize = FMath::FRandRange(minGap, maxGap);
			}
			else {
				gapSize = 0.0f;
			}
			float offset = buildingWidth + gapSize + smallOffset;
			position += direction * offset;
			distance -= offset;
		}
		if (distance > minWidth) {
			// create last building
			TArray<bool> isCorner = {false, true};
			BuildingFootprint building = CreateBuilding(position, direction, distance, isCorner, cornerDir);
			if (building.vertices.Num() > 0) {
				AddBuilding(building, buildings, cityBlock);
			}
		}
	}
	return buildings;
}

BuildingFootprint BuildingGenerator::CreateBuilding(FVector2D start, FVector2D dir, float width, TArray<bool> isCorner, TArray<FVector2D> cornerDir) {
	BuildingFootprint building;
	FVector2D rightDir(-dir.Y, dir.X);
	building.vertices.Add(start);
	building.vertices.Add(start + dir * width);
	FVector2D inside1, inside2;

	float buildingDepth = FMath::FRandRange(minDepth, maxDepth);

	inside1 = building.vertices[1] + rightDir * buildingDepth;
	inside2 = building.vertices[0] + rightDir * buildingDepth;

	if (isCorner[0]) {
		inside2 = CalculateCornerPoint(true, cornerDir[0], building.vertices, {inside1, inside2});
	}
	if (isCorner[1]) {
		inside1 = CalculateCornerPoint(false, cornerDir[1], building.vertices, {inside1, inside2});
	}

	if (inside1.IsNearlyZero() || inside2.IsNearlyZero()) {
		return {};
	}

	TArray<FVector2D> insidePoints;
	insidePoints.Add(inside1);
	insidePoints.Add(inside2);
	building.vertices.Append(insidePoints);

	return building;
}

FVector2D BuildingGenerator::CalculateCornerPoint(bool isFirstCorner, FVector2D cornerDir, TArray<FVector2D> building, TArray<FVector2D> insidePoints) {
	FVector cornerPoint;
	FVector2D p1start;
	FVector2D p1end;
	FVector2D p2start;
	FVector2D p2end;
	//TArray<FVector2D> outPoints;

	float maxSizeCheck = 2000.0f;

	if (isFirstCorner) {
		p1start = building[0];
		p1end = p1start + cornerDir * maxSizeCheck;
		p2start = insidePoints[0];
		p2end = insidePoints[0] + (insidePoints[1] - insidePoints[0]).GetSafeNormal() * maxSizeCheck;
	}
	else {
		p1start = building[1];
		p1end = p1start + cornerDir * maxSizeCheck;
		p2start = insidePoints[1];
		p2end = insidePoints[1] + (insidePoints[0] - insidePoints[1]).GetSafeNormal() * maxSizeCheck;
	}
	bool bIntersect = FMath::SegmentIntersection2D(
		FVector(p1start, 0.0f),
		FVector(p1end, 0.0f),
		FVector(p2start, 0.0f),
		FVector(p2end, 0.0f),
		cornerPoint
	);

	if (!bIntersect) {
		return {};
	}

	return FVector2D(cornerPoint.X, cornerPoint.Y);
}

bool DoPolygonsOverlap(const TArray<FVector2D>& PolyA, const TArray<FVector2D>& PolyB) {
	int NumA = PolyA.Num();
	int NumB = PolyB.Num();

	// 1️⃣ Check if any edges intersect
	for (int i = 0; i < NumA; i++) {
		FVector2D A1 = PolyA[i];
		FVector2D A2 = PolyA[(i + 1) % NumA];

		for (int j = 0; j < NumB; j++) {
			FVector2D B1 = PolyB[j];
			FVector2D B2 = PolyB[(j + 1) % NumB];

			FVector intersection;
			if (FMath::SegmentIntersection2D(FVector(A1, 0.0f),  FVector(A2, 0.0f),  FVector(B1, 0.0f),  FVector(B2, 0.0f),  intersection)) {
				return true;
			}
		}
	}

	// 2️⃣ Check if one polygon is completely inside the other
	if (PolyA.Num() > 0 && PolyB.Num() > 0) {
		if (FGeomTools2D::IsPointInPolygon(PolyA[0], PolyB) || FGeomTools2D::IsPointInPolygon(PolyB[0], PolyA)) {
			return true;
		}
	}

	return false;
}

void BuildingGenerator::AddBuilding(BuildingFootprint building, TArray<BuildingFootprint>& buildings, const TArray<FVector2D> cityBlock) {
	bool addBuilding = true;

	for (auto& existingBuilding : buildings) {
		if (DoPolygonsOverlap(building.vertices, existingBuilding.vertices)) {
			addBuilding = false;
			break; 
		}
	}

	if (!FGeomTools2D::IsPointInPolygon(building.vertices[2], cityBlock)) {
		addBuilding = false;
	}

	if (addBuilding) {
		building.height = FMath::FRandRange(minHeight, maxHeight);
		buildings.Add(building);
	}
}


/*
FVector2D BuildingGenerator::CalculateSafeStart(
	const BuildingFootprint& building,
	const FVector2D& nextVert,
	const FVector2D& nextDirection) {
	if (building.vertices.Num() < 4) {
		// Not enough vertices to compute — return zero vector
		return FVector2D::ZeroVector;
	}

	// Distance from nextVert to building vertex[2]
	float distTo2 = (building.vertices[2] - nextVert).Size();
	float distTo3 = -FLT_MAX;

	FVector projPoint = FVector::ZeroVector;
	// Rotate nextDirection 90° counterclockwise
	FVector rotatedNextDir(-nextDirection.Y, nextDirection.X, 0.0f);

	// Define the line from building vertex[3] in the rotated direction

	FVector p1start = FVector(nextVert, 0.0f);
	FVector p1end = FVector(nextVert + nextDirection * 10000.0f, 0.0f);
	FVector p2start = FVector(building.vertices[3], 0.0f);
	FVector p2end = FVector(building.vertices[3], 0.0f) + rotatedNextDir * 10000.0f;

	// Try to find intersection
	bool intersect = FMath::SegmentIntersection2D(p1start, p1end, p2start, p2end, projPoint);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *p1start.ToString());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *p1end.ToString());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *p2start.ToString());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *p2end.ToString());
	FVector p3 = FVector((building.vertices[3] + nextDirection * 10000.0f), 0.0f);

	UE_LOG(LogTemp, Warning, TEXT("%s"), *p2start.ToString());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *p3.ToString());


	UE_LOG(LogTemp, Warning, TEXT("Proj Point: %s"), *projPoint.ToString());

	// Compute distance to projected intersection point if valid
	if (intersect) {
		distTo3 = (FVector2D(projPoint.X, projPoint.Y) - nextVert).Size();
	}

	// Choose whichever is farther away
	if (distTo2 > distTo3) {
		return building.vertices[2];
	}
	else {
		return FVector2D(projPoint.X, projPoint.Y);
	}
}
*/