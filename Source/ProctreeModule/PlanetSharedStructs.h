#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Mesh/RealtimeMeshSimpleData.h>
#include "PlanetSharedStructs.generated.h"

class QuadTreeNode;

UENUM(BlueprintType)
enum class EdgeOrientation : uint8 {
	LEFT,
	TOP,
	RIGHT,
	BOTTOM
};

UENUM(BlueprintType)
enum class EFaceDirection : uint8 {
	XPositive UMETA(DisplayName = "X+"),
	XNegative UMETA(DisplayName = "X-"),
	YPositive UMETA(DisplayName = "Y+"),
	YNegative UMETA(DisplayName = "Y-"),
	ZPositive UMETA(DisplayName = "Z+"),
	ZNegative UMETA(DisplayName = "Z-")
};

UENUM(BlueprintType)
enum  class EMeshUpdateType : uint8 {
	ADD UMETA(DisplayName = "Add"),
	REMOVE UMETA(DisplayName = "Remove"),
	UPDATE UMETA(DisplayName = "Update"),
	SHOW UMETA(DisplayName = "Show"),
	HIDE UMETA(DisplayName = "Hide")
};

USTRUCT(BlueprintType)
struct FIntBounds {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int max;
};

USTRUCT(BlueprintType)
struct FDoubleBounds {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double max;
};

USTRUCT(BlueprintType)
struct FVector4Bounds {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 max;
};

USTRUCT(BlueprintType)
struct FVectorBounds {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector max;
};

USTRUCT(BlueprintType)
struct FPlanetNoiseGeneratorParameters {
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int noiseType = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int noiseSeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double noiseScale = .1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentFaultScale = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 continentFaultDomainOffset = FVector4(0, 0, 0, 0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentFaultDistortionScale = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentFaultDistortionFrequency = .25;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentVarianceScale = .5;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentVarianceMultiplier = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 continentVarianceOffset = FVector4(0, 0, 0, 0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mountainRidgeExponent = 16;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double mountainRidgeMultiplier = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4 mountainRidgeOffset = FVector4(0, 0, 0, 0);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mountainPeakExponent = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double mountainPeakScale = 32;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double mountainPeakMultiplier = 1;
};

USTRUCT(BlueprintType)
struct FPlanetNoiseGeneratorParameters2 {
	GENERATED_BODY()
public:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int noiseSeed = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double noiseScale = .03;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double planetShapeScale = .35;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector planetShapeOffset = FVector(-100, -100, -100);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentFaultScale = .75;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector continentFaultOffset = FVector(100, 100, 100);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double planetShapeMagnitude = .5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentFaultMagnitude = 1.0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double tectonicBlend = .5;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double seaLevel = 1; //0=no ocean, 1=50/50, 2 = all ocean
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentMinElevation = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double continentMaxElevation = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mountainRidgeExponent = 8;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector mountainRidgeOffset = FVector(2, 1, 0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double mountainRidgeFloor = .25;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int mountainPeakExponent = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double mountainPeakScale = 64;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double mountainMultiplier = .15;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//double craterScale = .15;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//FVector craterOffset = FVector(0, 0, 0);
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//double craterEdge = 3;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//double craterBase = -1;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//double craterFloor = .25;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int craterOctaves1 = 3;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//int craterOctaves2 = 8;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//double craterMultiplier = 0;
};

USTRUCT(BlueprintType)
struct FPlanetNoiseGeneratorBounds2 {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds noiseScaleBounds = { .01, .04 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds planetShapeScaleBounds = { .5, 2 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVectorBounds planetShapeOffsetBounds = { FVector(-1000, -1000, -1000), FVector(1000, 1000, 1000) };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentFaultScaleBounds = { .5, 4 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVectorBounds continentFaultOffsetBounds = { FVector(-1000, -1000, -1000), FVector(1000, 1000, 1000) };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds planetShapeMagnitudeBounds = {.5, 1};
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentFaultMagnitudeBounds = { .5, 1 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds tectonicBlendBounds = { .1, .9 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds seaLevelBounds = { 0.5,1.5 }; //0=no ocean, 1=50/50, 2 = all ocean???
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentMinElevationBounds = { -1, -.5 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentMaxElevationBounds = { .5, 1 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntBounds mountainRidgeExponentBounds = { 6, 12 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVectorBounds mountainRidgeOffsetBounds = { FVector(-4, -4, 0), FVector(4, 4, 0) };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds mountainRidgeFloorBounds = { .25,.5 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntBounds mountainPeakExponentBounds = { 2,4 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds mountainPeakScaleBounds = { 32,96 };
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds mountainMultiplierBounds = { 1,2 };
};

USTRUCT(BlueprintType)
struct FPlanetParameters {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int seed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double planetRadius = 20000000.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	double planetDensity = 1.0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FPlanetNoiseGeneratorParameters2 noiseParameters;
};

USTRUCT(BlueprintType)
struct FPlanetNoiseGeneratorBounds {
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int> noiseTypes = {0,1,2};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntBounds seedBounds = { MIN_int32, MAX_int32 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds noiseScaleBounds = { .005, .05 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentFaultScaleBounds = { .25, 2 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4Bounds continentFaultDomainOffsetBounds = { FVector4(-1000, -1000, -1000, -1000), FVector4(1000, 1000, 1000, 1000) };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentFaultDistortionScaleBounds = { 4, 12 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentFaultDistortionFrequencyBounds = { .0, .5 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentVarianceScaleBounds = { .25, 2 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds continentVarianceMultiplierBounds = { 1, 1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4Bounds continentVarianceOffsetBounds = { FVector4(-1000, -1000, -1000, -1000), FVector4(1000, 1000, 1000, 1000) };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntBounds mountainRidgeExponentBounds = { 6, 12 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds mountainRidgeMultiplierBounds = { 1, 1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector4Bounds mountainRidgeOffsetBounds = { FVector4(-10, -10, -10, -10), FVector4(10, 10, 10, 10) };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FIntBounds mountainPeakExponentBounds = { 3, 6 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds mountainPeakScaleBounds = { 16, 64 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDoubleBounds mountainPeakMultiplierBounds = { 1, 1 };
};

struct PROCTREEMODULE_API FMeshStreamBuilders {
	TRealtimeMeshStreamBuilder<FVector, FVector3f>* PositionBuilder;
	TRealtimeMeshStreamBuilder<FRealtimeMeshTangentsHighPrecision, FRealtimeMeshTangentsNormalPrecision>* TangentBuilder;
	TRealtimeMeshStreamBuilder<FVector2f, FVector2DHalf>* TexCoordsBuilder;
	TRealtimeMeshStreamBuilder<FColor>* ColorBuilder;
	TRealtimeMeshStreamBuilder<TIndex3<uint32>>* TrianglesBuilder;
	TRealtimeMeshStreamBuilder<uint32, uint16>* PolygroupsBuilder;

	int32 NumVerts;
	int32 NumTriangles;
};

struct PROCTREEMODULE_API FMeshUpdateStatus {
	FString NodeId;
	bool RenderStatus;
};

struct PROCTREEMODULE_API FMeshUpdateData {
	double UpdateTime;
	FString NodeID;
	int32 Priority;
	int32 LodLevel;
	EMeshUpdateType UpdateType;
	//TSharedPtr<FRealtimeMeshSimpleMeshData> MeshData;
	//TRealtimeMeshBuilderLocal:: MeshBuilder;


};

struct PROCTREEMODULE_API FMeshUpdateDataBatch {
	double UpdateTimestamp;
	int Priority;
	TArray<TSharedPtr<FMeshUpdateData>> MeshAdds;
	TArray<TSharedPtr<FMeshUpdateData>> MeshRemoves;
};

enum PROCTREEMODULE_API ENeighborState
{
	NONE = 0,
	N0 = 1,
	N1 = 2,
	ALL = 3
};

enum PROCTREEMODULE_API EChildPosition {
	BOTTOM_LEFT = 0,
	TOP_LEFT = 1,
	BOTTOM_RIGHT = 2,
	TOP_RIGHT = 3
};


struct PROCTREEMODULE_API LoopBounds {
	LoopBounds() {};
	LoopBounds(int InStart, int InEnd) : StartIdx(InStart), EndIdx(InEnd) {}
	int StartIdx;
	int EndIdx;
};
struct PROCTREEMODULE_API LoopBoundsPair {
	LoopBoundsPair() {};
	LoopBoundsPair(int InStartX, int InEndX, int InStartY, int InEndY) {
		xBounds = LoopBounds(InStartX, InEndX);
		yBounds = LoopBounds(InStartY, InEndY);
	}
	LoopBounds xBounds;
	LoopBounds yBounds;
};

struct PROCTREEMODULE_API FVertexCoord
{
	FVertexCoord() : X(0), Y(0) {}
	FVertexCoord(int InX, int InY) : X(InX), Y(InY) {}

	int X;
	int Y;
};

struct PROCTREEMODULE_API FMeshTemplate : public TSharedFromThis<FMeshTemplate> {
	TArray<FIntPoint> Vertices;
	TArray<int> TriangleIndices;
	TArray<FIndex3UI> Triangles;
};
struct PROCTREEMODULE_API FMeshTemplateTables {
	FMeshTemplateTables() {};
	FMeshTemplateTables(int InResolution) {
		InnerChunkLoopBounds.Add(EChildPosition::BOTTOM_LEFT, LoopBoundsPair(1, InResolution, 0, InResolution - 1));
		InnerChunkLoopBounds.Add(EChildPosition::TOP_LEFT, LoopBoundsPair(1, InResolution, 1, InResolution));
		InnerChunkLoopBounds.Add(EChildPosition::BOTTOM_RIGHT, LoopBoundsPair(0, InResolution - 1, 0, InResolution - 1));
		InnerChunkLoopBounds.Add(EChildPosition::TOP_RIGHT, LoopBoundsPair(0, InResolution, 1, InResolution));

		CornerIndexMap.Add(EChildPosition::BOTTOM_LEFT, { 0, InResolution});
		CornerIndexMap.Add(EChildPosition::TOP_LEFT, { 0, 0 });
		CornerIndexMap.Add(EChildPosition::BOTTOM_RIGHT, { InResolution, InResolution });
		CornerIndexMap.Add(EChildPosition::TOP_RIGHT, { InResolution, 0 });

		EdgeChunkLoopBounds.Add(EChildPosition::BOTTOM_LEFT, LoopBoundsPair(0, InResolution + 1,0, InResolution)); //x loop, y = n-1 | y loop, x=0   
		EdgeChunkLoopBounds.Add(EChildPosition::TOP_LEFT, LoopBoundsPair(0, InResolution + 1,1, InResolution + 1));    //x loop, y = 0   | y loop, x=0
		EdgeChunkLoopBounds.Add(EChildPosition::BOTTOM_RIGHT, LoopBoundsPair(0, InResolution + 1,0, InResolution));//x loop, y = n-1 | y loop, x=n-1
		EdgeChunkLoopBounds.Add(EChildPosition::TOP_RIGHT, LoopBoundsPair(0, InResolution + 1,1, InResolution + 1));   //x loop, y = 0   | y loop, x=n-1
		
		EdgeChunkEdges.Add(EChildPosition::BOTTOM_LEFT, LoopBounds(0, InResolution)); //x loop, y = n-1 | y loop, x=0   
		EdgeChunkEdges.Add(EChildPosition::TOP_LEFT, LoopBounds(0, 0));    //x loop, y = 0   | y loop, x=0
		EdgeChunkEdges.Add(EChildPosition::BOTTOM_RIGHT, LoopBounds(InResolution, InResolution));//x loop, y = n-1 | y loop, x=n-1
		EdgeChunkEdges.Add(EChildPosition::TOP_RIGHT, LoopBounds(InResolution, 0));   //x loop, y = 0   | y loop, x=n-1

		Initialize(InResolution);
	};

	TMap<EChildPosition, FVertexCoord> CornerIndexMap;
	TMap<EChildPosition, LoopBounds> EdgeChunkEdges;

	TMap<EChildPosition, LoopBoundsPair> EdgeChunkLoopBounds;
	TMap<EChildPosition, LoopBoundsPair> InnerChunkLoopBounds;
	TMap<EChildPosition, TSharedPtr<FMeshTemplate>> InnerChunkMap;
	TMap<EChildPosition, TMap<ENeighborState, TSharedPtr<FMeshTemplate>>> EdgeChunkMap;
	void GenerateInnerChunkTemplates(int Resolution) {
		// Clear any existing data and initialize templates
		for (int c = 0; c < 4; c++) {
			EChildPosition cp = EChildPosition(c);
			InnerChunkMap.FindOrAdd(cp, MakeShared<FMeshTemplate>());
			EdgeChunkMap.FindOrAdd(cp).Add(ENeighborState::ALL, MakeShared<FMeshTemplate>());
			EdgeChunkMap.FindOrAdd(cp).Add(ENeighborState::N0, MakeShared<FMeshTemplate>());
			EdgeChunkMap.FindOrAdd(cp).Add(ENeighborState::N1, MakeShared<FMeshTemplate>());
			EdgeChunkMap.FindOrAdd(cp).Add(ENeighborState::NONE, MakeShared<FMeshTemplate>());
		}

		// Function to determine if a quad touches the parent-facing edge
		auto QuadTouchesParentEdge = [Resolution](int x, int y, EChildPosition ChildPos) -> bool {
			switch (ChildPos) {
			case TOP_LEFT:
				return x == 0 || y == 0;
			case TOP_RIGHT:
				return x == Resolution - 1 || y == 0;
			case BOTTOM_LEFT:
				return x == 0 || y == Resolution - 1;
			case BOTTOM_RIGHT:
				return x == Resolution - 1 || y == Resolution - 1;
			default:
				return false;
			}
			};

		// Process each child position
		for (int c = 0; c < 4; c++) {
			EChildPosition cp = EChildPosition(c);
			auto& InnerChunkMesh = InnerChunkMap[cp];
			auto& EdgeChunkMesh0 = *EdgeChunkMap.Find(cp)->Find(ENeighborState::NONE);
			auto& EdgeChunkMesh1 = *EdgeChunkMap.Find(cp)->Find(ENeighborState::N0);
			auto& EdgeChunkMesh2 = *EdgeChunkMap.Find(cp)->Find(ENeighborState::N1);
			auto& EdgeChunkMesh3 = *EdgeChunkMap.Find(cp)->Find(ENeighborState::ALL);

			int ActualResolution = Resolution + 1;

			// First, create all possible vertices in a single array
			TArray<FIntPoint> AllPoints;
			for (int y = 0; y < ActualResolution; y++) {
				for (int x = 0; x < ActualResolution; x++) {
					AllPoints.Add(FIntPoint(x, y));
				}
			}

			// Maps to track indices in each chunk
			TMap<int, int> InnerVertexMap;  // Global index -> Inner index
			TMap<int, int> EdgeVertexMap0;  // Global index -> Edge0 index
			TMap<int, int> EdgeVertexMap1;  // Global index -> Edge1 index
			TMap<int, int> EdgeVertexMap2;  // Global index -> Edge2 index
			TMap<int, int> EdgeVertexMap3;  // Global index -> Edge3 index

			// Process quads and generate triangles
			for (int y = 0; y < Resolution; y++) {
				for (int x = 0; x < Resolution; x++) {
					// Calculate global indices for the quad corners
					int topLeft = y * ActualResolution + x;
					int topRight = topLeft + 1;
					int bottomLeft = (y + 1) * ActualResolution + x;
					int bottomRight = bottomLeft + 1;

					// Determine if this quad belongs to edge or inner mesh
					bool isEdgeQuad = QuadTouchesParentEdge(x, y, cp);

					if (isEdgeQuad) {
						// This quad belongs to edge chunks
						// Add vertices to edge meshes (if not already added)

						// For EdgeChunkMesh0
						int tlIdx0, trIdx0, blIdx0, brIdx0;
						if (EdgeVertexMap0.Contains(topLeft)) {
							tlIdx0 = EdgeVertexMap0[topLeft];
						}
						else {
							tlIdx0 = EdgeChunkMesh0->Vertices.Num();
							EdgeChunkMesh0->Vertices.Add(AllPoints[topLeft]);
							EdgeVertexMap0.Add(topLeft, tlIdx0);
						}

						if (EdgeVertexMap0.Contains(topRight)) {
							trIdx0 = EdgeVertexMap0[topRight];
						}
						else {
							trIdx0 = EdgeChunkMesh0->Vertices.Num();
							EdgeChunkMesh0->Vertices.Add(AllPoints[topRight]);
							EdgeVertexMap0.Add(topRight, trIdx0);
						}

						if (EdgeVertexMap0.Contains(bottomLeft)) {
							blIdx0 = EdgeVertexMap0[bottomLeft];
						}
						else {
							blIdx0 = EdgeChunkMesh0->Vertices.Num();
							EdgeChunkMesh0->Vertices.Add(AllPoints[bottomLeft]);
							EdgeVertexMap0.Add(bottomLeft, blIdx0);
						}

						if (EdgeVertexMap0.Contains(bottomRight)) {
							brIdx0 = EdgeVertexMap0[bottomRight];
						}
						else {
							brIdx0 = EdgeChunkMesh0->Vertices.Num();
							EdgeChunkMesh0->Vertices.Add(AllPoints[bottomRight]);
							EdgeVertexMap0.Add(bottomRight, brIdx0);
						}

						// Add triangles to EdgeChunkMesh0 with CLOCKWISE winding
						EdgeChunkMesh0->Triangles.Add(FIndex3UI(tlIdx0, trIdx0, blIdx0));  // Changed order
						EdgeChunkMesh0->TriangleIndices.Add(tlIdx0);
						EdgeChunkMesh0->TriangleIndices.Add(trIdx0);
						EdgeChunkMesh0->TriangleIndices.Add(blIdx0);

						EdgeChunkMesh0->Triangles.Add(FIndex3UI(trIdx0, brIdx0, blIdx0));  // Changed order
						EdgeChunkMesh0->TriangleIndices.Add(trIdx0);
						EdgeChunkMesh0->TriangleIndices.Add(brIdx0);
						EdgeChunkMesh0->TriangleIndices.Add(blIdx0);

						// Repeat for EdgeChunkMesh1
						int tlIdx1, trIdx1, blIdx1, brIdx1;
						if (EdgeVertexMap1.Contains(topLeft)) {
							tlIdx1 = EdgeVertexMap1[topLeft];
						}
						else {
							tlIdx1 = EdgeChunkMesh1->Vertices.Num();
							EdgeChunkMesh1->Vertices.Add(AllPoints[topLeft]);
							EdgeVertexMap1.Add(topLeft, tlIdx1);
						}

						if (EdgeVertexMap1.Contains(topRight)) {
							trIdx1 = EdgeVertexMap1[topRight];
						}
						else {
							trIdx1 = EdgeChunkMesh1->Vertices.Num();
							EdgeChunkMesh1->Vertices.Add(AllPoints[topRight]);
							EdgeVertexMap1.Add(topRight, trIdx1);
						}

						if (EdgeVertexMap1.Contains(bottomLeft)) {
							blIdx1 = EdgeVertexMap1[bottomLeft];
						}
						else {
							blIdx1 = EdgeChunkMesh1->Vertices.Num();
							EdgeChunkMesh1->Vertices.Add(AllPoints[bottomLeft]);
							EdgeVertexMap1.Add(bottomLeft, blIdx1);
						}

						if (EdgeVertexMap1.Contains(bottomRight)) {
							brIdx1 = EdgeVertexMap1[bottomRight];
						}
						else {
							brIdx1 = EdgeChunkMesh1->Vertices.Num();
							EdgeChunkMesh1->Vertices.Add(AllPoints[bottomRight]);
							EdgeVertexMap1.Add(bottomRight, brIdx1);
						}

						// Add triangles to EdgeChunkMesh1 with CLOCKWISE winding
						EdgeChunkMesh1->Triangles.Add(FIndex3UI(tlIdx1, trIdx1, blIdx1));  // Changed order
						EdgeChunkMesh1->TriangleIndices.Add(tlIdx1);
						EdgeChunkMesh1->TriangleIndices.Add(trIdx1);
						EdgeChunkMesh1->TriangleIndices.Add(blIdx1);

						EdgeChunkMesh1->Triangles.Add(FIndex3UI(trIdx1, brIdx1, blIdx1));  // Changed order
						EdgeChunkMesh1->TriangleIndices.Add(trIdx1);
						EdgeChunkMesh1->TriangleIndices.Add(brIdx1);
						EdgeChunkMesh1->TriangleIndices.Add(blIdx1);

						// Repeat for EdgeChunkMesh2
						int tlIdx2, trIdx2, blIdx2, brIdx2;
						if (EdgeVertexMap2.Contains(topLeft)) {
							tlIdx2 = EdgeVertexMap2[topLeft];
						}
						else {
							tlIdx2 = EdgeChunkMesh2->Vertices.Num();
							EdgeChunkMesh2->Vertices.Add(AllPoints[topLeft]);
							EdgeVertexMap2.Add(topLeft, tlIdx2);
						}

						if (EdgeVertexMap2.Contains(topRight)) {
							trIdx2 = EdgeVertexMap2[topRight];
						}
						else {
							trIdx2 = EdgeChunkMesh2->Vertices.Num();
							EdgeChunkMesh2->Vertices.Add(AllPoints[topRight]);
							EdgeVertexMap2.Add(topRight, trIdx2);
						}

						if (EdgeVertexMap2.Contains(bottomLeft)) {
							blIdx2 = EdgeVertexMap2[bottomLeft];
						}
						else {
							blIdx2 = EdgeChunkMesh2->Vertices.Num();
							EdgeChunkMesh2->Vertices.Add(AllPoints[bottomLeft]);
							EdgeVertexMap2.Add(bottomLeft, blIdx2);
						}

						if (EdgeVertexMap2.Contains(bottomRight)) {
							brIdx2 = EdgeVertexMap2[bottomRight];
						}
						else {
							brIdx2 = EdgeChunkMesh2->Vertices.Num();
							EdgeChunkMesh2->Vertices.Add(AllPoints[bottomRight]);
							EdgeVertexMap2.Add(bottomRight, brIdx2);
						}

						// Add triangles to EdgeChunkMesh2 with CLOCKWISE winding
						EdgeChunkMesh2->Triangles.Add(FIndex3UI(tlIdx2, trIdx2, blIdx2));  // Changed order
						EdgeChunkMesh2->TriangleIndices.Add(tlIdx2);
						EdgeChunkMesh2->TriangleIndices.Add(trIdx2);
						EdgeChunkMesh2->TriangleIndices.Add(blIdx2);

						EdgeChunkMesh2->Triangles.Add(FIndex3UI(trIdx2, brIdx2, blIdx2));  // Changed order
						EdgeChunkMesh2->TriangleIndices.Add(trIdx2);
						EdgeChunkMesh2->TriangleIndices.Add(brIdx2);
						EdgeChunkMesh2->TriangleIndices.Add(blIdx2);

						// Repeat for EdgeChunkMesh3
						int tlIdx3, trIdx3, blIdx3, brIdx3;
						if (EdgeVertexMap3.Contains(topLeft)) {
							tlIdx3 = EdgeVertexMap3[topLeft];
						}
						else {
							tlIdx3 = EdgeChunkMesh3->Vertices.Num();
							EdgeChunkMesh3->Vertices.Add(AllPoints[topLeft]);
							EdgeVertexMap3.Add(topLeft, tlIdx3);
						}

						if (EdgeVertexMap3.Contains(topRight)) {
							trIdx3 = EdgeVertexMap3[topRight];
						}
						else {
							trIdx3 = EdgeChunkMesh3->Vertices.Num();
							EdgeChunkMesh3->Vertices.Add(AllPoints[topRight]);
							EdgeVertexMap3.Add(topRight, trIdx3);
						}

						if (EdgeVertexMap3.Contains(bottomLeft)) {
							blIdx3 = EdgeVertexMap3[bottomLeft];
						}
						else {
							blIdx3 = EdgeChunkMesh3->Vertices.Num();
							EdgeChunkMesh3->Vertices.Add(AllPoints[bottomLeft]);
							EdgeVertexMap3.Add(bottomLeft, blIdx3);
						}

						if (EdgeVertexMap3.Contains(bottomRight)) {
							brIdx3 = EdgeVertexMap3[bottomRight];
						}
						else {
							brIdx3 = EdgeChunkMesh3->Vertices.Num();
							EdgeChunkMesh3->Vertices.Add(AllPoints[bottomRight]);
							EdgeVertexMap3.Add(bottomRight, brIdx3);
						}

						// Add triangles to EdgeChunkMesh3 with CLOCKWISE winding
						EdgeChunkMesh3->Triangles.Add(FIndex3UI(tlIdx3, trIdx3, blIdx3));  // Changed order
						EdgeChunkMesh3->TriangleIndices.Add(tlIdx3);
						EdgeChunkMesh3->TriangleIndices.Add(trIdx3);
						EdgeChunkMesh3->TriangleIndices.Add(blIdx3);

						EdgeChunkMesh3->Triangles.Add(FIndex3UI(trIdx3, brIdx3, blIdx3));  // Changed order
						EdgeChunkMesh3->TriangleIndices.Add(trIdx3);
						EdgeChunkMesh3->TriangleIndices.Add(brIdx3);
						EdgeChunkMesh3->TriangleIndices.Add(blIdx3);
					}
					else {
						// This quad belongs to inner mesh
						int tlIdx, trIdx, blIdx, brIdx;

						// Add vertices to inner mesh (if not already added)
						if (InnerVertexMap.Contains(topLeft)) {
							tlIdx = InnerVertexMap[topLeft];
						}
						else {
							tlIdx = InnerChunkMesh->Vertices.Num();
							InnerChunkMesh->Vertices.Add(AllPoints[topLeft]);
							InnerVertexMap.Add(topLeft, tlIdx);
						}

						if (InnerVertexMap.Contains(topRight)) {
							trIdx = InnerVertexMap[topRight];
						}
						else {
							trIdx = InnerChunkMesh->Vertices.Num();
							InnerChunkMesh->Vertices.Add(AllPoints[topRight]);
							InnerVertexMap.Add(topRight, trIdx);
						}

						if (InnerVertexMap.Contains(bottomLeft)) {
							blIdx = InnerVertexMap[bottomLeft];
						}
						else {
							blIdx = InnerChunkMesh->Vertices.Num();
							InnerChunkMesh->Vertices.Add(AllPoints[bottomLeft]);
							InnerVertexMap.Add(bottomLeft, blIdx);
						}

						if (InnerVertexMap.Contains(bottomRight)) {
							brIdx = InnerVertexMap[bottomRight];
						}
						else {
							brIdx = InnerChunkMesh->Vertices.Num();
							InnerChunkMesh->Vertices.Add(AllPoints[bottomRight]);
							InnerVertexMap.Add(bottomRight, brIdx);
						}

						// Add triangles to inner mesh with CLOCKWISE winding
						InnerChunkMesh->Triangles.Add(FIndex3UI(tlIdx, trIdx, blIdx));  // Changed order
						InnerChunkMesh->TriangleIndices.Add(tlIdx);
						InnerChunkMesh->TriangleIndices.Add(trIdx);
						InnerChunkMesh->TriangleIndices.Add(blIdx);

						InnerChunkMesh->Triangles.Add(FIndex3UI(trIdx, brIdx, blIdx));  // Changed order
						InnerChunkMesh->TriangleIndices.Add(trIdx);
						InnerChunkMesh->TriangleIndices.Add(brIdx);
						InnerChunkMesh->TriangleIndices.Add(blIdx);
					}
				}
			}
		}
	}
	void Initialize(int InResolution) {
		//InResolution += 1;
		GenerateInnerChunkTemplates(InResolution);
		// Generate inner chunk templates
		//for (int c = 0; c < 4; c++) {
		//	EChildPosition cp = EChildPosition(c);
		//	TSharedPtr<FMeshTemplate> InnerChunkMesh = MakeShared<FMeshTemplate>();
		//	auto lb = *InnerChunkLoopBounds.Find(cp);

		//	// First, add all vertices in a grid pattern
		//	for (int y = lb.yBounds.StartIdx; y <= lb.yBounds.EndIdx; y++) {
		//		for (int x = lb.xBounds.StartIdx; x <= lb.xBounds.EndIdx; x++) {
		//			InnerChunkMesh->Vertices.Add({ x, y });
		//		}
		//	}

		//	// Calculate dimensions of the grid
		//	int width = lb.xBounds.EndIdx - lb.xBounds.StartIdx;
		//	int height = lb.yBounds.EndIdx - lb.yBounds.StartIdx;

		//	// Create triangles using the same logic as your existing code
		//	for (int y = 0; y < height - 1; y++) {
		//		for (int x = 0; x < width - 1; x++) {
		//			// Calculate vertex indices for this quad
		//			int topLeft = y * width + x;
		//			int topRight = topLeft + 1;
		//			int bottomLeft = (y + 1) * width + x;
		//			int bottomRight = bottomLeft + 1;

		//			// Add triangles with the same winding as your existing code
		//			InnerChunkMesh->Triangles.Add(FIndex3UI(topLeft, topRight, bottomLeft));
		//			InnerChunkMesh->TriangleIndices.Add(topLeft);
		//			InnerChunkMesh->TriangleIndices.Add(topRight);
		//			InnerChunkMesh->TriangleIndices.Add(bottomLeft);

		//			InnerChunkMesh->Triangles.Add(FIndex3UI(topRight, bottomRight, bottomLeft));
		//			InnerChunkMesh->TriangleIndices.Add(topRight);
		//			InnerChunkMesh->TriangleIndices.Add(bottomRight);
		//			InnerChunkMesh->TriangleIndices.Add(bottomLeft);

		//		}
		//	}

		//	// Add the completed mesh to the map
		//	InnerChunkMap.Add(cp, InnerChunkMesh);

		//	// Now generate edge chunk templates
		//	for (int i = 0; i < 4; i++) {
		//		ENeighborState ns = ENeighborState(i);
		//		TSharedPtr<FMeshTemplate> EdgeChunkMesh = MakeShared<FMeshTemplate>();

		//		auto edgeLB = *EdgeChunkLoopBounds.Find(cp);
		//		auto edgeAlignEdge = *EdgeChunkEdges.Find(cp);

		//		// === HORIZONTAL EDGE STRIP ===
		//		int yEdge = edgeAlignEdge.EndIdx;
		//		int yInner = (cp == BOTTOM_LEFT || cp == BOTTOM_RIGHT) ? yEdge - 1 : yEdge + 1;

		//		// Add both edge and inner vertices for horizontal strip
		//		for (int x = edgeLB.xBounds.StartIdx; x < edgeLB.xBounds.EndIdx; x++) {
		//			// Add edge vertex
		//			EdgeChunkMesh->Vertices.Add({ x, yEdge });
		//		}

		//		for (int x = edgeLB.xBounds.StartIdx; x < edgeLB.xBounds.EndIdx; x++) {
		//			// Add inner vertex
		//			EdgeChunkMesh->Vertices.Add({ x, yInner });
		//		}

		//		// === VERTICAL EDGE STRIP ===
		//		int xEdge = edgeAlignEdge.StartIdx;
		//		int xInner = (cp == BOTTOM_LEFT || cp == TOP_LEFT) ? xEdge + 1 : xEdge - 1;

		//		// Add both edge and inner vertices for vertical strip
		//		// Skip vertices at the corner which are already added
		//		for (int y = edgeLB.yBounds.StartIdx; y < edgeLB.yBounds.EndIdx; y++) {
		//			//if (y == yEdge) continue; // Skip already added corner vertex

		//			// Add edge vertex
		//			EdgeChunkMesh->Vertices.Add({ xEdge, y });
		//		}

		//		for (int y = edgeLB.yBounds.StartIdx; y < edgeLB.yBounds.EndIdx; y++) {
		//			//if (y == yEdge) continue; // Skip corresponding inner vertex at corner

		//			// Add inner vertex
		//			EdgeChunkMesh->Vertices.Add({ xInner, y });
		//		}

		//		// Calculate dimensions
		//		int horzWidth = edgeLB.xBounds.EndIdx - edgeLB.xBounds.StartIdx;
		//		int vertHeight = edgeLB.yBounds.EndIdx - edgeLB.yBounds.StartIdx;
		//		if (yEdge >= edgeLB.yBounds.StartIdx && yEdge < edgeLB.yBounds.EndIdx) {
		//			vertHeight--; // Adjust if corner vertex is in range
		//		}

		//		// Create triangles for horizontal strip
		//		for (int x = 0; x < horzWidth - 1; x++) {
		//			// Match the same indexing pattern as your existing code
		//			int topLeft = x;                 // Edge vertices start at index 0
		//			int topRight = x + 1;
		//			int bottomLeft = horzWidth + x;   // Inner vertices start after all edge vertices
		//			int bottomRight = horzWidth + x + 1;

		//			// Add triangles with the same winding as your existing code
		//			EdgeChunkMesh->Triangles.Add(FIndex3UI(topLeft, topRight, bottomLeft));
		//			EdgeChunkMesh->TriangleIndices.Add(topLeft);
		//			EdgeChunkMesh->TriangleIndices.Add(bottomLeft);
		//			EdgeChunkMesh->TriangleIndices.Add(topRight);

		//			EdgeChunkMesh->Triangles.Add(FIndex3UI(topRight, bottomRight, bottomLeft));
		//			EdgeChunkMesh->TriangleIndices.Add(topRight);
		//			EdgeChunkMesh->TriangleIndices.Add(bottomLeft);
		//			EdgeChunkMesh->TriangleIndices.Add(bottomRight);
		//		}

		//		// Create triangles for vertical strip
		//		int vertStartIndex = horzWidth * 2;
		//		for (int y = 0; y < vertHeight - 1; y++) {
		//			// Match the same indexing pattern as your existing code
		//			int topLeft = vertStartIndex + y;                  // Vertical edge vertices
		//			int topRight = vertStartIndex + vertHeight + y;    // Corresponding inner vertices
		//			int bottomLeft = vertStartIndex + y + 1;
		//			int bottomRight = vertStartIndex + vertHeight + y + 1;

		//			// Add triangles with the same winding as your existing code
		//			EdgeChunkMesh->Triangles.Add(FIndex3UI(topLeft, bottomLeft, topRight));
		//			EdgeChunkMesh->TriangleIndices.Add(topLeft);
		//			EdgeChunkMesh->TriangleIndices.Add(bottomLeft);
		//			EdgeChunkMesh->TriangleIndices.Add(topRight);

		//			EdgeChunkMesh->Triangles.Add(FIndex3UI(topRight, bottomLeft, bottomRight));
		//			EdgeChunkMesh->TriangleIndices.Add(topRight);
		//			EdgeChunkMesh->TriangleIndices.Add(bottomLeft);
		//			EdgeChunkMesh->TriangleIndices.Add(bottomRight);
		//		}

		//		// Add the completed edge mesh to the map
		//		EdgeChunkMap.FindOrAdd(cp).Add(ns, EdgeChunkMesh);
		//	}
		//}
	}
};