#pragma once

#include "CoreMinimal.h"
#include "FProctreeStructures.h"
#include "HAL/Runnable.h"
#include <UProctreeNodeWrapper.h>
/**
 * 
 */
class PROCTREEPLUGIN_API FProctreeAsync
{
public:
	FProctreeAsync();
	~FProctreeAsync();
	static FRWLock TreeLock;
};

class PROCTREEPLUGIN_API FAsyncBulkInsertTask : public FRunnable
{
protected:
	//Thread parameters
	bool bStop = false;
	FRunnableThread* Thread;
	class TPromise<TArray<TSharedPtr<FProctreeNode>>>* biPromise;
	//Task Parameters
	TSharedPtr<FProctreeNode> rNode;
	TArray<FVector> rPoints;
	int32 rDepth;
	FProctreeNodeRandomizationBounds rRandomizationBounds;
	bool rActivated;

public:
	FAsyncBulkInsertTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> ReferenceNode, TArray<FVector> Points, int32 Depth, FProctreeNodeRandomizationBounds RandomizationBounds, bool Activated);
	~FAsyncBulkInsertTask();
	//Start executing the thread
	void Start();

	//Thread life cycle methods
	bool Init();
	uint32 Run();
	void Stop();
	void Exit();
};

class PROCTREEPLUGIN_API FAsyncFindInDepthTask : public FRunnable
{
protected:
	//Thread parameters
	bool bStop = false;
	FRunnableThread* Thread;
	class TPromise<TArray<TSharedPtr<FProctreeNode>>>* fidPromise;
	//Task Parameters
	TSharedPtr<FProctreeNode> rNode;
	int32 rDepth;
	EProctreeNodeType rFilter;

public:
	FAsyncFindInDepthTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> ReferenceNode, int32 Depth, EProctreeNodeType FilterType = EProctreeNodeType::All);
	~FAsyncFindInDepthTask();
	//Start executing the thread
	void Start();
	//Thread life cycle methods
	bool Init();
	uint32 Run();
	void Stop();
	void Exit();
};

class PROCTREEPLUGIN_API FAsyncFindAtDepthTask : public FRunnable
{
protected:
	//Thread parameters
	bool bStop = false;
	FRunnableThread* Thread;
	class TPromise<TArray<TSharedPtr<FProctreeNode>>>* fadPromise;

	//Task Parameters
	TSharedPtr<FProctreeNode> rNode;
	int32 rDepth;
	EProctreeNodeType rFilter;

public:
	FAsyncFindAtDepthTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> ReferenceNode, int32 Depth, EProctreeNodeType FilterType = EProctreeNodeType::All);
	~FAsyncFindAtDepthTask();
	//Start executing the thread
	void Start();
	//Thread life cycle methods
	bool Init();
	uint32 Run();
	void Stop();
	void Exit();
};

class PROCTREEPLUGIN_API FAsyncNearestNeighborTask : public FRunnable
{
protected:
	bool bStop = false;
	FRunnableThread* Thread;
	class TPromise<TArray<TSharedPtr<FProctreeNode>>>* nnPromise;

	TSharedPtr<FProctreeNode> rNode;
	FVector rOrigin;
	int32 rDepth;
	int32 rIterations;
	EProctreeNodeType rFilter;

public:
	FAsyncNearestNeighborTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> inReferenceNode, FVector inStartPosition, int32 inDepth, int32 inIterations, EProctreeNodeType FilterType = EProctreeNodeType::All);
	~FAsyncNearestNeighborTask();
	//Start execution
	void Start();
	//FRunnable implementation
	bool Init();
	uint32 Run();
	void Stop();
	void Exit();
};

class PROCTREEPLUGIN_API FAsyncLineTraceTask : public FRunnable
{
protected:
	bool bStop = false;
	FRunnableThread* Thread;
	class TPromise<TArray<TSharedPtr<FProctreeNode>>>* ltPromise;

	TSharedPtr<FProctreeNode> rNode;
	FVector rOrigin;
	FVector rDirection;
	int32 rDepth;
	int32 rIterations;
	EProctreeNodeType rFilter;

public:
	FAsyncLineTraceTask(TPromise<TArray<TSharedPtr<FProctreeNode>>>* Promise, TSharedPtr<FProctreeNode> inReferenceNode, FVector inPosition, FVector inDirection, int32 inDepth, int32 inIterations, EProctreeNodeType FilterType = EProctreeNodeType::All);
	~FAsyncLineTraceTask();
	//Start execution
	void Start();
	//FRunnable implementation
	bool Init();
	uint32 Run();
	void Stop();
	void Exit();
};