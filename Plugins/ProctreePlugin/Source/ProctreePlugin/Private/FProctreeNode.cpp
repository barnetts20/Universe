#include "FProctreeNode.h"
//The path is in the form of a string of 0-7 each representing an octree index based on the octant the binary of the index represents
//0 = 000 = 0
//1 = 001 = 1
//2 = 010 = 2
//3 = 011 = 3
//4 = 100 = 4
//5 = 101 = 5
//6 = 110 = 6
//7 = 111 = 7
//The root node is always 0, and the children of the root node are 00, 01, 10, 11, 100, 101, 110, 111

FProctreeNode::FProctreeNode()
{
	this->Children = { nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr };
}

FProctreeNode::~FProctreeNode()
{
}

//Constructors/Destructors
TSharedPtr<FProctreeNode> FProctreeNode::InsertNodeByPointAndDepth(TSharedPtr<FProctreeNode> refNode, FVector Point, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{
	//FVector NormalizedPoint = (Point - refNode->NodeData.Center) / (refNode->NodeData.Size * 2);
	//If depth is 0, update and return this node
	if (Depth == 1) {
		//Need to normalize the point relative to the node in order to get the location offset
		{
			return TSharedPtr<FProctreeNode>(refNode->CreateOrUpdateNodeByIndex(refNode->NodeData.Index, { 0,0,0 }, RandBounds, Activate));
		}
	}
	else {
		//Otherwise, we need to recurse down the tree, inserting new nodes as needed, until we reach the desired depth
		//First, we need to find the octant the point is in
		int32 OctantIndex = 0;
		//If the point is in the positive X direction
		if (Point.X > refNode->NodeData.Center.X) {
			OctantIndex += 1;
		}
		//If the point is in the positive Y direction
		if (Point.Y > refNode->NodeData.Center.Y) {
			OctantIndex += 2;
		}
		//If the point is in the positive Z direction
		if (Point.Z > refNode->NodeData.Center.Z) {
			OctantIndex += 4;
		}
		//Create or update the child node as the current node, we only update the location offset if we are at the desired depth
		TSharedPtr<FProctreeNode> NewChild = refNode -> CreateOrUpdateNodeByIndex(OctantIndex, { 0,0,0 }, RandBounds, false);
		return InsertNodeByPointAndDepth(NewChild, Point, Depth - 1, RandBounds, Activate);
	}
}

TSharedPtr<FProctreeNode> FProctreeNode::InsertNodeByNormalizedPointAndDepth(TSharedPtr<FProctreeNode> refNode, FVector Point, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{
	//Denormalized the point relative to the Node, then invoke the InsertNodeByPointAndDepth function
	//The point will be in the range of -1 to 1, so we need to multiply it by the half the size of the node, then add the center of the node to get the denormalized point

	FVector DenormalizedPoint = refNode->NodeData.Center + Point * refNode->NodeData.Size;

	return InsertNodeByPointAndDepth(refNode, DenormalizedPoint, Depth, RandBounds, Activate);
}

TSharedPtr<FProctreeNode> FProctreeNode::InsertNodeByPath(FString Path, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{
	TSharedPtr<FProctreeNode> CurrentNode = TSharedPtr<FProctreeNode>(AsShared());
	//We need to traverse up the CurrentNode's tree until we reach the root, then we can traverse down the tree using the path, inserting nodes as we go. When we get to the final node, we will flag it activated, and return it.
	while (CurrentNode->Parent.Pin()) {
		CurrentNode = CurrentNode->Parent.Pin();
	}

	int32 PathLength = Path.Len();

	//If the path is size 1, return the root node
	if (PathLength == 1) {
		return CurrentNode;
	}

	for (int32 i = 1; i < PathLength; i++) {
		// Get the integer value of the current character
		int32 OctantIndex = Path[i] - '0';
		bool Activated = (i == PathLength - 1);
		// Create or update child as the current node, insert by path has no LocationOffset
		CurrentNode = CurrentNode->CreateOrUpdateNodeByIndex(OctantIndex, { 0,0,0 }, RandBounds, Activated);
	}
	return CurrentNode;
}

TSharedPtr<FProctreeNode> FProctreeNode::CreateOrUpdateNodeByIndex(int32 Index, FVector LocationOffset, FProctreeNodeRandomizationBounds RandBounds, bool Activated)
{
	//Need to check if the child at the given index is null, if it is, we need to initilize NewChild's octree derived values
	if (!Children[Index].IsValid())
	{
		TSharedPtr<FProctreeNode> NewNode = MakeShared<FProctreeNode>();
		// Initialize the new child's derived values
		NewNode->NodeData.Path = NodeData.Path + FString::FromInt(Index);
		NewNode->NodeData.Index = Index;
		NewNode->NodeData.Depth = NodeData.Depth + 1;
		NewNode->NodeData.Size = NodeData.Size * .5;
		NewNode->NodeData.Center = NodeData.Center +
			FVector(
				Index & 1 ? NodeData.Size : -NodeData.Size,
				Index & 2 ? NodeData.Size : -NodeData.Size,
				Index & 4 ? NodeData.Size : -NodeData.Size);
		NewNode->NodeData.RelativeScale = NodeData.RelativeScale;
		NewNode->NodeData.BoundingBox = FBox::BuildAABB(NewNode->NodeData.Center, FVector(NewNode->NodeData.Size));
		//Need to find the nodes seed by taking up to the last 9 characters of the path and converting them to an integer
		NewNode->NodeData.Seed = FCString::Atoi(*NewNode->NodeData.Path.Right(9));
		// Set the new child's parent to the current node
		//TSharedPtr<FProctreeNode> p = MakeShareable(this);
		NewNode->Parent = TWeakPtr<FProctreeNode>(AsShared());
		Children[Index] = NewNode;
	}
	TSharedPtr<FProctreeNode> NewChild = Children[Index];

	//Create a new child node
	//Need to check if we are activated, if we are, we need to set the node to activated and populate its proceduralized data
	if (Activated) {
		//Need to get a UE5 RandomStream from the seed
		FRandomStream MyRandomStream = FRandomStream(NewChild->NodeData.Seed);
		auto tLocOffset = LocationOffset;
		NewChild->NodeData.NodeType = RandBounds.NodeType;
		//Initialize temporary values for the nodes proceduralized data based on the randomization bounds`
		float tRelativeScale = MyRandomStream.FRandRange(RandBounds.MinRelativeScale, RandBounds.MaxRelativeScale);
		float tDensity = MyRandomStream.FRandRange(RandBounds.MinDensity, RandBounds.MaxDensity);
		float tTemperature = MyRandomStream.FRandRange(RandBounds.MinTemperature, RandBounds.MaxTemperature);
		FLinearColor tComposition = FLinearColor(
			MyRandomStream.FRandRange(RandBounds.MinComposition.R, RandBounds.MaxComposition.R),
			MyRandomStream.FRandRange(RandBounds.MinComposition.G, RandBounds.MaxComposition.G),
			MyRandomStream.FRandRange(RandBounds.MinComposition.B, RandBounds.MaxComposition.B),
			MyRandomStream.FRandRange(RandBounds.MinComposition.A, RandBounds.MaxComposition.A)
		);
		//If the node is already activated, we need to merge the nodes data with the new data
		if (NewChild->NodeData.Activated)
		{
			//We will assume that merging nodes will sum the relative scales and take the minimum between it and 1,
			//Then we will weighted average the other values based on the relative scales
			float tCombinedScale = std::min(NewChild->NodeData.RelativeScale + tRelativeScale, 1.0f);
			//tLocOffset = (NewChild->NodeData.LocationOffset * NewChild->NodeData.RelativeScale + tLocOffset * tRelativeScale) / tCombinedScale;
			tDensity = (NewChild->NodeData.Density * NewChild->NodeData.RelativeScale + tDensity * tRelativeScale) / tCombinedScale;
			tTemperature = (NewChild->NodeData.Temperature * NewChild->NodeData.RelativeScale + tTemperature * tRelativeScale) / tCombinedScale;
			tComposition = (NewChild->NodeData.Composition * NewChild->NodeData.RelativeScale + tComposition * tRelativeScale) / tCombinedScale;
			tRelativeScale = tCombinedScale;
			NewChild->NodeData.ClusterHits++;
		}

		NewChild->NodeData.LocationOffset = tLocOffset;
		NewChild->NodeData.Activated = true;
		NewChild->NodeData.RelativeScale = tRelativeScale;
		NewChild->NodeData.Density = tDensity;
		NewChild->NodeData.Temperature = tTemperature;
		NewChild->NodeData.Composition = tComposition;
	}
	return NewChild;
}

TArray<TSharedPtr<FProctreeNode>> FProctreeNode::BulkInsertNodesByPointCloudAndDepth(TSharedPtr<FProctreeNode> refNode, TArray<FVector> Points, int32 Depth, FProctreeNodeRandomizationBounds RandBounds, bool Activate)
{
	//Lock the tree to insert the new nodes
	TArray<TSharedPtr<FProctreeNode>> ReturnNodes;
	for (int32 i = 0; i < Points.Num(); i++) {
		ReturnNodes.Add(InsertNodeByNormalizedPointAndDepth(refNode, Points[i], Depth, RandBounds, Activate));
	}
	return ReturnNodes;
}

TArray<TSharedPtr<FProctreeNode>> FProctreeNode::FindNodesAtDepthFromNode(TSharedPtr<FProctreeNode> Node, int32 Depth, EProctreeNodeType Filter)
{
	TArray<TSharedPtr<FProctreeNode>> Result;
	auto tDepth = Depth;
	// Check if the current node is active and has the desired depth
	if (Node->NodeData.Activated && Node->NodeData.Depth - NodeData.Depth == tDepth && (Filter == EProctreeNodeType::All || Filter == Node->NodeData.NodeType))
	{
		Result.Add(Node);
		return Result;
	}
	// If not, recursively search child nodes
	for (int32 i = 0; i < 8; i++)	{
		TSharedPtr<FProctreeNode> Child = Node->Children[i];
		// If the child node is not null, recurse down the tree
		if (Child)
		{
			Result.Append(FindNodesAtDepthFromNode(Child, Depth, Filter));
		}
	}
	return Result;
}

TArray<TSharedPtr<FProctreeNode>> FProctreeNode::FindNodesInDepthFromNode(TSharedPtr<FProctreeNode> Node, int32 Depth, EProctreeNodeType Filter)
{
	TArray<TSharedPtr<FProctreeNode>> Result;
	// Check if the current node is active and is at or below the desired depth
	if (Node->NodeData.Activated && Node->NodeData.Depth - NodeData.Depth == Depth && (Filter == EProctreeNodeType::All || Filter == Node->NodeData.NodeType)) {
		Result.Add(Node);
		return Result;
	}
	if (Node->NodeData.Activated && (Filter == EProctreeNodeType::All || Filter == Node->NodeData.NodeType))
	{
		Result.Add(Node);
	}
	// Recursively search child nodes
	for (int32 i = 0; i < 8; i++)
	{
		TSharedPtr<FProctreeNode> Child = Node->Children[i];
		// If the child node is not null, recurse down the tree
		if (Child)
		{
			Result.Append(FindNodesInDepthFromNode(Child, Depth, Filter));
		}
	}
	return Result;
}

TArray<TSharedPtr<FProctreeNode>> FProctreeNode::FindAllActiveDescendants(TSharedPtr<FProctreeNode> Node, EProctreeNodeType Filter)
{
	TArray<TSharedPtr<FProctreeNode>> Result;

	if (!Node.IsValid())
	{
		return Result; // Return empty if the node is invalid
	}

	// Use a stack to iteratively process all descendants
	TArray<TSharedPtr<FProctreeNode>> Stack;
	Stack.Push(Node);

	while (Stack.Num() > 0)
	{
		TSharedPtr<FProctreeNode> CurrentNode = Stack.Pop();

		// Check if the current node is active and matches the filter
		if (CurrentNode->NodeData.Activated &&
			(Filter == EProctreeNodeType::All || Filter == CurrentNode->NodeData.NodeType))
		{
			Result.Add(CurrentNode);
		}

		// Add all valid children of the current node to the stack
		for (int32 i = 0; i < 8; i++)
		{
			TSharedPtr<FProctreeNode> Child = CurrentNode->Children[i];

			// Only push non-null children onto the stack
			if (Child.IsValid())
			{
				Stack.Push(Child);
			}
		}
	}

	return Result;
}

TSharedPtr<FProctreeNode> FProctreeNode::FindNodeAtDepthByPosition(TSharedPtr<FProctreeNode> RootNode, FVector Position, int32 Depth)
{
	// Validate input
	if (!RootNode.IsValid() || Depth < 0)
	{
		return nullptr; // Invalid input or base case: return null
	}

	// If we're at the desired depth, return the current node
	if (Depth == 0)
	{
		return RootNode;
	}

	// Determine the octant index based on the position
	int32 OctantIndex = 0;
	if (Position.X > RootNode->NodeData.Center.X) OctantIndex += 1; // Positive X
	if (Position.Y > RootNode->NodeData.Center.Y) OctantIndex += 2; // Positive Y
	if (Position.Z > RootNode->NodeData.Center.Z) OctantIndex += 4; // Positive Z

	// Get the child node corresponding to the octant index
	TSharedPtr<FProctreeNode> ChildNode = RootNode->Children[OctantIndex];

	// If the child node is not valid, no node exists at this position and depth
	if (!ChildNode.IsValid())
	{
		return nullptr;
	}

	// Recursively search in the child node
	return FindNodeAtDepthByPosition(ChildNode, Position, Depth - 1);
}

TArray<TSharedPtr<FProctreeNode>> FProctreeNode::FindNearestNeighbors(TSharedPtr<FProctreeNode> RootNode, FVector StartPoint, int32 Depth, int32 Iterations, EProctreeNodeType Filter)
{
	TSharedPtr<FProctreeNode> root = RootNode->GetRoot();
	//Size here is equivalent to radius so * by 2 to step the entire "diameter" of a node
	double NodeScale = root->NodeData.Size * 2.0 / FMath::Pow(2.0, Depth);

	TArray<FVector> SamplePositions;
	// Add the current node's center point as a sample position
	SamplePositions.Add(StartPoint);

	// Iterate over each combination of x, y, z within the given distance range
	for (int x = 1; x <= Iterations; ++x)
	{
		for (int y = 1; y <= Iterations; ++y)
		{
			for (int z = 1; z <= Iterations; ++z)
			{
				// Generate all 8 permutations of positive and negative signs for (x, y, z) offsets
				SamplePositions.Add(StartPoint + FVector(x, y, z) * NodeScale);
				SamplePositions.Add(StartPoint + FVector(-x, y, z) * NodeScale);
				SamplePositions.Add(StartPoint + FVector(x, -y, z) * NodeScale);
				SamplePositions.Add(StartPoint + FVector(x, y, -z) * NodeScale);
				SamplePositions.Add(StartPoint + FVector(-x, -y, z) * NodeScale);
				SamplePositions.Add(StartPoint + FVector(-x, y, -z) * NodeScale);
				SamplePositions.Add(StartPoint + FVector(x, -y, -z) * NodeScale);
				SamplePositions.Add(StartPoint + FVector(-x, -y, -z) * NodeScale);
			}
		}
	}

	TArray<TSharedPtr<FProctreeNode>> ActiveDescendants;
	TSet<FString> UniquePaths; // Set to track unique paths
	for (FVector position : SamplePositions) {
		TSharedPtr<FProctreeNode> sampleNode = FindNodeAtDepthByPosition(root, position, Depth);
		if (sampleNode.IsValid()) {
			FString NodePath = sampleNode->NodeData.Path;
			if (!UniquePaths.Contains(NodePath)) {
				UniquePaths.Add(NodePath);
				ActiveDescendants.Append(sampleNode->FindAllActiveDescendants(sampleNode, Filter));
			}			
		}
	}
	ActiveDescendants.Sort([StartPoint](const TSharedPtr<FProctreeNode>& A, const TSharedPtr<FProctreeNode>& B)
		{
			float DistA = FVector::DistSquared(StartPoint, A->NodeData.Center);
			float DistB = FVector::DistSquared(StartPoint, B->NodeData.Center);
			return DistA < DistB; // Sort in ascending order of distance
		});
	return ActiveDescendants;
}

TArray<TSharedPtr<FProctreeNode>> FProctreeNode::FindLineTraceNodes(TSharedPtr<FProctreeNode> RootNode, FVector StartPoint, FVector Direction, int32 Depth, int32 Iterations, EProctreeNodeType Filter)
{
	TSharedPtr<FProctreeNode> root = RootNode->GetRoot();
	TArray<TSharedPtr<FProctreeNode>> ResultNodes;
	TSet<FString> UniquePaths; // Set to track unique paths

	FVector CurrentPoint = StartPoint;
	double StepSize = root->NodeData.Size / FMath::Pow(2.0, Depth);

	// Normalize the direction vector to ensure consistent step size
	Direction.Normalize();

	// Sample positions along the ray
	for (int i = 0; i < Iterations; ++i)
	{
		// Calculate the next point along the ray
		FVector SamplePosition = StartPoint + (Direction * StepSize * i);

		// Find the node at the sample position and reference node depth
		TSharedPtr<FProctreeNode> sampleNode = FindNodeAtDepthByPosition(root, SamplePosition, Depth);

		if (sampleNode.IsValid())
		{
			FString NodePath = sampleNode->NodeData.Path; // Get the node's unique path

			if (!UniquePaths.Contains(NodePath))
			{
				// Add the path to the set to ensure uniqueness
				UniquePaths.Add(NodePath);

				// Fetch all active descendants that match the filter
				TArray<TSharedPtr<FProctreeNode>> ActiveDescendants = sampleNode->FindAllActiveDescendants(sampleNode, Filter);

				// Sort active descendants by their distance to the StartPoint
				ActiveDescendants.Sort([StartPoint](const TSharedPtr<FProctreeNode>& A, const TSharedPtr<FProctreeNode>& B)
					{
						float DistA = FVector::DistSquared(StartPoint, A->NodeData.Center);
						float DistB = FVector::DistSquared(StartPoint, B->NodeData.Center);
						return DistA < DistB; // Sort in ascending order of distance
					});

				// Append sorted active descendants to the result array
				ResultNodes.Append(ActiveDescendants);
			}
		}
	}
	return ResultNodes;
}

TSharedPtr<FProctreeNode> FProctreeNode::GetRoot() {
	TSharedPtr<FProctreeNode> CurrentNode = SharedThis(this);
	while (CurrentNode.IsValid())
	{
		TWeakPtr<FProctreeNode> ParentNodeWeak = CurrentNode->Parent;
		TSharedPtr<FProctreeNode> ParentNode = ParentNodeWeak.Pin();
		if (!ParentNode.IsValid())
		{
			break;
		}
		CurrentNode = ParentNode;
	}
	return CurrentNode;
}