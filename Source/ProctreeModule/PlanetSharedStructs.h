#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Mesh/RealtimeMeshSimpleData.h>
#include "PlanetSharedStructs.generated.h"

class QuadTreeNode;

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

	//URealtimeMeshStreamSet* StreamSet;

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
};

struct PROCTREEMODULE_API FMeshUpdateDataBatch {
	double UpdateTimestamp;
	int Priority;
	TArray<TSharedPtr<FMeshUpdateData>> MeshAdds;
	TArray<TSharedPtr<FMeshUpdateData>> MeshRemoves;
};
struct PROCTREEMODULE_API InternalMeshData {
	InternalMeshData() { };
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
UENUM(BlueprintType)
enum class EdgeOrientation : uint8 {
	LEFT = 0,
	RIGHT = 1,
	UP = 2,
	DOWN = 3
};

UENUM(BlueprintType)
enum class EFaceDirection : uint8 {
	X_POS = 0 UMETA(DisplayName = "X+"),
	X_NEG = 1 UMETA(DisplayName = "X-"),
	Y_POS = 2 UMETA(DisplayName = "Y+"),
	Y_NEG = 3 UMETA(DisplayName = "Y-"),
	Z_POS = 4 UMETA(DisplayName = "Z+"),
	Z_NEG = 5 UMETA(DisplayName = "Z-")
};

enum PROCTREEMODULE_API EChildPosition {
	BOTTOM_LEFT = 0, //0b00,
	TOP_LEFT = 1, //0b01,
	BOTTOM_RIGHT = 2, //0b10,
	TOP_RIGHT = 3, //0b11
};

struct PROCTREEMODULE_API FaceTransition {
	uint8 TargetFace;
	uint8 QuadrantRemap[4];
	bool bFlipX = false;
	bool bFlipY = false;
};

struct PROCTREEMODULE_API FCubeTransform {
	FIntVector3 AxisMap;
	FIntVector3 AxisDir;
	EdgeOrientation NeighborEdgeMap[4];
	FaceTransition FaceTransitions[4];
	bool bFlipWinding;

	static const FCubeTransform FaceTransforms[6];
};



struct PROCTREEMODULE_API FQuadIndex {
	uint64 EncodedPath;
	uint8 FaceId;
	static constexpr uint64 SentinelBits = 0b11ULL;

	explicit FQuadIndex(uint8 InFaceId)
		: EncodedPath(SentinelBits), FaceId(InFaceId) {}

	FQuadIndex(uint64 InPath, uint8 InFaceId)
		: EncodedPath(InPath), FaceId(InFaceId) {}

	uint8 GetDepth() const {
		uint64 path = EncodedPath >> 2;
		uint8 depth = 0;
		while (path != 0) {
			depth++;
			path >>= 2;
		}
		return depth;
	}

	bool IsRoot() const {
		return GetDepth() == 0;
	}

	uint8 GetQuadrantAtDepth(uint8 Level) const {
		uint8 depth = GetDepth();
		if (Level >= depth) return 0;
		uint8 shiftAmount = (depth - Level - 1) * 2;
		return (EncodedPath >> shiftAmount) & 0x3;
	}

	uint8 GetQuadrant() const {
		if (IsRoot()) return 0;
		return GetQuadrantAtDepth(GetDepth() - 1);
	}

	FQuadIndex GetChildIndex(uint8 InChildIndex) const {
		if (GetDepth() >= 31) return *this;
		uint64 newPath = (EncodedPath << 2) | (InChildIndex & 0x3);
		return FQuadIndex(newPath, FaceId);
	}

	FQuadIndex GetParentIndex() const {
		if (IsRoot()) return *this;
		return FQuadIndex(EncodedPath >> 2, FaceId);
	}

	uint64 GetQuadrantPath() const {
		return EncodedPath >> 2;
	}

	uint64 MakeEncodedPath(uint64 QuadrantPath) const {
		return (QuadrantPath << 2) | SentinelBits;
	}

	uint8 ReflectQuadrant(uint8 quadrant, EdgeOrientation Direction) const {
		switch (Direction) {
		case EdgeOrientation::LEFT:
		case EdgeOrientation::RIGHT:
			return quadrant ^ 0x2;
		case EdgeOrientation::UP:
		case EdgeOrientation::DOWN:
			return quadrant ^ 0x1;
		default:
			return quadrant;
		}
	}

	uint8 ApplyFlip(uint8 quadrant, bool FlipX, bool FlipY) const {
		if (FlipX) {
			quadrant ^= 0x2; // Flip left/right
		}
		if (FlipY) {
			quadrant ^= 0x1; // Flip up/down
		}
		return quadrant;
	}

	// This implementation properly detects face-crossing edges regardless of node depth
	FQuadIndex GetNeighborIndex(EdgeOrientation Direction) const {
		// Check if this node is at the edge of its face in the given direction
		bool isFaceEdge = false;

		// Walk up the tree to find if we're at a face edge
		FQuadIndex current = *this;
		while (!current.IsRoot()) {
			uint8 quadrant = current.GetQuadrant();
			bool quadAtEdge = false;

			switch (Direction) {
			case EdgeOrientation::LEFT: quadAtEdge = (quadrant & 0x2) == 0; break;
			case EdgeOrientation::RIGHT: quadAtEdge = (quadrant & 0x2) != 0; break;
			case EdgeOrientation::UP: quadAtEdge = (quadrant & 0x1) == 0; break;
			case EdgeOrientation::DOWN: quadAtEdge = (quadrant & 0x1) != 0; break;
			}

			if (!quadAtEdge) {
				// This quadrant is not at the edge in this direction
				// So we're not at a face edge
				isFaceEdge = false;
				break;
			}

			// Continue checking parent
			current = current.GetParentIndex();
			isFaceEdge = true; // We're at face edge unless proven otherwise
		}

		// If we reached the root and all checked quadrants are at the edge,
		// then we need to cross to another face
		if (isFaceEdge || IsRoot()) {
			// We need to cross to another face
			// First, capture the entire path
			TArray<uint8> path;
			current = *this;

			while (!current.IsRoot()) {
				path.Insert(current.GetQuadrant(), 0);
				current = current.GetParentIndex();
			}

			// Get the face transition
			const FaceTransition& transition = FCubeTransform::FaceTransforms[FaceId].FaceTransitions[(uint8)Direction];

			// Remap the entire path
			TArray<uint8> remappedPath;
			for (uint8 quadrant : path) {
				remappedPath.Add(ApplyFlip(transition.QuadrantRemap[quadrant], transition.bFlipX, transition.bFlipY));
			}

			// Build the result node starting with the target face
			FQuadIndex result(transition.TargetFace);
			for (uint8 remappedQuad : remappedPath) {
				result = result.GetChildIndex(remappedQuad);
			}

			return result;
		}

		// If we're not at a face edge, we're at an internal edge or not at any edge
		uint8 quadrant = GetQuadrant();
		bool atEdge = false;

		switch (Direction) {
		case EdgeOrientation::LEFT: atEdge = (quadrant & 0x2) == 0; break;
		case EdgeOrientation::RIGHT: atEdge = (quadrant & 0x2) != 0; break;
		case EdgeOrientation::UP: atEdge = (quadrant & 0x1) == 0; break;
		case EdgeOrientation::DOWN: atEdge = (quadrant & 0x1) != 0; break;
		}

		if (!atEdge) {
			// We're not at any edge, just flip bit for internal neighbor
			uint64 neighborPath = EncodedPath;
			neighborPath ^= (Direction == EdgeOrientation::LEFT || Direction == EdgeOrientation::RIGHT) ? 0x2ULL : 0x1ULL;
			return FQuadIndex(neighborPath, FaceId);
		}

		// We're at an internal edge, needs parent lookup + reflection
		FQuadIndex parent = GetParentIndex();
		FQuadIndex neighborParent = parent.GetNeighborIndex(Direction);
		uint8 reflectedQuadrant = ReflectQuadrant(quadrant, Direction);
		return neighborParent.GetChildIndex(reflectedQuadrant);
	}

	FQuadIndex GetCrossFaceNeighbor(EdgeOrientation Direction) const {
		const FaceTransition& transition = FCubeTransform::FaceTransforms[FaceId].FaceTransitions[(uint8)Direction];
		uint64 newQuadrantPath = 0;
		uint8 depth = GetDepth();
		for (uint8 level = 0; level < depth; ++level) {
			newQuadrantPath = (newQuadrantPath << 2) | transition.QuadrantRemap[GetQuadrantAtDepth(level)];
		}
		return FQuadIndex((newQuadrantPath << 2) | SentinelBits, transition.TargetFace);;
	}

	bool operator==(const FQuadIndex& Other) const {
		return FaceId == Other.FaceId && EncodedPath == Other.EncodedPath;
	}

	friend uint32 GetTypeHash(const FQuadIndex& ID) {
		return HashCombine(GetTypeHash(ID.FaceId),
			GetTypeHash(uint32(ID.EncodedPath ^ (ID.EncodedPath >> 32))));
	}

	bool IsValid() const {
		return EncodedPath != 0;
	}

    FString ToString() const {
		FString Result = FString::Printf(TEXT("Face: %d, Depth: %d, Path: "), FaceId, GetDepth());

		for (int i = 0; i < GetDepth(); i++) {
			Result += FString::Printf(TEXT("%d"), GetQuadrantAtDepth(i));
		}

		Result += FString::Printf(TEXT(" (Encoded: 0x%llX)"), EncodedPath);
		return Result;
	}

	FString PathToBinary() {
		uint64 Path = EncodedPath;
		FString binaryString;
		int highestBit = 63;

		// Find the highest set bit
		while (highestBit >= 0 && !(Path & (1ULL << highestBit))) {
			highestBit--;
		}

		// Always start from an even bit to maintain pair alignment
		if (highestBit % 2 == 0) highestBit++;

		// Iterate bits, grouping into pairs
		for (int i = highestBit; i >= 0; --i) {
			binaryString += ((Path >> i) & 1ULL) ? "1" : "0";

			// Add space after every 2 bits
			if (i % 2 == 0) {
				binaryString += " ";
			}
		}

		return binaryString;
	}
};
