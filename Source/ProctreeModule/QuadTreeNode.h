// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "PlanetSharedStructs.h"
#include "FastNoise/FastNoise.h"

#include <Mesh/RealtimeMeshSimpleData.h>
#include "PlanetNoise.h"

class APlanetActor;
class URealtimeMeshSimple; // Forward declaration

class PROCTREEMODULE_API QuadTreeNode : public TSharedFromThis<QuadTreeNode>
{
public:
	QuadTreeNode(
		APlanetActor* InParentActor,
		TSharedPtr<INoiseGenerator> InNoiseGen,
		FQuadIndex InIndex,
		int InMinDepth,
		int InMaxDepth,
		FCubeTransform InFaceTransform,
		FVector InCenter, 
		float InSize, 
		float InRadius);
	~QuadTreeNode();

	//Mesh State Data
	bool IsLeaf() const;
	int GetDepth() const; //This also represents the current LOD level
	
	FRealtimeMeshLODKey lodKey = FRealtimeMeshLODKey::FRealtimeMeshLODKey(0);
	FRealtimeMeshSectionGroupKey landGroupKeyInner = FRealtimeMeshSectionGroupKey::Create(lodKey, FName("land_mesh_internal"));
	FRealtimeMeshSectionGroupKey seaGroupKeyInner = FRealtimeMeshSectionGroupKey::Create(lodKey, FName("sea_mesh_internal"));
	FRealtimeMeshSectionKey landSectionKeyInner = FRealtimeMeshSectionKey::CreateForPolyGroup(landGroupKeyInner, 0);
	FRealtimeMeshSectionKey seaSectionKeyInner = FRealtimeMeshSectionKey::CreateForPolyGroup(seaGroupKeyInner, 1);

	//Structure modification functions

	void TryMerge();

	//LOD and Mesh Update Functions
	void UpdateLod();

	void CollectLeaves(TArray<TSharedPtr<QuadTreeNode>>& LeafNodes);

	FMeshStreamBuilders InitializeStreamBuilders(FRealtimeMeshStreamSet& inMeshStream, int Resolution);

	FVector GetFacePoint(float step, double x, double y);
	FColor EncodeDepthColor(float depth);

	int GenerateVertex(double x, double y, double step, FMeshStreamBuilders& landBuilders, FMeshStreamBuilders& seaBuilders);

	void GenerateMeshData();
	void InitializeChunk();
	void SetChunkVisibility(bool inVisibility);
	void DestroyChunk();

	void Split(TSharedPtr<QuadTreeNode> inNode);
	//TFuture<void> AsyncSplit(TSharedPtr<QuadTreeNode> inNode);
	void Merge(TSharedPtr<QuadTreeNode> inNode);
	//TFuture<void> AsyncMerge(TSharedPtr<QuadTreeNode> inNode);
	//External References	
	APlanetActor* ParentActor;
	TSharedPtr<INoiseGenerator> NoiseGen;

	//Initialization Data
	//FString Id;
	FQuadIndex Index;
	TArray<uint8> Path;
	EFaceDirection FaceDirection;
	FCubeTransform FaceTransform;
	FVector Center;
	//int Resolution = 14;
	double SphereRadius;
	double Size;

	int MinDepth;
	int MaxDepth;
	double SeaLevel;

	TMap<EdgeOrientation, bool> NeighborLodChangeMap;
	TMap<EdgeOrientation, FVector> NeighborVirtualCentroids;
	TMap<EdgeOrientation, TArray<FVector>> NeighborEdgeCorners;
	//Derived Data	
	double HalfSize;
	double QuarterSize;

	//State
	bool CanMerge = false;
	bool IsInitialized = false;
	bool IsDirty = false;
	bool LastRenderedState = false;
	bool RenderSea = false;
	//Bounds Data
	FVector LandCentroid;
	FVector SeaCentroid;
	FVector SphereCentroid;
	double MaxNodeRadius;

	double MinLandRadius;
	double MaxLandRadius;

	//Populate in first mesh pass
	FVector VirtualNeighborCentroids[2] = { FVector::ZeroVector, FVector::ZeroVector };
	//Family
	TWeakPtr<QuadTreeNode> Parent;
	TArray<TSharedPtr<QuadTreeNode>> Children;
	ENeighborState CurrentNeighborState = ENeighborState::NONE;
	//Payload
	FRealtimeMeshStreamSet LandMeshStreamInner;
	FRealtimeMeshStreamSet SeaMeshStreamInner;
	FRealtimeMeshStreamSet LandMeshStreamEdge;
	FRealtimeMeshStreamSet SeaMeshStreamEdge;
	URealtimeMeshComponent* ChunkComponent;
	URealtimeMeshSimple* RtMesh;
	void TrySetLod();
	bool UpdateNeighborLods();
	bool ShouldSplit(FVector centroid, FVector lastCamPos, double fov, double k);

protected:
	FRWLock MeshDataLock;
	void RecurseRemoveChildren(TSharedPtr<QuadTreeNode> InNode);
	void RecurseUpdateLod(TWeakPtr<QuadTreeNode> InNode);
};