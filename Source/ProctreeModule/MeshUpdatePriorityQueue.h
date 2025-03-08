#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlanetSharedStructs.h"

class PROCTREEMODULE_API UMeshUpdatePriorityQueue
{
private: 
	//TArray<int> SortedPriorities;
	//TMap<int, TArray<TSharedPtr<FMeshUpdateDataBatch>>> PriorityQueueMap;
	//TMap<FString, double> LastUpdateTimestampMap;
	FRWLock qLock;
	TQueue<TSharedPtr<FMeshUpdateDataBatch>> PriorityQueue;
public:
	// Sets default values for this empty's properties
	UMeshUpdatePriorityQueue();
	~UMeshUpdatePriorityQueue();
	void EnqueUpdateData(TSharedPtr<FMeshUpdateDataBatch> InUpdateData);
	TArray<TSharedPtr<FMeshUpdateDataBatch>> GetNextUpdates(int InNumUpdates);
};
