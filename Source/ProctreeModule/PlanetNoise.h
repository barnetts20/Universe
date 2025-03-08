// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlanetSharedStructs.h"
#include "UObject/NoExportTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FastNoise/FastNoise.h"
#include "PlanetNoise.generated.h"

#define CRATER_TYPE 1
struct FCraterCurve
{
	double X;
	double K1 = 5.0;
	double K2 = 5.0;
	double FloorMulti = -.1;
	double FloorExp = -.1;
	double FloorOffset = -.1;
	double CavityMulti = 4.0;
	double CavityExp = 1.0;
	double RidgeMulti = 2;
	double RidgeExp = 1;
	double FinalMulti = 1.5;

	// Function declarations
	inline double Smooth(double A, double B, double K) {
		return -log(exp(-K * A) + exp(-K * B)) / K;
	}
	
	inline double CraterFloor() {
		return FloorMulti * Smooth(Smooth(1.0 - pow(abs(4.0 * X - 4.0), FloorExp), 0.0, -K2), 0.0, K2) + FloorOffset;
	}
	
	inline double CraterCavity() {
		return (X >= 0.0 && X <= 2.0) ? -CavityMulti * pow(0.5 * (sin((X + 1.5) * PI) + 1.0), CavityExp) + 2.0 : 2.0;
	}
	
	inline double CraterRidge() {
		return (X >= 0.0 && X <= 2.0) ? RidgeMulti * pow(0.5 * (sin((X + 1.5) * PI) + 1.0), RidgeExp) : 0.0;
	}
	
	inline double Crater() {
		return (Smooth(CraterRidge(), Smooth(CraterFloor(), CraterCavity(), -K1), K1) + FMath::Max(0.0, (2.0 - K1))) * FinalMulti;
	}

	inline void SeedCrater(const FVector& RandomFactor){
		FVector FracRandom = RandomFactor - FVector(FMath::Floor(RandomFactor.X), FMath::Floor(RandomFactor.Y), FMath::Floor(RandomFactor.Z));
		K1 = 5.0;
		K2 = 5.0;
		FloorMulti = FMath::Lerp(-4.0, -1.0, FracRandom.X);
		FloorExp = FMath::Lerp(-0.2, 0.05, FracRandom.Y);
		FloorOffset = FMath::Lerp(-4.0, -0.1, FracRandom.Z);
		CavityMulti = FMath::Lerp(2.0, 8.0, FracRandom.X);
		CavityExp = FMath::Lerp(1.0, 4.0, FracRandom.Y);
		RidgeMulti = FMath::Lerp(1.5, 2.0, FracRandom.Z);
		RidgeExp = FMath::Lerp(1.0, 4.0, FracRandom.X);
		FinalMulti = FMath::Lerp(0.25, 1.75, FracRandom.Y);
	}

	inline void SeedVolcano(const FVector& RandomFactor){
		FVector FracRandom = RandomFactor - FVector(FMath::Floor(RandomFactor.X), FMath::Floor(RandomFactor.Y), FMath::Floor(RandomFactor.Z));
		FloorMulti = FMath::Lerp(-4.0, -1.0, FracRandom.X);
		FloorExp = FMath::Lerp(-0.2, -0.05, FracRandom.Y);
		FloorOffset = FMath::Lerp(-8.0, -4.0, FracRandom.Z);
		CavityMulti = FMath::Lerp(6.0, 10.0, FracRandom.X);
		CavityExp = FMath::Lerp(10.0, 100.0, FracRandom.Y);
		RidgeMulti = FMath::Lerp(2.0, 4.0, FracRandom.Z);
		RidgeExp = FMath::Lerp(1.0, 2.0, FracRandom.X);
		FinalMulti = FMath::Lerp(0.25, .5, FracRandom.Y);
	}
};

struct FCraterNoise
{
	int Octaves = 6;
	double OctaveFrequencyScale = 3;
	double OctaveAmplitudeScale = .33;
	bool isVolcanoType = false;

	// Function declarations
	inline FVector Noise(const FVector& P) {
		return FVector(
			FMath::Frac(FMath::Sin(FVector::DotProduct(P, FVector(127.1, 311.7, 591.1))) * 43758.5454453),
			FMath::Frac(FMath::Sin(FVector::DotProduct(P, FVector(269.5, 183.3, 113.5))) * 43758.5454453),
			FMath::Frac(FMath::Sin(FVector::DotProduct(P, FVector(419.2, 371.9, 4297.7))) * 43758.5454453)
		);
	}

	inline FVector4 CalculateTwoSphereIntersect(const FVector& SphereCenter1, double SphereRadius1, const FVector& SphereCenter2, double SphereRadius2) {
		// Calculate the distance between the centers of the two spheres
		double DistanceBetweenCenters = FVector::Dist(SphereCenter1, SphereCenter2);

		// Check if the spheres do not intersect or are completely contained within each other
		if (DistanceBetweenCenters > SphereRadius1 + SphereRadius2 || DistanceBetweenCenters < FMath::Abs(SphereRadius1 - SphereRadius2))
		{
			return FVector4(SphereCenter2, 0.0); // No intersection
		}

		// Calculate the radius of the intersection circle
		double IntersectionRadius = FMath::Sqrt(SphereRadius1 * SphereRadius1 - FMath::Square((DistanceBetweenCenters * DistanceBetweenCenters + SphereRadius1 * SphereRadius1 - SphereRadius2 * SphereRadius2) / (2.0 * DistanceBetweenCenters)));

		// Calculate the center of the intersection circle
		FVector IntersectionCenter = SphereCenter1 + (SphereCenter2 - SphereCenter1) * (SphereRadius1 * SphereRadius1 - SphereRadius2 * SphereRadius2 + DistanceBetweenCenters * DistanceBetweenCenters) / (2.0 * DistanceBetweenCenters * DistanceBetweenCenters);

		return FVector4(IntersectionCenter, FMath::Max(0.0, IntersectionRadius));
	}

	inline FVector4 VoronoiCraterNoise(const FVector& SamplePos) {
		FVector IntegerPos = SamplePos.GridSnap(1.0); // Floor equivalent
		FVector FracPos = SamplePos - IntegerPos;     // Frac equivalent

		FVector ClosestFeatureCenter(0.0, 0.0, 0.0);
		FVector ClosestFeaturePoint(0.0, 0.0, 0.0);
		FVector ClosestDiff(0.0, 0.0, 0.0);
		double MinEdgeDistance = 8.0;
		double MinGradientDistance = 8.0;

		// First pass: find the closest feature point
		for (int k = -1; k <= 1; k++)
		{
			for (int j = -1; j <= 1; j++)
			{
				for (int i = -1; i <= 1; i++)
				{
					FVector NeighborCell(i, j, k);
					FVector FeaturePoint = NeighborCell + Noise(IntegerPos + NeighborCell);
					FVector Diff = FeaturePoint - FracPos;
					double Distance = Diff.SizeSquared(); // dot(diff, diff) equivalent

					if (Distance < MinEdgeDistance)
					{
						MinEdgeDistance = Distance;
						MinGradientDistance = Distance;
						ClosestDiff = Diff;
						ClosestFeatureCenter = FeaturePoint;
						ClosestFeaturePoint = NeighborCell;
					}
				}
			}
		}

		// Second pass: calculate the distance to the cell borders
		MinEdgeDistance = 8.0;
		MinGradientDistance = 8.0;
		for (int k = -2; k <= 2; k++)
		{
			for (int j = -2; j <= 2; j++)
			{
				for (int i = -2; i <= 2; i++)
				{
					FVector NeighborCell= FVector(i, j, k) + ClosestFeaturePoint;
					FVector NeighborFeaturePoint = NeighborCell + Noise(IntegerPos + NeighborCell);
					double DistToNeighbor = FVector::Dist(NeighborFeaturePoint, ClosestFeatureCenter);
					FVector Diff = NeighborFeaturePoint - FracPos;
					if (DistToNeighbor > 0.0)
					{
						MinEdgeDistance = FMath::Min(MinEdgeDistance, DistToNeighbor);
						MinGradientDistance = FMath::Min(MinGradientDistance, FVector::DotProduct(0.5 * (ClosestDiff + Diff), (Diff - ClosestDiff).GetSafeNormal()));
					}
				}
			}
		}

		double VoronoiSphereRadius = MinEdgeDistance * 0.5;
		FVector VoronoiCellCenter = ClosestFeatureCenter + IntegerPos;
		double SampleSphereRadius = SamplePos.Size();
		FVector SampleSphereCenter(0, 0, 0);

		// Calculate the intersection sphere using the CalculateTwoSphereIntersect function
		FVector4 IntersectionSphere = CalculateTwoSphereIntersect(SampleSphereCenter, SampleSphereRadius, VoronoiCellCenter, VoronoiSphereRadius);

		// Calculate displacement, ejecta, sphere distance, and max distance
		double VSphereDistance = FMath::Max(0.0, 1.0 - (FVector::Dist(IntersectionSphere, SamplePos) / IntersectionSphere.W));

		FCraterCurve CC;
		CC.X = FMath::Clamp(VSphereDistance, 0.0, 1.0);
		CC.K1 = 5.0;
		CC.K2 = 5.0;
		
		if (isVolcanoType) {
			CC.SeedVolcano(ClosestFeaturePoint);
		}
		else {
			CC.SeedCrater(ClosestFeaturePoint);
		}
		
		double Displacement = CC.Crater();

		double Ejecta = 0;// CalculateEjecta(IntersectionSphere, IntersectionSphere.W, FVector(0, 0, 0), SamplePos);
		double MaxDistance = IntersectionSphere.W;

		return FVector4(Displacement, Ejecta, VSphereDistance, MinGradientDistance);
	}

	inline double CalculateEjecta(const FVector& GradientCenter, double GradientRadius, const FVector& ObjectCenter, const FVector& SamplePosition) {
		// Calculate the surface normal at the gradient center
		FVector N = (GradientCenter - ObjectCenter).GetSafeNormal();

		// Calculate tangent and bitangent vectors to define the tangent plane
		FVector Up = FMath::Abs(N.Y) < 0.999 ? FVector(0.0, 1.0, 0.0) : FVector(1.0, 0.0, 0.0);
		FVector Tangent = FVector::CrossProduct(Up, N).GetSafeNormal();
		FVector Bitangent = FVector::CrossProduct(N, Tangent);

		// Project the sample position onto the tangent plane
		FVector ToSample = SamplePosition - GradientCenter;
		double U = FVector::DotProduct(ToSample, Tangent);
		double V = FVector::DotProduct(ToSample, Bitangent);

		// Radial distance from the center of the gradient sphere
		double DistanceFromCenter = FMath::Sqrt(U * U + V * V);

		// Normalize the distance in the range [0, 1]
		double NormalizedDistance = DistanceFromCenter / GradientRadius;

		// Azimuthal angle (angle around the normal vector in the tangent plane)
		double Angle = FMath::RadiansToDegrees(FMath::Atan2(V, U));
		Angle = FMath::Fmod(Angle + 360.0, 360.0); // Ensure angle is in the range [0, 360)

		// Ejecta strength falls off with distance
		double EjectaStrength = FMath::Exp(-NormalizedDistance * 3.0);  // Example: Exponential falloff

		// Generate a random factor based on the integer part of the gradient center to vary properties per ejecta site
		FVector IntGradientCenter = GradientCenter.GridSnap(1.0); // Equivalent to floor(gradientCenter)
		double RandomFactor = Noise(IntGradientCenter).X;  // Use the X component of the noise result as a random float
		RandomFactor = FMath::Frac(RandomFactor * 10.0);  // Keep it in the range [0, 1]

		// Determine the number of angular segments
		int NumSegments = FMath::Lerp(5, 10, RandomFactor); // Random between 5 and 10 mirrored segments

		// Initialize variables for angle segment checking
		bool bAngleInSegment = false;
		double SegmentStrength = 1.0;
		double SegmentFrequency = 1.0;

		for (int i = 0; i < NumSegments; i++)
		{
			// Generate a random starting angle for the segment, ensuring it covers the full 360 degrees
			double SegmentStart = FMath::Frac(FMath::Sin(FVector::DotProduct(IntGradientCenter + FVector(i, i, i), FVector(12.9898, 78.233, 45.164))) * 43758.5453123) * 360.0;
			SegmentStart = FMath::Fmod(SegmentStart, 360.0); // Ensure it stays within [0, 360)

			// Generate a random segment width between 15 to 25 degrees
			double SegmentWidth = FMath::Lerp(5.0, 25.0, FMath::Frac(FMath::Sin(FVector::DotProduct(IntGradientCenter + FVector(i + 1, i + 1, i + 1), FVector(12.9898, 78.233, 45.164))) * 43758.5453123));

			// Calculate the mirrored start angle
			double MirroredStart = FMath::Fmod(SegmentStart + 180.0, 360.0);

			// Check if the current angle falls within this segment or its mirrored segment
			if ((Angle >= SegmentStart && Angle <= SegmentStart + SegmentWidth) ||
				(Angle >= MirroredStart && Angle <= MirroredStart + SegmentWidth))
			{
				bAngleInSegment = true;

				// Increase strength variance within segments
				SegmentStrength = FMath::Lerp(0.25, 4.0, FMath::Frac(FMath::Sin(FVector::DotProduct(IntGradientCenter + FVector(i + 2, i + 2, i + 2), FVector(12.9898, 78.233, 45.164))) * 43758.5453123));

				// Bind frequency randomization to the segment
				SegmentFrequency = FMath::Lerp(0.10, 4.0, FMath::Frac(FMath::Sin(FVector::DotProduct(IntGradientCenter + FVector(i + 3, i + 3, i + 3), FVector(12.9898, 78.233, 45.164))) * 43758.5453123));
				break;
			}
		}

		// If the angle falls within any of the segments, apply the segment's strength and frequency
		if (bAngleInSegment)
		{
			EjectaStrength *= SegmentStrength;

			// Calculate the streak pattern with segment-specific variations
			double StreakPattern = FMath::SmoothStep(0.5 - SegmentFrequency * 0.1, 0.5 + SegmentFrequency * 0.1, 0.5 + 0.5 * FMath::Sin(Angle * SegmentFrequency));

			// Combine streaks with the radial falloff and segment-specific strength
			double Ejecta = EjectaStrength * StreakPattern;
			return FMath::Clamp(Ejecta, 0.0, 1.0); // Saturate equivalent in C++
		}

		return 0.0;
	}

	inline FVector4 CraterFBM(const FVector& Pos) {
		FVector CurSamplePos = Pos;
		double CurAmplitudeCoeff = 1.0;
		FVector4 AccumulatedCraterData(0.0, 0.0, 0.0, 0.0);

		for (int i = 0; i < Octaves; i++) {
			AccumulatedCraterData += VoronoiCraterNoise(CurSamplePos) * CurAmplitudeCoeff;
			CurAmplitudeCoeff *= OctaveAmplitudeScale;
			CurSamplePos *= OctaveFrequencyScale;
		}

		return AccumulatedCraterData;
	}
};

struct IntMinMax {
	int min;
	int max;
};

struct FloatMinMax {
	float min;
	float max;
};

struct VectorMinMax {
	FVector min;
	FVector max;
};

UCLASS()
class PROCTREEMODULE_API UPlanetNoise : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UPlanetNoise();
	~UPlanetNoise();
};

class INoiseGenerator {
protected:
	int Seed = 666;
	bool UsePreprocess = false;
	bool UsePostProcess = false;
	double AmplitudeScale = .05;
	double FrequencyScale = 1.0;
	float SeaLevel = 0.0;
	float MinBound = -1.0;
	float MaxBound = 1.0;
	FastNoise::SmartNode<> OutputNode;

public:
	virtual ~INoiseGenerator() {}
	virtual void InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel) = 0;
	virtual FVector PreProcess(FVector position) { return position; }
	virtual FVector PostProcess(FVector position) { return position; }

	int GetSeed() {
		return this->Seed;
	}
	
	double GetAmplitudeScale() {
		return this->AmplitudeScale;
	}
	
	double GetFrequencyScale() {
		return this->FrequencyScale;
	}

	double GetSeaLevel() {
		return this->SeaLevel;
	}

	double GetMinBound() {
		return this->MinBound;
	}

	double GetMaxBound() {
		return this->MaxBound;
	}

	virtual FVector GetNoiseFromPosition(FVector inPosition) {
		FVector adjustedPoint = inPosition;
		float preProcessNoiseValue = 0;
		float postProcessNoiseValue = 0;
		if (this->UsePreprocess) { 
			adjustedPoint = this->PreProcess(inPosition);
		}
		
		adjustedPoint *= OutputNode->GenSingle3D(adjustedPoint.X, adjustedPoint.Y, adjustedPoint.Z, this->Seed) * AmplitudeScale + 1;

		if (this->UsePostProcess) { 
			adjustedPoint = this->PostProcess(adjustedPoint);
		}
		return adjustedPoint;
	}
};

class TerrestrialNoiseGenerator : public INoiseGenerator {
public:

	struct TerrestrialParams {
		//Params
		int continentOctaves = 8;
		float continentAboveSeaLevelCuttoff = .2;
		float continentBelowSeaLevelCutoff = -.8;

		float faultWidth = .2;
		float faultSmoothness = .5;
		FVector faultOfffset = FVector(.25, .25, .25);

		float valleyEffect = .3;
		float valleyScale = 4;
		float valleyWarpAmp = 1;
		float valleyWarpFreq = .5;
		float valleyBias = .5;
		float valleyCutoff = .1;

		int mountainOctaves = 8;
		float mountainScale = 32;
		int mountainFalloff = 2;
		float mountainPeakMultiplier = 1;
		float mountainRidgeMultiplier = 1;

		float finalContinentMultiplier = 1;
		float finalValleyMultiplier = 1;
		float finalMountainMultiplier = 1;
		float finalScaleMultiplier = 1;
	};

	struct TerrestrialParamBounds {
		IntMinMax continentOctaves = { 4, 12 };

		FloatMinMax continentAboveSeaLevelCuttoff = { .1,.4 };
		FloatMinMax continentBelowSeaLevelCutoff = { -.8,-.5 };

		FloatMinMax faultWidth = { .1, .3 };
		FloatMinMax faultSmoothness = { .3, .7 };
		VectorMinMax faultOfffset = { FVector(.25, .25, .25), FVector(.5,.5,.5) };

		FloatMinMax valleyEffect = { .1, .4 };
		FloatMinMax valleyScale = { 3,7 };
		FloatMinMax valleyWarpAmp = { .25, 1 };
		FloatMinMax valleyWarpFreq = { .25,.75 };
		FloatMinMax valleyBias = { .2,.6 };
		FloatMinMax valleyCutoff = { .1,.3 };

		IntMinMax mountainOctaves = { 6,12 };
		FloatMinMax mountainScale = { 24,48 };
		IntMinMax mountainFalloff = { 1,2 };
		FloatMinMax mountainPeakMultiplier = { .8,1.2 };
		FloatMinMax mountainRidgeMultiplier = { .8,1.2 };

		FloatMinMax finalContinentMultiplier = { .8,1.2 };
		FloatMinMax finalValleyMultiplier = { .8,1.2 };
		FloatMinMax finalMountainMultiplier = { .8,1.2 };

		FloatMinMax finalScaleMultiplier = { .333, 1.666 };
	};

	TerrestrialParams params;

	TerrestrialParamBounds paramBounds;

	virtual void InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel) override;

	void InitializeParams(int inSeed);
};

class MoltenNoiseGenerator : public INoiseGenerator {
public:
	struct MoltenParams {
		int continentOctaves = 4;
		float continentAboveSeaLevelCuttoff = .2;
		float continentBelowSeaLevelCutoff = -.8;

		float flowWidth = .9;
		float flowSmoothness = .5;

		float volcanoRidgeWidth = .1;
		float volcanoRidgeSmoothness = .5;
		FVector faultOfffset = FVector(.25, .25, .25);

		float lfScale = 8;
		float baseOffset = .2;
		float distortIntensity = 1;
		float volcanoNoiseRounding = .3;
		float volcanoCutoff = .5;
		int volcanoPow = 2;
		float volcanoMulti = 2;
		float volcanoPeakInversionCutoffScale = .5;
		float peakCutoffMin = .5;
		float peakCutoffMax = .75;
		int volcanoOctaveCount = 12;
		float volcanoFrequency = 24;
		float invertSmoothness = .5;
		float cavityMultiplier = 10;
		float finalVolcanoMultiplier = 1;
		float crackScale = 24;
		int crackFalloff = 6;
		float crackIntensity = .05;
		float lfrmMin = 0;
		float lfrmMax = .05;
		float flowRemapMin = -.5;
		float flowRemapMax = .5;
	};

	struct MoltenParamBounds {
		IntMinMax continentOctaves = { 8, 16 };

		FloatMinMax continentAboveSeaLevelCuttoff = { .1,.4 };
		FloatMinMax continentBelowSeaLevelCutoff = { -.8,-.5 };


		FloatMinMax flowWidth = { .6,.4 };
		FloatMinMax flowSmoothness = { .4, .6 };

		FloatMinMax volcanoRidgeWidth = { .05 , .1};
		FloatMinMax volcanoRidgeSmoothness = { .4, .6 };

		VectorMinMax faultOfffset = { FVector(.25, .25, .25), FVector(.5,.5,.5) };
		FloatMinMax lfScale = { 6,12 };
		FloatMinMax baseOffset = { .1,.3 };
		FloatMinMax distortIntensity = { .5, 1 };
		FloatMinMax volcanoNoiseRounding = { .2,.4 };
		FloatMinMax volcanoCutoff = { .4, .6 };
		IntMinMax volcanoPow = { 2,2 };
		FloatMinMax volcanoMulti = { 2 ,3 };
		FloatMinMax volcanoPeakInversionCutoffScale = { .5,.5 };
		FloatMinMax peakCutoffMin = { .4,.6 };
		FloatMinMax peakCutoffMax = { .6, .8 };
		IntMinMax volcanoOctaveCount = { 8,12 };
		FloatMinMax volcanoFrequency = { 22,32 };
		FloatMinMax invertSmoothness = {.4,.6};
		FloatMinMax finalVolcanoMultiplier = { 1,1.2 };
		FloatMinMax crackScale = { 20,30 };
		IntMinMax crackFalloff = { 4, 6 };
		FloatMinMax crackIntensity = { .025, .075 };
		FloatMinMax lfrmMin = { 0,0 };
		FloatMinMax lfrmMax = { .04,.07 };
		FloatMinMax flowRemapMin = { -.1,0 };
		FloatMinMax flowRemapMax = { .25, .75 };
	};

	//auto cMaxSmooth1 = FastNoise::New < FastNoise::MaxSmooth>();
	//cMaxSmooth1->SetLHS(fOffset);
	//cMaxSmooth1->SetRHS(1 - this->params.flowWidth);
	//cMaxSmooth1->SetSmoothness(this->params.flowSmoothness);

	//auto cMaxSmooth2 = FastNoise::New < FastNoise::MaxSmooth>();
	//cMaxSmooth2->SetLHS(fOffset);
	//cMaxSmooth2->SetRHS(1 - this->params.volcanoRidgeWidth);
	//cMaxSmooth2->SetSmoothness(this->params.volcanoRidgeSmoothness);

	//auto fRemap = FastNoise::New < FastNoise::Remap>();
	//fRemap->SetSource(cMaxSmooth1);
	//fRemap->SetRemap(1 - this->params.flowWidth, 1, 0, 1);

	//auto fRemap1 = FastNoise::New < FastNoise::Remap>();
	//fRemap1->SetSource(cMaxSmooth2);
	//fRemap1->SetRemap(1 - this->params.volcanoRidgeWidth, 1, 0, 1);

	MoltenParams params;
	MoltenParamBounds paramBounds;

	virtual void InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel) override;
	void InitializeParams(int inSeed);
};

class FrozenNoiseGenerator : public INoiseGenerator {
public:
	virtual void InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel) override;
};

class RockyNoiseGenerator : public INoiseGenerator {
public:
	virtual void InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel) override;
	virtual FVector PreProcess(FVector inPosition) override;
};

class DuneNoiseGenerator : public INoiseGenerator {
public:
	virtual void InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel) override;
};