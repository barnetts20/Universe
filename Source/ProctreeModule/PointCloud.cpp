#pragma once
#include "PointCloud.h"
#include "CoreMinimal.h"
#include "Math/UnrealMathUtility.h"
#include "FastNoise/FastNoise.h"

using namespace FastNoise;

TArray<FVector> URandomRegularPointCloudGenerator::GeneratePointCloud_Internal()
{
    TArray<FVector> returnPoints;
    for (int i = 0; i < this->NumPoints; i++) {
        FVector randomPoint = FVector(this->RandomStream.FRandRange(-1, 1), this->RandomStream.FRandRange(-1, 1), this->RandomStream.FRandRange(-1, 1));
        returnPoints.Add(randomPoint);
    }
    return returnPoints;
}

URandomIrregularPointCloudGenerator::URandomIrregularPointCloudGenerator()
{
    //Init a noise pattern
    auto fnPerlin = FastNoise::New<FastNoise::Simplex>();

    //Path 1
    auto fnFractal1 = FastNoise::New<FastNoise::FractalFBm>();
    fnFractal1->SetSource(fnPerlin);
    fnFractal1->SetGain(.5);
    fnFractal1->SetWeightedStrength(.5);
    fnFractal1->SetOctaveCount(16);
    fnFractal1->SetLacunarity(2);

    auto fnFractalRidged = FastNoise::New<FastNoise::FractalRidged>();
    fnFractalRidged->SetSource(fnPerlin);
    fnFractalRidged->SetGain(.5);
    fnFractalRidged->SetWeightedStrength(0);
    fnFractalRidged->SetOctaveCount(16);
    fnFractalRidged->SetLacunarity(2);

    auto fnRemap = FastNoise::New<FastNoise::Remap>();
    fnRemap->SetSource(fnFractalRidged);
    fnRemap->SetRemap(-1, 1, 0, 1);

    auto fnPow = FastNoise::New<FastNoise::PowFloat>();
    fnPow->SetPow(8);
    fnPow->SetValue(fnRemap);

    auto fnDomainScale = FastNoise::New<FastNoise::DomainScale>();
    fnDomainScale->SetSource(fnPow);
    fnDomainScale->SetScale(.25);

    auto fnDomainWarp = FastNoise::New<FastNoise::DomainWarpGradient>();
    fnDomainWarp->SetSource(fnDomainScale);
    fnDomainWarp->SetWarpAmplitude(2);
    fnDomainWarp->SetWarpFrequency(.5);

    this->NoiseNode = fnDomainWarp;
}

TArray<FVector> URandomIrregularPointCloudGenerator::GeneratePointCloud_Internal()
{
    TArray<FVector> returnPoints;
    while (returnPoints.Num() <= this->NumPoints) {
        auto vector = FVector(this->RandomStream.FRandRange(-1.0, 1.0), this->RandomStream.FRandRange(-1.0, 1.0), this->RandomStream.FRandRange(-1.0, 1.0));
        auto noiseValue = this->NoiseNode->GenSingle3D(vector.X, vector.Y, vector.Z, this->RandomStream.GetCurrentSeed());
        if (noiseValue > this->Threshold) {
            returnPoints.Add(vector);
        }
    }
    return returnPoints;
}

TArray<FVector> UGlobularRegularPointCloudGenerator::GeneratePointCloud_Internal()
{
    TArray<FVector> returnPoints;
    for (int i = 0; i < this->NumPoints; i++) {
        auto globularWeight = pow((1 - this->GlobularWeight) + this->RandomStream.FRandRange(0, this->GlobularWeight), this->GlobularExponent);
        FVector randomPoint = FVector(this->RandomStream.FRandRange(-1, 1), this->RandomStream.FRandRange(-1, 1), this->RandomStream.FRandRange(-1, 1)) * globularWeight;
        returnPoints.Add(randomPoint);
    }
    return returnPoints;
}

UGlobularIrregularPointCloudGenerator::UGlobularIrregularPointCloudGenerator()
{
    //Init a noise pattern
    auto fnPerlin = FastNoise::New<FastNoise::Simplex>();

    auto fnFractalRidged = FastNoise::New<FastNoise::FractalRidged>();
    fnFractalRidged->SetSource(fnPerlin);
    fnFractalRidged->SetGain(.5);
    fnFractalRidged->SetWeightedStrength(0);
    fnFractalRidged->SetOctaveCount(16);
    fnFractalRidged->SetLacunarity(2);

    auto fnRemap = FastNoise::New<FastNoise::Remap>();
    fnRemap->SetSource(fnFractalRidged);
    fnRemap->SetRemap(-1, 1, 0, 1);

    auto fnPow = FastNoise::New<FastNoise::PowFloat>();
    fnPow->SetPow(8);
    fnPow->SetValue(fnRemap);

    auto fnDomainScale = FastNoise::New<FastNoise::DomainScale>();
    fnDomainScale->SetSource(fnPow);
    fnDomainScale->SetScale(0.25);
    
    auto fnDomainWarp = FastNoise::New<FastNoise::DomainWarpGradient>();
    fnDomainWarp->SetSource(fnDomainScale);
    fnDomainWarp->SetWarpAmplitude(2);
    fnDomainWarp->SetWarpFrequency(.5);

    this->NoiseNode = fnDomainWarp;
}

TArray<FVector> UGlobularIrregularPointCloudGenerator::GeneratePointCloud_Internal()
{
    TArray<FVector> returnPoints;
    while (returnPoints.Num() <= this->NumPoints) {
        auto globularWeight = pow((1 - this->GlobularWeight) + this->RandomStream.FRandRange(0, this->GlobularWeight), this->GlobularExponent);       
        auto vector = FVector(this->RandomStream.FRandRange(-1.0, 1.0), this->RandomStream.FRandRange(-1.0, 1.0), this->RandomStream.FRandRange(-1.0, 1.0)) * globularWeight;
        auto noiseValue = this->NoiseNode->GenSingle3D(vector.X, vector.Y, vector.Z, this->RandomStream.GetCurrentSeed());
        if (noiseValue > this->Threshold) {
            returnPoints.Add(vector);
        }
    }
    return returnPoints;
}

TArray<FVector> URingPointCloudGenerator::GeneratePointCloud_Internal()
{
    TArray<FVector> returnPoints;
    FRotator rotator = FRotator(
        this->RandomStream.FRandRange(this->TiltMin.X, this->TiltMax.X),
        this->RandomStream.FRandRange(this->TiltMin.Y, this->TiltMax.Y),
        this->RandomStream.FRandRange(this->TiltMin.Z, this->TiltMax.Z)
    );

    for (int i = 0; i < this->NumPoints; i++) {
        auto tAngle = this->RandomStream.FRand() * PI * 2;
        auto tx = sqrt(this->RandomStream.FRandRange(this->MajorRadius, this->MinorRadius)) * cos(tAngle);
        auto ty = sqrt(this->RandomStream.FRandRange(this->MajorRadius, this->MinorRadius)) * sin(tAngle);
        auto tz = this->RandomStream.FRandRange(this->ZVariance * -1, this->ZVariance) * this->RandomStream.FRand();
        auto newPoint = FVector(tx, ty, tz);
        returnPoints.Add(rotator.RotateVector(newPoint));
    }
    return returnPoints;
}

TArray<FVector> USpiralPointCloudGenerator::GeneratePointCloud_Internal()
{
    TArray<FVector> tempPoints = TArray<FVector>();
    TArray<FVector> returnPoints = TArray<FVector>();

    int pointsPerArm = this->NumPoints / this->ArmCount;
    double increment = (2 * PI) / pointsPerArm;

    FVector minBounds = { FLT_MAX, FLT_MAX, FLT_MAX };
    FVector maxBounds = { FLT_MIN, FLT_MIN, FLT_MIN };

    FRotator rotator = FRotator(
        this->RandomStream.FRandRange(this->TiltMin.X, this->TiltMax.X),
        this->RandomStream.FRandRange(this->TiltMin.Y, this->TiltMax.Y),
        this->RandomStream.FRandRange(this->TiltMin.Z, this->TiltMax.Z)
    );

    double aPi = PI * this->A;

    for (int i = 0; i < this->ArmCount; i++) {
        double tempTheta = 0.0;
        double thetaOffset = (aPi / this->ArmCount) * i;
        for (int j = 0; j < pointsPerArm; j++) {
            double tempR = pow(this->B * tempTheta, 2.71828);
            double combinedTheta = tempTheta + thetaOffset;
            double tX = tempR * cos(combinedTheta);
            double tY = tempR * sin(combinedTheta);
            double tZ = pow(this->RandomStream.FRand(), this->ZVarianceFalloff) * this->RandomStream.FRandRange(this->ZVariance * -1, this->ZVariance);

            FVector tVector = FVector(tX, tY, tZ);
            tVector = tVector + (FVector(0, 0, 0) - (tVector * this->ShiftTowardsCenter));

            double xyRand = pow(this->RandomStream.FRand(), this->XYVarianceFalloff);
            double xyJitter = this->XYVariance * .5;
            double xShift = this->RandomStream.FRandRange(xyJitter * -1, xyJitter) * xyRand;
            double yShift = this->RandomStream.FRandRange(xyJitter * -1, xyJitter) * xyRand;
            FVector finalVector = tVector + FVector(xShift, yShift, 0);

            minBounds = FVector(FMath::Min(minBounds.X, finalVector.X), FMath::Min(minBounds.Y, finalVector.Y), FMath::Min(minBounds.Z, finalVector.Z));
            maxBounds = FVector(FMath::Max(maxBounds.X, finalVector.X), FMath::Max(maxBounds.Y, finalVector.Y), FMath::Max(maxBounds.Z, finalVector.Z));

            tempPoints.Add(finalVector);
            tempTheta = tempTheta + increment;
        }
    }

    //Normalize the resulting pointcloud to the desired bounds -1,-1,-1 to 1,1,1 and rotate it
    for (int i = 0; i < tempPoints.Num(); i++) {
        FVector normPoint = (((tempPoints[i] - minBounds) / (maxBounds - minBounds)) * 2) - 1;
        normPoint = FVector(normPoint.X, normPoint.Y, tempPoints[i].Z);
        returnPoints.Add(rotator.RotateVector(normPoint));
    }
    return returnPoints;
}

void UPointCloud::GeneratePointCloud()
{
    // Start asynchronous generation
    AsyncTask(ENamedThreads::AnyNormalThreadHiPriTask, [=]()
        {
            TArray<FVector> Points = GeneratePointCloud_Internal();

            // Fire event on game thread
            FGraphEventRef Task = FFunctionGraphTask::CreateAndDispatchWhenReady([=]()
                {
                    PointCloudBroadcaster.Broadcast(Points);
                }, TStatId(), nullptr, ENamedThreads::GameThread);
        });
}
