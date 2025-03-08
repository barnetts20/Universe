#include "UProctreeNodeWrapper.h"
#include "FProctreeStructures.h"
#include <FProctreeAsync.h>

UProctreeNodeWrapper::UProctreeNodeWrapper()
{
	this->Node = TSharedPtr<FProctreeNode>(new FProctreeNode());
}

UProctreeNodeWrapper::~UProctreeNodeWrapper()
{
	this->Node = nullptr;
	Super::BeginDestroy();
}


UProctreeNodeWrapper* UProctreeNodeWrapper::GetParent() const
{
	if (!this->Node->Parent.IsValid()) {
		return nullptr;
	}
	UProctreeNodeWrapper* tParent = NewObject<UProctreeNodeWrapper>();
	tParent->Node = this->Node->Parent.Pin();
	return tParent;
}

UProctreeNodeWrapper* UProctreeNodeWrapper::GetChild(int32 index) const
{
	if (!this->Node->Children[index].IsValid()) {
		return nullptr;
	}
	UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
	tChild->Node = this->Node->Children[index];
	return tChild;
}

TArray<UProctreeNodeWrapper*> UProctreeNodeWrapper::GetChildren() const
{
	TArray<UProctreeNodeWrapper*> ChildrenArray;
	for (TSharedPtr<FProctreeNode> Child : this->Node->Children)
	{
		if (Child == nullptr) {
			ChildrenArray.Add(nullptr);
		}
		else {
			UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
			tChild->Node = Child;
			ChildrenArray.Add(tChild);
		}
	}
	return ChildrenArray;
}

FProctreeNodeData UProctreeNodeWrapper::GetNodeData() const
{
	return Node->NodeData;
}

UProctreeNodeWrapper* UProctreeNodeWrapper::InsertNodeByPath(FString Path, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{
	UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
	tChild->Node = Node->InsertNodeByPath(Path, RandBounds, Activate);
	return tChild;
}

UProctreeNodeWrapper* UProctreeNodeWrapper::InsertNodeByPointAndDepth(FVector Point, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{
	UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
	tChild->Node = Node->InsertNodeByPointAndDepth(Node, Point, Depth, RandBounds, Activate);
	return tChild;
}

UProctreeNodeWrapper* UProctreeNodeWrapper::InsertNodeByNormalizedPointAndDepth(FVector Point, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{	
	UProctreeNodeWrapper* tChild = NewObject<UProctreeNodeWrapper>();
	tChild->Node = Node->InsertNodeByNormalizedPointAndDepth(Node, Point, Depth, RandBounds, Activate);
	return tChild;
}

TSharedPtr<FProctreeNode> UProctreeNodeWrapper::GetInternalNode() const
{
	return this->Node;	
}

void UProctreeNodeWrapper::SetInternalNode(TSharedPtr<FProctreeNode> inNode)
{
	this->Node = inNode;
}

FProctreeBulkNodeResult::FProctreeBulkNodeResult()
{
}

FProctreeBulkNodeResult::~FProctreeBulkNodeResult()
{
}
