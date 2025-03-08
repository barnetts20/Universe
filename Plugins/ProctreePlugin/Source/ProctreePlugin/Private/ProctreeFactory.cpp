// Fill out your copyright notice in the Description page of Project Settings.


#include "ProctreeFactory.h"

UProctreeWrapper* UProctreeFactory::ConstructProctree(int32 MaxDepth, int32 SizeMultiplier)
{
	UProctreeWrapper* returnWrapper = NewObject<UProctreeWrapper>();
	returnWrapper->Initialize(SizeMultiplier, MaxDepth);
	return returnWrapper;
}
