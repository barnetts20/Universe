#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
//#include "Templates/SharedFromThis.h"
#include "FProctreeStructures.generated.h"
/**
 * 
 */
class PROCTREEPLUGIN_API FProctreeStructures
{
public:
	FProctreeStructures();
	~FProctreeStructures();
};

UENUM(BlueprintType)
enum class EProctreeNodeType : uint8
{
    None,
    Root,
    GalacticCluster,
    Galaxy,
    BlackHole,
    StellarCluster,
    GasCloud,
    Nebula,
    Star,
    Planet,
    Moon,
    SpaceStation,
    All
};

USTRUCT(BlueprintType)
struct PROCTREEPLUGIN_API FProctreeNodeRandomizationBounds
{
	GENERATED_BODY()

public:
    //This struct should provide randomization bounds for the proceduralized data properties of the node's proceduralized data properties
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        float MinRelativeScale = 0;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        float MaxRelativeScale = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        float MinTemperature = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        float MaxTemperature = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        float MinDensity = 0;
    
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        float MaxDensity = 1;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        FLinearColor MinComposition = { 0.0,0.0,0.0,0.0 };
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        FLinearColor MaxComposition = { 1.0,1.0,1.0,1.0 };

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeRandomizationBounds")
        EProctreeNodeType NodeType = EProctreeNodeType::None;

};

USTRUCT(BlueprintType)
struct PROCTREEPLUGIN_API FProctreeNodeData
{
    GENERATED_BODY()

public:
    //Derived Tree Node Properties
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        FString Path = "0";

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        int32 Index = 0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        int32 Depth = 0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        FVector Center = { 0.0,0.0,0.0 };

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        float Size = 0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        FBox BoundingBox = FBox::BuildAABB({0,0,0}, {0,0,0});

    //Derived Data Properties
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        int32 Seed = 0;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        bool Activated = false;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        FVector LocationOffset = { 0.0,0.0,0.0 };

    //Proceduralized Data Properties
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        float RelativeScale = 1;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        float Temperature = 1;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        float Density = 1;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        FLinearColor Composition = {0.0,0.0,0.0,0.0};

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "FProctreeNodeData")
        int32 ClusterHits = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "FProctreeNodeData")
		EProctreeNodeType NodeType = EProctreeNodeType::None;
};