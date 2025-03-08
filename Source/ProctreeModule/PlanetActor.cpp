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
	//MeshTemplateTables = FMeshTemplateTables(FaceResolution);
	//this->PrimaryActorTick.TickInterval = 1.0f;
	InitializeFaceTransforms();
}

void APlanetActor::InitializeFaceTransforms() {
	// X+ face (points along +X axis)
	FaceTransforms.Add(EFaceDirection::X_POS, {
		FIntVector(1, 2, 0),  // Y=face X, Z=face Y, X=normal
		FIntVector(1, 1, 1), // All positive
		true
	});

	// X- face (points along -X axis)
	FaceTransforms.Add(EFaceDirection::X_NEG, {
		FIntVector(1, 2, 0),  // Y=face X, Z=face Y, X=normal
		FIntVector(-1, 1, -1), // -Y, +Z, -X (changed)
		true
	});

	// Y+ face (points along +Y axis)
	FaceTransforms.Add(EFaceDirection::Y_POS, {
		FIntVector(0, 2, 1),  // X=face X, Z=face Y, Y=normal
		FIntVector(1, 1, 1)   // +X, +Z, +Y (changed)
		});

	// Y- face (points along -Y axis) - This one was correct per your observation
	FaceTransforms.Add(EFaceDirection::Y_NEG, {
		FIntVector(0, 2, 1),  // X=face X, Z=face Y, Y=normal
		FIntVector(1, -1, -1) // +X, -Z, -Y
		});

	// Z+ face (points along +Z axis) - This one was correct per your observation
	FaceTransforms.Add(EFaceDirection::Z_POS, {
		FIntVector(0, 1, 2),  // X=face X, Y=face Y, Z=normal
		FIntVector(1, -1, 1)  // +X, -Y, +Z
		});

	// Z- face (points along -Z axis) - This one was correct per your observation
	FaceTransforms.Add(EFaceDirection::Z_NEG, {
		FIntVector(0, 1, 2),  // X=face X, Y=face Y, Z=normal
		FIntVector(1, 1, -1)  // +X, +Y, -Z
		});
}

//Called when the actor is destroyed
void APlanetActor::BeginDestroy()
{	
	Super::BeginDestroy();

	XPosRootNode = nullptr;
	XNegRootNode = nullptr;
	YPosRootNode = nullptr;
	YNegRootNode = nullptr;
	ZPosRootNode = nullptr;
	ZNegRootNode = nullptr;	
	//NoiseGen = nullptr;
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

	TSharedPtr<INoiseGenerator> NoiseGen0 = MakeShared<MoltenNoiseGenerator>();
	NoiseGen0->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen1 = MakeShared<MoltenNoiseGenerator>();
	NoiseGen1->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen2 = MakeShared<MoltenNoiseGenerator>();
	NoiseGen2->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen3 = MakeShared<MoltenNoiseGenerator>();
	NoiseGen3->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen4 = MakeShared<MoltenNoiseGenerator>();
	NoiseGen4->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);
	TSharedPtr<INoiseGenerator> NoiseGen5 = MakeShared<MoltenNoiseGenerator>();
	NoiseGen5->InitializeNode(this->PlanetMeshParameters.seed, NoiseAmplitude, NoiseFrequency, SeaLevel);

	double size = 1000.0;
	double halfSize = size * .5;	

	this->XPosRootNode = MakeShared<QuadTreeNode>(this, NoiseGen0, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::X_POS), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::X_POS, *FaceTransforms.Find(EFaceDirection::X_POS), FVector(halfSize, 0.0f, 0.0f), size, this->PlanetMeshParameters.planetRadius);
	this->XNegRootNode = MakeShared<QuadTreeNode>(this, NoiseGen1, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::X_NEG), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::X_NEG, *FaceTransforms.Find(EFaceDirection::X_NEG), FVector(-halfSize, 0.0f, 0.0f), size, this->PlanetMeshParameters.planetRadius);
	this->YPosRootNode = MakeShared<QuadTreeNode>(this, NoiseGen2, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::Y_POS), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::Y_POS, *FaceTransforms.Find(EFaceDirection::Y_POS), FVector(0.0f, halfSize, 0.0f), size, this->PlanetMeshParameters.planetRadius);
	this->YNegRootNode = MakeShared<QuadTreeNode>(this, NoiseGen3, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::Y_NEG), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::Y_NEG, *FaceTransforms.Find(EFaceDirection::Y_NEG), FVector(0.0f, -halfSize, 0.0f), size, this->PlanetMeshParameters.planetRadius);
	this->ZPosRootNode = MakeShared<QuadTreeNode>(this, NoiseGen4, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::Z_POS), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::Z_POS, *FaceTransforms.Find(EFaceDirection::Z_POS), FVector(0.0f, 0.0f, halfSize), size, this->PlanetMeshParameters.planetRadius);
	this->ZNegRootNode = MakeShared<QuadTreeNode>(this, NoiseGen5, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::Z_NEG), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::Z_NEG, *FaceTransforms.Find(EFaceDirection::Z_NEG), FVector(0.0f, 0.0f, -halfSize), size, this->PlanetMeshParameters.planetRadius);

	this->XPosRootNode->GenerateMeshData();
	this->XNegRootNode->GenerateMeshData();
	this->YPosRootNode->GenerateMeshData();
	this->YNegRootNode->GenerateMeshData();
	this->ZPosRootNode->GenerateMeshData();
	this->ZNegRootNode->GenerateMeshData();

	this->XPosRootNode->InitializeChunk();
	this->XNegRootNode->InitializeChunk();
	this->YPosRootNode->InitializeChunk();
	this->YNegRootNode->InitializeChunk();
	this->ZPosRootNode->InitializeChunk();
	this->ZNegRootNode->InitializeChunk();

	this->IsInitialized = true;
}

void APlanetActor::UpdateLOD()
{
	Async(EAsyncExecution::LargeThreadPool, [this]() {
		TArray<TSharedPtr<QuadTreeNode>> leaves;
		this->XPosRootNode->CollectLeaves(leaves);
		this->XNegRootNode->CollectLeaves(leaves);
		this->YPosRootNode->CollectLeaves(leaves);
		this->YNegRootNode->CollectLeaves(leaves);
		this->ZPosRootNode->CollectLeaves(leaves);
		this->ZNegRootNode->CollectLeaves(leaves);

		for (TSharedPtr<QuadTreeNode> leaf : leaves) {
			if(leaf) leaf->TrySetLod();
		}
	});
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

void APlanetActor::EnqueueTask(TFunction<void()> Task)
{
	this->TaskQueue.Enqueue(Task);
}

void APlanetActor::DequeueTask()
{
	TFunction<void()> Task;
	for (int i = 0; i < 12; i++) {
		if (TaskQueue.Dequeue(Task))
		{
			Task();
		}
		else {
			break;
		}
	}
}

//FPlanetNoiseGeneratorParameters2 APlanetActor::GetNoiseParameters()
//{
//	if (this->NoiseGen) {
//		return this->NoiseGen->Parameters;
//	}
//	else {
//		return FPlanetNoiseGeneratorParameters2();
//	}	
//}

// Called every frame
void APlanetActor::TickActor(float DeltaTime, ELevelTick TickType, FActorTickFunction& ThisTickFunction)
{
	//Super::TickActor(DeltaTime, TickType, ThisTickFunction);
	this->DequeueTask();
	this->TimeSinceLastLodUpdate += DeltaTime;
	if (this->TimeSinceLastLodUpdate >= .25f && this->IsInitialized) {
		
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

		this->TimeSinceLastLodUpdate = 0.0;
		this->UpdateLOD();
	}
}

TFuture<URealtimeMeshComponent*> APlanetActor::CreateRealtimeMeshComponentAsync()
{
	TPromise<URealtimeMeshComponent*> Promise;
	TFuture<URealtimeMeshComponent*> Future = Promise.GetFuture();

	// Execute on game thread
	AsyncTask(ENamedThreads::GameThread, [this, Promise = MoveTemp(Promise)]() mutable {
		URealtimeMeshComponent* NewComponent = NewObject<URealtimeMeshComponent>(this, URealtimeMeshComponent::StaticClass());
		NewComponent->RegisterComponent();
		NewComponent->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		NewComponent->SetMaterial(0, this->GetRealtimeMeshComponent()->GetMaterial(0));
		NewComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		NewComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

		Promise.SetValue(NewComponent);
	});

	return Future;
}
// Toggles if preview and lod updates will be avaialble from the editor viewport
bool APlanetActor::ShouldTickIfViewportsOnly() const
{
	return this->TickInEditor;
}