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
	

	FRealtimeMeshLODKey LodKey = FRealtimeMeshLODKey::FRealtimeMeshLODKey(0);
	FRealtimeMeshSectionGroupKey LandGroupKeyInner = FRealtimeMeshSectionGroupKey::Create(LodKey, FName("land_mesh_internal"));
	FRealtimeMeshSectionGroupKey SeaGroupKeyInner = FRealtimeMeshSectionGroupKey::Create(LodKey, FName("sea_mesh_internal"));
	FRealtimeMeshSectionKey LandSectionKeyInner = FRealtimeMeshSectionKey::CreateForPolyGroup(LandGroupKeyInner, 0);
	FRealtimeMeshSectionKey SeaSectionKeyInner = FRealtimeMeshSectionKey::CreateForPolyGroup(SeaGroupKeyInner, 1);

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

	bool IsRestructuring = false;
	static void Split(TSharedPtr<QuadTreeNode> inNode);
	static void Merge(TSharedPtr<QuadTreeNode> inNode);

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
	FVector CenterOnSphere;
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
	bool IsDirty = true;
	bool LastRenderedState = false;
	bool RenderSea = false;
	//Bounds Data
	FVector LandCentroid;
	FVector SeaCentroid;
	FVector SphereCentroid;
	double MaxNodeRadius;

	double MinLandRadius;
	double MaxLandRadius;

	//Family
	TWeakPtr<QuadTreeNode> Parent;
	TArray<TSharedPtr<QuadTreeNode>> Children;
	int NeighborLods[4] = {0,0,0,0};
	//Payload
	InternalMeshData NodeMeshData;

	FRealtimeMeshStreamSet LandMeshStreamInner;
	FRealtimeMeshStreamSet SeaMeshStreamInner;
	FRealtimeMeshStreamSet LandMeshStreamEdge;
	FRealtimeMeshStreamSet SeaMeshStreamEdge;
	URealtimeMeshComponent* ChunkComponent;
	URealtimeMeshSimple* RtMesh;
	void TrySetLod();
	void UpdateNeighborEdge(EdgeOrientation InEdge, int InLod);
	void UpdateNeighborLod(int InLod);
	void UpdateMesh();
	void LogNeighborLods();

	bool ShouldSplit(FVector centroid, FVector lastCamPos, double fov, double k);

protected:
	FRWLock MeshDataLock;
	void RecurseRemoveChildren(TSharedPtr<QuadTreeNode> InNode);
	void RecurseUpdateLod(TWeakPtr<QuadTreeNode> InNode);
};

struct PROCTREEMODULE_API InternalMeshData {
	InternalMeshData(int32 InResolution) {
		ModifiedResolution = InResolution + 2;
	};

	int ModifiedResolution;
	TArray<FVector> VertexGrid;
	TArray<FIndex3UI> BaseTriangles;
	TMap<uint8, int32> EdgeTriangles;
	///TMap<uint8, int32> CornerTriangles; //TOP LEFT, TOP RIGHT, BOTTOM RIGHT, BOTTOM LEFT

	FVector& GetVertex(int32 InX, int32 InY) {
		int32 X = InX + 1;
		int32 Y = InY + 1;
		check(X >= 0 && X < ModifiedResolution && Y >= 0 && Y < ModifiedResolution);
		return VertexGrid[Y * ModifiedResolution + X];
	}
};