// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlanetModule.h"

#define LOCTEXT_NAMESPACE "FPlanetModuleModule"

void FPlanetModuleModule::StartupModule()
{
	/*FastNoise::SmartNode<FastNoise::OpenSimplex2> os2Noise = FastNoise::New<FastNoise::OpenSimplex2>();
	FastSIMD::eLevel simdLevel = os2Noise->GetSIMDLevel();*/
	
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FPlanetModuleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FPlanetModuleModule, PlanetModule)