// Fill out your copyright notice in the Description page of Project Settings.
#include "QuadTreeNode.h"
#include "Async/Async.h"
#include "CoreMinimal.h"
#include "PlanetActor.h"
#include "FastNoise/FastNoise.h"
#include "HAL/Runnable.h"
#include <Mesh/RealtimeMeshSimpleData.h>
#include "ProceduralMeshComponent.h"
#include "Mesh/RealtimeMeshDistanceField.h"
#include <Mesh/RealtimeMeshAlgo.h>

//This structure is for internal use only, anytime it's data is needed it should be wrapped in a FMeshUpdateData struct
QuadTreeNode::QuadTreeNode(APlanetActor* InParentActor, TSharedPtr<INoiseGenerator> InNoiseGen, FCubeTransform InFaceTransform, FQuadIndex InIndex, FVector InCenter, float InSize, float InRadius, int InMinDepth, int InMaxDepth) : Index(InIndex)
{
	ParentActor = InParentActor;
	NoiseGen = InNoiseGen;
	MinDepth = InMinDepth;
	MaxDepth = InMaxDepth;
	FaceTransform = InFaceTransform;
	Center = InCenter;
	SphereRadius = InRadius;
	Size = InSize;
	HalfSize = Size * .5;
	QuarterSize = HalfSize * .5;

	LodKey = FRealtimeMeshLODKey::FRealtimeMeshLODKey(0);

	SeaLevel = InRadius;

	int myDepth = Index.GetDepth();
	
	FaceResolution = ParentActor->FaceResolution;

	NeighborLods[0] = myDepth;
	NeighborLods[1] = myDepth;
	NeighborLods[2] = myDepth;
	NeighborLods[3] = myDepth;
}

//Externally Called Actions and their counterpart functions
void QuadTreeNode::UpdateLod()
{
	TArray<TSharedPtr<QuadTreeNode>> leaves;
	CollectLeaves(AsShared(), leaves);
	ParallelFor(leaves.Num(), [&](int32 i) {
		leaves[i]->TrySetLod();
	});
}
void QuadTreeNode::TrySetLod() {
	if (IsInitialized && IsLeaf()) {
		double k = 8;
		double fov = ParentActor->GetCameraFOV();
		FVector lastCamPos = ParentActor->GetLastCameraPosition();
		auto lastCamRot = ParentActor->GetLastCameraRotation();

		//Since we are doing origin rebasing frequently, the actors location can "change" arbitrarily and needs to be accounted for
		FVector planetCenter = ParentActor->GetActorLocation();

		// Calculate the world-space position of the *unperturbed* chunk center on the sphere
		FVector unperturbedPoint = Center.GetSafeNormal() * SphereRadius;

		//Transform world space to local space by subtracting the world offset
		FVector NodeCentroid = LandCentroid + unperturbedPoint;
		if (RenderSea && FVector::Dist(SeaCentroid + unperturbedPoint, lastCamPos) < FVector::Dist(LandCentroid + unperturbedPoint, lastCamPos)) NodeCentroid = SeaCentroid + unperturbedPoint;
		FVector adjustedCentroid = NodeCentroid * ParentActor->GetActorScale().X + planetCenter;

		auto ParentCentroid = adjustedCentroid;
		auto ParentSize = MaxNodeRadius * ParentActor->GetActorScale().X;

		double planetRadius = FVector::Distance(ParentActor->GetActorLocation(), adjustedCentroid);

		if (Parent.IsValid()) {
			TSharedPtr<QuadTreeNode> tParent = Parent.Pin();
			//Calculate the world-space position of the *unperturbed* parent center on the sphere
			FVector pUnperturbedPoint = tParent->Center.GetSafeNormal() * SphereRadius;

			ParentCentroid = tParent->LandCentroid + pUnperturbedPoint;
			if (tParent->RenderSea && FVector::Dist(tParent->SeaCentroid + pUnperturbedPoint, lastCamPos) < FVector::Dist(tParent->LandCentroid + pUnperturbedPoint, lastCamPos)) ParentCentroid = tParent->SeaCentroid + pUnperturbedPoint;
			ParentCentroid = ParentCentroid * ParentActor->GetActorScale().X + planetCenter;
			ParentSize = tParent->MaxNodeRadius * ParentActor->GetActorScale().X;
		}

		double d1 = FVector::Distance(lastCamPos, adjustedCentroid);
		double d2 = FVector::Distance(lastCamPos, ParentCentroid);
		if (ShouldSplit(d1, fov, k)) {
			CanMerge = false;
			if (LastRenderedState) {
				QuadTreeNode::Split(AsShared());
			}
		}
		else if (ShouldMerge(d2, ParentSize, fov, k)) {
			CanMerge = true;
			if (Index.GetQuadrant() == 3)
				Parent.Pin()->TryMerge();
		}
		else {
			CanMerge = false;
		}
	}
}
void QuadTreeNode::UpdateNeighbors() {
	TArray<TSharedPtr<QuadTreeNode>> leaves;
	CollectLeaves(AsShared(), leaves);
	TArray<TSharedPtr<QuadTreeNode>> updateLeaves;
	ParallelFor(leaves.Num(), [&](int32 i) {
		if (leaves[i]->CheckNeighbors()) leaves[i]->UpdateEdgeMeshBuffer();
	});
}
bool QuadTreeNode::CheckNeighbors() {
	//TODO: Edge processing of neighbor can be broken out into it's own function and it would reduce complexity in this function quite a bit
	if (!HasGenerated) return false; //Cant do neighbor updates until after base mesh data is generated
	int myIndex = Index.GetQuadrant();
	TSharedPtr<QuadTreeNode> n1;
	TSharedPtr<QuadTreeNode> n2;
	bool neighborStateChange = false;
	switch (myIndex) {
	case (uint8)EChildPosition::BOTTOM_LEFT:
		n1 = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::LEFT));
		if (n1) {
			int d = n1->GetDepth();
			if (NeighborLods[(uint8)EdgeOrientation::LEFT] != d) {
				neighborStateChange = true;
				NeighborLods[(uint8)EdgeOrientation::LEFT] = d;
			}
		}
		n2 = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::UP));
		if (n2) {
			int d = n2->GetDepth();
			if (NeighborLods[(uint8)EdgeOrientation::UP] != d) {
				neighborStateChange = true;
				NeighborLods[(uint8)EdgeOrientation::UP] = d;
			}
		}
		break;
	case (uint8)EChildPosition::TOP_LEFT:
		n1 = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::LEFT));
		if (n1) {
			int d = n1->GetDepth();
			if (NeighborLods[(uint8)EdgeOrientation::LEFT] != d) {
				neighborStateChange = true;
				NeighborLods[(uint8)EdgeOrientation::LEFT] = d;
			}
		}
		n2 = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::DOWN));
		if (n2) {
			int d = n2->GetDepth();
			//NeighborLods[(uint8)EdgeOrientation::DOWN] = 0;
			if (NeighborLods[(uint8)EdgeOrientation::DOWN] != d) {
				neighborStateChange = true;
				NeighborLods[(uint8)EdgeOrientation::DOWN] = d;
			}
		}
		break;
	case (uint8)EChildPosition::BOTTOM_RIGHT:
		n1 = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::RIGHT));
		if (n1) {
			int d = n1->GetDepth();
			if (NeighborLods[(uint8)EdgeOrientation::RIGHT] != d) {
				neighborStateChange = true;
				NeighborLods[(uint8)EdgeOrientation::RIGHT] = d;
			}
		}
		n2 = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::UP));
		if (n2) {
			int d = n2->GetDepth();
			if (NeighborLods[(uint8)EdgeOrientation::UP] != d) {
				neighborStateChange = true;
				NeighborLods[(uint8)EdgeOrientation::UP] = d;
			}
		}
		break;
	case (uint8)EChildPosition::TOP_RIGHT:
		n1 = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::RIGHT));
		if (n1) {
			int d = n1->GetDepth();
			if (NeighborLods[(uint8)EdgeOrientation::RIGHT] != d) {
				neighborStateChange = true;
				NeighborLods[(uint8)EdgeOrientation::RIGHT] = d;
			}
		}
		n2 = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::DOWN));
		if (n2) {
			int d = n2->GetDepth();
			if (NeighborLods[(uint8)EdgeOrientation::DOWN] != d) {
				neighborStateChange = true;
				NeighborLods[(uint8)EdgeOrientation::DOWN] = d;
			}
		}
		break;
	}
	return neighborStateChange;
}
void QuadTreeNode::UpdateAllMesh() {
	TArray<TSharedPtr<QuadTreeNode>> leaves;
	CollectLeaves(AsShared(), leaves);	
	ParallelFor(leaves.Num(), [&](int32 i) {
		leaves[i]->UpdateMesh();
	});
}
void QuadTreeNode::UpdateMesh() {
	AsyncTask(ENamedThreads::GameThread, [this]() {
		FReadScopeLock ReadLock(MeshDataLock);
		if (!IsInitialized || !HasGenerated) return;
		if (isEdgeDirty) {
			isEdgeDirty = false;
			if (RenderSea) {
				auto SeaUpdateStream = FRealtimeMeshStreamSet(SeaMeshStreamEdge);
				RtMesh->UpdateSectionGroup(SeaGroupKeyEdge, SeaUpdateStream);
			}
			auto UpdateStream = FRealtimeMeshStreamSet(LandMeshStreamEdge);
			RtMesh->UpdateSectionGroup(LandGroupKeyEdge, UpdateStream);
			RtMesh->UpdateSectionConfig(LandSectionKeyEdge, RtMesh->GetSectionConfig(LandSectionKeyEdge), GetDepth() >= MaxDepth - 3);
		}
		if (isPatchDirty) {
			isPatchDirty = false;
			if (RenderSea) {
				auto SeaUpdateStream = FRealtimeMeshStreamSet(SeaMeshStreamInner);
				RtMesh->UpdateSectionGroup(SeaGroupKeyInner, SeaUpdateStream);
			}
			auto UpdateStream = FRealtimeMeshStreamSet(LandMeshStreamInner);
			RtMesh->UpdateSectionGroup(LandGroupKeyInner, UpdateStream).Then([this](TFuture<ERealtimeMeshProxyUpdateStatus> completedFuture) {
				AsyncTask(ENamedThreads::GameThread, [this]() {
					LastRenderedState = true;
					if (Parent.IsValid()) { 
						auto tParent = Parent.Pin();
						if (tParent->Children[0]->LastRenderedState && tParent->Children[1]->LastRenderedState && tParent->Children[2]->LastRenderedState && tParent->Children[3]->LastRenderedState) {
							while (tParent) {
								tParent->SetChunkVisibility(false);
								tParent = tParent->Parent.Pin();
							}
						}
					}
				});
			});
			RtMesh->UpdateSectionConfig(LandSectionKeyInner, RtMesh->GetSectionConfig(LandSectionKeyInner), GetDepth() >= MaxDepth - 3);
		}
	});
}

//LOD and restructuring operations
//Helper function used in ShouldSplit/ShouldMerge
float s(float z, float hFov) {
	return 2.0f * z * FMath::Tan(FMath::DegreesToRadians(hFov) / 2.0f);
}
bool QuadTreeNode::ShouldMerge(double d2, double parentSize, double fov, double k) {
	return (Parent.IsValid() && Parent.Pin()->GetDepth() >= MinDepth) && k * 1.05 * parentSize < s(d2, fov);
}
bool QuadTreeNode::ShouldSplit(double d1, double fov, double k) {
	int d = GetDepth();
	if (d >= MaxDepth) return false;
	return d < MinDepth || (k * MaxNodeRadius * ParentActor->GetActorScale().X > s(d1, fov));
}
void QuadTreeNode::Split(TSharedPtr<QuadTreeNode> inNode)
{
	if (!inNode.IsValid() || !inNode->IsLeaf() || inNode->IsRestructuring) return;
	inNode->IsRestructuring = true;
	int newDepth = inNode->Index.GetDepth() + 1;
	//Children laid out according to morton XY ordered indexing
	FVector2d childOffsets[4] = {
		FVector2d(-inNode->QuarterSize, -inNode->QuarterSize), // Bottom-left  0b00  0
		FVector2d(-inNode->QuarterSize,  inNode->QuarterSize), // Top-left     0b01  1
		FVector2d(inNode->QuarterSize,  -inNode->QuarterSize), // Bottom-right 0b10  2
		FVector2d(inNode->QuarterSize,   inNode->QuarterSize)  // Top-right    0b11  3
	};

	for (int i = 0; i < 4; i++) {
		// Start with parent center
		FVector childCenter = inNode->Center;
		childCenter[inNode->FaceTransform.AxisMap[0]] += inNode->FaceTransform.AxisDir[0] * childOffsets[i].X;
		childCenter[inNode->FaceTransform.AxisMap[1]] += inNode->FaceTransform.AxisDir[1] * childOffsets[i].Y;
		inNode->Children.Add(MakeShared<QuadTreeNode>(inNode->ParentActor, inNode->NoiseGen, inNode->FaceTransform, inNode->Index.GetChildIndex(i), childCenter, inNode->HalfSize, inNode->SphereRadius, inNode->MinDepth, inNode->MaxDepth));
		inNode->Children[i]->Parent = inNode.ToWeakPtr();
	}
	for (int i = 0; i < 4; i++) {
		inNode->Children[i]->CheckNeighbors();
	}
	Async(EAsyncExecution::TaskGraphMainThread, [inNode]() {
		for (TSharedPtr<QuadTreeNode> child : inNode->Children) {
			child->InitializeChunk(); // Initialize component on main thread then dispatch mesh update
		}
		Async(EAsyncExecution::LargeThreadPool, [inNode]() {
			int newLod = inNode->Index.GetDepth() + 1;
			for (int i = 0; i < 4; i++) {
				inNode->Children[i]->GenerateMeshData();
			}
			inNode->IsRestructuring = false;
		});
	});
}
void QuadTreeNode::TryMerge()
{
	if (IsLeaf()) return;

	bool willMerge = true;
	for (TSharedPtr<QuadTreeNode> child : Children)
	{
		if (!child->CanMerge || !child->LastRenderedState)
		{
			willMerge = false;
		}
	}
	if (willMerge) {
		QuadTreeNode::Merge(AsShared());
	}
}
void QuadTreeNode::Merge(TSharedPtr<QuadTreeNode> inNode)
{
	if (!inNode.IsValid() || inNode->IsLeaf() || inNode->IsRestructuring) return;
	inNode->CheckNeighbors();
	AsyncTask(ENamedThreads::GameThread, [inNode]() mutable {
		inNode->IsRestructuring = true;
		inNode->SetChunkVisibility(true);
		inNode->RemoveChildren(inNode->AsShared());
		inNode->IsRestructuring = false;
		});
}
void QuadTreeNode::RemoveChildren(TSharedPtr<QuadTreeNode> InNode)
{
	if (!InNode.IsValid()) {
		return;
	}

	TArray<TSharedPtr<QuadTreeNode>> nodeStack;
	TArray<TSharedPtr<QuadTreeNode>> processOrder;
	nodeStack.Push(InNode);
	TSet<TSharedPtr<QuadTreeNode>> visitedNodes;

	while (nodeStack.Num() > 0) {
		TSharedPtr<QuadTreeNode> currentNode = nodeStack.Last();
		bool allChildrenProcessed = true;
		if (!currentNode->IsLeaf()) {
			for (int i = currentNode->Children.Num() - 1; i >= 0; --i) {
				if (currentNode->Children[i].IsValid() && !visitedNodes.Contains(currentNode->Children[i])) {
					nodeStack.Push(currentNode->Children[i]);
					allChildrenProcessed = false;
				}
			}
		}
		if (allChildrenProcessed) {
			nodeStack.Pop();
			processOrder.Add(currentNode);
			visitedNodes.Add(currentNode);
		}
	}
	//Join game thread to perform component destructions
	AsyncTask(ENamedThreads::GameThread, [this, processOrder, InNode]() {
		for (const auto& node : processOrder) {
			// Skip the root node as we only want to process children
			if (node != InNode) {
				node->DestroyChunk();
			}
			node->Children.Reset();
		}
	});
}

//Property Getters/Child Collection
bool QuadTreeNode::IsLeaf() const
{
	return Children.Num() == 0;
}
int QuadTreeNode::GetDepth() const
{
	return Index.GetDepth();
}
void QuadTreeNode::CollectLeaves(TSharedPtr<QuadTreeNode> InNode, TArray<TSharedPtr<QuadTreeNode>>& OutLeafNodes) {
	FReadScopeLock ReadLock(InNode->MeshDataLock);
	if (!InNode.IsValid()) {
		return;
	}

	TArray<TSharedPtr<QuadTreeNode>> nodeStack;
	nodeStack.Push(InNode);
	while (nodeStack.Num() > 0) {
		TSharedPtr<QuadTreeNode> currentNode = nodeStack.Pop();
		if (!currentNode.IsValid()) {
			continue;
		}
		if (currentNode->IsLeaf()) {
			OutLeafNodes.Add(currentNode);
			continue;
		}
		for (int i = currentNode->Children.Num() - 1; i >= 0; --i) {
			if (currentNode && currentNode->Children[i].IsValid()) {
				nodeStack.Add(currentNode->Children[i]);
			}
		}
	}
}

////MESH STUFF - Must invoke on game thread
void QuadTreeNode::InitializeChunk() {
	RtMesh = NewObject<URealtimeMeshSimple>(ParentActor);
	ChunkComponent = NewObject<URealtimeMeshComponent>(ParentActor, URealtimeMeshComponent::StaticClass());
	ChunkComponent->RegisterComponent();
	ChunkComponent->SetRenderCustomDepth(true);

	FRealtimeMeshCollisionConfiguration cConfig;
	cConfig.bShouldFastCookMeshes = false;
	cConfig.bUseComplexAsSimpleCollision = true;
	cConfig.bDeformableMesh = false;
	cConfig.bUseAsyncCook = true;
	RtMesh->SetCollisionConfig(cConfig);
	RtMesh->SetupMaterialSlot(0, "LandMaterial");
	RtMesh->SetupMaterialSlot(1, "SeaMaterial");
	RtMesh->ClearInternalFlags(EInternalObjectFlags::Async);

	ChunkComponent->AttachToComponent(ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	ChunkComponent->SetMaterial(0, ParentActor->LandMaterial);
	ChunkComponent->SetMaterial(1, ParentActor->SeaMaterial);
	ChunkComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ChunkComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	ChunkComponent->SetRealtimeMesh(RtMesh);

	// Calculate the world-space position of the *unperturbed* chunk center on the sphere
	FVector unperturbedPoint = Center.GetSafeNormal() * SphereRadius;

	// Set the world offset.
	ChunkComponent->AddWorldOffset(unperturbedPoint + ParentActor->GetActorLocation());

	RtMesh->CreateSectionGroup(LandGroupKeyInner, LandMeshStreamInner);
	RtMesh->CreateSectionGroup(SeaGroupKeyInner, SeaMeshStreamInner);

	RtMesh->CreateSectionGroup(LandGroupKeyEdge, LandMeshStreamEdge);
	RtMesh->CreateSectionGroup(SeaGroupKeyEdge, SeaMeshStreamEdge);

	IsInitialized = true;
}
void QuadTreeNode::SetChunkVisibility(bool inVisibility) {
	if (RenderSea) {
		RtMesh->SetSectionVisibility(SeaSectionKeyEdge, inVisibility);
		RtMesh->SetSectionVisibility(SeaSectionKeyInner, inVisibility);
	}

	RtMesh->SetSectionVisibility(LandSectionKeyInner, inVisibility);
	RtMesh->SetSectionVisibility(LandSectionKeyEdge, inVisibility).Then([this, inVisibility](TFuture<ERealtimeMeshProxyUpdateStatus> completedFuture) {
		LastRenderedState = inVisibility;
	});
}
void QuadTreeNode::DestroyChunk() {
	if (IsInitialized && ChunkComponent) {
		ParentActor->RemoveOwnedComponent(ChunkComponent);
		ChunkComponent->DestroyComponent();
	}
}

//Mesh Data Generation, can be multithreaded
FMeshStreamBuilders QuadTreeNode::InitializeStreamBuilders(FRealtimeMeshStreamSet& inMeshStream, int inResolution) {
	//TODO: Needs optimization to account for edge vs center patch cases (edge case uses different amount of verts/tris
	FMeshStreamBuilders Builders;

	inMeshStream.Empty();
	Builders.NumVerts = inResolution * inResolution + (inResolution * 8);
	Builders.NumTriangles = (inResolution - 1) * (inResolution - 1) * 2;

	Builders.PositionBuilder = new TRealtimeMeshStreamBuilder<FVector, FVector3f>(inMeshStream.AddStream(FRealtimeMeshStreams::Position, GetRealtimeMeshBufferLayout<FVector3f>()));
	Builders.TangentBuilder = new TRealtimeMeshStreamBuilder<FRealtimeMeshTangentsHighPrecision, FRealtimeMeshTangentsNormalPrecision>(inMeshStream.AddStream(FRealtimeMeshStreams::Tangents, GetRealtimeMeshBufferLayout<FRealtimeMeshTangentsNormalPrecision>()));
	Builders.TexCoordsBuilder = new TRealtimeMeshStreamBuilder<FVector2f, FVector2DHalf>(inMeshStream.AddStream(FRealtimeMeshStreams::TexCoords, GetRealtimeMeshBufferLayout<FVector2DHalf>()));
	Builders.ColorBuilder = new TRealtimeMeshStreamBuilder<FColor>(inMeshStream.AddStream(FRealtimeMeshStreams::Color, GetRealtimeMeshBufferLayout<FColor>()));
	Builders.TrianglesBuilder = new TRealtimeMeshStreamBuilder<TIndex3<uint32>>(inMeshStream.AddStream(FRealtimeMeshStreams::Triangles, GetRealtimeMeshBufferLayout<TIndex3<uint32>>()));
	Builders.PolygroupsBuilder = new TRealtimeMeshStreamBuilder<uint32, uint16>(inMeshStream.AddStream(FRealtimeMeshStreams::PolyGroups, GetRealtimeMeshBufferLayout<uint16>()));

	Builders.PositionBuilder->Reserve(Builders.NumVerts);
	Builders.TangentBuilder->Reserve(Builders.NumVerts);
	Builders.TexCoordsBuilder->Reserve(Builders.NumVerts);
	Builders.ColorBuilder->Reserve(Builders.NumVerts);
	Builders.TrianglesBuilder->Reserve(Builders.NumTriangles);
	Builders.PolygroupsBuilder->Reserve(Builders.NumTriangles);

	return Builders;
}
FColor QuadTreeNode::EncodeDepthColor(float depth) {
	//Encodes depth in vertex color
	union {
		float f;
		uint32 i;
	} depthUnion;

	depthUnion.f = depth;
	//Encoding depth into FColor for more precision
	FColor encodeColor;
	encodeColor.R = (depthUnion.i >> 24) & 0xFF;
	encodeColor.G = (depthUnion.i >> 16) & 0xFF;
	encodeColor.B = (depthUnion.i >> 8) & 0xFF;
	encodeColor.A = depthUnion.i & 0xFF;
	return encodeColor;
}
FVector QuadTreeNode::GetFacePoint(float step, double x, double y) {
	//Translates loop x/y into local face positioning
	// Create a result vector starting with the node's center
	FVector result = FVector::ZeroVector;

	// Get normalized coordinates in face-local space
	double normX = -HalfSize + step * x;
	double normY = -HalfSize + step * y;

	// Get the axis indices and signs
	int xAxisIndex = FaceTransform.AxisMap[0]; // Which world axis maps to face X
	int yAxisIndex = FaceTransform.AxisMap[1]; // Which world axis maps to face Y
	int normalAxisIndex = FaceTransform.AxisMap[2]; // Which world axis is the normal

	int xAxisSign = FaceTransform.AxisDir[0]; // Direction of face X axis
	int yAxisSign = FaceTransform.AxisDir[1]; // Direction of face Y axis
	int normalAxisSign = FaceTransform.AxisDir[2]; // Direction of normal

	// Create offset vector for each component
	double offsets[3] = { 0, 0, 0 };

	// Apply offsets to the appropriate axes
	offsets[xAxisIndex] += xAxisSign * normX;
	offsets[yAxisIndex] += yAxisSign * normY;
	offsets[normalAxisIndex] = normalAxisSign * 0; // No offset along normal (already in Center)

	// Apply the offsets to the result
	result.X += offsets[0];
	result.Y += offsets[1];
	result.Z += offsets[2];

	return result;
}

int QuadTreeNode::GenerateVertex(double x, double y, double step) {
	FVector facePoint = GetFacePoint(step, x, y); // Face point relative to Center.

	// Calculate world-space position of the *unperturbed* chunk center on the sphere
	FVector unperturbedPoint = Center.GetSafeNormal() * SphereRadius;

	// Now get the actual point on the face in world space
	FVector worldPoint = Center + facePoint;

	// Normalize the world space point (still without noise).
	FVector normalizedPoint = worldPoint.GetSafeNormal();

	// Add noise to get the final land position.
	FVector landPoint = NoiseGen->GetNoiseFromPosition(normalizedPoint) * SphereRadius;
	FVector seaPoint = normalizedPoint * SphereRadius;

	// Calculate local position relative to the *unperturbed* point
	FVector localLandPoint = landPoint - unperturbedPoint;
	FVector localSeaPoint = seaPoint - unperturbedPoint;

	double landRadius = FVector::Distance(landPoint, ParentActor->GetActorLocation());
	double seaRadius = SphereRadius;

	//Ignore virtual verts/tris for the purpose of calculating node min/max
	if (x > -1 && y > -1 && x < FaceResolution && y < FaceResolution) {
		MinLandRadius = FMath::Min(landRadius, MinLandRadius);
		MaxLandRadius = FMath::Max(landRadius, MaxLandRadius);
		LandCentroid += localLandPoint;
		SeaCentroid += localSeaPoint;
		VisibleVertexCount++;
	}

	FVector2f UV = FVector2f((atan2(normalizedPoint.Y, normalizedPoint.X) + PI) / (2 * PI), (acos(normalizedPoint.Z / normalizedPoint.Size()) / PI));

	int returnIndex = LandVertices.Add(localLandPoint);
	SeaVertices.Add(localSeaPoint);
	TexCoords.Add(UV);
	LandColors.Add(EncodeDepthColor(landRadius - seaRadius));
	SeaColors.Add(EncodeDepthColor(seaRadius - landRadius));

	return returnIndex;
}
void QuadTreeNode::GenerateMeshData() {
	if (!NoiseGen || !IsInitialized) return;
	{
		FWriteScopeLock WriteLock(MeshDataLock);
		LandVertices.Reset();
		SeaVertices.Reset();
		LandNormals.Reset();
		SeaNormals.Reset();
		LandColors.Reset();
		SeaColors.Reset();
		TexCoords.Reset();
		AllTriangles.Reset();
		PatchTriangleIndices.Reset();

		FVector sphereCenter = ParentActor->GetActorLocation();
		CenterOnSphere = Center.GetSafeNormal() * SphereRadius;

		float step = (Size) / (float)(ParentActor->FaceResolution - 1);
		int ModifiedResolution = ParentActor->FaceResolution + 2;
		int curLodLevel = GetDepth();

		LandCentroid = FVector::ZeroVector;
		SeaCentroid = FVector::ZeroVector;
		MinLandRadius = SphereRadius * 10.0;
		MaxLandRadius = 0.0;
		MaxNodeRadius = 0.0;

		// Calculate the world-space position of the *unperturbed* chunk center on the sphere
		FVector unperturbedPoint = Center.GetSafeNormal() * SphereRadius;

		for (int32 x = 0; x < ModifiedResolution; x++) {
			for (int32 y = 0; y < ModifiedResolution; y++) {
				int idx = GenerateVertex(x - 1, y - 1, step);
			}
		}

		//Populate triangles & subdiv
		int tResolution = ModifiedResolution - 1;
		for (int32 x = 0; x < tResolution; x++) {
			for (int32 y = 0; y < tResolution; y++) {
				// Calculate base vertex indices for this quad
				int topLeft = x * ModifiedResolution + y;
				int topRight = topLeft + 1;
				int bottomLeft = topLeft + ModifiedResolution;
				int bottomRight = bottomLeft + 1;

				// Check which edges need LOD transitions
				bool isVirtual = x == 0 || y == 0 || x == tResolution - 1 || y == tResolution - 1;
				bool isEdge = x == 1 || y == 1 || x == tResolution - 2 || y == tResolution - 2;

				bool leftLodChange = x == 1 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::LEFT];
				bool topLodChange = y == 1 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::UP];
				bool rightLodChange = x == tResolution - 2 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::RIGHT];
				bool bottomLodChange = y == tResolution - 2 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::DOWN];

				TArray<FIndex3UI> TrianglesToAdd;

				if ((x + y) % 2 == 0) {
					TrianglesToAdd.Add(FIndex3UI(topLeft, bottomLeft, bottomRight));
					TrianglesToAdd.Add(FIndex3UI(topLeft, bottomRight, topRight));
				}
				else {
					TrianglesToAdd.Add(FIndex3UI(topLeft, bottomLeft, topRight));
					TrianglesToAdd.Add(FIndex3UI(topRight, bottomLeft, bottomRight));
				}

				for (FIndex3UI aTriangle : TrianglesToAdd) {
					int addedIdx = -1;
					if (FaceTransform.bFlipWinding) {
						addedIdx = AllTriangles.Add(FIndex3UI(aTriangle.V0, aTriangle.V2, aTriangle.V1));
					}
					else {
						addedIdx = AllTriangles.Add(aTriangle);
					}

					if (!isVirtual && !isEdge) {
						PatchTriangleIndices.Add(addedIdx);
					}
				}
			}
		}

		LandCentroid = LandCentroid / VisibleVertexCount;
		SeaCentroid = SeaCentroid / VisibleVertexCount;

		uint32 numPos = (uint32)LandVertices.Num();
		for (uint32 i = 0; i < numPos; i++)
		{
			MaxNodeRadius = FMath::Max(MaxNodeRadius, FVector::Dist(LandCentroid, LandVertices[i]));
			FVector vertexNormal = FVector::ZeroVector;

			// Calculate the normal by averaging the normals of neighboring triangles
			for (int32 j = 0; j < AllTriangles.Num(); j++)
			{
				auto tri = AllTriangles[j];
				if (tri[0] == i || tri[1] == i || tri[2] == i)
				{
					// Use LOCAL vertex positions for the normal calculation
					const FVector& P0 = LandVertices[tri[0]];
					const FVector& P1 = LandVertices[tri[1]];
					const FVector& P2 = LandVertices[tri[2]];

					// Calculate the face normal using LOCAL vertex positions
					FVector FaceNormal = FVector::CrossProduct(P1 - P0, P2 - P0).GetSafeNormal();

					// Add the face normal to the vertex normal
					vertexNormal += FaceNormal;
				}
			}

			// Normalize the resulting vertex normal
			vertexNormal = vertexNormal.GetSafeNormal();

			// Since we're working in local space now, use the local position relative to the unperturbed point for the reference vector
			FVector referenceVector = (LandVertices[i]+unperturbedPoint).GetSafeNormal(); // Directly use the normalized local vector.

			// Correct normal orientation if needed
			if (FVector::DotProduct(vertexNormal, referenceVector) < 0)
			{
				vertexNormal *= -1; // Invert the normal
			}

			LandNormals.Add((FVector3f)vertexNormal); // Store the local normal.

			// Calculate sea normal
			FVector seaNormal = (SeaVertices[i]+unperturbedPoint).GetSafeNormal();
			SeaNormals.Add((FVector3f)seaNormal);
		}

		double seaThreshold = 100;
		if (MinLandRadius - seaThreshold < SphereRadius) RenderSea = true;
		HasGenerated = true;
	}
	UpdateEdgeMeshBuffer();
	UpdatePatchMeshBuffer();
}
void QuadTreeNode::UpdateEdgeMeshBuffer() {
	if (!HasGenerated) return;

	FWriteScopeLock WriteLock(MeshDataLock);
	float step = (Size) / (float)(ParentActor->FaceResolution - 1);
	int ModifiedResolution = ParentActor->FaceResolution + 2;

	int curLodLevel = GetDepth();

	int tResolution = ParentActor->FaceResolution;

	bool leftLodChange = Index.GetDepth() > NeighborLods[(uint8)EdgeOrientation::LEFT];
	bool topLodChange = Index.GetDepth() > NeighborLods[(uint8)EdgeOrientation::UP];
	bool rightLodChange = Index.GetDepth() > NeighborLods[(uint8)EdgeOrientation::RIGHT];
	bool bottomLodChange = Index.GetDepth() > NeighborLods[(uint8)EdgeOrientation::DOWN];

	TArray<FIndex3UI> BufferTriangles;

	FIndex3UI topOddTri;
	FIndex3UI bottomOddTri;
	FIndex3UI leftOddTri;
	FIndex3UI rightOddTri;

	//The internal cases here can probably be abstracted
	for (int i = 1; i < tResolution; i++) {
		{ 
			//TOP EDGE TRIANGLES
			int x = i;
			int y = 1;
			
			int topLeft = x * ModifiedResolution + y;
			int bottomLeft = (x + 1) * ModifiedResolution + y;

			//			   TOP LEFT, TOP RIGHT,  BOTTOM LEFT, BOTTOM RIGHT
			int quad[4] = { topLeft, topLeft + 1, bottomLeft, bottomLeft + 1 };

			//Odd quads
			if (x % 2 != 0) {
				//Persist odd "top right" triangles to process in even iterations
				topOddTri = FIndex3UI(quad[3], quad[0], quad[2]);
				//Always generate this triangle unless it is on the corner
				if (x != 1) {
					BufferTriangles.Add(FIndex3UI(quad[0], quad[3], quad[1]));
				}
			}
			//Even quads
			else {
				//If there is a lod change, we modify the previous iterations triangle instead of generating a new one
				if (topLodChange) {
					topOddTri[2] = quad[2];
					BufferTriangles.Add(topOddTri);
				}
				//If there is no lod change, add the last iterations triangle and the new triangle
				else {
					BufferTriangles.Add(topOddTri);
					BufferTriangles.Add(FIndex3UI(quad[0], quad[2], quad[1]));
				}
				//Always generate this triangle unless it is on the corner
				if (x != tResolution - 1) {
					BufferTriangles.Add(FIndex3UI(quad[1], quad[2], quad[3]));
				}
			}
		}

		{ 
			//BOTTOM EDGE TRIANGLES
			int x = i;
			int y = tResolution - 1;
			
			int topLeft = x * ModifiedResolution + y;
			int bottomLeft = (x + 1) * ModifiedResolution + y;

			//			   TOP LEFT, TOP RIGHT,  BOTTOM LEFT, BOTTOM RIGHT
			int quad[4] = { bottomLeft, bottomLeft + 1, topLeft, topLeft + 1 };

			//Odd quads
			if (x % 2 != 0) {
				//Persist odd "top right" triangles to process in even iterations
				bottomOddTri = FIndex3UI(quad[3], quad[0], quad[1]);
				//Always generate this triangle unless it is on the corner
				if (x != 1) {
					BufferTriangles.Add(FIndex3UI(quad[3], quad[2], quad[0]));
				}
			}
			//Even quads
			else {
				//If there is a lod change, we modify the previous iterations triangle instead of generating a new one
				if (bottomLodChange) {
					bottomOddTri[2] = quad[1];
					BufferTriangles.Add(bottomOddTri);
				}
				//If there is no lod change, add the last iterations triangle and the new triangle
				else {
					BufferTriangles.Add(bottomOddTri);
					BufferTriangles.Add(FIndex3UI(quad[1], quad[3], quad[2]));
				}
				//Always generate this triangle unless it is on the corner
				if (x != tResolution - 1) {
					BufferTriangles.Add(FIndex3UI(quad[0], quad[1], quad[2]));
				}
			}
		}

		{
			//LEFT EDGE TRIANGLES
			int y = i;
			int x = 1;

			int topLeft = x * ModifiedResolution + y;
			int bottomLeft = (x + 1) * ModifiedResolution + y;

			//			   TOP LEFT, TOP RIGHT,  BOTTOM LEFT, BOTTOM RIGHT
			int quad[4] = { topLeft, bottomLeft, topLeft + 1, bottomLeft + 1 };

			//Odd quads
			if (y % 2 != 0) {
				//Persist odd "top right" triangles to process in even iterations
				leftOddTri = FIndex3UI(quad[0], quad[3], quad[2]);
				//Always generate this triangle unless it is on the corner
				if (y != 1) {
					BufferTriangles.Add(FIndex3UI(quad[3], quad[0], quad[1]));
				}
			}
			//Even quads
			else {
				//If there is a lod change, we modify the previous iterations triangle instead of generating a new one
				if (leftLodChange) {
					leftOddTri[2] = quad[2];
					BufferTriangles.Add(leftOddTri);
				}
				//If there is no lod change, add the last iterations triangle and the new triangle
				else {
					BufferTriangles.Add(leftOddTri);
					BufferTriangles.Add(FIndex3UI(quad[2], quad[0], quad[1]));
				}
				//Always generate this triangle unless it is on the corner
				if (y != tResolution - 1) {
					BufferTriangles.Add(FIndex3UI(quad[2], quad[1], quad[3]));
				}
			}
		}

		{
			//RIGHT EDGE TRIANGLES
			int y = i;
			int x = tResolution - 1;

			int topLeft = x * ModifiedResolution + y;
			int bottomLeft = (x + 1) * ModifiedResolution + y;

			//			   TOP LEFT, TOP RIGHT,  BOTTOM LEFT, BOTTOM RIGHT
			int quad[4] = { topLeft + 1 , bottomLeft + 1, topLeft, bottomLeft };

			//Odd quads
			if (y % 2 != 0) {
				//Persist odd "top right" triangles to process in even iterations
				rightOddTri = FIndex3UI(quad[0], quad[3], quad[1]);
				//Always generate this triangle unless it is on the corner
				if (y != 1) {
					BufferTriangles.Add(FIndex3UI(quad[2], quad[3], quad[0]));
				}
			}
			//Even quads
			else {
				//If there is a lod change, we modify the previous iterations triangle instead of generating a new one
				if (rightLodChange) {
					rightOddTri[2] = quad[1];
					BufferTriangles.Add(rightOddTri);
				}
				//If there is no lod change, add the last iterations triangle and the new triangle
				else {
					BufferTriangles.Add(rightOddTri);
					BufferTriangles.Add(FIndex3UI(quad[3], quad[1], quad[2]));
				}
				//Always generate this triangle unless it is on the corner
				if (y != tResolution - 1) {
					BufferTriangles.Add(FIndex3UI(quad[1], quad[0], quad[2]));
				}
			}
		}
	}

	auto landEdgeBuilders = InitializeStreamBuilders(LandMeshStreamEdge, ParentActor->FaceResolution);
	auto seaEdgeBuilders = InitializeStreamBuilders(SeaMeshStreamEdge, ParentActor->FaceResolution);

	for (FIndex3UI tri : BufferTriangles) {
		if (FaceTransform.bFlipWinding) {
			tri = FIndex3UI(tri[0], tri[2], tri[1]);
		}
		int lv0 = landEdgeBuilders.PositionBuilder->Add(LandVertices[tri[0]]);
		landEdgeBuilders.ColorBuilder->Add(LandColors[tri[0]]);
		landEdgeBuilders.TexCoordsBuilder->Add(TexCoords[tri[0]]);
		FRealtimeMeshTangentsHighPrecision lTan0;
		lTan0.SetNormal(LandNormals[tri[0]]);
		landEdgeBuilders.TangentBuilder->Add(lTan0);

		int lv1 = landEdgeBuilders.PositionBuilder->Add(LandVertices[tri[1]]);
		landEdgeBuilders.ColorBuilder->Add(LandColors[tri[1]]);
		landEdgeBuilders.TexCoordsBuilder->Add(TexCoords[tri[1]]);
		FRealtimeMeshTangentsHighPrecision lTan1;
		lTan1.SetNormal(LandNormals[tri[1]]);
		landEdgeBuilders.TangentBuilder->Add(lTan1);

		int lv2 = landEdgeBuilders.PositionBuilder->Add(LandVertices[tri[2]]);
		landEdgeBuilders.ColorBuilder->Add(LandColors[tri[2]]);
		landEdgeBuilders.TexCoordsBuilder->Add(TexCoords[tri[2]]);
		FRealtimeMeshTangentsHighPrecision lTan2;
		lTan2.SetNormal(LandNormals[tri[2]]);
		landEdgeBuilders.TangentBuilder->Add(lTan2);

		landEdgeBuilders.TrianglesBuilder->Add(FIndex3UI(lv0, lv1, lv2));
		landEdgeBuilders.PolygroupsBuilder->Add(0);

		int sv0 = seaEdgeBuilders.PositionBuilder->Add(SeaVertices[tri[0]]);
		seaEdgeBuilders.ColorBuilder->Add(SeaColors[tri[0]]);
		seaEdgeBuilders.TexCoordsBuilder->Add(TexCoords[tri[0]]);
		FRealtimeMeshTangentsHighPrecision sTan0;
		sTan0.SetNormal(SeaNormals[tri[0]]);
		seaEdgeBuilders.TangentBuilder->Add(sTan0);

		int sv1 = seaEdgeBuilders.PositionBuilder->Add(SeaVertices[tri[1]]);
		seaEdgeBuilders.ColorBuilder->Add(SeaColors[tri[1]]);
		seaEdgeBuilders.TexCoordsBuilder->Add(TexCoords[tri[1]]);
		FRealtimeMeshTangentsHighPrecision sTan1;
		sTan1.SetNormal(SeaNormals[tri[1]]);
		seaEdgeBuilders.TangentBuilder->Add(sTan1);

		int sv2 = seaEdgeBuilders.PositionBuilder->Add(SeaVertices[tri[2]]);
		seaEdgeBuilders.ColorBuilder->Add(SeaColors[tri[2]]);
		seaEdgeBuilders.TexCoordsBuilder->Add(TexCoords[tri[2]]);
		FRealtimeMeshTangentsHighPrecision sTan2;
		sTan2.SetNormal(SeaNormals[tri[2]]);
		seaEdgeBuilders.TangentBuilder->Add(sTan2);

		seaEdgeBuilders.TrianglesBuilder->Add(FIndex3UI(sv0, sv1, sv2));
		seaEdgeBuilders.PolygroupsBuilder->Add(1);
	}
	isEdgeDirty = true;
}
void QuadTreeNode::UpdatePatchMeshBuffer() {
	if (!HasGenerated) return;
	FReadScopeLock ReadLock(MeshDataLock);
	auto landBuilders = InitializeStreamBuilders(LandMeshStreamInner, ParentActor->FaceResolution);
	auto seaBuilders = InitializeStreamBuilders(SeaMeshStreamInner, ParentActor->FaceResolution);

	for (int32 patchIdx : PatchTriangleIndices) {
		FIndex3UI tri = AllTriangles[patchIdx];

		int lv0 = landBuilders.PositionBuilder->Add(LandVertices[tri[0]]);
		landBuilders.ColorBuilder->Add(LandColors[tri[0]]);
		landBuilders.TexCoordsBuilder->Add(TexCoords[tri[0]]);
		FRealtimeMeshTangentsHighPrecision lTan0;
		lTan0.SetNormal(LandNormals[tri[0]]);
		landBuilders.TangentBuilder->Add(lTan0);

		int lv1 = landBuilders.PositionBuilder->Add(LandVertices[tri[1]]);
		landBuilders.ColorBuilder->Add(LandColors[tri[1]]);
		landBuilders.TexCoordsBuilder->Add(TexCoords[tri[1]]);
		FRealtimeMeshTangentsHighPrecision lTan1;
		lTan1.SetNormal(LandNormals[tri[1]]);
		landBuilders.TangentBuilder->Add(lTan1);

		int lv2 = landBuilders.PositionBuilder->Add(LandVertices[tri[2]]);
		landBuilders.ColorBuilder->Add(LandColors[tri[2]]);
		landBuilders.TexCoordsBuilder->Add(TexCoords[tri[2]]);
		FRealtimeMeshTangentsHighPrecision lTan2;
		lTan2.SetNormal(LandNormals[tri[2]]);
		landBuilders.TangentBuilder->Add(lTan2);

		FIndex3UI newLandTri = FIndex3UI(lv0, lv1, lv2);
		landBuilders.TrianglesBuilder->Add(newLandTri);
		landBuilders.PolygroupsBuilder->Add(0);

		int sv0 = seaBuilders.PositionBuilder->Add(SeaVertices[tri[0]]);
		seaBuilders.ColorBuilder->Add(SeaColors[tri[0]]);
		seaBuilders.TexCoordsBuilder->Add(TexCoords[tri[0]]);
		FRealtimeMeshTangentsHighPrecision sTan0;
		sTan0.SetNormal(SeaNormals[tri[0]]);
		seaBuilders.TangentBuilder->Add(sTan0);

		int sv1 = seaBuilders.PositionBuilder->Add(SeaVertices[tri[1]]);
		seaBuilders.ColorBuilder->Add(SeaColors[tri[1]]);
		seaBuilders.TexCoordsBuilder->Add(TexCoords[tri[1]]);
		FRealtimeMeshTangentsHighPrecision sTan1;
		sTan1.SetNormal(SeaNormals[tri[1]]);
		seaBuilders.TangentBuilder->Add(sTan1);

		int sv2 = seaBuilders.PositionBuilder->Add(SeaVertices[tri[2]]);
		seaBuilders.ColorBuilder->Add(SeaColors[tri[2]]);
		seaBuilders.TexCoordsBuilder->Add(TexCoords[tri[2]]);
		FRealtimeMeshTangentsHighPrecision sTan2;
		sTan2.SetNormal(SeaNormals[tri[2]]);
		seaBuilders.TangentBuilder->Add(sTan2);

		FIndex3UI newSeaTri = FIndex3UI(sv0, sv1, sv2);
		seaBuilders.TrianglesBuilder->Add(newSeaTri);
		seaBuilders.PolygroupsBuilder->Add(1);
	}
	isPatchDirty = true;
}