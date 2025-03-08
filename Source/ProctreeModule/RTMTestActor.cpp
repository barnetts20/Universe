// Fill out your copyright notice in the Description page of Project Settings.


#include "RTMTestActor.h"
#include <Mesh/RealtimeMeshBasicShapeTools.h>
#include <Mesh/RealtimeMeshBlueprintMeshBuilder.h>
//#include <RealtimeMeshTests/Public/FunctionalTests/RealtimeMeshBasicUsageActor.h>


// Sets default values
ARTMTestActor::ARTMTestActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARTMTestActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARTMTestActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARTMTestActor::OnGenerateMesh_Implementation()
{
	Super::OnGenerateMesh_Implementation();

	// Initialize the simple mesh
	rtMesh = GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();
	// Setup the two material slots
	rtMesh->SetupMaterialSlot(0, "PrimaryMaterial");
	rtMesh->SetupMaterialSlot(1, "SecondaryMaterial");

	//TestAddSection();
}

void ARTMTestActor::TestAddLODSections() {
	FRealtimeMeshLODConfig LODConfig1;
	LODConfig1.ScreenSize = 0.2f;

	FRealtimeMeshLODKey inKey = rtMesh->AddLOD(LODConfig1);
	FRealtimeMeshLODKey oKey = FRealtimeMeshLODKey::FRealtimeMeshLODKey(0);

	for (int i = 0; i <= 5; i++) {

	}
}

void ARTMTestActor::TestAddSection() {
	////rtMesh->AddLOD();
	//{	// Create a basic single section
	//	TArray<FMeshInitData> d = UPlanetNoise::GenerateQuadSphere(500); //Generate some test data
	//	int idx = 0;
	//	for (auto md : d) {
	//		FRealtimeMeshSimpleMeshData meshData;
	//		FRealtimeMeshSimpleMeshData populateData;
	//		populateData.Positions = md.Vertices;
	//		populateData.UV0 = md.UV;
	//		populateData.Normals = md.Normals;
	//		populateData.Triangles = md.Triangles;
	//		populateData.MaterialIndex = { 0 };
	//		URealtimeMeshSimpleBasicShapeTools::AppendMesh(meshData, populateData, FTransform::Identity, 0);//Append funct might be worth using just for simple transfrom? could get rid of all the actor position usage in the quadtree

	//		auto lkey = FRealtimeMeshLODKey::FRealtimeMeshLODKey(0);
	//		FString nodeIDString = FString::Printf(TEXT("%d"), idx);
	//		idx++;
	//		SectionGroupKey = FRealtimeMeshSectionGroupKey::Create(lkey, FName(nodeIDString));

	//		rtMesh->CreateSectionGroup(SectionGroupKey, meshData);
	//	}
	//	auto SectionGroup = rtMesh->GetMeshData()->GetSectionGroupAs<FRealtimeMeshSectionGroupSimple>(SectionGroupKey);
	//	SectionGroup->SetPolyGroupSectionHandler(FRealtimeMeshPolyGroupConfigHandler::CreateUObject(this, &ARTMTestActor::OnAddSectionToPolyGroup));

	//	FRealtimeMeshSectionConfig VisibleConfig;
	//	VisibleConfig.bIsVisible = true;

	//	rtMesh->UpdateSectionConfig(FRealtimeMeshSectionKey::CreateForPolyGroup(SectionGroupKey, 1), VisibleConfig);
	//}
}

FRealtimeMeshSectionConfig ARTMTestActor::OnAddSectionToPolyGroup(int32 PolyGroupIndex)
{
	return FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Static, PolyGroupIndex);
}