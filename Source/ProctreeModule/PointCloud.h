// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FastNoise/FastNoise.h"
#include "UObject/NoExportTypes.h"
#include "PointCloud.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class PROCTREEMODULE_API UPointCloud : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	FRandomStream RandomStream;

	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	int NumPoints = 100;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPointCloudGenerationComplete, const TArray<FVector>&, PointCloudResult);

	UPROPERTY(BlueprintAssignable, Category = "Point Event Broadcaster")
		FPointCloudGenerationComplete PointCloudBroadcaster;

	UFUNCTION(BlueprintCallable, Category = "Generate Point Cloud")
	void GeneratePointCloud();

	virtual TArray<FVector> GeneratePointCloud_Internal() {
		return TArray<FVector>();
	};
};

UCLASS(BlueprintType)
class PROCTREEMODULE_API URandomRegularPointCloudGenerator : public UPointCloud
{
	GENERATED_BODY()

public:
	virtual TArray<FVector> GeneratePointCloud_Internal() override;

	UFUNCTION(BlueprintCallable, Category = "Point Cloud Factory")
	static URandomRegularPointCloudGenerator* CreateRandomRegularPointCloudGenerator() {
		return NewObject <URandomRegularPointCloudGenerator>();
	}
};

UCLASS(BlueprintType)
class PROCTREEMODULE_API URandomIrregularPointCloudGenerator : public UPointCloud
{
	GENERATED_BODY()

public:
	URandomIrregularPointCloudGenerator();

	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double Threshold = .7;

	FastNoise::SmartNode<> NoiseNode;

	virtual TArray<FVector> GeneratePointCloud_Internal() override;

	UFUNCTION(BlueprintCallable, Category = "Point Cloud Factory")
	static URandomIrregularPointCloudGenerator* CreateRandomIrregularPointCloudGenerator() {
		return NewObject <URandomIrregularPointCloudGenerator>();
	}
};

UCLASS(BlueprintType)
class PROCTREEMODULE_API UGlobularRegularPointCloudGenerator : public UPointCloud
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double GlobularWeight = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double GlobularExponent = 1;

	virtual TArray<FVector> GeneratePointCloud_Internal() override;

	UFUNCTION(BlueprintCallable, Category = "Point Cloud Factory")
	static UGlobularRegularPointCloudGenerator* CreateGlobularRegularPointCloudGenerator() {
		return NewObject <UGlobularRegularPointCloudGenerator>();
	}
};

UCLASS(BlueprintType)
class PROCTREEMODULE_API UGlobularIrregularPointCloudGenerator : public UPointCloud
{
	GENERATED_BODY()

public:
	UGlobularIrregularPointCloudGenerator();

	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double Threshold = .7;

	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double GlobularWeight = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double GlobularExponent = 1;

	FastNoise::SmartNode<> NoiseNode;

	virtual TArray<FVector> GeneratePointCloud_Internal() override;

	UFUNCTION(BlueprintCallable, Category = "Point Cloud Factory")
	static UGlobularIrregularPointCloudGenerator* CreateGlobularIrregularPointCloudGenerator() {
		return NewObject <UGlobularIrregularPointCloudGenerator>();
	}
};

UCLASS(BlueprintType)
class PROCTREEMODULE_API URingPointCloudGenerator : public UPointCloud
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double MajorRadius = 1;
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double MinorRadius = 0;
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double ZVariance = .05;
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	FVector TiltMin = FVector(0,0,0);
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	FVector TiltMax = FVector(0,0,0);

	virtual TArray<FVector> GeneratePointCloud_Internal() override;

	UFUNCTION(BlueprintCallable, Category = "Point Cloud Factory")
	static URingPointCloudGenerator* CreateRingPointCloudGenerator() {
		return NewObject <URingPointCloudGenerator>();
	}
};


UCLASS(BlueprintType)
class PROCTREEMODULE_API USpiralPointCloudGenerator : public UPointCloud
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	int ArmCount = 6;
	
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double B = .5;
	
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double A = 2;
	
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double XYVariance = 20;
	
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double XYVarianceFalloff = 1;
	
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double ZVariance = .125;
	
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double ZVarianceFalloff = 1.0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	double ShiftTowardsCenter = 0;
	
	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	FVector TiltMin = { 0,0,0 };

	UPROPERTY(BlueprintReadWrite, Category = "Point Cloud Attributes")
	FVector TiltMax = { 0,0,0 };

	virtual TArray<FVector> GeneratePointCloud_Internal() override;

	UFUNCTION(BlueprintCallable, Category = "Point Cloud Factory")
	static USpiralPointCloudGenerator* CreateSpiralPointCloudGenerator() {
		return NewObject <USpiralPointCloudGenerator>();
	}
};
