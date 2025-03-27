#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetNoise.h"
#include "RealtimeMeshSimple.h"
#include "RealtimeMeshActor.h"
#include <Camera/CameraComponent.h>
#include <Mesh/RealtimeMeshSimpleData.h>
#include "PlanetSharedStructs.h"
#include "MeshUpdatePriorityQueue.h"
#include "PlanetActor.generated.h"

class QuadTreeNode;
class INoiseGenerator;
struct FRealtimeMeshSimpleMeshData;

UCLASS(Blueprintable)
class PROCTREEMODULE_API APlanetActor : public ARealtimeMeshActor
{
	GENERATED_BODY()
	
public:
	APlanetActor();

	// Material properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* LandMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* SeaMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Attributes")
	FPlanetParameters PlanetMeshParameters;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	bool TickInEditor = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	int MinNodeDepth = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	int MaxNodeDepth = 12;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
    int FaceResolution = 13;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float SeaLevel = -.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float NoiseAmplitude = .1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float NoiseFrequency = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	bool UseCameraPositionOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	FVector CameraOverridePosition;

	float TimeSinceLastLodUpdate = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Planet Config")
	FVector GetLastCameraPosition();

	UFUNCTION(BlueprintCallable, Category = "Planet Config")
	FRotator GetLastCameraRotation();

	UFUNCTION(BlueprintCallable, Category = "Planet Config")
	double GetCameraFOV();

	UFUNCTION(BlueprintCallable, Category = "Planet Config")
	void SetCameraOverrideState(bool WillOverride);

	UFUNCTION(BlueprintCallable, Category = "Planet Config")
	bool GetCameraOverrideState();

	UFUNCTION(BlueprintCallable, Category = "Planet Config")
	void SetCameraOverridePosition(FVector InOverridePosition);

	UFUNCTION(BlueprintCallable, Category = "Planet Config")
	FVector GetCameraOverridePosition();

	TSharedPtr<QuadTreeNode> GetNodeByIndex(const FQuadIndex& Index) const;
    
protected:
	//Root nodes for each face
	TSharedPtr<QuadTreeNode> RootNodes[6];

	//State
	bool IsDestroyed = false;
	bool IsInitialized = false;
	bool IsDataUpdateRunning = false;
	bool IsMeshUpdateRunning = false;
	bool UseCameraPositionOverrideInternal = false;

	//Settings and camera data
	double CameraFov = 90;
	FVector LastCameraPositionInternal;
	FRotator LastCameraRotationInternal;
	FVector CameraOverridePositionInternal;
	
	//Overrides
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	virtual bool ShouldTickIfViewportsOnly() const override;
	virtual void BeginDestroy() override;

	void InitializePlanet();
	void UpdateLOD();
	void UpdateMesh();
	void ScheduleDataUpdate(float IntervalInSeconds);
	void ScheduleMeshUpdate(float IntervalInSeconds);

	//Node Locks
	FRWLock updateLock;
};