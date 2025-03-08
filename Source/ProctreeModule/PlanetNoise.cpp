// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "PlanetNoise.h"
#include "PlanetSharedStructs.h"
#include "FastNoise/FastNoise.h"
#include <iostream>
using namespace FastNoise;

UPlanetNoise::UPlanetNoise()
{
}

UPlanetNoise::~UPlanetNoise()
{
}

void TerrestrialNoiseGenerator::InitializeParams(int inSeed) {
	// Create a random stream using the seed
	FRandomStream RandomStream(inSeed);

	// Randomize each parameter based on its bounds
	params.continentOctaves = RandomStream.RandRange(paramBounds.continentOctaves.min, paramBounds.continentOctaves.max);
	params.continentAboveSeaLevelCuttoff = RandomStream.FRandRange(paramBounds.continentAboveSeaLevelCuttoff.min, paramBounds.continentAboveSeaLevelCuttoff.max);
	params.continentBelowSeaLevelCutoff = RandomStream.FRandRange(paramBounds.continentBelowSeaLevelCutoff.min, paramBounds.continentBelowSeaLevelCutoff.max);

	params.faultWidth = RandomStream.FRandRange(paramBounds.faultWidth.min, paramBounds.faultWidth.max);
	params.faultSmoothness = RandomStream.FRandRange(paramBounds.faultSmoothness.min, paramBounds.faultSmoothness.max);
	params.faultOfffset.X = RandomStream.FRandRange(paramBounds.faultOfffset.min.X, paramBounds.faultOfffset.max.X);
	params.faultOfffset.Y = RandomStream.FRandRange(paramBounds.faultOfffset.min.Y, paramBounds.faultOfffset.max.Y);
	params.faultOfffset.Z = RandomStream.FRandRange(paramBounds.faultOfffset.min.Z, paramBounds.faultOfffset.max.Z);

	params.valleyEffect = RandomStream.FRandRange(paramBounds.valleyEffect.min, paramBounds.valleyEffect.max);
	params.valleyScale = RandomStream.FRandRange(paramBounds.valleyScale.min, paramBounds.valleyScale.max);
	params.valleyWarpAmp = RandomStream.FRandRange(paramBounds.valleyWarpAmp.min, paramBounds.valleyWarpAmp.max);
	params.valleyWarpFreq = RandomStream.FRandRange(paramBounds.valleyWarpFreq.min, paramBounds.valleyWarpFreq.max);
	params.valleyBias = RandomStream.FRandRange(paramBounds.valleyBias.min, paramBounds.valleyBias.max);
	params.valleyCutoff = RandomStream.FRandRange(paramBounds.valleyCutoff.min, paramBounds.valleyCutoff.max);

	params.mountainOctaves = RandomStream.RandRange(paramBounds.mountainOctaves.min, paramBounds.mountainOctaves.max);
	params.mountainScale = RandomStream.FRandRange(paramBounds.mountainScale.min, paramBounds.mountainScale.max);
	params.mountainFalloff = RandomStream.RandRange(paramBounds.mountainFalloff.min, paramBounds.mountainFalloff.max);
	params.mountainPeakMultiplier = RandomStream.FRandRange(paramBounds.mountainPeakMultiplier.min, paramBounds.mountainPeakMultiplier.max);
	params.mountainRidgeMultiplier = RandomStream.FRandRange(paramBounds.mountainRidgeMultiplier.min, paramBounds.mountainRidgeMultiplier.max);

	params.finalContinentMultiplier = RandomStream.FRandRange(paramBounds.finalContinentMultiplier.min, paramBounds.finalContinentMultiplier.max);
	params.finalValleyMultiplier = RandomStream.FRandRange(paramBounds.finalValleyMultiplier.min, paramBounds.finalValleyMultiplier.max);
	params.finalMountainMultiplier = RandomStream.FRandRange(paramBounds.finalMountainMultiplier.min, paramBounds.finalMountainMultiplier.max);

	params.finalScaleMultiplier = RandomStream.FRandRange(paramBounds.finalScaleMultiplier.min, paramBounds.finalScaleMultiplier.max);
}


void TerrestrialNoiseGenerator::InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel) {
	this->Seed = inSeed;
	this->AmplitudeScale = inAmplitudeScale;
	this->FrequencyScale = inFrequencyScale;
	this->SeaLevel = inSeaLevel;

	//We are doing remaps using -1 and 1 frequently, this is to ensure we dont hit an error remapping between the same numbers
	if (this->SeaLevel == -1.0) {
		this->SeaLevel = -1.0001;
	}
	else if (this->SeaLevel == 1) {
		this->SeaLevel = 1.00001;
	}

	InitializeParams(inSeed);

	//Main Continent
	auto cSimp = FastNoise::New<FastNoise::OpenSimplex2>();
	auto cFrac = FastNoise::New<FastNoise::FractalFBm>();
	cFrac->SetSource(cSimp);
	cFrac->SetOctaveCount(this->params.continentOctaves);

	auto cAdd = FastNoise::New<FastNoise::Add>();
	cAdd->SetLHS(cFrac);
	cAdd->SetRHS(-this->SeaLevel);

	float modSeaLevelMin = -1 - this->SeaLevel;
	float modSeaLevelMax = 1 - this->SeaLevel;
	float normMin = FMath::Max(this->params.continentBelowSeaLevelCutoff, modSeaLevelMin);
	float normMax = FMath::Min(this->params.continentAboveSeaLevelCuttoff, modSeaLevelMax);

	auto cMinSmooth = FastNoise::New<FastNoise::MinSmooth>();
	cMinSmooth->SetLHS(cAdd);
	cMinSmooth->SetRHS(this->params.continentAboveSeaLevelCuttoff);
	cMinSmooth->SetSmoothness(1);

	auto continentBase = FastNoise::New<FastNoise::MaxSmooth>();
	continentBase->SetLHS(cMinSmooth);
	continentBase->SetRHS(this->params.continentBelowSeaLevelCutoff);
	continentBase->SetSmoothness(1);
	
	//Normalized
	auto continentNormalized = FastNoise::New < FastNoise::Remap>();
	continentNormalized->SetSource(continentBase);
	continentNormalized->SetRemap(normMin, normMax, -1, 1);

	//Fault Lines Z+
 	auto cFracRidge = FastNoise::New < FastNoise::FractalRidged>();
	cFracRidge->SetSource(cFrac);
	cFracRidge->SetOctaveCount(1);

	auto cMaxSmooth1 = FastNoise::New < FastNoise::MaxSmooth>();
	cMaxSmooth1->SetLHS(cFracRidge);
	cMaxSmooth1->SetRHS(1 - this->params.faultWidth);
	cMaxSmooth1->SetSmoothness(this->params.faultSmoothness);

	auto fRemap = FastNoise::New < FastNoise::Remap>();
	fRemap->SetSource(cMaxSmooth1);
	fRemap->SetRemap(1 - this->params.faultWidth, 1, 0, 1);

	auto fOffset = FastNoise::New<FastNoise::DomainOffset>();
	fOffset->SetSource(fRemap);
	fOffset->SetOffset<FastNoise::Dim::X>(this->params.faultOfffset.X);
	fOffset->SetOffset<FastNoise::Dim::Y>(this->params.faultOfffset.Y);
	fOffset->SetOffset<FastNoise::Dim::Z>(this->params.faultOfffset.Z);

	auto continentFaultLines = FastNoise::New<FastNoise::Multiply>();
	continentFaultLines->SetLHS(continentNormalized);
	continentFaultLines->SetRHS(fOffset);

	//Valleys Z-
	auto cRemap = FastNoise::New < FastNoise::Remap>();
	cRemap->SetSource(continentFaultLines);
	cRemap->SetRemap(0, 1, 0, this->params.valleyEffect);

	auto cDomainScale = FastNoise::New < FastNoise::DomainScale>();
	cDomainScale->SetSource(cRemap);
	cDomainScale->SetScale(this->params.valleyScale);

	auto valleys = FastNoise::New<FastNoise::DomainWarpGradient>();
	valleys->SetSource(cDomainScale);
	valleys->SetWarpAmplitude(this->params.valleyWarpAmp);
	valleys->SetWarpFrequency(this->params.valleyWarpFreq);

	auto vmSmoothMin = FastNoise::New<FastNoise::MinSmooth>();
	vmSmoothMin->SetLHS(valleys);
	vmSmoothMin->SetRHS(1 - this->params.valleyCutoff);

	auto vmRemap = FastNoise::New<FastNoise::Remap>();
	vmRemap->SetSource(continentFaultLines);
	vmRemap->SetRemap(0,1,1,0);

	auto vMul = FastNoise::New<FastNoise::Multiply>();
	vMul->SetLHS(vmRemap);
	vMul->SetRHS(vmSmoothMin);

	auto cAdd1 = FastNoise::New<FastNoise::Add>();
	cAdd1->SetLHS(continentBase);
	cAdd1->SetRHS(this->params.valleyBias);

	auto cMax = FastNoise::New<FastNoise::Max>();
	cMax->SetLHS(cAdd1);
	cMax->SetRHS(0);

	auto valleyFinal = FastNoise::New<FastNoise::Multiply>();
	valleyFinal->SetLHS(cMax);
	valleyFinal->SetRHS(vMul);

	//Mountain peaks
	auto mpCell = FastNoise::New<FastNoise::CellularDistance>();
	auto mpFrac = FastNoise::New < FastNoise::FractalFBm>();
	mpFrac->SetSource(mpCell);
	mpFrac->SetOctaveCount(this->params.mountainOctaves);

	auto mpScale = FastNoise::New < FastNoise::DomainScale>();
	mpScale->SetSource(mpFrac);
	mpScale->SetScale(this->params.mountainScale);

	auto mpPow = FastNoise::New < FastNoise::PowInt>();
	mpPow->SetValue(mpScale);
	mpPow->SetPow(this->params.mountainFalloff);

	//Combine peaks with fault lines
	auto mpMul = FastNoise::New < FastNoise::Multiply>();
	mpMul->SetLHS(mpPow);
	mpMul->SetRHS(continentFaultLines);

	//Elevation contribution from fault lines
	auto mpElv = FastNoise::New < FastNoise::Multiply>();
	mpElv->SetLHS(continentBase);
	mpElv->SetRHS(continentFaultLines);

	//Final multiply of Peaks
	auto mpMul1 = FastNoise::New < FastNoise::Multiply>();
	mpMul1->SetLHS(mpMul);
	mpMul1->SetRHS(this->params.mountainPeakMultiplier);

	//Final multiply of Ridge
	auto mpMul2 = FastNoise::New < FastNoise::Multiply>();
	mpMul2->SetLHS(mpElv);
	mpMul2->SetRHS(this->params.mountainRidgeMultiplier);

	//Combine it final mountain data
	auto mountains = FastNoise::New < FastNoise::Add>();
	mountains->SetLHS(mpMul1);
	mountains->SetRHS(mpMul2);


	//Composite layers
	auto contFinalMul = FastNoise::New < FastNoise::Multiply>();
	contFinalMul->SetLHS(continentBase);
	contFinalMul->SetRHS(this->params.finalContinentMultiplier);

	auto valFinalMul = FastNoise::New < FastNoise::Multiply>();
	valFinalMul->SetLHS(valleyFinal);
	valFinalMul->SetRHS(this->params.finalValleyMultiplier);

	auto mountFinalMul = FastNoise::New < FastNoise::Multiply>();
	mountFinalMul->SetLHS(mountains);
	mountFinalMul->SetRHS(this->params.finalMountainMultiplier);

	auto sub1 = FastNoise::New<FastNoise::Subtract>();
	sub1->SetLHS(contFinalMul);
	sub1->SetRHS(valFinalMul);

	auto add1 = FastNoise::New<FastNoise::Add>();
	add1->SetLHS(sub1);
	add1->SetRHS(mountFinalMul);

	auto finalScale = FastNoise::New<FastNoise::DomainScale>();
	finalScale->SetSource(add1);
	finalScale->SetScale(this->FrequencyScale * params.finalScaleMultiplier);

	this->OutputNode = finalScale;
}

void MoltenNoiseGenerator::InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel)
{
	this->Seed = inSeed;
	this->AmplitudeScale = inAmplitudeScale;
	this->FrequencyScale = inFrequencyScale;
	this->SeaLevel = inSeaLevel;
	auto nodeStr = "GgABGQAbACAAFwAAAAAAAACAPwAAAL8AAABAGwAgABsADQAIAAAAAAAAQBMAzczMPRQABwABFgABAAAA//8AAAH//wEAAAAAAAAAAAAAAAAAAAA/AAAAAAAAAACAQAAAAAAAAAAAgD8AAACAPwEgAB8AFwAAAAAAAACAPwAAwD8AAIC/DQAIAAAAAAAAQBMAAADAQAsAAQAAAAAAAAABAAAABAAAAAAAAIA/AAAAAD8AAAAAAADNzMw9AAAAAD8AAACAvwAAAAA/AAAAAD8AAACAPgEbABcAAAAAAJqZmT4AAAAAAACAPx8AIAD//wUAAAAAAAAAzczMPQCamZk+AM3MzD0BEwAAAABAGgABHwAbABAAAAAAPxMAzcxMPhcAzczMPgAAgD8AAAAAAACAPyAAHwALAAEAAAABAAAAAAAAAAIAAAAAAACAPwELAAAAAAABAAAAAAAAAAIAAAAAAACAPwAAAAA/AM3MzD4AAACAPgEbAA0ACAAAAAAAAEAJAAAAAAA/AAAAAAAAzczMPgEhAAAAAACAQAAAAAAAQAEUABMAzczMPQYAAf//IQAB//8hAAH//yEAAAAAAAABFwAAAIC/AACAPwAAwD/NzMw/EwAAAIA///8bAAAAAIA/ARsAGgABIAD//yQAAf//JgAAAACAPwH//yYAAAAAgEIBJAACAAAAGwAfAP//BQAAAAAAAAAAAIA/AAAAgL8=";
	auto encodedTest = FastNoise::NewFromEncodedNodeTree(nodeStr);
	auto scaleEncoded = FastNoise::New<FastNoise::DomainScale>();
	scaleEncoded->SetSource(encodedTest);
	scaleEncoded->SetScale(32 * this->FrequencyScale);

	InitializeParams(this->Seed); //comment back in once proc harness integrated

	//MAIN CONTINENT
	auto cSimp = FastNoise::New<FastNoise::OpenSimplex2>();
	auto cFrac = FastNoise::New<FastNoise::FractalFBm>();
	cFrac->SetSource(cSimp);
	cFrac->SetOctaveCount(this->params.continentOctaves);

	auto cAdd = FastNoise::New<FastNoise::Add>();
	cAdd->SetLHS(cFrac);
	cAdd->SetRHS(-this->SeaLevel);

	float modSeaLevelMin = -1 - this->SeaLevel;
	float modSeaLevelMax = 1 - this->SeaLevel;
	float normMin = FMath::Max(this->params.continentBelowSeaLevelCutoff, modSeaLevelMin);
	float normMax = FMath::Min(this->params.continentAboveSeaLevelCuttoff, modSeaLevelMax);

	auto cMinSmooth = FastNoise::New<FastNoise::MinSmooth>();
	cMinSmooth->SetLHS(cAdd);
	cMinSmooth->SetRHS(this->params.continentAboveSeaLevelCuttoff);
	cMinSmooth->SetSmoothness(1);

	auto continentBase = FastNoise::New<FastNoise::MaxSmooth>();
	continentBase->SetLHS(cMinSmooth);
	continentBase->SetRHS(this->params.continentBelowSeaLevelCutoff);
	continentBase->SetSmoothness(1);

	//NORMALIZED CONTINENT
	auto continentNormalized = FastNoise::New < FastNoise::Remap>();
	continentNormalized->SetSource(continentBase);
	continentNormalized->SetRemap(normMin, normMax, -1, 1);

	auto continentNormalizedMaxed = FastNoise::New<FastNoise::MaxSmooth>();
	continentNormalizedMaxed->SetLHS(continentNormalized);
	continentNormalizedMaxed->SetRHS(0);
	continentNormalizedMaxed->SetSmoothness(.5);

	//VOLCANIC RIDGE
	auto cFracRidge = FastNoise::New < FastNoise::FractalRidged>();
	cFracRidge->SetSource(cFrac);
	cFracRidge->SetOctaveCount(1);

	auto fOffset = FastNoise::New<FastNoise::DomainOffset>();
	fOffset->SetSource(cFracRidge);
	fOffset->SetOffset<FastNoise::Dim::X>(this->params.faultOfffset.X);
	fOffset->SetOffset<FastNoise::Dim::Y>(this->params.faultOfffset.Y);
	fOffset->SetOffset<FastNoise::Dim::Z>(this->params.faultOfffset.Z);

	auto cMaxSmooth1 = FastNoise::New < FastNoise::MaxSmooth>();
	cMaxSmooth1->SetLHS(fOffset);
	cMaxSmooth1->SetRHS(1 - this->params.flowWidth);
	cMaxSmooth1->SetSmoothness(this->params.flowSmoothness);

	auto cMaxSmooth2 = FastNoise::New < FastNoise::MaxSmooth>();
	cMaxSmooth2->SetLHS(fOffset);
	cMaxSmooth2->SetRHS(1 - this->params.volcanoRidgeWidth);
	cMaxSmooth2->SetSmoothness(this->params.volcanoRidgeSmoothness);

	auto vfRemap = FastNoise::New < FastNoise::Remap>();
	vfRemap->SetSource(cMaxSmooth1);
	vfRemap->SetRemap(1 - this->params.flowWidth, 1, 0, 1);

	auto vrRemap = FastNoise::New < FastNoise::Remap>();
	vrRemap->SetSource(cMaxSmooth2);
	vrRemap->SetRemap(1 - this->params.volcanoRidgeWidth, 1, 0, 1);

	auto volcanicFlowMask = FastNoise::New<FastNoise::Multiply>();
	volcanicFlowMask->SetLHS(vfRemap);
	volcanicFlowMask->SetRHS(continentNormalizedMaxed);

	auto volcanicFlowRemap = FastNoise::New<FastNoise::Remap>();
	volcanicFlowRemap->SetSource(volcanicFlowMask);
	volcanicFlowRemap->SetRemap(0, 1, params.flowRemapMin, params.flowRemapMax);

	auto invertedFlowMask = FastNoise::New<FastNoise::Remap>();
	invertedFlowMask->SetSource(volcanicFlowMask);
	invertedFlowMask->SetRemap(0,1,1,0);

	auto volcanicRidgeMask = FastNoise::New<FastNoise::Multiply>();
	volcanicRidgeMask->SetLHS(vrRemap);
	volcanicRidgeMask->SetRHS(continentNormalizedMaxed);

	//lAVA FLOWS
	//simplex
	//frac ridge
	auto lfSimp = FastNoise::New<FastNoise::Simplex>();
	auto lfFracRidge = FastNoise::New<FastNoise::FractalRidged>();
	lfFracRidge->SetSource(lfSimp);
	lfFracRidge->SetOctaveCount(10);
	
	//pow2
	auto lfPow = FastNoise::New<FastNoise::PowInt>();
	lfPow->SetValue(lfFracRidge);
	lfPow->SetPow(2);	
	//remap
	auto lfRemap = FastNoise::New<FastNoise::Remap>();
	lfRemap->SetSource(lfPow);
	lfRemap->SetRemap(1, 0, params.lfrmMin, params.lfrmMax);
	//offset/DW

	auto lfDomainScale = FastNoise::New<FastNoise::DomainScale>();
	lfDomainScale->SetSource(lfRemap);
	lfDomainScale->SetScale(params.lfScale);

	auto lfMultiFlowMask1 = FastNoise::New<FastNoise::Multiply>();
	lfMultiFlowMask1->SetLHS(volcanicFlowMask);
	lfMultiFlowMask1->SetRHS(params.distortIntensity);

	auto lfDomainWarp = FastNoise::New<FastNoise::DomainWarpGradient>();
	lfDomainWarp->SetSource(lfDomainScale);
	lfDomainWarp->SetWarpAmplitude(lfMultiFlowMask1);

	auto lavaFlow = FastNoise::New<FastNoise::Add>();
	lavaFlow->SetLHS(lfDomainWarp);
	lavaFlow->SetRHS(volcanicFlowRemap);

	//VOLCANOS
	//Set up base cellular noise, remove spiky peaks and normalize
	//Base cellular noise has a very sharp "cone" pattern than when inverted leads to a lot of small sharp peaks
	//This rounds that off and inverts the noise giving more isolated volcano like peaks
	auto vCellDistance = FastNoise::New<FastNoise::CellularDistance>();
	vCellDistance->SetDistanceFunction(FastNoise::DistanceFunction::Euclidean);
	
	auto vMaxSmooth0 = FastNoise::New<FastNoise::MaxSmooth>();
	vMaxSmooth0->SetLHS(vCellDistance);
	vMaxSmooth0->SetRHS(params.volcanoNoiseRounding);

	auto vRemap0 = FastNoise::New<FastNoise::Remap>();
	vRemap0->SetSource(vMaxSmooth0);
	vRemap0->SetRemap(params.volcanoNoiseRounding, 1, 1, 0);

	auto vBaseNoise = FastNoise::New<FastNoise::FractalFBm>();
	vBaseNoise->SetSource(vRemap0);
	vBaseNoise->SetOctaveCount(params.volcanoOctaveCount);
	//End Base Noise

	//Blocking out mountains
	auto vMaxSmooth1 = FastNoise::New<FastNoise::MaxSmooth>();
	vMaxSmooth1->SetLHS(vBaseNoise);
	vMaxSmooth1->SetRHS(params.volcanoCutoff);

	auto vRemap1 = FastNoise::New<FastNoise::Remap>();
	vRemap1->SetSource(vMaxSmooth1);
	vRemap1->SetRemap(params.volcanoCutoff, 1, 0, 1);

	auto vPow0 = FastNoise::New<FastNoise::PowInt>();
	vPow0->SetValue(vRemap1);
	vPow0->SetPow(params.volcanoPow);

	auto vBaseMountains = FastNoise::New<FastNoise::Multiply>();
	vBaseMountains->SetLHS(vPow0);
	vBaseMountains->SetRHS(params.volcanoMulti);

	//Create peak inversion cutoff
	auto vDomScale0 = FastNoise::New<FastNoise::DomainScale>();
	vDomScale0->SetSource(vBaseNoise);
	vDomScale0->SetScale(params.volcanoPeakInversionCutoffScale);

	auto vPeakInversionCutoff = FastNoise::New<FastNoise::Remap>();
	vPeakInversionCutoff->SetSource(vDomScale0);
	vPeakInversionCutoff->SetRemap(0, 1, params.peakCutoffMin, params.peakCutoffMax);

	//Split mountains @ the inversion cutoff
	//The min here is the top "edge" of the volcano, cutoff so that we can substract the remaining peak to create the cavity
	auto vMinSmooth0 = FastNoise::New<FastNoise::MinSmooth>();
	vMinSmooth0->SetLHS(vBaseMountains);
	vMinSmooth0->SetRHS(vPeakInversionCutoff);
	vMinSmooth0->SetSmoothness(params.invertSmoothness);

	//everything over the cutoff becomes a vocanic cavity
	auto vMaxSmooth2 = FastNoise::New<FastNoise::MaxSmooth>();
	vMaxSmooth2->SetLHS(vBaseMountains);
	vMaxSmooth2->SetRHS(vPeakInversionCutoff);
	vMaxSmooth2->SetSmoothness(params.invertSmoothness);

	//Because our new "min" for the smoothmaxed peaks is the cutoff noise, we need to send it back to 0 by subtracting it... 
	//this will also impact the magnitude of the peaks but we will be applying a multiplier to them anyway
	auto vSub0 = FastNoise::New<FastNoise::Subtract>();
	vSub0->SetLHS(vMaxSmooth2);
	vSub0->SetRHS(vPeakInversionCutoff);

	auto vMul1 = FastNoise::New<FastNoise::Multiply>();
	vMul1->SetLHS(vSub0);
	vMul1->SetRHS(params.cavityMultiplier);

	//Finally we composite the mountains, cavities, and create a floor to avoid outlier depressions from going too far
	auto vSub1 = FastNoise::New<FastNoise::Subtract>();
	vSub1->SetLHS(vMinSmooth0);
	vSub1->SetRHS(vMul1);

	//Apply a final multiplier
	auto vMul2 = FastNoise::New<FastNoise::Multiply>();
	vMul2->SetLHS(vSub1);
	vMul2->SetRHS(params.finalVolcanoMultiplier);

	////We will use -.1 which should always be comfortably below sea level
	//auto vMax0 = FastNoise::New<FastNoise::Max>();
	//vMax0->SetLHS(vMul2);
	//vMax0->SetRHS(-.1);

	//finally apply the desired domain scale
	auto vDomScale1 = FastNoise::New<FastNoise::DomainScale>();
	vDomScale1->SetSource(vMul2);
	vDomScale1->SetScale(params.volcanoFrequency);

	auto volcanos = FastNoise::New<FastNoise::Multiply>();
	volcanos->SetLHS(vDomScale1);
	volcanos->SetRHS(vrRemap);

	
	//CRACKS
	auto cImport = FastNoise::NewFromEncodedNodeTree("IAAfABcAAAAAAAAAgD8AAMA/AACAvw0ABgAAAAAAAEATAAAAwEALAAEAAAAAAAAAAQAAAAQAAAAAAACAPwAAAAA/AAAAAAAAzcxMPQDD9ag+AM3MTL0Aw/WoPg==");
	auto cDomScale = FastNoise::New<FastNoise::DomainScale>();
	cDomScale->SetSource(cImport);
	cDomScale->SetScale(params.crackScale);

	auto invertedFlowPow = FastNoise::New<FastNoise::PowInt>();
	invertedFlowPow->SetValue(invertedFlowMask);
	invertedFlowPow->SetPow(params.crackFalloff);

	auto cMul0 = FastNoise::New<FastNoise::Multiply>();
	cMul0->SetLHS(invertedFlowPow);
	cMul0->SetRHS(cDomScale);

	auto cracks = FastNoise::New<FastNoise::Multiply>();
	cracks->SetLHS(cDomScale);
	cracks->SetRHS(params.crackIntensity);
	//COMPOSITE
	//auto vMulti = FastNoise::New<FastNoise::Multiply>();
	//vMulti->SetLHS(vDomScale0);
	//vMulti->SetRHS(3);

	//auto fMulti = FastNoise::New<FastNoise::Multiply>();
	//fMulti->SetLHS(lavaFlow);
	//fMulti->SetRHS(1);

	auto cBulge = FastNoise::New<FastNoise::Multiply>();
	cBulge->SetLHS(volcanicFlowMask);
	cBulge->SetRHS(continentBase);

	auto cMax = FastNoise::New<FastNoise::MaxSmooth>();
	cMax->SetLHS(cBulge);
	cMax->SetRHS(cracks);

	auto add0 = FastNoise::New<FastNoise::Add>();
	add0->SetLHS(continentBase);
	add0->SetRHS(cracks);



	//auto add1 = FastNoise::New<FastNoise::Add>();
	//add1->SetLHS(add0);
	//add1->SetRHS(vMul2);

	//Add a tiny amount so we dont see the ocean z fighting
	auto add1 = FastNoise::New<FastNoise::Add>();
	add1->SetLHS(continentBase);
	add1->SetRHS(lavaFlow);

	auto add2 = FastNoise::New<FastNoise::Add>();
	add2->SetLHS(volcanicFlowRemap);
	add2->SetRHS(volcanos);

	auto smMax = FastNoise::New<FastNoise::MaxSmooth>();
	smMax->SetLHS(add2);
	smMax->SetRHS(lavaFlow);
	smMax->SetSmoothness(.25);
	
	auto smMax1 = FastNoise::New<FastNoise::MaxSmooth>();
	smMax1->SetLHS(smMax);
	smMax1->SetRHS(add0);
	smMax1->SetSmoothness(.1);

	this->OutputNode = smMax1;
}

void MoltenNoiseGenerator::InitializeParams(int inSeed)
{
	// Create a random stream using the seed
	FRandomStream RandomStream(inSeed);

	params.continentOctaves = RandomStream.RandRange(paramBounds.continentOctaves.min, paramBounds.continentOctaves.max);
	params.continentAboveSeaLevelCuttoff = RandomStream.FRandRange(paramBounds.continentAboveSeaLevelCuttoff.min, paramBounds.continentAboveSeaLevelCuttoff.max);
	params.continentBelowSeaLevelCutoff = RandomStream.FRandRange(paramBounds.continentBelowSeaLevelCutoff.min, paramBounds.continentBelowSeaLevelCutoff.max);

	params.flowWidth = RandomStream.FRandRange(paramBounds.flowWidth.min, paramBounds.flowWidth.max);
	params.flowSmoothness = RandomStream.FRandRange(paramBounds.flowSmoothness.min, paramBounds.flowSmoothness.max);
	
	params.volcanoRidgeWidth = RandomStream.FRandRange(paramBounds.volcanoRidgeWidth.min, paramBounds.volcanoRidgeWidth.max);
	params.volcanoRidgeSmoothness = RandomStream.FRandRange(paramBounds.volcanoRidgeSmoothness.min, paramBounds.volcanoRidgeSmoothness.max);

	params.faultOfffset.X = RandomStream.FRandRange(paramBounds.faultOfffset.min.X, paramBounds.faultOfffset.max.X);
	params.faultOfffset.Y = RandomStream.FRandRange(paramBounds.faultOfffset.min.Y, paramBounds.faultOfffset.max.Y);
	params.faultOfffset.Z = RandomStream.FRandRange(paramBounds.faultOfffset.min.Z, paramBounds.faultOfffset.max.Z);

	params.lfScale = RandomStream.FRandRange(paramBounds.lfScale.min, paramBounds.lfScale.max);
	params.baseOffset = RandomStream.FRandRange(paramBounds.baseOffset.min, paramBounds.baseOffset.max);
	params.distortIntensity = RandomStream.FRandRange(paramBounds.distortIntensity.min, paramBounds.distortIntensity.max);
	params.volcanoNoiseRounding = RandomStream.FRandRange(paramBounds.volcanoNoiseRounding.min, paramBounds.volcanoNoiseRounding.max);
	params.volcanoCutoff = RandomStream.FRandRange(paramBounds.volcanoCutoff.min, paramBounds.volcanoCutoff.max);
	params.volcanoPow = RandomStream.RandRange(paramBounds.volcanoPow.min, paramBounds.volcanoPow.max);
	params.volcanoMulti = RandomStream.FRandRange(paramBounds.volcanoMulti.min, paramBounds.volcanoMulti.max);
	params.volcanoPeakInversionCutoffScale = RandomStream.FRandRange(paramBounds.volcanoPeakInversionCutoffScale.min, paramBounds.volcanoPeakInversionCutoffScale.max);
	params.peakCutoffMin = RandomStream.FRandRange(paramBounds.peakCutoffMin.min, paramBounds.peakCutoffMin.max);
	params.peakCutoffMax = RandomStream.FRandRange(paramBounds.peakCutoffMax.min, paramBounds.peakCutoffMax.max);
	params.volcanoOctaveCount = RandomStream.RandRange(paramBounds.volcanoOctaveCount.min, paramBounds.volcanoOctaveCount.max);
	params.volcanoFrequency = RandomStream.FRandRange(paramBounds.volcanoFrequency.min, paramBounds.volcanoFrequency.max);
	params.invertSmoothness = RandomStream.FRandRange(paramBounds.invertSmoothness.min, paramBounds.invertSmoothness.max);
	params.cavityMultiplier = RandomStream.FRandRange(8, 12);
	params.finalVolcanoMultiplier = RandomStream.FRandRange(paramBounds.finalVolcanoMultiplier.min, paramBounds.finalVolcanoMultiplier.max);
	params.crackScale = RandomStream.FRandRange(paramBounds.crackScale.min, paramBounds.crackScale.max);
	params.crackFalloff = RandomStream.RandRange(paramBounds.crackFalloff.min, paramBounds.crackFalloff.max);
	params.crackIntensity = RandomStream.FRandRange(paramBounds.crackIntensity.min, paramBounds.crackIntensity.max);
	params.lfrmMin = RandomStream.FRandRange(paramBounds.lfrmMin.min, paramBounds.lfrmMin.max);
	params.lfrmMax = RandomStream.FRandRange(paramBounds.lfrmMax.min, paramBounds.lfrmMax.max);
	params.flowRemapMin = RandomStream.FRandRange(paramBounds.flowRemapMin.min, paramBounds.flowRemapMin.max);
	params.flowRemapMax = RandomStream.FRandRange(paramBounds.flowRemapMax.min, paramBounds.flowRemapMax.max);
}

void FrozenNoiseGenerator::InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel)
{
	this->Seed = inSeed;
	this->AmplitudeScale = inAmplitudeScale;
	this->FrequencyScale = inFrequencyScale;
	this->SeaLevel = inSeaLevel;
	auto nodeStr = "GQAbABcAAACAPpqZmT4AAAA/AAAAvxMAmpmZPSAAHwANABAAAAAAAABAEAAAAAA/FwAAAIC/AAAAAAAAgD8AAAAACwABAAAAAAAAAAEAAAACAAAAAAAAgD8AAACAPwAAAAA/AAAAAAAAmpmZPgDNzMw9AAAAgD4AzczMPQETAM3MTD0XAAAAAAAAAIA/AAAAAAAAAEASAAYAAAAAAABA//8CAAAAAAA/AAAAAAABGwD//wcAASAAGQAbABcAAACAvwAAgD8AAAAAAACAPxMAAACgQBsAGQAUAB8AHwAlAAAAgD8AAEBAAACAPwAAgD8HAAEVAM3MTD4AAAAAAAAAABYAAQAAAP//DQAAzczMPQEVAM3MTL4AAAAAAAAAABYAAgAAAP//DQAAzczMPQETAAAAgD7//wwAAf//FAAAAAAAAAAAAAAAARMAAACAPv//FQAAAACAPwAAAAA+AAAAgD4BFwAAAAAAAACAPwAAAAAAAABAJAACAAAADQAIAAAAAAAAQAsAAQAAAAAAAAABAAAAAAAAAAAAAIA/AAAAAD8AAAAAAADNzMw8";
	auto encodedTest = FastNoise::NewFromEncodedNodeTree(nodeStr);
	auto scaleEncoded = FastNoise::New<FastNoise::DomainScale>();
	scaleEncoded->SetSource(encodedTest);
	scaleEncoded->SetScale(32 * this->FrequencyScale);
	this->OutputNode = scaleEncoded;
}

void RockyNoiseGenerator::InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel)
{
	this->UsePreprocess = true;
	this->Seed = inSeed;
	this->AmplitudeScale = inAmplitudeScale;
	this->FrequencyScale = inFrequencyScale;
	this->SeaLevel = inSeaLevel;
	auto nodeStr = "EwDNzMw9FwC4HoW+AAAAPwAAgL8AAIA/GwAfACAAGwAbABkAEwAAAAA/DQAMAAAAAAAAQAsAAQAAAAAAAAABAAAAAAAAAAAAAIA/AAAAAD8AAAAAAAAAAIC+AAAAAEAAAAAAQAAAAAAAAAAAgD8AAACAPgAAAIA/ARcAAACAvwAAgD8AAAAAAACAPw0ADAAAAAAAAEAbAAcAAAAAAEAAAAAAPwAAAAAA";
	auto encodedTest = FastNoise::NewFromEncodedNodeTree(nodeStr);
	auto scaleEncoded = FastNoise::New<FastNoise::DomainScale>();
	scaleEncoded->SetSource(encodedTest);
	scaleEncoded->SetScale(32 * this->FrequencyScale);
	this->OutputNode = scaleEncoded;
}

FVector RockyNoiseGenerator::PreProcess(FVector inPosition)
{
	FCraterNoise cNoise;
	FVector scaledPos = inPosition;
	FVector4 craterOutput = cNoise.CraterFBM(scaledPos*2);
	double finalDisplacement = craterOutput.X;
	finalDisplacement *= 4.0; 
	return inPosition * (finalDisplacement * AmplitudeScale + 1);
}

void DuneNoiseGenerator::InitializeNode(int inSeed, double inAmplitudeScale, double inFrequencyScale, double inSeaLevel)
{
	this->Seed = inSeed;
	this->AmplitudeScale = inAmplitudeScale;
	this->FrequencyScale = inFrequencyScale;
	this->SeaLevel = inSeaLevel;
	auto nodeStr = "IAAZABsAEwDNzMw9DQAGAAAAAAAAQAcAAAAAAD8AAAAAAAAAAABAARsAEwAAAEBAGQAbABcAAACAvwAAgD8AAAAAAACAPx8AHwAlAAAAAD8AAIA/AACAPwAAgD8HAAEVAJqZmT4AAAAAAAAAAP//BQAAzczMPQEVAJqZmb4AAAAAAAAAAP//BQAAzczMPQEZABsAHAABGwAaAAEUABkA//8JAAAAAAA/AM3MzD0AAAAAAAAAAAAAAAAAAAAB//8LAAEaAAEUAP//CwAAAAAAAADNzMw9AAAAAAAAAAAAAAH//wsAAM3MzD0BFAAfAB8AJQAAAIA/AAAgQQAAIEEAAIA///8JAAEVAM3MTD4AAAAAAAAAAP//EgAAzczMPQEVAM3MTL4AAAAAAAAAAP//EwAAzczMPQH//w0AAf//DwAAAAAAAAAAAAAAAAAAgD8BGwATAAAAQED//xoAAMP1qD4AmpkZPgEZABoAAf//AwAAzczMPQETAM3MTD4bAB8AJAAGAAAADQAMAAAAAAAAQAsAAQAAAAEAAAAAAAAAAgAAAAAAAIA/AAAAAD8AAAAAAACPwnU9AM3MzD0Aw/XoQADNzEw9";
	auto encodedTest = FastNoise::NewFromEncodedNodeTree(nodeStr);
	auto scaleEncoded = FastNoise::New<FastNoise::DomainScale>();
	scaleEncoded->SetSource(encodedTest);
	scaleEncoded->SetScale(32 * this->FrequencyScale);
	this->OutputNode = scaleEncoded;
}