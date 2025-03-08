// Copyright TriAxis Games, L.L.C. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DynamicMesh/DynamicMesh3.h"
#include "Mesh/RealtimeMeshDataStream.h"
#include "UObject/Object.h"
#include "RealtimeMeshDynamicMeshConverter.generated.h"


class UDynamicMesh;
class URealtimeMeshSimple;
class URealtimeMeshStreamSet;


USTRUCT(BlueprintType)
struct REALTIMEMESHEXT_API FStreamSetDynamicMeshConversionOptions
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantNormals = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantTangents = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantUVs = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantVertexColors = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantMaterialIDs = true;
};

USTRUCT(BlueprintType)
struct REALTIMEMESHEXT_API FRealtimeMeshDynamicMeshConversionOptions
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	FRealtimeMeshSectionGroupKey SectionGroup;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantNormals = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantTangents = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantUVs = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantVertexColors = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="RealtimeMesh|Conversion")
	bool bWantPolyGroups = true;
};

/**
 * 
 */
UCLASS()
class REALTIMEMESHEXT_API URealtimeMeshDynamicMeshConverter : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	static bool CopyStreamSetToDynamicMesh(const RealtimeMesh::FRealtimeMeshStreamSet& InStreamSet, UE::Geometry::FDynamicMesh3& OutDynamicMesh, FStreamSetDynamicMeshConversionOptions Options = FStreamSetDynamicMeshConversionOptions());

	static bool CopyStreamSetFromDynamicMesh(const UE::Geometry::FDynamicMesh3& InDynamicMesh, RealtimeMesh::FRealtimeMeshStreamSet& OutStreamSet, FStreamSetDynamicMeshConversionOptions Options = FStreamSetDynamicMeshConversionOptions());

private:

	UFUNCTION(BlueprintCallable, Category = "RealtimeMesh|Conversion", meta = (ExpandEnumAsExecs = "Outcome"))
	static UPARAM(DisplayName = "Stream Set") URealtimeMeshStreamSet* CopyStreamSetFromDynamicMesh(
		UDynamicMesh* FromDynamicMesh, 
		URealtimeMeshStreamSet* ToStreamSet, 
		FStreamSetDynamicMeshConversionOptions Options,
		ERealtimeMeshOutcomePins& Outcome);

	UFUNCTION(BlueprintCallable, Category = "RealtimeMesh|Conversion", meta = (ExpandEnumAsExecs = "Outcome"))
	static UPARAM(DisplayName = "Dynamic Mesh") UDynamicMesh* CopyStreamSetToDynamicMesh(
		URealtimeMeshStreamSet* FromStreamSet, 
		UDynamicMesh* ToDynamicMesh,
		FStreamSetDynamicMeshConversionOptions Options,
		ERealtimeMeshOutcomePins& Outcome);

	UFUNCTION(BlueprintCallable, Category = "RealtimeMesh|Conversion", meta = (ExpandEnumAsExecs = "Outcome"))
	static UPARAM(DisplayName = "Realtime Mesh") URealtimeMeshSimple* CopyRealtimeMeshFromDynamicMesh(
		UDynamicMesh* FromDynamicMesh, 
		URealtimeMeshSimple* ToRealtimeMesh, 
		FRealtimeMeshDynamicMeshConversionOptions Options,
		ERealtimeMeshOutcomePins& Outcome);

	UFUNCTION(BlueprintCallable, Category = "RealtimeMesh|Conversion", meta = (ExpandEnumAsExecs = "Outcome"))
	static UPARAM(DisplayName = "Dynamic Mesh") UDynamicMesh* CopyRealtimeMeshToDynamicMesh(
		URealtimeMeshSimple* FromRealtimeMesh, 
		UDynamicMesh* ToDynamicMesh,
		FRealtimeMeshDynamicMeshConversionOptions Options,
		ERealtimeMeshOutcomePins& Outcome);	
};
