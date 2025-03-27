#include "PlanetActor.h"
#include "QuadTreeNode.h"
#include "Async/Async.h"
#include "RealtimeMeshActor.h"
#include <Kismet/GameplayStatics.h>
#include <Camera/CameraComponent.h>
#include <Mesh/RealtimeMeshSimpleData.h>
#include <Mesh/RealtimeMeshBasicShapeTools.h>
#include "QuadTreeConcurrencyManager.h"

// Sets default values
APlanetActor::APlanetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
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
void APlanetActor::SetCameraOverrideState(bool WillOverride) {
	//This function causes the lod updates to ignore the camera location and instead use the override position
	//Useful for things like "Warping" into position, the geometry can be forced to preload for the final position
	this->UseCameraPositionOverrideInternal = WillOverride;
}
bool APlanetActor::GetCameraOverrideState()
{
	return this->UseCameraPositionOverrideInternal;
}
void APlanetActor::SetCameraOverridePosition(FVector InOverridePosition) {
	//This function is used to manually set the position that will be used in LOD updates. Useful for things like preloading geometry before warping to it.
	this->CameraOverridePositionInternal = InOverridePosition;
}
FVector APlanetActor::GetCameraOverridePosition()
{
	return this->CameraOverridePositionInternal;
}

TSharedPtr<QuadTreeNode> APlanetActor::GetNodeByIndex(const FQuadIndex& Index) const
{
	//Node lookup at wrapper level to support cross face neighbor lookup
	TSharedPtr<QuadTreeNode> currentNode = RootNodes[Index.FaceId];
	if (Index.IsRoot())	return currentNode;
	uint8 targetDepth = Index.GetDepth();
	for (uint8 level = 0; level < targetDepth; ++level)
	{
		uint8 quadrant = Index.GetQuadrantAtDepth(level);
		if (currentNode->IsLeaf()) return currentNode;
		currentNode = currentNode->Children[quadrant];
	}
	return currentNode;
}

void APlanetActor::BeginPlay()
{
	Super::BeginPlay();
	this->InitializePlanet();
}
void APlanetActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(this->GetActorTransform());
	this->CameraOverridePositionInternal = this->CameraOverridePosition;
	this->UseCameraPositionOverrideInternal = this->UseCameraPositionOverride;
	this->InitializePlanet();
}
void APlanetActor::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	// Called every frame, updates relevant camera transform data used in LOD calculations
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
bool APlanetActor::ShouldTickIfViewportsOnly() const
{
	// Toggles if LOD updates will occur while in the editor (not running)
	return this->TickInEditor;
}
void APlanetActor::BeginDestroy()
{
	//Called before the actor is destroyed
	IsDestroyed = true;
	RootNodes->Reset();
	Super::BeginDestroy();
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

	//TODO: More procedural way to do this/get rid of hardcoding
	TSharedPtr<INoiseGenerator> NoiseGen0 = MakeShared<TerrestrialNoiseGenerator>();
	TSharedPtr<INoiseGenerator> NoiseGen1 = MakeShared<TerrestrialNoiseGenerator>();
	TSharedPtr<INoiseGenerator> NoiseGen2 = MakeShared<TerrestrialNoiseGenerator>();
	TSharedPtr<INoiseGenerator> NoiseGen3 = MakeShared<TerrestrialNoiseGenerator>();
	TSharedPtr<INoiseGenerator> NoiseGen4 = MakeShared<TerrestrialNoiseGenerator>();
	TSharedPtr<INoiseGenerator> NoiseGen5 = MakeShared<TerrestrialNoiseGenerator>();
	NoiseGen0->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	NoiseGen1->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	NoiseGen2->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	NoiseGen3->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	NoiseGen4->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	NoiseGen5->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);

	//"Size" in this case actually has no impact on the resulting size of the planet, but is needed for recursive construction
	//"Size" in this context is the scale the mesh is constructed at pre-normalize and should attempt to strike a balance with precision
	double size = 1000.0;
	double halfSize = size * .5;	

	RootNodes[(uint8)EFaceDirection::X_POS] = MakeShared<QuadTreeNode>(this, NoiseGen0, FQuadIndex((uint8)EFaceDirection::X_POS), FCubeTransform::FaceTransforms[(uint8)EFaceDirection::X_POS], FVector( halfSize, 0.0f, 0.0f), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::X_NEG] = MakeShared<QuadTreeNode>(this, NoiseGen1, FQuadIndex((uint8)EFaceDirection::X_NEG), FCubeTransform::FaceTransforms[(uint8)EFaceDirection::X_NEG], FVector(-halfSize, 0.0f, 0.0f), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::Y_POS] = MakeShared<QuadTreeNode>(this, NoiseGen2, FQuadIndex((uint8)EFaceDirection::Y_POS), FCubeTransform::FaceTransforms[(uint8)EFaceDirection::Y_POS], FVector(0.0f,  halfSize, 0.0f), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::Y_NEG] = MakeShared<QuadTreeNode>(this, NoiseGen3, FQuadIndex((uint8)EFaceDirection::Y_NEG), FCubeTransform::FaceTransforms[(uint8)EFaceDirection::Y_NEG], FVector(0.0f, -halfSize, 0.0f), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::Z_POS] = MakeShared<QuadTreeNode>(this, NoiseGen4, FQuadIndex((uint8)EFaceDirection::Z_POS), FCubeTransform::FaceTransforms[(uint8)EFaceDirection::Z_POS], FVector(0.0f, 0.0f,  halfSize), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);
	RootNodes[(uint8)EFaceDirection::Z_NEG] = MakeShared<QuadTreeNode>(this, NoiseGen5, FQuadIndex((uint8)EFaceDirection::Z_NEG), FCubeTransform::FaceTransforms[(uint8)EFaceDirection::Z_NEG], FVector(0.0f, 0.0f, -halfSize), size, this->PlanetMeshParameters.planetRadius, this->MinNodeDepth, this->MaxNodeDepth);

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
void APlanetActor::ScheduleDataUpdate(float IntervalInSeconds)
{
	//Schedules periodic updates to the data structures based on LOD
	if (!IsDataUpdateRunning && !IsDestroyed)
	{
		IsDataUpdateRunning = true;

		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([this, IntervalInSeconds]()
			{
				//**********BEGIN IMPLEMENTATION BLOCK***************
				//**********BEGIN IMPLEMENTATION BLOCK***************
				//**********BEGIN IMPLEMENTATION BLOCK***************
				{
					FWriteScopeLock WriteLock(updateLock);
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
	//Shedules periodic updates to the rendered mesh based on the cached data
	if (!IsMeshUpdateRunning && !IsDestroyed)
	{
		IsMeshUpdateRunning = true;

		FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([this, IntervalInSeconds]()
			{
				//**********BEGIN IMPLEMENTATION BLOCK***************
				//**********BEGIN IMPLEMENTATION BLOCK***************
				//**********BEGIN IMPLEMENTATION BLOCK***************
				{
					FWriteScopeLock WriteLock(updateLock);
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

