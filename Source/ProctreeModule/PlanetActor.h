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
	int MaxNodeDepth = 16;
	
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
    int FaceResolution = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	bool UseCameraPositionOverride = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float SeaLevel = -.1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float NoiseAmplitude = .1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Planet Config")
	float NoiseFrequency = 1.0;

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
	TFuture<URealtimeMeshComponent*> CreateRealtimeMeshComponentAsync();
	//Root nodes for each face
	TSharedPtr<QuadTreeNode> XPosRootNode;
	TSharedPtr<QuadTreeNode> XNegRootNode;
	TSharedPtr<QuadTreeNode> YPosRootNode;
	TSharedPtr<QuadTreeNode> YNegRootNode;
	TSharedPtr<QuadTreeNode> ZPosRootNode;
	TSharedPtr<QuadTreeNode> ZNegRootNode;

    //FMeshTemplateTables MeshTemplateTables;
    
protected:
	virtual void BeginPlay() override;
	virtual void TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;

	virtual bool ShouldTickIfViewportsOnly() const override;

	//Settings and camera data
	double CameraFov = 90;
	FVector CameraOverridePositionInternal;
	bool UseCameraPositionOverrideInternal = false;
	FVector LastCameraPositionInternal;
	FRotator LastCameraRotationInternal;

	//Structural data, noise generator, mesh queue
	//TSharedPtr<PlanetNoiseGenerator2> NoiseGen;

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