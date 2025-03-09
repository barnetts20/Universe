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

struct PROCTREEMODULE_API FCubeTransform {
	FIntVector3 AxisMap;
	FIntVector3 AxisDir;

	bool bFlipWinding = false;
};

struct PROCTREEMODULE_API FaceTransition {
	uint8 TargetFace;
	uint8 RotationQuadrants; // 0=0°, 1=90°, 2=180°, 3=270°
	bool FlipX;
	bool FlipY;
};

struct PROCTREEMODULE_API FQuadIndex {
	uint64 EncodedPath;
	uint8 FaceId;

	static const FaceTransition FaceTransitions[6][4];

	explicit FQuadIndex(uint8 InFaceId)
		: EncodedPath(0b11), FaceId(InFaceId) {}  // Starts with the sentinel bits

	FQuadIndex(uint64 InPath, uint8 InFaceId)
		: EncodedPath(InPath), FaceId(InFaceId) {}

	// Get the depth from the encoded path
	// Count the number of quadrant pairs after the sentinel bits
	uint8 GetDepth() const {
		uint64 path = EncodedPath >> 2; // Remove sentinel bits for counting
		uint8 depth = 0;
		while (path != 0) {
			depth++;
			path >>= 2; // Move to the next quadrant
		}
		return depth;
	}

	bool IsRoot() const {
		return GetDepth() == 0;
	}

	// Get the quadrant index of this node (the last quadrant in the path)
	uint8 GetQuadrant() const {
		if (IsRoot()) return 0; // Root has no quadrant
		return GetQuadrantAtDepth(GetDepth() - 1);
	}

	uint8 GetQuadrantAtDepth(uint8 Level) const {
		if (Level >= GetDepth()) return 0;
		return (EncodedPath >> (2 + 2 * Level)) & 0x3; // Skip sentinel bits and shift to the correct depth level
	}

	// Get a child ID with the specified child index
	FQuadIndex GetChildIndex(uint8 InChildIndex) const {
		if (GetDepth() >= 31) return *this; // Max depth reached (31 levels + 2 sentinel bits)
		uint64 newPath = EncodedPath << 2;   // Shift existing path left to make room for new child index
		newPath |= (InChildIndex & 0x3);     // Add new child index at the least significant bits
		return FQuadIndex(newPath, FaceId);  // Return new FQuadIndex with updated path
	}

	// Get parent ID (decreases depth)
	FQuadIndex GetParentIndex() const {
		if (IsRoot()) return *this; // Already at root
		uint64 newPath = EncodedPath >> 2;  // Right shift to remove the last quadrant
		return FQuadIndex(newPath, FaceId); // Return new FQuadIndex with updated path
	}

	// Get the neighbor index in a specific direction
	FQuadIndex GetNeighborIndex(EdgeOrientation Direction) const {
		// If we're at the root, we always cross to another face
		if (IsRoot()) {
			return GetCrossFaceNeighbor(Direction);
		}

		uint8 depth = GetDepth();
		uint8 lastQuad = GetQuadrant();

		// Determine if we cross a boundary
		bool crossesBoundary = false;
		switch (Direction) {
		case EdgeOrientation::LEFT:
			crossesBoundary = (lastQuad == BOTTOM_LEFT || lastQuad == TOP_LEFT);
			break;
		case EdgeOrientation::RIGHT:
			crossesBoundary = (lastQuad == BOTTOM_RIGHT || lastQuad == TOP_RIGHT);
			break;
		case EdgeOrientation::UP:
			crossesBoundary = (lastQuad == TOP_LEFT || lastQuad == TOP_RIGHT);
			break;
		case EdgeOrientation::DOWN:
			crossesBoundary = (lastQuad == BOTTOM_LEFT || lastQuad == BOTTOM_RIGHT);
			break;
		}

		if (!crossesBoundary) {
			// Simple case: neighbor is within same parent, just flip the appropriate bit
			uint64 newPath = EncodedPath;
			uint8 newQuad = lastQuad;

			switch (Direction) {
			case EdgeOrientation::LEFT:
				newQuad = lastQuad & 0x2; // Clear rightmost bit (convert to a left quadrant)
				break;
			case EdgeOrientation::RIGHT:
				newQuad = lastQuad | 0x2; // Set rightmost bit (convert to a right quadrant)
				break;
			case EdgeOrientation::UP:
				newQuad = lastQuad | 0x1; // Set second bit (convert to a top quadrant)
				break;
			case EdgeOrientation::DOWN:
				newQuad = lastQuad & 0x2; // Clear second bit (convert to a bottom quadrant)
				break;
			}

			// Replace the last quadrant in the path
			newPath &= ~(uint64(0x3)); // Clear last quadrant
			newPath |= newQuad;        // Add new quadrant

			return FQuadIndex(newPath, FaceId);
		}
		else {
			// We need to go up to a common ancestor and then down

			// Find the common ancestor level where the edge exists
			uint8 ancestorLevel = depth - 1;
			uint64 path = EncodedPath >> 2; // Skip last quadrant

			// Keep moving up until we find a level where we don't cross the boundary
			while (ancestorLevel > 0) {
				uint8 quadrant = path & 0x3;
				bool crossesAtThisLevel = false;

				switch (Direction) {
				case EdgeOrientation::LEFT:
					crossesAtThisLevel = (quadrant == BOTTOM_LEFT || quadrant == TOP_LEFT);
					break;
				case EdgeOrientation::RIGHT:
					crossesAtThisLevel = (quadrant == BOTTOM_RIGHT || quadrant == TOP_RIGHT);
					break;
				case EdgeOrientation::UP:
					crossesAtThisLevel = (quadrant == TOP_LEFT || quadrant == TOP_RIGHT);
					break;
				case EdgeOrientation::DOWN:
					crossesAtThisLevel = (quadrant == BOTTOM_LEFT || quadrant == BOTTOM_RIGHT);
					break;
				}

				if (!crossesAtThisLevel) {
					break; // Found the ancestor that doesn't cross
				}

				ancestorLevel--;
				path >>= 2;
			}

			// If we couldn't find a common ancestor, we need to cross faces
			if (ancestorLevel == 0 && ((path & 0x3) == 0)) {
				return GetCrossFaceNeighbor(Direction);
			}

			// Compute the path to the neighbor through the common ancestor
			// First, get the path to the common ancestor
			uint64 ancestorPath = EncodedPath >> (2 * (depth - ancestorLevel));
			ancestorPath <<= (2 * (depth - ancestorLevel));

			// Get the quadrant at the common ancestor level
			uint8 ancestorQuad = (EncodedPath >> (2 * (depth - ancestorLevel))) & 0x3;

			// Flip the bit for the appropriate direction at the ancestor level
			uint8 neighborAncestorQuad = ancestorQuad;
			switch (Direction) {
			case EdgeOrientation::LEFT:
				neighborAncestorQuad = ancestorQuad & 0x2; // Clear rightmost bit
				break;
			case EdgeOrientation::RIGHT:
				neighborAncestorQuad = ancestorQuad | 0x2; // Set rightmost bit
				break;
			case EdgeOrientation::UP:
				neighborAncestorQuad = ancestorQuad | 0x1; // Set second bit
				break;
			case EdgeOrientation::DOWN:
				neighborAncestorQuad = ancestorQuad & 0x2; // Clear second bit
				break;
			}

			// Replace the quadrant at the ancestor level
			uint64 neighborPath = ancestorPath;
			neighborPath &= ~(uint64(0x3) << (2 * (depth - ancestorLevel)));
			neighborPath |= (uint64(neighborAncestorQuad) << (2 * (depth - ancestorLevel)));

			// Now we need to add the mirrored quadrants for all levels below the ancestor
			for (uint8 level = ancestorLevel + 1; level <= depth; level++) {
				uint8 quadrant = (EncodedPath >> (2 * (depth - level))) & 0x3;
				uint8 mirroredQuadrant = quadrant;

				// Mirror the quadrant based on which edge we're crossing
				switch (Direction) {
				case EdgeOrientation::LEFT:
				case EdgeOrientation::RIGHT:
					mirroredQuadrant = quadrant ^ 0x2; // Flip x-axis (horizontal)
					break;
				case EdgeOrientation::UP:
				case EdgeOrientation::DOWN:
					mirroredQuadrant = quadrant ^ 0x1; // Flip y-axis (vertical)
					break;
				}

				neighborPath |= (uint64(mirroredQuadrant) << (2 * (depth - level)));
			}

			return FQuadIndex(neighborPath, FaceId);
		}
	}

	// Handle neighbors that cross to a different face
	FQuadIndex GetCrossFaceNeighbor(EdgeOrientation Direction) const {

		// Get the transition data for this face and direction
		const FaceTransition& transition = FaceTransitions[FaceId][static_cast<uint8>(Direction)];

		// Create the base index for the target face
		FQuadIndex neighborIndex(transition.TargetFace);

		// If we're at the root, just return the adjacent face root
		if (IsRoot()) {
			return neighborIndex;
		}

		// Transform the path to the coordinate system of the target face
		for (uint8 level = 0; level < GetDepth(); level++) {
			uint8 quadrant = GetQuadrantAtDepth(level);
			uint8 transformedQuadrant = quadrant;

			// Apply flips
			if (transition.FlipX) {
				transformedQuadrant ^= 0x2; // Flip x-axis (BOTTOM_LEFT<->BOTTOM_RIGHT, TOP_LEFT<->TOP_RIGHT)
			}
			if (transition.FlipY) {
				transformedQuadrant ^= 0x1; // Flip y-axis (BOTTOM_LEFT<->TOP_LEFT, BOTTOM_RIGHT<->TOP_RIGHT)
			}

			// Apply rotation
			for (uint8 i = 0; i < transition.RotationQuadrants; i++) {
				// Rotate 90° clockwise: (x,y) -> (y,-x)
				// In quadrant encoding, this is a specific bit manipulation
				bool topBit = (transformedQuadrant & 0x1) != 0;
				bool rightBit = (transformedQuadrant & 0x2) != 0;
				transformedQuadrant = (topBit ? 0x2 : 0x0) | (rightBit ? 0x0 : 0x1);
			}

			neighborIndex = neighborIndex.GetChildIndex(transformedQuadrant);
		}

		return neighborIndex;
	}

	// Equality operator
	bool operator==(const FQuadIndex& Other) const {
		return FaceId == Other.FaceId && EncodedPath == Other.EncodedPath;
	}

	// Hash function for use in TMap
	friend uint32 GetTypeHash(const FQuadIndex& ID) {
		return HashCombine(GetTypeHash(ID.FaceId),
			GetTypeHash(uint32(ID.EncodedPath ^ (ID.EncodedPath >> 32))));
	}

	// Check if ID is valid
	bool IsValid() const {
		return EncodedPath != 0;
	}

	FString ToString() const {
		FString Result = FString::Printf(TEXT("Face:%d Depth:%d Path:"), FaceId, GetDepth());
		for (int i = 0; i < GetDepth(); i++) {
			Result += FString::Printf(TEXT("%d"), GetQuadrantAtDepth(i));
		}
		return Result;
	}
};
