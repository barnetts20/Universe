#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <Mesh/RealtimeMeshSimpleData.h>
#include "PlanetSharedStructs.generated.h"

class QuadTreeNode;

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

struct PROCTREEMODULE_API FQuadIndex {
	uint64 EncodedPath;
	uint8 FaceId;

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
