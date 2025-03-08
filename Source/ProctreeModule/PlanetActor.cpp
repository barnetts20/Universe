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

	//Pretty sure size is arbitrary and radius is what determines our scale. 
	//We may even lose precision trying to generate at a large scale, normalizing to 1 size, and then scaling back up
	//Would probably be better to use a size closer to the normalization to avoid precision loss 
	double size = 100.0;
	this->XPosRootNode = MakeShared<QuadTreeNode>(this, NoiseGen0, xPosLock, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::XPositive), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::XPositive, FVector(.5f * (this->PlanetMeshParameters.planetRadius * 2), 0.0f, 0.0f), (this->PlanetMeshParameters.planetRadius * 2), this->PlanetMeshParameters.planetRadius);
	this->XNegRootNode = MakeShared<QuadTreeNode>(this, NoiseGen1, xNegLock, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::XNegative), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::XNegative, FVector(-.5f * (this->PlanetMeshParameters.planetRadius * 2), 0.0f, 0.0f), (this->PlanetMeshParameters.planetRadius * 2), this->PlanetMeshParameters.planetRadius);
	this->YPosRootNode = MakeShared<QuadTreeNode>(this, NoiseGen2, yPosLock, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::YPositive), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::YPositive, FVector(0.0f, .5f * (this->PlanetMeshParameters.planetRadius * 2), 0.0f), (this->PlanetMeshParameters.planetRadius * 2), this->PlanetMeshParameters.planetRadius);
	this->YNegRootNode = MakeShared<QuadTreeNode>(this, NoiseGen3, yNegLock, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::YNegative), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::YNegative, FVector(0.0f, -.5f * (this->PlanetMeshParameters.planetRadius * 2), 0.0f), (this->PlanetMeshParameters.planetRadius * 2), this->PlanetMeshParameters.planetRadius);
	this->ZPosRootNode = MakeShared<QuadTreeNode>(this, NoiseGen4, zPosLock, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::ZPositive), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::ZPositive, FVector(0.0f, 0.0f, .5f * (this->PlanetMeshParameters.planetRadius * 2)), (this->PlanetMeshParameters.planetRadius * 2), this->PlanetMeshParameters.planetRadius);
	this->ZNegRootNode = MakeShared<QuadTreeNode>(this, NoiseGen5, zNegLock, FString::Printf(TEXT("%d"), (uint8)EFaceDirection::ZNegative), this->MinNodeDepth, this->MaxNodeDepth, EFaceDirection::ZNegative, FVector(0.0f, 0.0f, -.5f * (this->PlanetMeshParameters.planetRadius * 2)), (this->PlanetMeshParameters.planetRadius * 2), this->PlanetMeshParameters.planetRadius);

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