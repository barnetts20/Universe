#include "MeshUpdatePriorityQueue.h"

// Sets default values
// Priority Queue Implementation
UMeshUpdatePriorityQueue::UMeshUpdatePriorityQueue() {

}
UMeshUpdatePriorityQueue::~UMeshUpdatePriorityQueue()
{
	// Clearing all data structures
	//this->SortedPriorities.Empty();
	//this->PriorityQueueMap.Empty();
	//this->LastUpdateTimestampMap.Empty();
	this->PriorityQueue.Empty();
}

void UMeshUpdatePriorityQueue::EnqueUpdateData(TSharedPtr<FMeshUpdateDataBatch> InUpdateData)
{
	this->PriorityQueue.Enqueue(InUpdateData);
}

TArray<TSharedPtr<FMeshUpdateDataBatch>> UMeshUpdatePriorityQueue::GetNextUpdates(int InNumUpdates)
{
	int updatesRetrieved = 0;
	int culledCount = 0;
	TArray<TSharedPtr<FMeshUpdateDataBatch>> updates;	
	if (this->PriorityQueue.IsEmpty()) {
		return updates;
	}
	//FWriteScopeLock WriteLock(this->qLock);
	for (int i = 0; i < InNumUpdates; i++) {
		TSharedPtr<FMeshUpdateDataBatch> db;
		this->PriorityQueue.Dequeue(db);
		if (db.IsValid()) {
			updates.Add(db);
		}
		else {
			break;
		}
	}
	return updates;
}
