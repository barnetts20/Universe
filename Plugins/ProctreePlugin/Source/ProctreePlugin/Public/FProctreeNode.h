#pragma once
#include "CoreMinimal.h"
#include "FProctreeStructures.h"
//This structure is for internal use only, any time this data needs to be exposed to BP it should be wrapped in a UProctreeNodeWrapper
struct PROCTREEPLUGIN_API FProctreeNode : public TSharedFromThis<FProctreeNode>
{

public:
    FProctreeNode();
    ~FProctreeNode();

    //Family
    TWeakPtr<FProctreeNode> Parent;
    TArray<TSharedPtr<FProctreeNode>> Children;

    //Non recursive data
    FProctreeNodeData NodeData;

    //Creation helper nodes, must be provided a randomization bounds used to proceduralize the voxel core data
    TSharedPtr<FProctreeNode> InsertNodeByPath(FString Path, FProctreeNodeRandomizationBounds RandBounds, bool Activate);
    TSharedPtr<FProctreeNode> CreateOrUpdateNodeByIndex(int32 Index, FVector Point, FProctreeNodeRandomizationBounds RandBounds, bool Activate);
    TSharedPtr<FProctreeNode> InsertNodeByPointAndDepth(TSharedPtr<FProctreeNode> Node, FVector Point, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate);
    TSharedPtr<FProctreeNode> InsertNodeByNormalizedPointAndDepth(TSharedPtr<FProctreeNode> Node, FVector Point, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate);
    TArray<TSharedPtr<FProctreeNode>> BulkInsertNodesByPointCloudAndDepth(TSharedPtr<FProctreeNode> Node, TArray<FVector> Points, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate);
    
    //Fetch methods 
    //This function should find activated nodes at a specific depth from the passed in node
    TArray<TSharedPtr<FProctreeNode>> FindNodesAtDepthFromNode(TSharedPtr<FProctreeNode> Node, int32 Depth, EProctreeNodeType Filter = EProctreeNodeType::All); //TODO: Implement voxel core enum type filtering
    //This function should find all activated nodes between the passed FProctreeNode and the depth, inclusive
    TArray<TSharedPtr<FProctreeNode>> FindNodesInDepthFromNode(TSharedPtr<FProctreeNode> Node, int32 Depth, EProctreeNodeType Filter = EProctreeNodeType::All); //TODO: Implement voxel core enum type filtering
    //This function should find all activated descendants of the reference node matching the filter type
    TArray<TSharedPtr<FProctreeNode>> FindAllActiveDescendants(TSharedPtr<FProctreeNode> Node, EProctreeNodeType Filter = EProctreeNodeType::All);
    //This function should return the node at a given depth containing the point regardless of activation or type, if there is no node at that depth it should return null
    TSharedPtr<FProctreeNode> FindNodeAtDepthByPosition(TSharedPtr<FProctreeNode> Node, FVector Position, int32 Depth);
    //This should return all neighbors within "Distance" cells at the same level as the passed in node, results include neighbors and original node
    TArray<TSharedPtr<FProctreeNode>> FindNearestNeighbors(TSharedPtr<FProctreeNode> Node, FVector StartPoint, int32 Depth, int32 Iterations,  EProctreeNodeType Filter = EProctreeNodeType::All);
    //This should return all existing nodes along a ray at the same depth as the passed in node
    TArray<TSharedPtr<FProctreeNode>> FindLineTraceNodes(TSharedPtr<FProctreeNode> Node, FVector StartPoint, FVector Direction, int32 Depth, int32 Iterations, EProctreeNodeType Filter = EProctreeNodeType::All);
    //This should crawl the parent chain to find the root node
    TSharedPtr<FProctreeNode> GetRoot();
};