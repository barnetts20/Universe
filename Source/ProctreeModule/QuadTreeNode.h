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
		FCubeTransform InFaceTransform,
		FQuadIndex InIndex,
		FVector InCenter, 
		float InSize, 
		float InRadius,
		int InMinDepth,
		int InMaxDepth
	);

	//External References	
	APlanetActor* ParentActor;
	TSharedPtr<INoiseGenerator> NoiseGen;

	//Family & Neighbor Data
	TWeakPtr<QuadTreeNode> Parent;
	TArray<TSharedPtr<QuadTreeNode>> Children;
	int NeighborLods[4] = { 0,0,0,0 };
	
	//Initialization Data
	FQuadIndex Index;
	FCubeTransform FaceTransform;

	int MinDepth;
	int MaxDepth;

	FVector Center;
	FVector CenterOnSphere;
	double SphereRadius;
	double Size;
	double SeaLevel;
	double HalfSize;
	double QuarterSize;

	//State
	bool HasGenerated = false;
	bool IsRestructuring = false;
	bool CanMerge = false;
	bool IsInitialized = false;
	bool LastRenderedState = false;
	bool RenderSea = false;

	//Computed Bound/Centroid Data
	FVector LandCentroid;
	FVector SeaCentroid;
	FVector SphereCentroid;
	double MaxNodeRadius;
	double MinLandRadius;
	double MaxLandRadius;

	//Mesh State Data
	int FaceResolution;
	TArray<FVector> LandVertices;
	TArray<FVector3f> LandNormals;
	TArray<FColor> LandColors;
	TArray<FVector> SeaVertices;
	TArray<FVector3f> SeaNormals;
	TArray<FColor> SeaColors;
	TArray<FVector2f> TexCoords;
	TArray<FIndex3UI> AllTriangles;
	TArray<int> PatchTriangleIndices;

	//RT Mesh
	//Mesh Keys
	FRealtimeMeshLODKey LodKey = FRealtimeMeshLODKey::FRealtimeMeshLODKey(0);
	FRealtimeMeshSectionGroupKey LandGroupKeyInner = FRealtimeMeshSectionGroupKey::Create(LodKey, "land_inner");
	FRealtimeMeshSectionGroupKey SeaGroupKeyInner = FRealtimeMeshSectionGroupKey::Create(LodKey, "sea_inner");
	FRealtimeMeshSectionKey LandSectionKeyInner = FRealtimeMeshSectionKey::CreateForPolyGroup(LandGroupKeyInner, 0);
	FRealtimeMeshSectionKey SeaSectionKeyInner = FRealtimeMeshSectionKey::CreateForPolyGroup(SeaGroupKeyInner, 1);
	FRealtimeMeshSectionGroupKey LandGroupKeyEdge = FRealtimeMeshSectionGroupKey::Create(LodKey, "land_edge");
	FRealtimeMeshSectionGroupKey SeaGroupKeyEdge = FRealtimeMeshSectionGroupKey::Create(LodKey, "sea_edge");
	FRealtimeMeshSectionKey LandSectionKeyEdge = FRealtimeMeshSectionKey::CreateForPolyGroup(LandGroupKeyEdge, 0);
	FRealtimeMeshSectionKey SeaSectionKeyEdge = FRealtimeMeshSectionKey::CreateForPolyGroup(SeaGroupKeyEdge, 1);
	
	//Streams, Chunks, & RT Mesh
	FRealtimeMeshStreamSet LandMeshStreamInner;
	FRealtimeMeshStreamSet SeaMeshStreamInner;
	FRealtimeMeshStreamSet LandMeshStreamEdge;
	FRealtimeMeshStreamSet SeaMeshStreamEdge;
	URealtimeMeshComponent* ChunkComponent;
	URealtimeMeshSimple* RtMesh;
	
	//LOD Update Functions
	void UpdateLod();
	void UpdateNeighbors();//Recursively propagates neighbor updates 
	bool CheckNeighbors(); //Checks the relevant neighbors for a node
	void TrySetLod();
	void TryMerge();
	bool ShouldMerge(double d2, double parentSize, double fov, double k);
	static void Merge(TSharedPtr<QuadTreeNode> inNode);
	bool ShouldSplit(double d1, double fov, double k);
	static void Split(TSharedPtr<QuadTreeNode> inNode);

	//Data checks, leaf collection
	bool IsLeaf() const;
	int GetDepth() const; //This also represents the current LOD level
	static void CollectLeaves(TSharedPtr<QuadTreeNode> InNode, TArray<TSharedPtr<QuadTreeNode>>& LeafNodes);

	//Chunk lifecycle
	void InitializeChunk();
	void SetChunkVisibility(bool inVisibility);
	void DestroyChunk();

	//Mesh Generation
	FMeshStreamBuilders InitializeStreamBuilders(FRealtimeMeshStreamSet& inMeshStream, int Resolution);
	FColor EncodeDepthColor(float depth);
	FVector GetFacePoint(float step, double x, double y);
	int VisibleVertexCount = 0;
	int GenerateVertex(double x, double y, double step);
	void RemoveChildren(TSharedPtr<QuadTreeNode> InNode);
	bool isPatchDirty = false;
	bool isEdgeDirty = false;

	void UpdateEdgeMeshBuffer();
	void UpdatePatchMeshBuffer();
	void GenerateMeshData();
	void UpdateMesh(); 
	void UpdateAllMesh();
protected:
	FRWLock MeshDataLock;
};