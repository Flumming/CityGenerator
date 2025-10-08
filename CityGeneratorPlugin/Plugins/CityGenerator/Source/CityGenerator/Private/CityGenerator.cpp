// Copyright Epic Games, Inc. All Rights Reserved.

#include "CityGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "ProceduralMeshComponent.h"
#include "CompGeom/PolygonTriangulation.h"
#include "IndexTypes.h"

#define LOCTEXT_NAMESPACE "FCityGeneratorModule"

void FCityGeneratorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FSoftObjectPath ActorRef(TEXT("/CityGenerator/Blueprints/Node.Node_C"));
	ActorClass = Cast<UClass>(ActorRef.TryLoad());
	FSoftObjectPath ActorRef2(TEXT("/CityGenerator/Blueprints/Node1.Node1_C"));
	ActorClass2 = Cast<UClass>(ActorRef2.TryLoad());

	FSoftObjectPath MeshRef(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	CylinderMesh = Cast<UStaticMesh>(MeshRef.TryLoad());

	WorldInitHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FCityGeneratorModule::OnWorldStarted);
}

void FCityGeneratorModule::OnWorldStarted(UWorld* World, const UWorld::InitializationValues IVS) {
	if (!World->IsGameWorld()) {
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("World started"));

	//CityGenerator generator(World);
	//generator.GenerateCity(50000.0f);	
}

void FCityGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void CityGenerator::GenerateCity(float size) {
	// Clear previous road meshes
	for (AActor* actor : roadMeshes) {
		if (actor) {
			actor->Destroy();
		}
	}
	roadMeshes.Empty();
	GenerateRoads(size);
}

void CityGenerator::GenerateRoads(float size) {
	roadGraph.size = size;

	TArray<FVector2d> startpositions = {FVector2d(0.0f, 0.0f)};
	roadGraph.GenerateRoad(startpositions, 3.0f);

	TArray<TArray<FVector>> intersectionRoads;
	TArray<TArray<FVector>> intersections;
	float smallRoadChange = 125.0f;

	for (int i = 0; i < roadGraph.outNodes.Num(); i++) {

		auto node = roadGraph.outNodes[i];
		FVector position(node->position, 0.0f);

		// Calculate intersection points
		TArray<FVector> interVertices;
		TArray<Neighbor> sortedNeighbors = node->neighbors;

		sortedNeighbors.Sort([position](const Neighbor& A, const Neighbor& B) {
			FVector dirA = (FVector(A.node->position, 0.0f) - position).GetSafeNormal();
			FVector dirB = (FVector(B.node->position, 0.0f) - position).GetSafeNormal();

			float angleA = FMath::Atan2(dirA.Y, dirA.X);
			float angleB = FMath::Atan2(dirB.Y, dirB.X);

			return angleA < angleB;
							 });

		int numNeighbors = sortedNeighbors.Num();

		for (int j = 0; j < numNeighbors; j++) {
			auto* prevNeighbor = sortedNeighbors[(j - 1 + numNeighbors) % numNeighbors].node;
			auto* neighbor = sortedNeighbors[j].node;

			float currRoadWidth = 400.0f - (static_cast<int>(sortedNeighbors[j].roadType) * smallRoadChange);
			float currIntersectionSize = currRoadWidth * 6.0f;
			float prevRoadWidth = 400.0f - (static_cast<int>(sortedNeighbors[(j - 1 + numNeighbors) % numNeighbors].roadType) * smallRoadChange);
			float prevIntersectionSize = prevRoadWidth * 6.0f;

			FVector nPosition(neighbor->position, 0.0f);
			FVector pnPosition(prevNeighbor->position, 0.0f);

			FVector nDirection = (nPosition - position).GetSafeNormal();
			FVector nRight = nDirection.RotateAngleAxis(90.f, FVector(0.f, 0.f, 1.f)).GetSafeNormal();

			FVector pnDirection = (pnPosition - position).GetSafeNormal();
			FVector pnRight = pnDirection.RotateAngleAxis(90.f, FVector(0.f, 0.f, 1.f)).GetSafeNormal();

			FVector sideL = (position + nDirection * currIntersectionSize) - (nRight * currRoadWidth);
			FVector sideR = (position + nDirection * currIntersectionSize) + (nRight * currRoadWidth);

			FVector pnSideR = (position + pnDirection * prevIntersectionSize) + (pnRight * prevRoadWidth);

			FVector nEnd = sideL - nDirection * (currIntersectionSize * 2.0f);
			FVector pnEnd = pnSideR - pnDirection * (prevIntersectionSize * 2.0f);

			FVector intersection;
			if (nDirection.Equals(pnDirection, 0.01f) || nDirection.Equals(-pnDirection, 0.01f)) {
				// Parallel lines, use average point
				intersection = (sideL + pnSideR) * 0.5f;
				interVertices.Add(intersection);
			}
			else {
				if (FMath::SegmentIntersection2D(sideL, nEnd, pnSideR, pnEnd, intersection)) {
					interVertices.Add(intersection);
				}
			}

			interVertices.Add(sideL);
			interVertices.Add(sideR);
		}


		//for (int j = 0; j < interVertices.Num(); j++) {
		//	interVertices[j] = FVector(interVertices[j].X, interVertices[j].Y, intersectionHeight);
		//}

		// Split into 4 squares and middle of intersection
		int interDegree = interVertices.Num() / 3;

		for (int j = 0; j < interDegree; j++) {
			TArray<FVector> currentRoad;
			currentRoad.Add(interVertices[j * 3 + 1]);
			currentRoad.Add(interVertices[j * 3 + 2]);
			currentRoad.Add(interVertices[((j * 3 + 3) % interVertices.Num())]);
			currentRoad.Add(interVertices[j * 3 + 0]);
			intersectionRoads.Add(currentRoad);
		}

		TArray<FVector> intersectionMiddle;
		for (int j = 0; j < interDegree; j++) {
			intersectionMiddle.Add(interVertices[j * 3]);
		}
		intersections.Add(intersectionMiddle);

		// Create roads
		for (Neighbor neighbor : node->neighbors) {

			float roadWidth = 400.0f - (static_cast<int>(neighbor.roadType) * smallRoadChange);
			float textureScale = roadWidth * 2.0f;
			float intersectionSize = roadWidth * 6.0f;

			Node* neighborNode = neighbor.node;
			if (node->id < neighborNode->id) continue; // Avoid double drawing

			FVector start = position;
			FVector end(neighborNode->position.X, neighborNode->position.Y, 0.0f);
			FVector midpoint = (start + end) * 0.5f;

			FVector direction = end - start;
			FVector right = direction.RotateAngleAxis(90.f, FVector(0.f, 0.f, 1.f)).GetSafeNormal();

			float length = direction.Size();

			direction = direction.GetSafeNormal();
			//FRotator rotation = direction.Rotation() + FRotator(90.f, 0.f, 0.f);
			FVector offset = direction * intersectionSize;
			length -= offset.Size() * 2.0f;

			TArray<FVector> vertices;
			vertices.Add(start + offset + right * roadWidth);
			vertices.Add(start + offset - right * roadWidth);
			vertices.Add(end - offset - right * roadWidth);
			vertices.Add(end - offset + right * roadWidth);

			// Triangles
			TArray<int32> triangles = {
				0, 2, 1,
				0, 3, 2
			};

			TArray<FVector2D> uvs;
			uvs.Add(FVector2D(0, 0));
			uvs.Add(FVector2D(0, 1));
			uvs.Add(FVector2D(length / textureScale, 1));
			uvs.Add(FVector2D(length / textureScale, 0));
			UMaterialInterface* RoadMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_Road.M_Road"));
			CreateProceduralMesh(vertices, triangles, RoadMat, uvs);
		}
	}
	
	// Create intersections
	for (auto intersectionRoad : intersectionRoads) {
		TArray<int32> triangles = {0, 2, 1, 0, 3, 2};
		UMaterialInterface* RoadMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_Road.M_Road"));
		float length1 = (intersectionRoad[0] - intersectionRoad[3]).Size(); 
		float length2 = (intersectionRoad[1] - intersectionRoad[2]).Size();

		TArray<FVector2D> uvs;
		uvs.Add(FVector2D(0, 0)); 
		uvs.Add(FVector2D(0, 1)); 
		uvs.Add(FVector2D(length2 / 400.0f, 1)); 
		uvs.Add(FVector2D(length1 / 400.0f, 0));

		CreateProceduralMesh(intersectionRoad, triangles, RoadMat, uvs);
	}
	// Create intersection middles
	for (auto intersection : intersections) {
		TArray<int32> triangles = {0, 2, 1, 0, 3, 2};
		UMaterialInterface* AsphaltMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_Asphalt.M_Asphalt"));
		
		TArray<FVector2D> uvs;
		for (int j = 0; j < intersection.Num(); j++) {
			uvs.Add(FVector2D(intersection[j].X / 400.0f, intersection[j].Y / 400.0f));
		}
		CreateProceduralMesh(intersection, triangles, AsphaltMat, uvs);
	}

	TArray<TArray<FVector>> faceVertices;
	
	for (auto face : roadGraph.outFaces) {

		TArray<FVector> vertices;
		for (int i = 0; i < face.Num(); i++) {
			Node* prev = face[(i - 1 + face.Num()) % face.Num()];
			Node* curr = face[i];
			Node* next = face[(i + 1) % face.Num()];

			Neighbor* prevNeighbor = nullptr;
			for (Neighbor& n : curr->neighbors) {
				if (n.node == prev) {
					prevNeighbor = &n;
					break;
				}
			}

			Neighbor* nextNeighbor = nullptr;
			for (Neighbor& n : curr->neighbors) {
				if (n.node == next) {
					nextNeighbor = &n;
					break;
				}
			}

			if (!prevNeighbor || !nextNeighbor) {
				continue; // Should not happen
			}

			float prevRoadWidth = 400.0f - (static_cast<int>(prevNeighbor->roadType) * smallRoadChange);
			float prevIntersectionSize = prevRoadWidth * 6.0f;

			float nextRoadWidth = 400.0f - (static_cast<int>(nextNeighbor->roadType) * smallRoadChange);
			float nextIntersectionSize = nextRoadWidth * 6.0f;

			FVector2D dirToPrev = (prev->position - curr->position).GetSafeNormal();
			FVector2D dirToNext = (next->position - curr->position).GetSafeNormal();

			FVector2D leftDirToPrev = FVector2D(dirToPrev.Y, -dirToPrev.X);
			FVector2D rightDirToNext = FVector2D(-dirToNext.Y, dirToNext.X);

			FVector2D prevInside = (curr->position + dirToPrev * prevIntersectionSize) + (leftDirToPrev * prevRoadWidth);
			FVector2D nextInside = (curr->position + dirToNext * nextIntersectionSize) + (rightDirToNext * nextRoadWidth);

			FVector2D prevEnd = prevInside - dirToPrev * (prevIntersectionSize * 2.0f);
			FVector2D nextEnd = nextInside - dirToNext * (nextIntersectionSize * 2.0f);

			FVector intersection;
			if (dirToPrev.Equals(dirToNext, 0.01f) || dirToPrev.Equals(-dirToNext, 0.01f)) {
				// Parallel lines, use average point
				intersection = FVector(((prevInside + nextInside) * 0.5f), 5.0f);
				vertices.Add(intersection);
			}
			else {
				if (FMath::SegmentIntersection2D(FVector(prevInside, 0.0f), FVector(prevEnd, 0.0f), FVector(nextInside, 0.0f), FVector(nextEnd, 0.0f), intersection)) {
					vertices.Add(FVector(intersection.X, intersection.Y, 0.0f));
				}
			}
		}

		faceVertices.Add(vertices);

		/*
		TArray<UE::Geometry::FIndex3i> triangles3i;
		PolygonTriangulation::TriangulateSimplePolygon(vertices, triangles3i);
		TArray<int32> triangles;
		for (auto tri : triangles3i) {
			triangles.Add(tri.A);
			triangles.Add(tri.B);
			triangles.Add(tri.C);
		}

		UMaterialInterface* VertexColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_VertexColor.M_VertexColor"));
		FColor FaceColor = FColor::MakeRandomColor();
		TArray<FVector2D> uvs;
		for (int j = 0; j < vertices.Num(); j++) {
			uvs.Add(FVector2D(vertices[j].X / 400.0f, vertices[j].Y / 400.0f));
		}
		CreateProceduralMesh(vertices, triangles, VertexColorMat, uvs, FaceColor, true);
		*/
	}

	// Create sidewalks

	TArray<TArray<FVector>> newFaces;

	for (auto face : faceVertices) {
		TArray<TArray<FVector>> sidewalks;
		int numVertices = face.Num();
		float sidewalkWidth = 200.0f;

		TArray<FVector> newFace;

		for (int i = 0; i < numVertices; i++) {
			TArray<FVector> sidewalk;

			FVector prev = face[(i - 1 + numVertices) % numVertices];
			FVector curr = face[i];
			FVector next = face[(i + 1) % numVertices];
			FVector nextNext = face[(i + 2) % numVertices];

			FVector currPrevDir = (prev - curr).GetSafeNormal();
			FVector currNextDir = (next - curr).GetSafeNormal();
			FVector nextCurrDir = -currNextDir;
			FVector nextNextDir = (nextNext - next).GetSafeNormal();
			
			sidewalk.Add(curr);
			sidewalk.Add(next);
			
			//FVector currInward = (currPrevDir + currNextDir).GetSafeNormal();
			//FVector nextInward = (nextCurrDir + nextNextDir).GetSafeNormal();

			FVector corner1;
			{
				FVector currToNextNext = (nextNext - curr).GetSafeNormal();
			
				FVector prevDir = nextCurrDir;
				FVector nextDir = nextNextDir;

				FVector prevDirRotated = FVector(prevDir.Y, -prevDir.X, 0.0f);
				FVector nextDirRotated = FVector(-nextDir.Y, nextDir.X, 0.0f);

				FVector prevPoint = next + prevDirRotated * sidewalkWidth;
				FVector nextPoint = next + nextDirRotated * sidewalkWidth;

				if (FVector::CrossProduct(currToNextNext, next - curr).Z < 0) {					
					//convex corner
					FVector prevEndPoint = prevPoint + prevDir * 1000.0f;
					FVector nextEndPoint = nextPoint + nextDir * 1000.0f;
					FVector intersection;
					if (FMath::SegmentIntersection2D(prevPoint, prevEndPoint, nextPoint, nextEndPoint, intersection)) {
						corner1 = intersection;
					}
					else {
						corner1 = next + nextDirRotated * sidewalkWidth;
					}
					
				}
				else {
					FVector prevEndPoint = prevPoint - prevDir * 1000.0f;
					FVector nextEndPoint = nextPoint - nextDir * 1000.0f;
					FVector intersection;
					if (FMath::SegmentIntersection2D(prevPoint, prevEndPoint, nextPoint, nextEndPoint, intersection)) {
						corner1 = intersection;
					}
					else {
						corner1 = next + nextDirRotated * sidewalkWidth;
					}
				}
			}

			FVector corner2;
			{
				/*
				FVector prevToNext = (next - prev).GetSafeNormal();
				FVector prevDir = currPrevDir;
				FVector nextDir = currNextDir;

				// Rotate prevDir counter clockwise 90 degrees
				FVector prevDirRotated = FVector(-prevDir.Y, prevDir.X, 0.0f);
				// Rotate nextDir clockwise 90 degrees
				FVector nextDirRotated = FVector(nextDir.Y, -nextDir.X, 0.0f);

				FVector prevPoint = curr + prevDirRotated * sidewalkWidth;
				FVector nextPoint = curr + nextDirRotated * sidewalkWidth;

				if (FVector::CrossProduct(prevToNext, curr - prev).Z < 0) {
					//convex corner
					//Check intersection between line from prevPoint in direction of prevDir and nextPoint in direction of nextDir
					FVector prevEndPoint = prevPoint + prevDir * 1000.0f;
					FVector nextEndPoint = nextPoint + nextDir * 1000.0f;
					FVector intersection;
					if (FMath::SegmentIntersection2D(prevPoint, prevEndPoint, nextPoint, nextEndPoint, intersection)) {
						corner1 = intersection;
					}
					else {
						//corner1 = next + nextInward * sidewalkWidth;
					}
				}
				else {
					//concave corner
					FVector prevEndPoint = prevPoint - prevDir * 1000.0f;
					FVector nextEndPoint = nextPoint - nextDir * 1000.0f;
					FVector intersection;
					if (FMath::SegmentIntersection2D(prevPoint, prevEndPoint, nextPoint, nextEndPoint, intersection)) {
						corner2 = intersection;
					}
					else {
						//corner1 = next + nextInward * sidewalkWidth;
					}
				}
				*/
				FVector prevToNext = (next - prev).GetSafeNormal();

				FVector prevDir = currPrevDir;
				FVector nextDir = currNextDir;

				FVector prevDirRotated = FVector(prevDir.Y, -prevDir.X, 0.0f);
				FVector nextDirRotated = FVector(-nextDir.Y, nextDir.X, 0.0f);

				FVector prevPoint = curr + prevDirRotated * sidewalkWidth;
				FVector nextPoint = curr + nextDirRotated * sidewalkWidth;

				if (FVector::CrossProduct(prevToNext, curr - prev).Z < 0) {
					//convex corner
					FVector prevEndPoint = prevPoint + prevDir * 1000.0f;
					FVector nextEndPoint = nextPoint + nextDir * 1000.0f;
					FVector intersection;
					if (FMath::SegmentIntersection2D(prevPoint, prevEndPoint, nextPoint, nextEndPoint, intersection)) {
						corner2 = intersection;
					}
					else {
						corner2 = curr + nextDirRotated * sidewalkWidth;
					}
				}
				else {
					//concave corner
					FVector prevEndPoint = prevPoint - prevDir * 1000.0f;
					FVector nextEndPoint = nextPoint - nextDir * 1000.0f;
					FVector intersection;
					if (FMath::SegmentIntersection2D(prevPoint, prevEndPoint, nextPoint, nextEndPoint, intersection)) {
						corner2 = intersection;
					}
					else {
						corner2 = curr + nextDirRotated * sidewalkWidth;
					}					
				}
			}
			

			sidewalk.Add(corner1);
			sidewalk.Add(corner2);

			sidewalks.Add(sidewalk);

			newFace.Add(corner1);
		}

		for (auto sidewalk : sidewalks) {
			TArray<int32> triangles = {0, 2, 1, 0, 3, 2};
			UMaterialInterface* sideWalkMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_Sidewalk.M_Sidewalk"));
			TArray<FVector2D> uvs;

			for (int j = 0; j < sidewalk.Num(); j++) {
				uvs.Add(FVector2D(sidewalk[j].X / 50.0f, sidewalk[j].Y / 50.0f));
				sidewalk[j] = FVector(sidewalk[j].X, sidewalk[j].Y, 10.0f);
			}
			CreateProceduralMesh(sidewalk, triangles, sideWalkMat, uvs);

			TArray<FVector> sidewalkSide;
			sidewalkSide.Add(sidewalk[1]);
			sidewalkSide.Add(sidewalk[0]);
			sidewalkSide.Add(FVector(sidewalk[0].X, sidewalk[0].Y, 0.0f));
			sidewalkSide.Add(FVector(sidewalk[1].X, sidewalk[1].Y, 0.0f));
			TArray<int32> sideTriangles = {0, 2, 1, 0, 3, 2};
			TArray<FVector2D> sideUVs;
			float segmentLength = ((sidewalkSide[1] - sidewalkSide[0]).Size()) / 25.0f;
			float height = (sidewalkSide[1].Z - sidewalkSide[2].Z) / 25.0f;

			sideUVs.Add(FVector2D(0, height));
			sideUVs.Add(FVector2D(segmentLength, height));
			sideUVs.Add(FVector2D(segmentLength, 0));
			sideUVs.Add(FVector2D(0, 0));          
			
				
			CreateProceduralMesh(sidewalkSide, sideTriangles, sideWalkMat, sideUVs);
		}
		newFaces.Add(newFace);
	}

	
	for (auto face : newFaces) {
		TArray<UE::Geometry::FIndex3i> triangles3i;
		PolygonTriangulation::TriangulateSimplePolygon(face, triangles3i);
		TArray<int32> triangles;
		for (auto tri : triangles3i) {
			triangles.Add(tri.A);
			triangles.Add(tri.B);
			triangles.Add(tri.C);
		}

		UMaterialInterface* VertexColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_VertexColor.M_VertexColor"));
		FColor FaceColor = FColor::MakeRandomColor();
		TArray<FVector2D> uvs;
		for (int j = 0; j < face.Num(); j++) {
			uvs.Add(FVector2D(face[j].X / 400.0f, face[j].Y / 400.0f));
			face[j] = FVector(face[j].X, face[j].Y, 10.0f);
		}
		CreateProceduralMesh(face, triangles, VertexColorMat, uvs, FaceColor, true);
	}
	
}

void CityGenerator::CreateProceduralMesh(
	const TArray<FVector>& Vertices,
	const TArray<int32>& Triangles,
	UMaterialInterface* Material,
	const TArray<FVector2D>& InUVs,
	const FColor InVertexColor /* white */,
	bool bEnableCollision /* true */) {
	if (!World) return;

	FActorSpawnParameters SpawnParams;
	AActor* PolygonActor = World->SpawnActor<AActor>(
		AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!PolygonActor) return;

	UProceduralMeshComponent* ProcMesh = NewObject<UProceduralMeshComponent>(PolygonActor);
	ProcMesh->RegisterComponent();
	ProcMesh->AttachToComponent(PolygonActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	// Buffers
	TArray<FVector> Normals;
	TArray<FProcMeshTangent> Tangents;
	TArray<FColor> VertexColors;
	FVector Normal = FVector::UpVector;

	for (int32 j = 0; j < Vertices.Num(); j++) {
		Normals.Add(Normal);
		Tangents.Add(FProcMeshTangent(1, 0, 0));
		VertexColors.Add(InVertexColor);
	}

	// Fallbacks if user didn’t supply UVs / colors
	TArray<FVector2D> UVs = InUVs;
	if (UVs.Num() != Vertices.Num()) {
		UVs.SetNum(Vertices.Num());
	}

	// Build mesh
	ProcMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, bEnableCollision);

	if (bEnableCollision) {
		ProcMesh->ContainsPhysicsTriMeshData(true);
	}

	if (Material) {
		ProcMesh->SetMaterial(0, Material);
	}

	roadMeshes.Add(PolygonActor);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCityGeneratorModule, CityGenerator)