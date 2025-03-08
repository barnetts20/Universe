#include "UProctreeWrapper.h"
#include "Math/Box.h"
#include <FProctreeAsync.h>

UProctreeWrapper::UProctreeWrapper()
{
}

UProctreeWrapper::~UProctreeWrapper()
{
	this->RootNode = nullptr;
	Super::BeginDestroy();
}

void UProctreeWrapper::Initialize(int32 SizeMultiplier, int32 MaxDepth)
{
	this->LeafSizeMultiplier = SizeMultiplier;
	this->MaximumDepth = MaxDepth;
	
	FProctreeNodeData* initRootData = new FProctreeNodeData();

	//Initialize Root Values
	initRootData->Path = "0";
	initRootData->Index = 0;
	initRootData->Depth = 0;
	initRootData->Center = { 0.0,0.0,0.0 };
	initRootData->Size = ((LeafSizeMultiplier) * pow(2, MaximumDepth)) * .5f;
	initRootData->BoundingBox = FBox::BuildAABB(initRootData->Center, FVector(initRootData->Size));
	initRootData->Activated = true;
	initRootData->LocationOffset = { 0.0,0.0,0.0 };
	initRootData->RelativeScale = 1;
	initRootData->Density = 1;
	initRootData->Temperature = 1;

	this->RootNode = TSharedPtr<FProctreeNode>(new FProctreeNode());
	this->RootNode->NodeData = *initRootData;
}

UProctreeNodeWrapper* UProctreeWrapper::GetRoot() const
{
	if (this->RootNode == nullptr) {
		return nullptr;
	}
	UProctreeNodeWrapper* tRoot = NewObject<UProctreeNodeWrapper>();
	tRoot->SetInternalNode(this->RootNode);
	return tRoot;
}

FProctreeBulkNodeResult UProctreeWrapper::BulkInsertNodesByPointCloudAndDepth(UProctreeNodeWrapper* Node, TArray<FVector> Points, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{
	TArray<TSharedPtr<FProctreeNode>> insertedNodes = Node->GetInternalNode()->BulkInsertNodesByPointCloudAndDepth(Node->GetInternalNode(), Points, Depth, RandBounds, Activate);
	TArray<UProctreeNodeWrapper*> tWrappers;
	for (TSharedPtr<FProctreeNode> iNode : insertedNodes)
	{
		UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
		tChild->SetInternalNode(iNode);
		tWrappers.Add(tChild);
	}

	FProctreeBulkNodeResult resultWrapper;
	resultWrapper.ReferenceNode = Node;
	resultWrapper.Depth = Depth;
	resultWrapper.Filter = RandBounds.NodeType;
	resultWrapper.ResultNodes = tWrappers;
	return resultWrapper;
}

FProctreeBulkNodeResult UProctreeWrapper::FindNodesAtDepthFromNode(UProctreeNodeWrapper* Node, int32 Depth, EProctreeNodeType Filter)
{
	TArray<TSharedPtr<FProctreeNode>> foundNodes = Node->GetInternalNode()->FindNodesAtDepthFromNode(Node->GetInternalNode(), Depth, Filter);
	TArray<UProctreeNodeWrapper*> tWrappers;
	for (TSharedPtr<FProctreeNode> iNode : foundNodes)
	{
		UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
		tChild->SetInternalNode(iNode);
		tWrappers.Add(tChild);
	}

	FProctreeBulkNodeResult resultWrapper;
	resultWrapper.ReferenceNode = Node;
	resultWrapper.Depth = Depth;
	resultWrapper.Filter = Filter;
	resultWrapper.ResultNodes = tWrappers;
	return resultWrapper;
}

FProctreeBulkNodeResult UProctreeWrapper::FindNodesInDepthFromNode(UProctreeNodeWrapper* Node, int32 Depth, EProctreeNodeType Filter)
{
	TArray<TSharedPtr<FProctreeNode>> foundNodes = Node->GetInternalNode()->FindNodesInDepthFromNode(Node->GetInternalNode(), Depth, Filter);
	TArray<UProctreeNodeWrapper*> tWrappers;
	for (TSharedPtr<FProctreeNode> iNode : foundNodes)
	{
		UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
		tChild->SetInternalNode(iNode);
		tWrappers.Add(tChild);
	}

	FProctreeBulkNodeResult resultWrapper;
	resultWrapper.ReferenceNode = Node;
	resultWrapper.Depth = Depth;
	resultWrapper.Filter = Filter;
	resultWrapper.ResultNodes = tWrappers;
	return resultWrapper;
}

FProctreeBulkNodeResult UProctreeWrapper::FindNearestNeighbors(UProctreeNodeWrapper* Node, FVector Position, int32 Depth, int32 Iterations, EProctreeNodeType Filter)
{
	TArray<TSharedPtr<FProctreeNode>> foundNodes = Node->GetInternalNode()->FindNearestNeighbors(Node->GetInternalNode(), Position, Depth, Iterations, Filter);
	TArray<UProctreeNodeWrapper*> tWrappers;
	for (TSharedPtr<FProctreeNode> iNode : foundNodes)
	{
		UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
		tChild->SetInternalNode(iNode);
		tWrappers.Add(tChild);
	}

	FProctreeBulkNodeResult resultWrapper;
	resultWrapper.ReferenceNode = Node;
	resultWrapper.Depth = Depth;
	resultWrapper.Filter = Filter;
	resultWrapper.ResultNodes = tWrappers;
	return resultWrapper;
}

FProctreeBulkNodeResult UProctreeWrapper::FindNodesInLineTrace(UProctreeNodeWrapper* Node, FVector Position, FVector Direction, int32 Depth, int32 Iterations, EProctreeNodeType Filter)
{
	TArray<TSharedPtr<FProctreeNode>> foundNodes = Node->GetInternalNode()->FindLineTraceNodes(Node->GetInternalNode(), Position, Direction, Depth, Iterations, Filter);
	TArray<UProctreeNodeWrapper*> tWrappers;
	for (TSharedPtr<FProctreeNode> iNode : foundNodes)
	{
		UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
		tChild->SetInternalNode(iNode);
		tWrappers.Add(tChild);
	}

	FProctreeBulkNodeResult resultWrapper;
	resultWrapper.ReferenceNode = Node;
	resultWrapper.Depth = Depth;
	resultWrapper.Filter = Filter;
	resultWrapper.ResultNodes = tWrappers;
	return resultWrapper;
}

void UProctreeWrapper::AsyncBulkInsertNodesByPointCloudAndDepth(UProctreeNodeWrapper* ReferenceNode, TArray<FVector> Points, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{
	int32 tDepth = Depth;
	TSharedPtr<FProctreeNode> refNode = ReferenceNode->GetInternalNode();
	TPromise<TArray<TSharedPtr<FProctreeNode>>>* promise = new TPromise<TArray<TSharedPtr<FProctreeNode>>>();
	TFuture<TArray<TSharedPtr<FProctreeNode>>> future = promise->GetFuture();
	FAsyncBulkInsertTask* bulkInsertTask = new FAsyncBulkInsertTask(promise, refNode, Points, Depth, RandBounds, Activate);

	future.Next([this, ReferenceNode, tDepth](TArray<TSharedPtr<FProctreeNode>> result)
	{
		AsyncTask(ENamedThreads::GameThread, [this, ReferenceNode, tDepth, result]()
		{
			TArray<UProctreeNodeWrapper*> tWrappers;
			for (TSharedPtr<FProctreeNode> iNode : result)
			{
				UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
				tChild->SetInternalNode(iNode);
				tWrappers.Add(tChild);
			}
			FProctreeBulkNodeResult* ResultWrapper = new FProctreeBulkNodeResult();
			ResultWrapper->ReferenceNode = ReferenceNode;
			ResultWrapper->ResultNodes = tWrappers;
			ResultWrapper->Depth = tDepth;
			//Need to dispatch an event here to notify the caller that the async task is complete.
			UE_LOG(LogTemp, Log, TEXT("Emitting inserted wrappers: %d"), tWrappers.Num());
			if (this->BulkInsertBroadcaster.IsBound()) {
				this->BulkInsertBroadcaster.Broadcast(*ResultWrapper);
			
			}
		}); 
	});
	bulkInsertTask->Start();
}

void UProctreeWrapper::AsyncFindNodesAtDepthFromNode(UProctreeNodeWrapper* ReferenceNode, int32 Depth, EProctreeNodeType Filter)
{
	TSharedPtr<FProctreeNode> refNode = ReferenceNode->GetInternalNode();
	int32 tDepth = Depth;
	EProctreeNodeType tFilter = Filter;

	TPromise<TArray<TSharedPtr<FProctreeNode>>>* promise = new TPromise<TArray<TSharedPtr<FProctreeNode>>>();
	TFuture<TArray<TSharedPtr<FProctreeNode>>> future = promise->GetFuture();
	FAsyncFindAtDepthTask* findAtDepthTask = new FAsyncFindAtDepthTask(promise, refNode, Depth, tFilter);

	future.Next([this, ReferenceNode, tDepth, tFilter](TArray<TSharedPtr<FProctreeNode>> result)
	{
		AsyncTask(ENamedThreads::GameThread, [this, ReferenceNode, tDepth, tFilter, result]()
		{
			TArray<UProctreeNodeWrapper*> tWrappers;
			for (TSharedPtr<FProctreeNode> iNode : result)
			{
				UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
				tChild->SetInternalNode(iNode);
				tWrappers.Add(tChild);
			}
			FProctreeBulkNodeResult* ResultWrapper = new FProctreeBulkNodeResult();
			ResultWrapper->ReferenceNode = ReferenceNode;
			ResultWrapper->ResultNodes = tWrappers;
			ResultWrapper->Depth = tDepth;
			ResultWrapper->Filter = tFilter;
			//Need to dispatch an event here to notify the caller that the async task is complete.
			UE_LOG(LogTemp, Log, TEXT("Emitting found wrappers: %d"), tWrappers.Num());
			if (this->FindAtDepthBroadcaster.IsBound()) {
				this->FindAtDepthBroadcaster.Broadcast(*ResultWrapper);
			}
		});
	});
	findAtDepthTask->Start();
}

void UProctreeWrapper::AsyncFindNodesInDepthFromNode(UProctreeNodeWrapper* ReferenceNode, int32 Depth, EProctreeNodeType Filter)
{
	TSharedPtr<FProctreeNode> refNode = ReferenceNode->GetInternalNode();
	int32 tDepth = Depth;
	EProctreeNodeType tFilter = Filter;
	
	TPromise<TArray<TSharedPtr<FProctreeNode>>>* promise = new TPromise<TArray<TSharedPtr<FProctreeNode>>>();
	TFuture<TArray<TSharedPtr<FProctreeNode>>> future = promise->GetFuture();
	FAsyncFindInDepthTask* findInDepthTask = new FAsyncFindInDepthTask(promise, refNode, Depth, tFilter);

	future.Next([this, ReferenceNode, tDepth, tFilter](TArray<TSharedPtr<FProctreeNode>> result)
	{
		AsyncTask(ENamedThreads::GameThread, [this, ReferenceNode, tDepth, tFilter, result]()
		{
			TArray<UProctreeNodeWrapper*> tWrappers;
			for (TSharedPtr<FProctreeNode> iNode : result)
			{
				UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
				tChild->SetInternalNode(iNode);
				tWrappers.Add(tChild);
			}

			FProctreeBulkNodeResult* ResultWrapper = new FProctreeBulkNodeResult();
			ResultWrapper->ReferenceNode = ReferenceNode;
			ResultWrapper->ResultNodes = tWrappers;
			ResultWrapper->Depth = tDepth;
			ResultWrapper->Filter = tFilter;
			//Need to dispatch an event here to notify the caller that the async task is complete.
			UE_LOG(LogTemp, Log, TEXT("Emitting found wrappers: %d"), tWrappers.Num());
			if (this->FindInDepthBroadcaster.IsBound()) {
				this->FindInDepthBroadcaster.Broadcast(*ResultWrapper);
			}
		});
	});
	findInDepthTask->Start();
}

void UProctreeWrapper::AsyncFindNearestNeighbors(UProctreeNodeWrapper* ReferenceNode, FVector Position, int32 Depth, int32 Iterations, EProctreeNodeType Filter)
{
	UProctreeNodeWrapper* root = GetRoot();
	FVector tOrigin = Position;
	int32 tDepth = Depth;
	int32 tIterations = Iterations;
	EProctreeNodeType tFilter = Filter;

	TPromise<TArray<TSharedPtr<FProctreeNode>>>* promise = new TPromise<TArray<TSharedPtr<FProctreeNode>>>();
	TFuture<TArray<TSharedPtr<FProctreeNode>>> future = promise->GetFuture();
	FAsyncNearestNeighborTask* findNearestNeighborsTask = new FAsyncNearestNeighborTask(promise, root->GetInternalNode(), tOrigin, tDepth, tIterations, tFilter);
	
	future.Next([this, root, tDepth, tFilter](TArray<TSharedPtr<FProctreeNode>> result)
		{
			AsyncTask(ENamedThreads::GameThread, [this, root, tDepth, tFilter, result]()
				{
					TArray<UProctreeNodeWrapper*> tWrappers;
					for (TSharedPtr<FProctreeNode> iNode : result)
					{
						UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
						tChild->SetInternalNode(iNode);
						tWrappers.Add(tChild);
					}

					FProctreeBulkNodeResult* ResultWrapper = new FProctreeBulkNodeResult();
					ResultWrapper->ReferenceNode = root;
					ResultWrapper->ResultNodes = tWrappers;
					ResultWrapper->Depth = tDepth;
					ResultWrapper->Filter = tFilter;
					//Need to dispatch an event here to notify the caller that the async task is complete.
					UE_LOG(LogTemp, Log, TEXT("Emitting found wrappers: %d"), tWrappers.Num());
					if (this->FindNearestNeighborBroadcaster.IsBound()) {
						this->FindNearestNeighborBroadcaster.Broadcast(*ResultWrapper);
					}
				});
		});
	findNearestNeighborsTask->Start();
}

void UProctreeWrapper::AsyncFindNodesInLineTrace(UProctreeNodeWrapper* ReferenceNode, FVector Position, FVector Direction, int32 Depth, int32 Iterations, EProctreeNodeType Filter)
{
	UProctreeNodeWrapper* root = GetRoot();
	int32 tDepth = Depth;
	FVector tOrigin = Position;
	FVector tDirection = Direction;
	int32 tIterations = Iterations;
	EProctreeNodeType tFilter = Filter;

	TPromise<TArray<TSharedPtr<FProctreeNode>>>* promise = new TPromise<TArray<TSharedPtr<FProctreeNode>>>();
	TFuture<TArray<TSharedPtr<FProctreeNode>>> future = promise->GetFuture();
	FAsyncLineTraceTask* findLineTraceTask = new FAsyncLineTraceTask(promise, root->GetInternalNode(), tOrigin, tDirection, tDepth, tIterations, tFilter);

	future.Next([this, root, tDepth, tFilter](TArray<TSharedPtr<FProctreeNode>> result)
		{
			AsyncTask(ENamedThreads::GameThread, [this, root, tDepth, tFilter, result]()
				{
					TArray<UProctreeNodeWrapper*> tWrappers;
					for (TSharedPtr<FProctreeNode> iNode : result)
					{
						UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
						tChild->SetInternalNode(iNode);
						tWrappers.Add(tChild);
					}

					FProctreeBulkNodeResult* ResultWrapper = new FProctreeBulkNodeResult();
					ResultWrapper->ReferenceNode = root;
					ResultWrapper->ResultNodes = tWrappers;
					ResultWrapper->Depth = tDepth;
					ResultWrapper->Filter = tFilter;
					//Need to dispatch an event here to notify the caller that the async task is complete.
					UE_LOG(LogTemp, Log, TEXT("Emitting found wrappers: %d"), tWrappers.Num());
					if (this->FindLineTraceBroadcaster.IsBound()) {
						this->FindLineTraceBroadcaster.Broadcast(*ResultWrapper);
					}
				});
		});
	findLineTraceTask->Start();
}
