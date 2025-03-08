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
	X_POS UMETA(DisplayName = "X+"),
	X_NEG UMETA(DisplayName = "X-"),
	Y_POS UMETA(DisplayName = "Y+"),
	Y_NEG UMETA(DisplayName = "Y-"),
	Z_POS UMETA(DisplayName = "Z+"),
	Z_NEG UMETA(DisplayName = "Z-")
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
	BOTTOM_LEFT = 0,
	TOP_LEFT = 1,
	BOTTOM_RIGHT = 2,
	TOP_RIGHT = 3
};

struct PROCTREEMODULE_API FCubeTransform {
	FIntVector3 AxisMap;
	FIntVector3 AxisDir;
};