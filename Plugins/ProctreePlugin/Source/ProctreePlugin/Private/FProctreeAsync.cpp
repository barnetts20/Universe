// Fill out your copyright notice in the Description page of Project Settings.


#include "FProctreeAsync.h"
// Define the static lock outside of the class definition
FRWLock FProctreeAsync::TreeLock;

FProctreeAsync::FProctreeAsync()
{
}

FProctreeAsync::~FProctreeAsync()
{
}

/// <summary>
/// Bulk Insert
/// </summary>
FAsyncBulkInsertTask::FAsyncBulkInsertTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> ReferenceNode, TArray<FVector> Points, int32 Depth, FProctreeNodeRandomizationBounds RandomizationBounds, bool Activated)
{
	rNode = ReferenceNode;
	rPoints = Points;
	rDepth = Depth;
	rRandomizationBounds = RandomizationBounds;
	rActivated = Activated;
	biPromise = Promise;
}

FAsyncBulkInsertTask::~FAsyncBulkInsertTask()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

void FAsyncBulkInsertTask::Start() {
	Thread = FRunnableThread::Create(this, TEXT("BulkInsert"));
}

bool FAsyncBulkInsertTask::Init()
{
	bStop = false;
	return true;
}

uint32 FAsyncBulkInsertTask::Run()
{
	FRWScopeLock Lock(FProctreeAsync::TreeLock, SLT_Write);
	try {		
		TArray<TSharedPtr<FProctreeNode>> ReturnNodes = rNode.ToSharedRef()->BulkInsertNodesByPointCloudAndDepth(rNode, rPoints, rDepth, rRandomizationBounds, rActivated);
		biPromise->SetValue(ReturnNodes);
		return 0;
	}
	catch (const std::exception& e) {
		// Your code to handle the exception goes here
		UE_LOG(LogTemp, Error, TEXT("Exception caught: %s"), UTF8_TO_TCHAR(e.what()));
		return 1;
	}
}

void FAsyncBulkInsertTask::Stop()
{
	bStop = true;
}

void FAsyncBulkInsertTask::Exit()
{
	delete biPromise;
}
/// <summary>
/// End Bulk Insert
/// </summary>

/// <summary>
/// Find At Depth
/// </summary>
FAsyncFindAtDepthTask::FAsyncFindAtDepthTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> ReferenceNode, int32 Depth, EProctreeNodeType FilterType)
{
	fadPromise = Promise;

	rNode = ReferenceNode;
	rDepth = Depth;
	rFilter = FilterType;
}

FAsyncFindAtDepthTask::~FAsyncFindAtDepthTask()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

void FAsyncFindAtDepthTask::Start()
{
	Thread = FRunnableThread::Create(this, TEXT("FindAtDepth"));
}

bool FAsyncFindAtDepthTask::Init()
{
	bStop = false;
	return true;
}

uint32 FAsyncFindAtDepthTask::Run()
{
	FRWScopeLock Lock(FProctreeAsync::TreeLock, SLT_ReadOnly);
	try {
		TArray<TSharedPtr<FProctreeNode>> ReturnNodes = rNode->FindNodesAtDepthFromNode(rNode, rDepth);

		ReturnNodes.Num();
		fadPromise->SetValue(ReturnNodes);
		//Return 0 if everything went fine
		return 0;
	}
	catch (const std::exception& e) {
		// Your code to handle the exception goes here
		UE_LOG(LogTemp, Error, TEXT("Exception caught: %s"), UTF8_TO_TCHAR(e.what()));
		return 1;
	}
}

void FAsyncFindAtDepthTask::Stop()
{
	bStop = true;
}

void FAsyncFindAtDepthTask::Exit()
{
	delete fadPromise;

}
/// <summary>
/// End Find At Depth
/// </summary>

/// <summary>
/// Find In Depth
/// </summary>
FAsyncFindInDepthTask::FAsyncFindInDepthTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> ReferenceNode, int32 Depth, EProctreeNodeType FilterType)
{
	fidPromise = Promise;

	rNode = ReferenceNode;
	rDepth = Depth;
	rFilter = FilterType;
}

FAsyncFindInDepthTask::~FAsyncFindInDepthTask()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

void FAsyncFindInDepthTask::Start()
{
	Thread = FRunnableThread::Create(this, TEXT("FindInDepth"));
}

bool FAsyncFindInDepthTask::Init()
{
	bStop = false;
	return true;
}

uint32 FAsyncFindInDepthTask::Run()
{
	FRWScopeLock Lock(FProctreeAsync::TreeLock, SLT_ReadOnly);
	try {
		TArray<TSharedPtr<FProctreeNode>> ReturnNodes = rNode->FindNodesInDepthFromNode(rNode, rDepth);
		fidPromise->SetValue(ReturnNodes);
		//Return 0 if everything went fine
		return 0;
	}
	catch (const std::exception& e) {
		// Your code to handle the exception goes here
		UE_LOG(LogTemp, Error, TEXT("Exception caught: %s"), UTF8_TO_TCHAR(e.what()));
		return 1;
	}
	return 0;
}

void FAsyncFindInDepthTask::Stop()
{
	bStop = true;
}

void FAsyncFindInDepthTask::Exit()
{
	delete fidPromise;
}
/// <summary>
/// End Find In Depth
/// </summary>

/// <summary>
/// Nearest Neighbor
/// </summary>
FAsyncNearestNeighborTask::FAsyncNearestNeighborTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> inReferenceNode, FVector inStartPosition, int32 inDepth, int32 inIterations, EProctreeNodeType FilterType)
{
	nnPromise = Promise;

	rNode = inReferenceNode;
	rOrigin = inStartPosition;
	rDepth = inDepth;
	rIterations = inIterations;
	rFilter = FilterType;
}

FAsyncNearestNeighborTask::~FAsyncNearestNeighborTask() {
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

void FAsyncNearestNeighborTask::Start() {
	Thread = FRunnableThread::Create(this, TEXT("NearestNeighbors"));
}

bool FAsyncNearestNeighborTask::Init() {
	bStop = false;
	return true;
}

uint32 FAsyncNearestNeighborTask::Run() {
	FRWScopeLock Lock(FProctreeAsync::TreeLock, SLT_ReadOnly);
	try {
		TArray<TSharedPtr<FProctreeNode>> ReturnNodes = rNode->FindNearestNeighbors(rNode, rOrigin, rDepth, rIterations, rFilter);

		ReturnNodes.Num();
		nnPromise->SetValue(ReturnNodes);
		//Return 0 if everything went fine
		return 0;
	}
	catch (const std::exception& e) {
		// Your code to handle the exception goes here
		UE_LOG(LogTemp, Error, TEXT("Exception caught: %s"), UTF8_TO_TCHAR(e.what()));
		return 1;
	}
}

void FAsyncNearestNeighborTask::Stop() {
	bStop = true;
}

void FAsyncNearestNeighborTask::Exit() {
	delete nnPromise;
}
/// <summary>
/// End Nearest Neighbor
/// </summary>

/// <summary>
/// Line Trace
/// </summary>
FAsyncLineTraceTask::FAsyncLineTraceTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> inReferenceNode, FVector inPosition, FVector inDirection, int32 inDepth, int32 inIterations, EProctreeNodeType FilterType)
{
	ltPromise = Promise;

	rNode = inReferenceNode;
	rOrigin = inPosition;
	rDirection = inDirection;
	rDepth = inDepth;
	rIterations = inIterations;
	rFilter = FilterType;	
}

FAsyncLineTraceTask::~FAsyncLineTraceTask()
{
	if (Thread)
	{
		Thread->Kill();
		delete Thread;
	}
}

void FAsyncLineTraceTask::Start()
{
	Thread = FRunnableThread::Create(this, TEXT("LineTrace"));
}

bool FAsyncLineTraceTask::Init()
{
	bStop = false;
	return true;
}

uint32 FAsyncLineTraceTask::Run()
{
	FRWScopeLock Lock(FProctreeAsync::TreeLock, SLT_ReadOnly);
	try {
		TArray<TSharedPtr<FProctreeNode>> ReturnNodes = rNode->FindLineTraceNodes(rNode,rOrigin,rDirection,rDepth,rIterations,rFilter);

		ReturnNodes.Num();
		ltPromise->SetValue(ReturnNodes);
		//Return 0 if everything went fine
		return 0;
	}
	catch (const std::exception& e) {
		// Your code to handle the exception goes here
		UE_LOG(LogTemp, Error, TEXT("Exception caught: %s"), UTF8_TO_TCHAR(e.what()));
		return 1;
	}
	return 0;
}

void FAsyncLineTraceTask::Stop()
{
	bStop = true;
}

void FAsyncLineTraceTask::Exit()
{
	delete ltPromise;
}
/// <summary>
/// End Line Trace
/// </summary>