#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CityGenerator.h"
#include "CityGeneratorActor.generated.h"   // this will be generated automatically

UCLASS()
class CITYGENERATOR_API ACityGeneratorActor : public AActor {
    GENERATED_BODY()

public:
    ACityGeneratorActor();

    // Expose city size in the editor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Generator")
    float CitySize;

    // Function callable in editor & Blueprints
    UFUNCTION(CallInEditor, BlueprintCallable, Category = "City Generator")
    void GenerateCity();

private:
    // Persistent generator instance
    CityGenerator Generator;

protected:
    virtual void BeginPlay() override;
};
