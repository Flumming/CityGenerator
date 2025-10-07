#include "CityGeneratorActor.h"

ACityGeneratorActor::ACityGeneratorActor()
    : Generator() // Will initialize in BeginPlay
{
    CitySize = 50000.0f;
    PrimaryActorTick.bCanEverTick = false;
}

void ACityGeneratorActor::BeginPlay() {
    Super::BeginPlay();

    if (UWorld* World = GetWorld()) {
        Generator.SetWorld(World); // Assuming you add a SetWorld method in CityGenerator
    }
}

void ACityGeneratorActor::GenerateCity() {
    Generator.GenerateCity(CitySize);
}
