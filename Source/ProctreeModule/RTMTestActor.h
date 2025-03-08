// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RealtimeMeshSimple.h"
#include "RealtimeMeshActor.h"
#include "PlanetNoise.h"
#include "RTMTestActor.generated.h"

UCLASS()
class PROCTREEMODULE_API ARTMTestActor : public ARealtimeMeshActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARTMTestActor();
	virtual void OnGenerateMesh_Implementation() override;
	void TestAddLODSections();
	void TestAddSection();
	FRealtimeMeshSectionConfig OnAddSectionToPolyGroup(int32 PolyGroupIndex);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	URealtimeMeshSimple* rtMesh;
	FRealtimeMeshSectionGroupKey SectionGroupKey;
};
