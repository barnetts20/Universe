#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "FProctreeStructures.h"
#include "FProctreeNode.h"
#include "UProctreeNodeWrapper.generated.h"
/**
 *
 */
UCLASS(BlueprintType)
class PROCTREEPLUGIN_API UProctreeNodeWrapper : public UObject
{
	GENERATED_BODY()

public:
	UProctreeNodeWrapper();
	~UProctreeNodeWrapper();

	// Getters
	UFUNCTION(BlueprintPure, Category = "ProctreeNodeWrapper")
		UProctreeNodeWrapper* GetParent() const;

	UFUNCTION(BlueprintPure, Category = "ProctreeNodeWrapper")
		UProctreeNodeWrapper* GetChild(int32 index) const;

	UFUNCTION(BlueprintPure, Category = "ProctreeNodeWrapper")
		TArray<UProctreeNodeWrapper*> GetChildren() const;

	UFUNCTION(BlueprintPure, Category = "ProctreeNodeWrapper")
		FProctreeNodeData GetNodeData() const;

	//Function Wrappers for FProctreeNode
	//These functions can run on any thread but should be thread safe in their modifications of the tree
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapper")
		UProctreeNodeWrapper* InsertNodeByPath(FString Path, FProctreeNodeRandomizationBounds RandBounds, bool Activate);

	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapper")
		UProctreeNodeWrapper* InsertNodeByPointAndDepth(FVector Point, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate);

	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapper")
		UProctreeNodeWrapper* InsertNodeByNormalizedPointAndDepth(FVector Point, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate);

		TSharedPtr<FProctreeNode> GetInternalNode() const;
		void SetInternalNode(TSharedPtr<FProctreeNode> inNode);

private:
	TSharedPtr<FProctreeNode> Node;
};

USTRUCT(BlueprintType)
struct PROCTREEPLUGIN_API FProctreeBulkNodeResult
{
	GENERATED_BODY()

public:
	FProctreeBulkNodeResult();
	~FProctreeBulkNodeResult();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
		UProctreeNodeWrapper* ReferenceNode;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
		TArray<UProctreeNodeWrapper*> ResultNodes;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
		int32 Depth = 0;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
		EProctreeNodeType Filter;
};
