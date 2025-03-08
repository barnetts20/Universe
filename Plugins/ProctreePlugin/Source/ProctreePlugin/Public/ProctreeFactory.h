// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UProctreeWrapper.h"
#include "ProctreeFactory.generated.h"

/**
 * 
 */
UCLASS()
class PROCTREEPLUGIN_API UProctreeFactory : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ProctreeFactory")
		static UProctreeWrapper* ConstructProctree(int32 MaxDepth, int32 SizeMultiplier);
};
