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

	TArray<FVector2d> startpositions;
	roadGraph.GenerateRoad(startpositions, 3.0f);

	TArray<TArray<FVector>> intersectionRoads;
	TArray<TArray<FVector>> intersections;
	float roadWidth = 400.0f;
	float textureScale = roadWidth * 2.0f;
	float intersectionSize = roadWidth * 6.0f;

	for (int i = 0; i < roadGraph.outNodes.Num(); i++) {

		auto node = roadGraph.outNodes[i];
		FVector position(node->position, 0.0f);

		// Calculate intersection points
		TArray<FVector> interVertices;
		TArray<Node*> sortedNeighbors = node->neighbors;

		sortedNeighbors.Sort([position](const Node& A, const Node& B) {
			FVector dirA = (FVector(A.position, 0.0f) - position).GetSafeNormal();
			FVector dirB = (FVector(B.position, 0.0f) - position).GetSafeNormal();

			float angleA = FMath::Atan2(dirA.Y, dirA.X);
			float angleB = FMath::Atan2(dirB.Y, dirB.X);

			return angleA < angleB;
		});

		int numNeighbors = sortedNeighbors.Num();

		for (int j = 0; j < numNeighbors; j++) {
			auto* prevNeighbor = sortedNeighbors[(j - 1 + numNeighbors) % numNeighbors];
			auto* neighbor = sortedNeighbors[j];

			FVector nPosition(neighbor->position, 0.0f);
			FVector pnPosition(prevNeighbor->position, 0.0f);

			FVector nDirection = (nPosition - position).GetSafeNormal();
			FVector nRight = nDirection.RotateAngleAxis(90.f, FVector(0.f, 0.f, 1.f)).GetSafeNormal();

			FVector pnDirection = (pnPosition - position).GetSafeNormal();
			FVector pnRight = pnDirection.RotateAngleAxis(90.f, FVector(0.f, 0.f, 1.f)).GetSafeNormal();

			FVector sideL = (position + nDirection * intersectionSize) - (nRight * roadWidth);
			FVector sideR = (position + nDirection * intersectionSize) + (nRight * roadWidth);

			FVector pnSideR = (position + pnDirection * intersectionSize) + (pnRight * roadWidth);

			FVector nEnd = sideL - nDirection * (intersectionSize * 2.0f);
			FVector pnEnd = pnSideR - pnDirection * (intersectionSize * 2.0f);

			FVector intersection;
			if (nDirection.Equals(pnDirection, 0.01f) || nDirection.Equals(-pnDirection, 0.01f)) {
				// Parallel lines, use average point
				intersection = (sideL + pnSideR) * 0.5f;
				interVertices.Add(intersection);
			} else {
				if (FMath::SegmentIntersection2D(sideL, nEnd, pnSideR, pnEnd, intersection)) {
					interVertices.Add(intersection);
				}
			}

			interVertices.Add(sideL);
			interVertices.Add(sideR);
		}

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
		for (auto* neighbor : node->neighbors) {
			if (node->id < neighbor->id) continue; // Avoid double drawing

			FVector start = position;
			FVector end(neighbor->position.X, neighbor->position.Y, 0.0f);
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
			vertices.Add(end   - offset - right * roadWidth);
			vertices.Add(end   - offset + right * roadWidth);

			// Triangles
			TArray<int32> triangles = {
				0, 2, 1,
				0, 3, 2
			};


			// Normals, UVs, tangents, and vertex colors
			TArray<FVector> procNormals;
			TArray<FVector2D> uvs;
			TArray<FVector2D> uvs1;
			TArray<FProcMeshTangent> tangents;
			TArray<FColor> vertexColors;
			FVector normal = FVector::UpVector; // Flat polygon facing up

			uvs.Add(FVector2D(0, 0));
			uvs.Add(FVector2D(0, 1));
			uvs.Add(FVector2D(length / textureScale, 1));
			uvs.Add(FVector2D(length / textureScale, 0));

			for (int32 j = 0; j < vertices.Num(); j++) {
				procNormals.Add(normal);
				tangents.Add(FProcMeshTangent(1, 0, 0));         
			}

			for (int32 j = 0; j < vertices.Num(); j++) {
				FVector localOffset = vertices[j] - midpoint;

				// red channel (width)
				float widthCoord = FVector::DotProduct(localOffset, right) / (2 * roadWidth);
				widthCoord += 0.5f; 
				uint8 red = static_cast<uint8>(FMath::Clamp(widthCoord, 0.f, 1.f) * 255.f);

				// green channel (length)
				float lengthCoord = FVector::DotProduct(localOffset, direction) / (length);
				lengthCoord += 0.5f; 
				uint8 green = static_cast<uint8>(FMath::Clamp(lengthCoord, 0.f, 1.f) * 255.f);

				vertexColors.Add(FColor(red, green, 0, 255));
			}

			// Spawn a dummy actor to hold the procedural mesh
			FActorSpawnParameters SpawnParams;
			AActor* PolygonActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
			if (!PolygonActor) continue;

			// Add procedural mesh component
			UProceduralMeshComponent* ProcMesh = NewObject<UProceduralMeshComponent>(PolygonActor);
			ProcMesh->RegisterComponent();
			ProcMesh->AttachToComponent(PolygonActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			ProcMesh->CreateMeshSection(0, vertices, triangles, procNormals, uvs, vertexColors, tangents, true);

			// Optional: enable collision
			ProcMesh->ContainsPhysicsTriMeshData(true);

			// Set material
			UMaterialInterface* RoadMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_Road.M_Road"));
			if (RoadMat) {
				ProcMesh->SetMaterial(0, RoadMat);
			}
		}
	}

	// Create intersections
	for (auto intersectionRoad : intersectionRoads) {
		float length1 = (intersectionRoad[0] - intersectionRoad[3]).Size();
		float length2 = (intersectionRoad[1] - intersectionRoad[2]).Size();

		//get longest length
		float length = FMath::Min(length1, length2);

		TArray<FVector> vertices = intersectionRoad;
		TArray<int32> triangles = {
			0, 2, 1,
			0, 3, 2
		};

		// Normals, UVs, tangents, and vertex colors
		TArray<FVector> procNormals;
		TArray<FVector2D> uvs;
		TArray<FVector2D> uvs1;
		TArray<FProcMeshTangent> tangents;
		TArray<FColor> vertexColors;
		FVector normal = FVector::UpVector; // Flat polygon facing up

		uvs.Add(FVector2D(0, 0));
		uvs.Add(FVector2D(0, 1));
		uvs.Add(FVector2D(length2 / textureScale, 1));
		uvs.Add(FVector2D(length1 / textureScale, 0));

		for (int32 j = 0; j < vertices.Num(); j++) {
			procNormals.Add(normal);
			tangents.Add(FProcMeshTangent(1, 0, 0));
		}

		// Spawn a dummy actor to hold the procedural mesh
		FActorSpawnParameters SpawnParams;
		AActor* PolygonActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!PolygonActor) continue;

		// Add procedural mesh component
		UProceduralMeshComponent* ProcMesh = NewObject<UProceduralMeshComponent>(PolygonActor);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(PolygonActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->CreateMeshSection(0, vertices, triangles, procNormals, uvs, vertexColors, tangents, true);

		// Optional: enable collision
		ProcMesh->ContainsPhysicsTriMeshData(true);

		// Set material
		UMaterialInterface* RoadMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_Road.M_Road"));
		if (RoadMat) {
			ProcMesh->SetMaterial(0, RoadMat);
		}
	}
	for (auto intersection : intersections) {
		TArray<FVector> vertices = intersection;
		TArray<int32> triangles = {
			0, 2, 1,
			0, 3, 2
		};

		// Normals, UVs, tangents, and vertex colors
		TArray<FVector> procNormals;
		TArray<FVector2D> uvs;
		TArray<FVector2D> uvs1;
		TArray<FProcMeshTangent> tangents;
		TArray<FColor> vertexColors;
		FVector normal = FVector::UpVector; // Flat polygon facing up

		for (int32 j = 0; j < vertices.Num(); j++) {
			uvs.Add(FVector2D(vertices[j].X / textureScale, vertices[j].Y / textureScale));
			procNormals.Add(normal);
			tangents.Add(FProcMeshTangent(1, 0, 0));
		}

		// Spawn a dummy actor to hold the procedural mesh
		FActorSpawnParameters SpawnParams;
		AActor* PolygonActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!PolygonActor) continue;

		// Add procedural mesh component
		UProceduralMeshComponent* ProcMesh = NewObject<UProceduralMeshComponent>(PolygonActor);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(PolygonActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->CreateMeshSection(0, vertices, triangles, procNormals, uvs, vertexColors, tangents, true);

		// Optional: enable collision
		ProcMesh->ContainsPhysicsTriMeshData(true);

		// Set material
		UMaterialInterface* RoadMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_Asphalt.M_Asphalt"));
		if (RoadMat) {
			ProcMesh->SetMaterial(0, RoadMat);
		}
	}

	for (auto face : roadGraph.outFaces) {
		
		TArray<FVector> vertices;
		for (int i = 0; i < face.Num(); i++) {
			Node* prev = face[(i - 1 + face.Num()) % face.Num()];
			Node* curr = face[i];
			Node* next = face[(i + 1) % face.Num()];

			FVector2D dirToPrev = (prev->position - curr->position).GetSafeNormal();
			FVector2D dirToNext = (next->position - curr->position).GetSafeNormal();

			FVector2D leftDirToPrev		= FVector2D(dirToPrev.Y, -dirToPrev.X);
			FVector2D rightDirToNext	= FVector2D(-dirToNext.Y, dirToNext.X);

			FVector2D prevInside = (curr->position + dirToPrev * intersectionSize) + (leftDirToPrev * roadWidth);
			FVector2D nextInside = (curr->position + dirToNext * intersectionSize) + (rightDirToNext * roadWidth);

			FVector2D prevEnd = prevInside - dirToPrev * (intersectionSize * 2.0f);
			FVector2D nextEnd = nextInside - dirToNext * (intersectionSize * 2.0f);

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

		TArray<UE::Geometry::FIndex3i> triangles3i;
		PolygonTriangulation::TriangulateSimplePolygon(vertices, triangles3i);
		TArray<int32> triangles;
		for (auto tri : triangles3i) {
			triangles.Add(tri.A);
			triangles.Add(tri.B);
			triangles.Add(tri.C);
		}

		// Normals, UVs, tangents, and vertex colors
		TArray<FVector> procNormals;
		TArray<FVector2D> uvs;
		TArray<FVector2D> uvs1;
		TArray<FProcMeshTangent> tangents;
		TArray<FColor> vertexColors;
		FVector normal = FVector::UpVector; // Flat polygon facing up
		FColor FaceColor = FColor::MakeRandomColor();

		for (int32 j = 0; j < vertices.Num(); j++) {
			uvs.Add(FVector2D(vertices[j].X / textureScale, vertices[j].Y / textureScale));
			procNormals.Add(normal);
			tangents.Add(FProcMeshTangent(1, 0, 0));
			vertexColors.Add(FaceColor);
		}

		// Spawn a dummy actor to hold the procedural mesh
		FActorSpawnParameters SpawnParams;
		AActor* PolygonActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!PolygonActor) continue;
		// Add procedural mesh component
		UProceduralMeshComponent* ProcMesh = NewObject<UProceduralMeshComponent>(PolygonActor);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(PolygonActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->CreateMeshSection(0, vertices, triangles, procNormals, uvs, vertexColors, tangents, true);
		// Optional: enable collision
		ProcMesh->ContainsPhysicsTriMeshData(true);
		// Set material
		UMaterialInterface* VertexColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_VertexColor.M_VertexColor"));
		ProcMesh->SetMaterial(0, VertexColorMat);
		if (VertexColorMat) {
			ProcMesh->SetMaterial(0, VertexColorMat);
		}
	}
}

void FCityGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCityGeneratorModule, CityGenerator)