#include "PlanetActor.h"
#include "QuadTreeNode.h"
#include "Async/Async.h"
#include "RealtimeMeshActor.h"
#include <Kismet/GameplayStatics.h>
#include <Camera/CameraComponent.h>
#include <Mesh/RealtimeMeshSimpleData.h>
#include <Mesh/RealtimeMeshBasicShapeTools.h>

// Sets default values
APlanetActor::APlanetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	MaxNodeDepth = 4;
//	InitializeFaceTransforms();
}

//Called when the actor is destroyed
void APlanetActor::BeginDestroy()
{	
	IsDestroyed = true;
	Super::BeginDestroy();

	for (int i = 0; i < 6; i++) {
		RootNodes[i] = nullptr;
	}
}

void APlanetActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(this->GetActorTransform());
	
	this->CameraOverridePositionInternal = this->CameraOverridePosition;
	this->UseCameraPositionOverrideInternal = this->UseCameraPositionOverride;
	this->InitializePlanet();
}

void APlanetActor::BeginPlay()
{
	Super::BeginPlay();
	
	this->InitializePlanet();
}

void APlanetActor::InitializePlanet()
{	
	this->IsInitialized = false;
	auto destroyComponentArray = this->GetRootComponent()->GetAttachChildren();
	for (TObjectPtr<USceneComponent> child : destroyComponentArray) {
		URealtimeMeshComponent* meshComponent = Cast<URealtimeMeshComponent>(child);
		if (meshComponent) {
			meshComponent->DestroyComponent();
		}

	}

	TSharedPtr<INoiseGenerator> NoiseGen0 = MakeShared<TerrestrialNoiseGenerator>();
	NoiseGen0->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen1 = MakeShared<TerrestrialNoiseGenerator>();
	NoiseGen1->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen2 = MakeShared<TerrestrialNoiseGenerator>();
	NoiseGen2->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen3 = MakeShared<TerrestrialNoiseGenerator>();
	NoiseGen3->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen4 = MakeShared<TerrestrialNoiseGenerator>();
	NoiseGen4->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen5 = MakeShared<TerrestrialNoiseGenerator>();
	NoiseGen5->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);

	double size = 1000.0;
	double halfSize = size * .5;	

	RootNodes[(uint8)EFaceDirection::X_POS] = MakeShared<QuadTreeNode>(this, NoiseGen0, FCubeTransform::FaceTransforms[(uint8)EFaceDirection::X_POS], FQuadIndex((uint8)EFaceDirection::X_POS), FVector( halfSize, 0.0f, 0.0f), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::X_NEG] = MakeShared<QuadTreeNode>(this, NoiseGen1, FCubeTransform::FaceTransforms[(uint8)EFaceDirection::X_NEG], FQuadIndex((uint8)EFaceDirection::X_NEG), FVector(-halfSize, 0.0f, 0.0f), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::Y_POS] = MakeShared<QuadTreeNode>(this, NoiseGen2, FCubeTransform::FaceTransforms[(uint8)EFaceDirection::Y_POS], FQuadIndex((uint8)EFaceDirection::Y_POS), FVector(0.0f,  halfSize, 0.0f), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::Y_NEG] = MakeShared<QuadTreeNode>(this, NoiseGen3, FCubeTransform::FaceTransforms[(uint8)EFaceDirection::Y_NEG], FQuadIndex((uint8)EFaceDirection::Y_NEG), FVector(0.0f, -halfSize, 0.0f), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::Z_POS] = MakeShared<QuadTreeNode>(this, NoiseGen4, FCubeTransform::FaceTransforms[(uint8)EFaceDirection::Z_POS], FQuadIndex((uint8)EFaceDirection::Z_POS), FVector(0.0f, 0.0f,  halfSize), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::Z_NEG] = MakeShared<QuadTreeNode>(this, NoiseGen5, FCubeTransform::FaceTransforms[(uint8)EFaceDirection::Z_NEG], FQuadIndex((uint8)EFaceDirection::Z_NEG), FVector(0.0f, 0.0f, -halfSize), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);

	for (int i = 0; i < 6; i++) {
		RootNodes[i]->InitializeChunk();
		RootNodes[i]->GenerateMeshData();
	}

	this->IsInitialized = true;
	ScheduleDataUpdate(.1);
	ScheduleMeshUpdate(.1);
}

void APlanetActor::UpdateLOD()
{
	Async(EAsyncExecution::LargeThreadPool, [this]() {
		ParallelFor(6, [&](int32 i) {
			RootNodes[i]->UpdateLod();
		});
		ParallelFor(6, [&](int32 i) {
			RootNodes[i]->UpdateNeighbors();
		});
	});
}

void APlanetActor::UpdateMesh()
{
	ParallelFor(6, [&](int32 i) {
		RootNodes[i]->UpdateAllMesh();
	});
}

TSharedPtr<QuadTreeNode> APlanetActor::GetNodeByIndex(const FQuadIndex& Index) const
{
	// Get the root node for the specified face
	TSharedPtr<QuadTreeNode> currentNode = RootNodes[Index.FaceId];

	// If we're looking for the root node, return it immediately
	if (Index.IsRoot())
		return currentNode;

	// Traverse down the tree following the quadrant path
	uint8 targetDepth = Index.GetDepth();
	for (uint8 level = 0; level < targetDepth; ++level)
	{
		// Get the quadrant at the current level
		uint8 quadrant = Index.GetQuadrantAtDepth(level);

		// Check if the current node is a leaf (has no children)
		if (currentNode->IsLeaf())
			return currentNode;  // Node doesn't exist at requested depth

		// Move to the next child in the path
		currentNode = currentNode->Children[quadrant];
	}

	return currentNode;
}

FVector APlanetActor::GetLastCameraPosition()
{
	return this->LastCameraPositionInternal;
}

FRotator APlanetActor::GetLastCameraRotation()
{
	return this->LastCameraRotationInternal;
}

double APlanetActor::GetCameraFOV()
{
	return this->CameraFov;
}

//This function causes the lod updates to ignore the camera location and instead use the override position
void APlanetActor::SetCameraOverrideState(bool WillOverride) {
	this->UseCameraPositionOverrideInternal = WillOverride;
}

bool APlanetActor::GetCameraOverrideState()
{
	return this->UseCameraPositionOverrideInternal;
}

//This function is used to manually set the position that will be used in LOD updates. Useful for things like preloading geometry before warping to it.
void APlanetActor::SetCameraOverridePosition(FVector InOverridePosition) {
	this->CameraOverridePositionInternal = InOverridePosition;
}

FVector APlanetActor::GetCameraOverridePosition()
{
	return this->CameraOverridePositionInternal;
}

// Called every frame
void APlanetActor::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	this->TimeSinceLastLodUpdate += DeltaTime;
	if (this->TimeSinceLastLodUpdate >= .05f && this->IsInitialized) {
		
		auto camManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
		if (this->UseCameraPositionOverride) {
			this->LastCameraPositionInternal = CameraOverridePosition;
		}
		else if (camManager) {
			this->CameraFov = camManager->GetFOVAngle();
			auto camRot = camManager->GetCameraRotation();
			auto camLoc = camManager->GetCameraLocation();
			this->LastCameraPositionInternal = camLoc;
			this->LastCameraRotationInternal = camRot;
		}
		else {
			auto world = GetWorld();
			if (world != nullptr) {
				auto viewLocations = world->ViewLocationsRenderedLastFrame;
				if (viewLocations.Num() > 0) {
					this->LastCameraPositionInternal = viewLocations[0];
				}
			}
		}
	}
	Super::TickActor(DeltaTime, TickType, ThisTickFunction);
}

void APlanetActor::ScheduleDataUpdate(float IntervalInSeconds)
{
	if (!IsDataUpdateRunning && !IsDestroyed)
	{
		IsDataUpdateRunning = true;

		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([this, IntervalInSeconds]()
			{
				//**********BEGIN IMPLEMENTATION BLOCK***************
				//**********BEGIN IMPLEMENTATION BLOCK***************
				//**********BEGIN IMPLEMENTATION BLOCK***************
				{
					FWriteScopeLock WriteLock(xPosLock);
					if (IsDestroyed) return;
					UpdateLOD();
				}
				//***********END IMPLEMENTATION BLOCK***************
				//***********END IMPLEMENTATION BLOCK***************
				//***********END IMPLEMENTATION BLOCK***************

				IsDataUpdateRunning = false;
				FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, IntervalInSeconds](float DeltaTime)
					{
						ScheduleDataUpdate(IntervalInSeconds);
						return false;
					}), IntervalInSeconds);
			}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);
	}
}

void APlanetActor::ScheduleMeshUpdate(float IntervalInSeconds)
{
	if (!IsMeshUpdateRunning && !IsDestroyed)
	{
		IsMeshUpdateRunning = true;

		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([this, IntervalInSeconds]()
			{
				//**********BEGIN IMPLEMENTATION BLOCK***************
				//**********BEGIN IMPLEMENTATION BLOCK***************
				//**********BEGIN IMPLEMENTATION BLOCK***************
				{
					FWriteScopeLock WriteLock(xPosLock);
					if (IsDestroyed) return;
					UpdateMesh();
				}
				//***********END IMPLEMENTATION BLOCK***************
				//***********END IMPLEMENTATION BLOCK***************
				//***********END IMPLEMENTATION BLOCK***************

				IsMeshUpdateRunning = false;
				FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([this, IntervalInSeconds](float DeltaTime)
					{
						ScheduleMeshUpdate(IntervalInSeconds);
						return false;
					}), IntervalInSeconds);
			}, TStatId(), nullptr, ENamedThreads::AnyBackgroundThreadNormalTask);
	}
}

// Toggles if preview and lod updates will be avaialble from the editor viewport
bool APlanetActor::ShouldTickIfViewportsOnly() const
{
	return this->TickInEditor;
}