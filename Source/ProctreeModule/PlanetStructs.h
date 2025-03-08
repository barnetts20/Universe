#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlanetStructs.generated.h"

UCLASS()
class PROCTREEMODULE_API APlanetStructs : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this empty's properties
	APlanetStructs();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
