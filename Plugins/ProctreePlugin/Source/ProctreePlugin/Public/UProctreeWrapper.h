#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UProctreeNodeWrapper.h"
#include "FProctreeStructures.h"
#include "FProctreeNode.h"
#include "UProctreeWrapper.generated.h"
/**
 *
 */
UCLASS(BlueprintType)
class PROCTREEPLUGIN_API UProctreeWrapper : public UObject
{
	GENERATED_BODY()

public:
	UProctreeWrapper();
	~UProctreeWrapper();

	UFUNCTION(BlueprintCallable, Category = "Proctree")
		void Initialize(int32 SizeMultiplier, int32 MaxDepth);

	//Getters
	UFUNCTION(BlueprintPure, Category = "Proctree")
		UProctreeNodeWrapper* GetRoot() const;

	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapper")
		FProctreeBulkNodeResult BulkInsertNodesByPointCloudAndDepth(UProctreeNodeWrapper* Node, TArray<FVector> Points, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate);
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapper")
		FProctreeBulkNodeResult FindNodesAtDepthFromNode(UProctreeNodeWrapper* Node, int32 Depth, EProctreeNodeType Filter = EProctreeNodeType::All);
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapper")
		FProctreeBulkNodeResult FindNodesInDepthFromNode(UProctreeNodeWrapper* Node, int32 Depth, EProctreeNodeType Filter = EProctreeNodeType::All);
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapper")
		FProctreeBulkNodeResult FindNearestNeighbors(UProctreeNodeWrapper* RootNode, FVector Position, int32 Depth, int32 Iterations, EProctreeNodeType Filter = EProctreeNodeType::All);
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapper")
		FProctreeBulkNodeResult FindNodesInLineTrace(UProctreeNodeWrapper* RootNode, FVector Position, FVector Direction, int32 Depth, int32 Iterations, EProctreeNodeType Filter = EProctreeNodeType::All);
	
	//These functions will be ran on a seperate thread
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapperAsync")
		void AsyncBulkInsertNodesByPointCloudAndDepth(UProctreeNodeWrapper* Node, TArray<FVector> Points, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate);
	//This function should find activated nodes at a specific depth from the passed in node
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapperAsync")
		void AsyncFindNodesAtDepthFromNode(UProctreeNodeWrapper* Node, int32 Depth, EProctreeNodeType Filter = EProctreeNodeType::All);
	//This function should find all activated nodes between the passed FProctreeNode and the depth, inclusive
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapperAsync")
		void AsyncFindNodesInDepthFromNode(UProctreeNodeWrapper* Node, int32 Depth, EProctreeNodeType Filter = EProctreeNodeType::All);
	
	//This function should find all nodes within a certain distance of the passed in node
	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapperAsync")
		void AsyncFindNearestNeighbors(UProctreeNodeWrapper* RootNode, FVector Position, int32 Depth, int32 Iterations, EProctreeNodeType Filter = EProctreeNodeType::All);

	UFUNCTION(BlueprintCallable, Category = "ProctreeNodeWrapperAsync")
		void AsyncFindNodesInLineTrace(UProctreeNodeWrapper* RootNode, FVector Position, FVector Direction, int32 Depth, int32 Iterations, EProctreeNodeType Filter = EProctreeNodeType::All);

	TSharedPtr<FProctreeNode> RootNode;
	int32 MaximumDepth = 30;
	int32 LeafSizeMultiplier = 1;
	const int32 LeafBaseSize = 2;

	// Define your custom event delegate with one parameter
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBulkInsertComplete, const FProctreeBulkNodeResult, NodeResult);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindAtDepthComplete, const FProctreeBulkNodeResult, NodeResult);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindInDepthComplete, const FProctreeBulkNodeResult, NodeResult);
	
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNearestNeighborComplete, const FProctreeBulkNodeResult, NodeResult);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLineTraceComplete, const FProctreeBulkNodeResult, NodeResult);
	// Expose the event delegate as a property
	UPROPERTY(BlueprintAssignable, Category = "ProctreeNodeWrapper")
		FBulkInsertComplete BulkInsertBroadcaster;

	UPROPERTY(BlueprintAssignable, Category = "ProctreeNodeWrapper")
		FFindAtDepthComplete FindAtDepthBroadcaster;

	UPROPERTY(BlueprintAssignable, Category = "ProctreeNodeWrapper")
		FFindInDepthComplete FindInDepthBroadcaster;

	UPROPERTY(BlueprintAssignable, Category = "ProctreeNodeWrapper")
		FNearestNeighborComplete FindNearestNeighborBroadcaster;

	UPROPERTY(BlueprintAssignable, Category = "ProctreeNodeWrapper")
		FLineTraceComplete FindLineTraceBroadcaster;
};