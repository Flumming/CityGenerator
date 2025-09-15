// Copyright Epic Games, Inc. All Rights Reserved.

#include "CityGenerator.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"

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
	startpositions.Add(FVector2d(0.0f, 0.0f));

	roadGraph.GenerateRoad(startpositions, 3.0f);

	for (int i = 0; i < roadGraph.nodes.Num(); i++) {

		auto node = roadGraph.nodes[i];

		FVector location(node->position, 0.0f);
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
			FVector End(neighbor->position.X, neighbor->position.Y, 0.f);
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
}

/*
void FCityGeneratorModule::SetupRoad() {
	graph.nodes.Empty();
	float roadLength = 15.0f;
	float randomRange = 1000.0f;

	// Generate random start and end points
	FVector2d start = FVector2d(FMath::RandRange(-randomRange, randomRange), FMath::RandRange(-randomRange, randomRange));
	FVector2d end = FVector2d(FMath::RandRange(-randomRange, randomRange), FMath::RandRange(-randomRange, randomRange));
	int index = 0;
	FVector2d position1 = start, position2 = start;

	while (FVector2D::Distance(position2, end) > roadLength) {
		FVector2d vector = end - start;
		FVector2d unitVector = vector.GetSafeNormal();

		//Rotate the unit vector by a random angle between -20 and 20 degrees
		float angle = FMath::RandRange(-20.0f, 20.0f);
		float radians = FMath::DegreesToRadians(angle);
		float cosAngle = FMath::Cos(radians);
		float sinAngle = FMath::Sin(radians);
		unitVector = FVector2d(
			unitVector.X * cosAngle - unitVector.Y * sinAngle,
			unitVector.X * sinAngle + unitVector.Y * cosAngle
		);

		position1 = position2;
		position2 = position1 + unitVector * roadLength;

		graph.AddNode(index, position1);
		graph.AddNode(index + 1, position2);
		graph.AddEdge(index, index + 1);
		index++;
	}

	// Add the last segment to the end point
	graph.AddNode(index + 1, end);
	graph.AddEdge(index, index + 1);
}
*/

void FCityGeneratorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCityGeneratorModule, CityGenerator)