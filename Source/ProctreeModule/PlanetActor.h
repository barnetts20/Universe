#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlanetNoise.h"
#include "RealtimeMeshSimple.h"
#include "RealtimeMeshActor.h"
#include <Camera/CameraComponent.h>
#include <Mesh/RealtimeMeshSimpleData.h>
#include "PlanetSharedStructs.h"
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

	void InitializeFaceTransforms();
	
	void BeginDestroy();
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UFUNCTION(BlueprintCallable, Category = "Planet Config")
	
	void InitializePlanet();
	
	void UpdateLOD();

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
    int FaceResolution = 17;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	bool UseCameraPositionOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float SeaLevel = -.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float NoiseAmplitude = .1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float NoiseFrequency = 1.0;

	//TMap<EFaceDirection, FCubeTransform> FaceTransforms;

	TQueue<TFunction<void()>> TaskQueue;

	void EnqueueTask(TFunction<void()> Task);
	
	float TimeSinceLastLodUpdate = 0.0f;

	void DequeueTask();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	bool UseNoise = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	FVector CameraOverridePosition;

	//UFUNCTION(BlueprintCallable, Category = "Planet Config")
	//FPlanetNoiseGeneratorParameters2 GetNoiseParameters();

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
	void ScheduleMeshUpdate(float IntervalInSeconds);
	TFuture<URealtimeMeshComponent*> CreateRealtimeMeshComponentAsync();
	//Root nodes for each face
	TSharedPtr<QuadTreeNode> RootNodes[6];

	void UpdateMesh();

	TSharedPtr<QuadTreeNode> GetNodeByIndex(const FQuadIndex& Index) const;
	TSharedPtr<QuadTreeNode> GetLeafNodeByIndex(const FQuadIndex& Index) const;
    
protected:
	virtual void BeginPlay() override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	bool IsDestroyed = false;
	bool IsDataUpdateRunning = false;
	bool IsMeshUpdateRunning = false;
	void ScheduleDataUpdate(float IntervalInSeconds);

	virtual bool ShouldTickIfViewportsOnly() const override;

	//Settings and camera data
	double CameraFov = 90;
	bool UseCameraPositionOverrideInternal = false;
	FVector CameraOverridePositionInternal;
	FVector LastCameraPositionInternal;
	FRotator LastCameraRotationInternal;

	int32 MaxTasksProcessing = 30;
	int32 TasksProcessing = 0;
	FCriticalSection TaskCounterLock; // Lock to protect the counter

	//Node Locks
	FRWLock xPosLock;
	FRWLock xNegLock;
	FRWLock yPosLock;
	FRWLock yNegLock;
	FRWLock zPosLock;
	FRWLock zNegLock;

	bool IsInitialized = false;
};