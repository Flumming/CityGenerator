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

	//SetupRoad();
	TArray<FVector2d> startpositions;
	//startpositions.Add(FVector2d(0.0f, 0.0f));

	roadGraph.GenerateRoad(startpositions, 3.0f);

	UE_LOG(LogTemp, Warning, TEXT("num %i"), roadGraph.nodes.Num());
	for (int i = 0; i < roadGraph.nodes.Num(); i++) {

		auto node = roadGraph.nodes[i];

		FVector location(node->position, 10.0f);
		FRotator rotation(0.f, 0.f, 0.f);

		FActorSpawnParameters spawnparams;
		spawnparams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (startpositions.Contains(FVector2d(node->position.X, node->position.Y))) {
			AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass2, location, rotation, spawnparams);
			continue;
		}
		AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, location, rotation, spawnparams);

		for (auto* neighbor : node->neighbors) {
			if (node->id < neighbor->id) continue; // Avoid double drawing

			FVector Start = location;
			FVector End(neighbor->position.X, neighbor->position.Y, 10.f);
			FVector MidPoint = (Start + End) / 2;

			FVector Direction = End - Start;
			float Length = Direction.Size();
			FRotator Rotation = Direction.Rotation() + FRotator(90.f, 0.f, 0.f);

			FActorSpawnParameters SpawnParams;
			AStaticMeshActor* Connector = World->SpawnActor<AStaticMeshActor>(MidPoint, Rotation, SpawnParams);
			if (Connector) {
				UStaticMeshComponent* MeshComp = Connector->GetStaticMeshComponent();
				if (MeshComp && CylinderMesh) {
					MeshComp->SetStaticMesh(CylinderMesh);
					MeshComp->SetWorldScale3D(FVector(0.1f, 0.1f, Length / 100.0f));
				}
			}
		}
	}

	// Generate polygons from road.Graph.faces list of faces
	for (auto face : roadGraph.faces) {
		if (face.Num() < 3) continue; 

		TArray<FVector> Vertices;
		for (auto node : face) {
			Vertices.Add(FVector(node->position.X, node->position.Y, 10.0f));
		}

		TArray<UE::Geometry::FIndex3i> Triangles3i;
		PolygonTriangulation::TriangulateSimplePolygon(Vertices, Triangles3i);

		// Convert to TArray<int32> for ProceduralMeshComponent
		TArray<int32> Triangles;
		Triangles.Reserve(Triangles3i.Num() * 3);
		for (const auto& Tri : Triangles3i) {
			Triangles.Add(Tri.A);
			Triangles.Add(Tri.B);
			Triangles.Add(Tri.C);
		}

		// Normals, UVs, tangents, and vertex colors
		TArray<FVector> ProcNormals;
		TArray<FVector2D> UVs;
		TArray<FProcMeshTangent> Tangents;
		TArray<FColor> VertexColors;

		FVector Normal = FVector::UpVector; // Flat polygon facing up
		FColor FaceColor = FColor::MakeRandomColor();

		for (int32 i = 0; i < Vertices.Num(); i++) {
			ProcNormals.Add(Normal);
			UVs.Add(FVector2D(Vertices[i].X, Vertices[i].Y));
			Tangents.Add(FProcMeshTangent(1, 0, 0));
			VertexColors.Add(FaceColor);
		}

		// Spawn a dummy actor to hold the procedural mesh
		FActorSpawnParameters SpawnParams;
		AActor* PolygonActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (!PolygonActor) continue;

		// Add procedural mesh component
		UProceduralMeshComponent* ProcMesh = NewObject<UProceduralMeshComponent>(PolygonActor);
		ProcMesh->RegisterComponent();
		ProcMesh->AttachToComponent(PolygonActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		ProcMesh->CreateMeshSection(0, Vertices, Triangles, ProcNormals, UVs, VertexColors, Tangents, true);

		// Optional: enable collision
		ProcMesh->ContainsPhysicsTriMeshData(true);

		// Set material
		UMaterialInterface* VertexColorMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/CityGenerator/Materials/M_VertexColor.M_VertexColor"));
		ProcMesh->SetMaterial(0, VertexColorMat);
	}
}

void FCityGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCityGeneratorModule, CityGenerator)