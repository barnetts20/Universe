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
QuadTreeNode::QuadTreeNode(APlanetActor* InParentActor, TSharedPtr<INoiseGenerator> InNoiseGen, FQuadIndex InIndex, int InMinDepth, int InMaxDepth, FCubeTransform InFaceTransform, FVector InCenter, float InSize, float InRadius) : Index(InIndex)
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
}

QuadTreeNode::~QuadTreeNode()
{
	//DestroyChunk();
}

void QuadTreeNode::UpdateLod()
{
	RecurseUpdateLod(AsShared());
}

void QuadTreeNode::RecurseUpdateLod(TWeakPtr<QuadTreeNode> InNode) {
	if (!InNode.IsValid()) {
		return;
	}
	TSharedPtr<QuadTreeNode> tNode = InNode.Pin();
	if (tNode->IsInitialized) {
		for (int i = 0; i < tNode->Children.Num(); i++) {
			RecurseUpdateLod(tNode->Children[i]);
		}
		tNode->TrySetLod();
	}
}

float s(float z, float hFov) {
	return 2.0f * z * FMath::Tan(FMath::DegreesToRadians(hFov) / 2.0f);
}

void QuadTreeNode::TrySetLod() {
	if (IsInitialized && IsLeaf()) {
		double k = 8;
		double fov = ParentActor->GetCameraFOV();
		FVector lastCamPos = ParentActor->GetLastCameraPosition();
		auto lastCamRot = ParentActor->GetLastCameraRotation();

		//Since we are doing origin rebasing frequently, the actors location can "change" arbitrarily and needs to be accounted for
		FVector planetCenter = ParentActor->GetActorLocation();
		FVector adjustedCentroid = LandCentroid * ParentActor->GetActorScale().X + planetCenter;
		auto parentCenter = adjustedCentroid;
		auto parentSize = MaxNodeRadius * ParentActor->GetActorScale().X;

		double planetRadius = FVector::Distance(ParentActor->GetActorLocation(), adjustedCentroid);

		auto parent = Parent;

		if (parent.IsValid()) {
			parentCenter = parent.Pin()->LandCentroid * ParentActor->GetActorScale().X + planetCenter;
			parentSize = parent.Pin()->MaxNodeRadius * ParentActor->GetActorScale().X;
		}

		double d1 = FVector::Distance(lastCamPos, adjustedCentroid);
		double d2 = FVector::Distance(lastCamPos, parentCenter);
		if (ShouldSplit(adjustedCentroid, lastCamPos, fov, k)) {
			CanMerge = false;
			if (LastRenderedState) {
				QuadTreeNode::Split(AsShared());
			}
		}
		else if ((parent.IsValid() && parent.Pin()->GetDepth() >= MinDepth) && k * 1.05 * parentSize < s(d2, fov)) {
			CanMerge = true;
			if (Index.GetQuadrant() == 3)
				parent.Pin()->TryMerge();
		}
		else {
			CanMerge = false;
		}
	}
}

bool QuadTreeNode::ShouldSplit(FVector centroid, FVector lastCamPos, double fov, double k) {
	double d1 = FVector::Distance(lastCamPos, centroid);
	return GetDepth() < MinDepth || (GetDepth() < MaxDepth && k * MaxNodeRadius * ParentActor->GetActorScale().X > s(d1, fov));
}

void QuadTreeNode::UpdateNeighborEdge(EdgeOrientation InEdge, int InLod) {
	if (NeighborLods[(uint8)InEdge] != InLod) {
		NeighborLods[(uint8)InEdge] = InLod;
		IsDirty = true;
		//Async(EAsyncExecution::LargeThreadPool, [this]() {
		UpdateEdgeMesh();
		//});
	}
}

//void QuadTreeNode::LogNeighborLods() {
//	// Log the node's index and its neighbor LOD values
//	UE_LOG(LogTemp, Warning, TEXT("Node %s Neighbor LODs: L: %d, U: %d, R: %d, D: %d"),
//		*Index.ToString(),
//		NeighborLods[(uint8)EdgeOrientation::LEFT],
//		NeighborLods[(uint8)EdgeOrientation::UP],
//		NeighborLods[(uint8)EdgeOrientation::RIGHT],
//		NeighborLods[(uint8)EdgeOrientation::DOWN]);
//}

FString PathToBinary(uint64 Path) {
	FString binaryString;
	int highestBit = 63;

	// Find the highest set bit
	while (highestBit >= 0 && !(Path & (1ULL << highestBit))) {
		highestBit--;
	}

	// Always start from an even bit to maintain pair alignment
	if (highestBit % 2 == 0) highestBit++;

	// Iterate bits, grouping into pairs
	for (int i = highestBit; i >= 0; --i) {
		binaryString += ((Path >> i) & 1ULL) ? "1" : "0";

		// Add space after every 2 bits
		if (i % 2 == 0) {
			binaryString += " ";
		}
	}

	return binaryString;
}


void QuadTreeNode::UpdateNeighborLod(int InLod) {
	TSharedPtr<QuadTreeNode> LeftNeighborNode = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::LEFT));
	TSharedPtr<QuadTreeNode> UpNeighborNode = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::UP));
	TSharedPtr<QuadTreeNode> RightNeighborNode = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::RIGHT));
	TSharedPtr<QuadTreeNode> DownNeighborNode = ParentActor->GetNodeByIndex(Index.GetNeighborIndex(EdgeOrientation::DOWN));

	//Verify neighbor mapping at planet scale
	auto LeftTestIndex = Index.GetNeighborIndex(EdgeOrientation::LEFT);
	auto RightTestIndex = Index.GetNeighborIndex(EdgeOrientation::RIGHT);
	auto UpTestIndex = Index.GetNeighborIndex(EdgeOrientation::UP);
	auto DownTestIndex = Index.GetNeighborIndex(EdgeOrientation::DOWN);

	auto LeftRemap = LeftTestIndex.GetNeighborIndex(EdgeOrientation::RIGHT);
	auto RightRemap = RightTestIndex.GetNeighborIndex(EdgeOrientation::LEFT);
	auto UpRemap = UpTestIndex.GetNeighborIndex(EdgeOrientation::DOWN);
	auto DownRemap = DownTestIndex.GetNeighborIndex(EdgeOrientation::UP);

	if (LeftNeighborNode) {
		if (LeftNeighborNode->Index.FaceId != Index.FaceId) {
			LeftNeighborNode->UpdateNeighborEdge(FaceTransform.NeighborEdgeMap[(uint8)EdgeOrientation::RIGHT], InLod);
		}
		else {
			LeftNeighborNode->UpdateNeighborEdge(EdgeOrientation::RIGHT, InLod);
		}
	}
	if (UpNeighborNode) {
		if (UpNeighborNode->Index.FaceId != Index.FaceId) {
			UpNeighborNode->UpdateNeighborEdge(FaceTransform.NeighborEdgeMap[(uint8)EdgeOrientation::DOWN], InLod);
		}
		else {
			UpNeighborNode->UpdateNeighborEdge(EdgeOrientation::DOWN, InLod);
		}
	}
	if (RightNeighborNode) {
		if (RightNeighborNode->Index.FaceId != Index.FaceId) {
			RightNeighborNode->UpdateNeighborEdge(FaceTransform.NeighborEdgeMap[(uint8)EdgeOrientation::LEFT], InLod);
		}
		else {
			RightNeighborNode->UpdateNeighborEdge(EdgeOrientation::LEFT, InLod);
		}
	}
	if (DownNeighborNode) {
		if (DownNeighborNode->Index.FaceId != Index.FaceId) {
			DownNeighborNode->UpdateNeighborEdge(FaceTransform.NeighborEdgeMap[(uint8)EdgeOrientation::UP], InLod);
		}
		else {
			DownNeighborNode->UpdateNeighborEdge(EdgeOrientation::UP, InLod);
		}
	}
}

void QuadTreeNode::UpdateMesh() {
	// If this node has children, recurse into them
	if (!IsLeaf())
	{
		for (auto& Child : Children)
		{
			if (Child)
				Child->UpdateMesh();
		}
		return;
	}
	else if (IsDirty) {
		GenerateMeshData();
	}
}

void QuadTreeNode::CollectLeaves(TArray<TSharedPtr<QuadTreeNode>>& OutLeafNodes) {
	if (IsLeaf()) {
		OutLeafNodes.Add(AsShared());
	}
	else {
		for (const auto& Child : Children) {
			Child->CollectLeaves(OutLeafNodes);
		}
	}
}

void QuadTreeNode::Split(TSharedPtr<QuadTreeNode> inNode)
{
	if (!inNode.IsValid() || !inNode->IsLeaf() || inNode->IsRestructuring) return;
	inNode->IsRestructuring = true;
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
		inNode->Children.Add(MakeShared<QuadTreeNode>(inNode->ParentActor, inNode->NoiseGen, inNode->Index.GetChildIndex(i), inNode->MinDepth, inNode->MaxDepth, inNode->FaceTransform, childCenter, inNode->HalfSize, inNode->SphereRadius));
		inNode->Children[i]->Parent = inNode.ToWeakPtr();
	}

	Async(EAsyncExecution::TaskGraphMainThread, [inNode]() {
		for (TSharedPtr<QuadTreeNode> child : inNode->Children) {
			child->InitializeChunk(); // Initialize component on main thread then dispatch mesh update
		}
		Async(EAsyncExecution::LargeThreadPool, [inNode]() {
			int newLod = inNode->Index.GetDepth() + 1;
			for (int i = 0; i < 4; i++) {
				inNode->Children[i]->GenerateMeshData();
				//inNode->Children[i]->UpdateNeighborLod(newLod);
			}
			Async(EAsyncExecution::TaskGraphMainThread, [inNode]() {
				inNode->SetChunkVisibility(false);
				}).Wait();
				inNode->IsRestructuring = false;
			});
		});
}

void QuadTreeNode::Merge(TSharedPtr<QuadTreeNode> inNode)
{
	if (!inNode.IsValid() || inNode->IsLeaf() || inNode->IsRestructuring) return;
	inNode->IsRestructuring = true;
	Async(EAsyncExecution::TaskGraphMainThread, [inNode]() mutable {
		if (!inNode.IsValid() || inNode->IsLeaf()) {
			return;
		}
		int newLod = inNode->Index.GetDepth();
		inNode->UpdateNeighborLod(newLod);
		inNode->SetChunkVisibility(true);
		for (int i = 0; i < 4; i++) {
			inNode->Children[i]->UpdateNeighborLod(newLod);
			inNode->Children[i]->DestroyChunk();
		}

		inNode->Children.Empty();
		inNode->IsRestructuring = false;
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

void QuadTreeNode::RecurseRemoveChildren(TSharedPtr<QuadTreeNode> InNode)
{
	for (TSharedPtr<QuadTreeNode> child : InNode->Children) {
		child->DestroyChunk();
		if (!child->IsLeaf())
		{
			child->RecurseRemoveChildren(child);
		}
	}
	InNode->Children.Reset();
}

bool QuadTreeNode::IsLeaf() const
{
	return Children.Num() == 0;
}

int QuadTreeNode::GetDepth() const
{
	return Index.GetDepth();
}

////MESH STUFF
//Must invoke on game thread
void QuadTreeNode::InitializeChunk() {
	RtMesh = NewObject<URealtimeMeshSimple>(ParentActor);
	ChunkComponent = NewObject<URealtimeMeshComponent>(ParentActor, URealtimeMeshComponent::StaticClass());
	ChunkComponent->RegisterComponent();

	if (NoiseGen) {
		ChunkComponent->SetRenderCustomDepth(true);
	}

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

	RtMesh->CreateSectionGroup(LandGroupKeyInner, LandMeshStreamInner);
	RtMesh->CreateSectionGroup(SeaGroupKeyInner, SeaMeshStreamInner);

	RtMesh->CreateSectionGroup(LandGroupKeyEdge, LandMeshStreamEdge);
	RtMesh->CreateSectionGroup(SeaGroupKeyEdge, SeaMeshStreamEdge);

	IsInitialized = true;
	LastRenderedState = true;
}
//Must invoke on game thread
void QuadTreeNode::DestroyChunk() {
	if (IsInitialized && ChunkComponent) {
		ParentActor->RemoveOwnedComponent(ChunkComponent);
		ChunkComponent->DestroyComponent();
	}
}
//Must invoke on game thread
void QuadTreeNode::SetChunkVisibility(bool inVisibility) {
	RtMesh->SetSectionVisibility(LandSectionKeyEdge, inVisibility);
	RtMesh->SetSectionVisibility(LandSectionKeyInner, inVisibility).Then([this, inVisibility](TFuture<ERealtimeMeshProxyUpdateStatus> completedFuture) {
		LastRenderedState = inVisibility;
		});
	if (RenderSea) {
		RtMesh->SetSectionVisibility(SeaSectionKeyEdge, inVisibility);
		RtMesh->SetSectionVisibility(SeaSectionKeyInner, inVisibility).Then([this, inVisibility](TFuture<ERealtimeMeshProxyUpdateStatus> completedFuture) {
			LastRenderedState = inVisibility;
			});
	}
}

FMeshStreamBuilders QuadTreeNode::InitializeStreamBuilders(FRealtimeMeshStreamSet& inMeshStream, int inResolution) {
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
	// Create a result vector starting with the node's center
	FVector result = Center;

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

int QuadTreeNode::GenerateVertex2(double x, double y, double step) {
	FVector facePoint = GetFacePoint(step, x, y);
	FVector normalizedPoint = facePoint.GetSafeNormal();
	FVector seaPoint = normalizedPoint * SphereRadius;//TODO: SEA LEVEL INTEGRATION
	FVector landPoint = NoiseGen->GetNoiseFromPosition(normalizedPoint) * SphereRadius;
	double landRadius = FVector::Distance(landPoint, ParentActor->GetActorLocation());
	double seaRadius = SphereRadius;
	MinLandRadius = FMath::Min(landRadius, MinLandRadius);
	MaxLandRadius = FMath::Max(landRadius, MaxLandRadius);
	MaxNodeRadius = FMath::Max(MaxNodeRadius, FVector::Dist(CenterOnSphere, landPoint));

	LandCentroid += landPoint;
	SeaCentroid += seaPoint;

	FVector2f UV = FVector2f((atan2(normalizedPoint.Y, normalizedPoint.X) + PI) / (2 * PI), (acos(normalizedPoint.Z / normalizedPoint.Size()) / PI));

	int returnIndex = LandVertices.Add(landPoint);
	SeaVertices.Add(seaPoint);

	TexCoords.Add(UV);
	LandColors.Add(EncodeDepthColor(landRadius - seaRadius));
	SeaColors.Add(EncodeDepthColor(seaRadius - landRadius));

	return returnIndex;
}

int QuadTreeNode::GenerateVertex(double x, double y, double step, FMeshStreamBuilders& landBuilders, FMeshStreamBuilders& seaBuilders) {
	FVector facePoint = GetFacePoint(step, x, y);
	FVector normalizedPoint = facePoint.GetSafeNormal();
	FVector seaPoint = normalizedPoint * SphereRadius;//TODO: SEA LEVEL INTEGRATION
	FVector landPoint = NoiseGen->GetNoiseFromPosition(normalizedPoint) * SphereRadius;
	double landRadius = FVector::Distance(landPoint, ParentActor->GetActorLocation());
	double seaRadius = SphereRadius;
	MinLandRadius = FMath::Min(landRadius, MinLandRadius);
	MaxLandRadius = FMath::Max(landRadius, MaxLandRadius);
	MaxNodeRadius = FMath::Max(MaxNodeRadius, FVector::Dist(CenterOnSphere, landPoint));

	//SphereCentroid += normalizedPoint * SphereRadius;
	LandCentroid += landPoint;
	SeaCentroid += seaPoint;

	FVector2f UV = FVector2f((atan2(normalizedPoint.Y, normalizedPoint.X) + PI) / (2 * PI), (acos(normalizedPoint.Z / normalizedPoint.Size()) / PI));

	int returnIndex = landBuilders.PositionBuilder->Add(landPoint);

	seaBuilders.PositionBuilder->Add(seaPoint);

	landBuilders.ColorBuilder->Add(EncodeDepthColor(landRadius - seaRadius));
	seaBuilders.ColorBuilder->Add(EncodeDepthColor(seaRadius - landRadius));

	landBuilders.TexCoordsBuilder->Add(UV);
	seaBuilders.TexCoordsBuilder->Add(UV);

	return returnIndex;
}

void QuadTreeNode::UpdateEdgeMesh() {
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
	RtMesh->UpdateSectionGroup(LandGroupKeyEdge, LandMeshStreamEdge);
	RtMesh->UpdateSectionConfig(LandSectionKeyEdge, RtMesh->GetSectionConfig(LandSectionKeyEdge), GetDepth() >= MaxDepth - 3);
}

void QuadTreeNode::UpdatePatchMesh() {
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
		seaBuilders.PolygroupsBuilder->Add(0);
	}

	RtMesh->UpdateSectionGroup(LandGroupKeyInner, LandMeshStreamInner);
	RtMesh->UpdateSectionConfig(LandSectionKeyInner, RtMesh->GetSectionConfig(LandSectionKeyInner), GetDepth() >= MaxDepth - 3);
}

void QuadTreeNode::GenerateMeshData2() {
	if (!IsDirty || !NoiseGen || !IsInitialized) return;
	IsGenerating = true;
	IsDirty = false;

	LandVertices.Reset();
	SeaVertices.Reset();
	LandNormals.Reset();
	SeaNormals.Reset();
	LandColors.Reset();
	SeaColors.Reset();
	TexCoords.Reset();
	AllTriangles.Reset();
	EdgeTriangleIndices.Reset();
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

	for (int32 x = 0; x < ModifiedResolution; x++) {
		for (int32 y = 0; y < ModifiedResolution; y++) {
			int idx = GenerateVertex2(x - 1, y - 1, step);
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

			if ((x + y) % 2 != 0) {
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

				//if (isEdge) {
				//	EdgeTriangleIndices.Add(addedIdx);
				//}
				if (!isVirtual) {
					PatchTriangleIndices.Add(addedIdx);
				}
			}
		}
	}

	LandCentroid = LandCentroid / LandVertices.Num();
	SeaCentroid = SeaCentroid / SeaVertices.Num();

	uint32 numPos = (uint32)LandVertices.Num();
	for (uint32 i = 0; i < numPos; i++)
	{
		FVector vertexNormal = FVector::ZeroVector;
		// Calculate the normal by averaging the normals of neighboring triangles
		for (int32 j = 0; j < AllTriangles.Num(); j++)
		{
			auto tri = AllTriangles[j];
			if (tri[0] == i || tri[1] == i || tri[2] == i)
			{
				const FVector& P0 = LandVertices[tri[0]];
				const FVector& P1 = LandVertices[tri[1]];
				const FVector& P2 = LandVertices[tri[2]];
				// Calculate the face normal
				FVector FaceNormal = FVector::CrossProduct(P1 - P0, P2 - P0).GetSafeNormal();
				// Add the face normal to the vertex normal
				vertexNormal += FaceNormal;
			}
		}
		// Normalize the resulting vertex normal
		vertexNormal;
		// Assuming 'center' is the center of your sphere or reference point
		FVector referenceVector = (LandVertices[i] - sphereCenter).GetSafeNormal();
		if (FVector::DotProduct(vertexNormal, referenceVector) < 0)
		{
			vertexNormal *= -1; // Invert the normal
		}

		//normals.Add(vertexNormal);
		LandNormals.Add((FVector3f)vertexNormal.GetSafeNormal());

		//Calculate sea normal
		FVector seaNormal = (SeaVertices[i] - sphereCenter).GetSafeNormal();
		SeaNormals.Add((FVector3f)seaNormal);
	}

	bool alwaysRenderOcean = false;
	double seaMeshTolerance = 10.0;

	//UpdatePatchMesh();
	UpdateEdgeMesh();
	//RtMesh->UpdateSectionGroup(LandGroupKeyEdge, LandMeshStreamEdge);
	//RtMesh->UpdateSectionConfig(LandSectionKeyEdge, RtMesh->GetSectionConfig(LandSectionKeyEdge), GetDepth() >= MaxDepth - 3);
	//if (alwaysRenderOcean || MinLandRadius + seaMeshTolerance <= SeaLevel) {
	//	RenderSea = true;
	//	RtMesh->UpdateSectionGroup(SeaGroupKeyInner, SeaMeshStreamInner);
	//}
}

void QuadTreeNode::GenerateMeshData()
{
	GenerateMeshData2();

	//if (!IsDirty || !NoiseGen || !IsInitialized) return;
	//IsDirty = false;
	//FVector sphereCenter = ParentActor->GetActorLocation();
	//CenterOnSphere = Center.GetSafeNormal() * SphereRadius;

	//int Resolution = ParentActor->FaceResolution;
	//int curLodLevel = GetDepth();
	//float step = (Size) / (float)(Resolution - 1);

	////Land stream builders
	//auto landBuilders = InitializeStreamBuilders(LandMeshStreamInner, ParentActor->FaceResolution);
	//auto seaBuilders = InitializeStreamBuilders(SeaMeshStreamInner, ParentActor->FaceResolution);

	//TMap<EdgeOrientation, TArray<int>> EdgeVertexIndexMap;
	//EdgeVertexIndexMap.Add(EdgeOrientation::LEFT);
	//EdgeVertexIndexMap.Add(EdgeOrientation::UP);
	//EdgeVertexIndexMap.Add(EdgeOrientation::RIGHT);
	//EdgeVertexIndexMap.Add(EdgeOrientation::DOWN);

	////These values track various max/min values found during iteration
	//double maxDist = 0.0;
	//LandCentroid = FVector::ZeroVector;
	//SeaCentroid = FVector::ZeroVector;
	////SphereCentroid = FVector::ZeroVector;
	//
	//MinLandRadius = SphereRadius * 10.0;
	//MaxLandRadius = 0.0;
	//MaxNodeRadius = 0.0;
	//float maxDepth = 0.0f;
	//
	//double seaLevel = NoiseGen->GetSeaLevel();
	//double maxDisplacement = SphereRadius * NoiseGen->GetAmplitudeScale();
	//double maxLandDisplacement = maxDisplacement * FMath::Abs(NoiseGen->GetMaxBound());
	//double maxSeaDisplacement = maxDisplacement * FMath::Abs(NoiseGen->GetMinBound());
	//FVector tSphereCentroid;
	////First build our regular grid
	//for (int32 x = 0; x < Resolution; x++) {
	//	for (int32 y = 0; y < Resolution; y++) {
	//		int idx = GenerateVertex(x, y, step, landBuilders, seaBuilders);
	//	}
	//}
	////SphereCentroid = SphereCentroid / landBuilders.PositionBuilder->Num();
	////tSphereCentroid = SphereCentroid;

	////Populate triangles & subdiv
	//int tResolution = Resolution - 1;
	//for (int32 x = 0; x < tResolution; x++) {
	//	for (int32 y = 0; y < tResolution; y++) {
	//		// Calculate base vertex indices for this quad
	//		int topLeft = x * ParentActor->FaceResolution + y;
	//		int topRight = topLeft + 1;
	//		int bottomLeft = topLeft + ParentActor->FaceResolution;
	//		int bottomRight = bottomLeft + 1;

	//		// Check which edges need LOD transitions
	//		bool leftLodChange = x == 0 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::LEFT];
	//		bool topLodChange = y == 0 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::UP];
	//		bool rightLodChange = x == tResolution - 1 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::RIGHT];
	//		bool bottomLodChange = y == tResolution - 1 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::DOWN];

	//		TArray<FIndex3UI> TrianglesToAdd;

	//		//Corner cases
	//		if (leftLodChange && topLodChange) {
	//			int leftMidPoint = GenerateVertex(x, y + 0.5, step, landBuilders, seaBuilders);
	//			int topMidPoint = GenerateVertex(x + 0.5, y, step, landBuilders, seaBuilders);
	//			// Triangulation for left+top corner
	//			TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomLeft, bottomRight));
	//			TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomRight, topLeft));
	//			TrianglesToAdd.Add(FIndex3UI(leftMidPoint, bottomRight, topRight));
	//			TrianglesToAdd.Add(FIndex3UI(leftMidPoint, topLeft, bottomRight));
	//		}
	//		else if (leftLodChange && bottomLodChange) {
	//			int leftMidPoint = GenerateVertex(x, y + 0.5, step, landBuilders, seaBuilders);
	//			int bottomMidPoint = GenerateVertex(x + 0.5, y + 1, step, landBuilders, seaBuilders);
	//			// Triangulation for left+bottom corner
	//			TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, bottomLeft, bottomRight));
	//			TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topRight, bottomLeft));
	//			TrianglesToAdd.Add(FIndex3UI(leftMidPoint, topLeft, bottomLeft));
	//			TrianglesToAdd.Add(FIndex3UI(leftMidPoint, bottomLeft, topRight));
	//		}
	//		else if (rightLodChange && topLodChange) {
	//			int rightMidPoint = GenerateVertex(x + 1, y + 0.5, step, landBuilders, seaBuilders);
	//			int topMidPoint = GenerateVertex(x + 0.5, y, step, landBuilders, seaBuilders);
	//			// Triangulation for right+top corner
	//			TrianglesToAdd.Add(FIndex3UI(topMidPoint, topRight, topLeft));
	//			TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomLeft, topRight));
	//			TrianglesToAdd.Add(FIndex3UI(rightMidPoint, bottomRight, topRight));//bugged
	//			TrianglesToAdd.Add(FIndex3UI(rightMidPoint, topRight, bottomLeft));
	//		}
	//		else if (rightLodChange && bottomLodChange) {
	//			int rightMidPoint = GenerateVertex(x + 1, y + 0.5, step, landBuilders, seaBuilders);
	//			int bottomMidPoint = GenerateVertex(x + 0.5, y + 1, step, landBuilders, seaBuilders);
	//			// Triangulation for right+bottom corner
	//			TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topRight, topLeft));
	//			TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topLeft, bottomRight));
	//			TrianglesToAdd.Add(FIndex3UI(rightMidPoint, topLeft, bottomLeft));
	//			TrianglesToAdd.Add(FIndex3UI(rightMidPoint, bottomRight, topLeft));
	//		}
	//		//Edge cases
	//		else if (leftLodChange) {
	//			int leftMidPoint = GenerateVertex(x, y + 0.5, step, landBuilders, seaBuilders);
	//			TrianglesToAdd.Add(FIndex3UI(leftMidPoint, topLeft, bottomLeft));
	//			TrianglesToAdd.Add(FIndex3UI(leftMidPoint, bottomLeft, bottomRight));
	//			TrianglesToAdd.Add(FIndex3UI(leftMidPoint, bottomRight, topRight));
	//		}
	//		else if (rightLodChange) {
	//			int rightMidPoint = GenerateVertex(x + 1, y + 0.5, step, landBuilders, seaBuilders);
	//			TrianglesToAdd.Add(FIndex3UI(rightMidPoint, bottomRight, topRight));
	//			TrianglesToAdd.Add(FIndex3UI(rightMidPoint, topRight, topLeft));
	//			TrianglesToAdd.Add(FIndex3UI(rightMidPoint, topLeft, bottomLeft));
	//		}
	//		else if (topLodChange) {
	//			int topMidPoint = GenerateVertex(x + 0.5, y, step, landBuilders, seaBuilders);
	//			TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomLeft, bottomRight));
	//			TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomRight, topRight));
	//			TrianglesToAdd.Add(FIndex3UI(topMidPoint, topRight, topLeft));
	//		}
	//		else if (bottomLodChange) {
	//			int bottomMidPoint = GenerateVertex(x + 0.5, y + 1, step, landBuilders, seaBuilders);
	//			TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, bottomLeft, bottomRight));
	//			TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topLeft, bottomLeft));
	//			TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topRight, topLeft));
	//		}
	//		//Base case
	//		else {
	//			if ((x+y) % 2 == 0) {
	//				TrianglesToAdd.Add(FIndex3UI(topLeft, bottomLeft, bottomRight));
	//				TrianglesToAdd.Add(FIndex3UI(topLeft, bottomRight, topRight));
	//			}
	//			else {
	//				TrianglesToAdd.Add(FIndex3UI(topLeft, bottomLeft, topRight));
	//				TrianglesToAdd.Add(FIndex3UI(topRight, bottomLeft, bottomRight));
	//			}
	//		}

	//		for (FIndex3UI aTriangle : TrianglesToAdd) {
	//			if (FaceTransform.bFlipWinding) {
	//				landBuilders.TrianglesBuilder->Add(FIndex3UI(aTriangle.V0, aTriangle.V2, aTriangle.V1));
	//				seaBuilders.TrianglesBuilder->Add(FIndex3UI(aTriangle.V0, aTriangle.V2, aTriangle.V1));
	//			}
	//			else {
	//				landBuilders.TrianglesBuilder->Add(aTriangle);
	//				seaBuilders.TrianglesBuilder->Add(aTriangle);
	//			}

	//			landBuilders.PolygroupsBuilder->Add(0);
	//			seaBuilders.PolygroupsBuilder->Add(1);
	//		}
	//	}
	//}

	//LandCentroid = LandCentroid / landBuilders.PositionBuilder->Num();
	//SeaCentroid = SeaCentroid / seaBuilders.PositionBuilder->Num();
	////SphereCentroid = tSphereCentroid;

	////RealtimeMeshAlgo::GenerateTangents(LandMeshStreamInner, true);
	////RealtimeMeshAlgo::GenerateTangents(SeaMeshStreamInner, true);
	//uint32 numPos = (uint32)landBuilders.PositionBuilder->Num();
	//for (uint32 i = 0; i < numPos; i++)
	//{
	//	FVector vertexNormal = FVector::ZeroVector;
	//	// Calculate the normal by averaging the normals of neighboring triangles
	//	for (int32 j = 0; j < landBuilders.TrianglesBuilder->Num(); j++)
	//	{
	//		auto tri = landBuilders.TrianglesBuilder->Get(j);
	//		if (tri[0] == i || tri[1] == i || tri[2] == i)
	//		{
	//			const FVector& P0 = landBuilders.PositionBuilder->GetValue(tri[0]);
	//			const FVector& P1 = landBuilders.PositionBuilder->GetValue(tri[1]);
	//			const FVector& P2 = landBuilders.PositionBuilder->GetValue(tri[2]);
	//			// Calculate the face normal
	//			FVector FaceNormal = FVector::CrossProduct(P1 - P0, P2 - P0).GetSafeNormal();
	//			// Add the face normal to the vertex normal
	//			vertexNormal += FaceNormal;
	//		}
	//	}
	//	// Normalize the resulting vertex normal
	//	vertexNormal.Normalize();
	//	// Assuming 'center' is the center of your sphere or reference point
	//	FVector referenceVector = (landBuilders.PositionBuilder->GetValue(i) - sphereCenter).GetSafeNormal();
	//	if (FVector::DotProduct(vertexNormal, referenceVector) < 0)
	//	{
	//		vertexNormal *= -1; // Invert the normal
	//	}

	//	//normals.Add(vertexNormal);
	//	FRealtimeMeshTangentsHighPrecision tan;
	//	tan.SetNormal(FVector3f(vertexNormal.X, vertexNormal.Y, vertexNormal.Z));
	//	landBuilders.TangentBuilder->Add(tan);

	//	//Calculate sea normal and tangent
	//	FVector seaNormal = (seaBuilders.PositionBuilder->GetValue(i) - sphereCenter).GetSafeNormal();
	//	FRealtimeMeshTangentsHighPrecision seaTan;
	//	FVector reference = FVector(0, 0, 1);
	//	if (FMath::Abs(FVector::DotProduct(seaNormal, reference)) > 0.99f) {
	//		reference = FVector(0, 1, 0);
	//	}
	//	FVector tangent = FVector::CrossProduct(seaNormal, reference).GetSafeNormal();
	//	seaTan.SetNormal(FVector3f(seaNormal.X, seaNormal.Y, seaNormal.Z));
	//	seaTan.SetTangent(FVector3f(tangent.X, tangent.Y, tangent.Z));
	//	seaBuilders.TangentBuilder->Add(seaTan);
	//}

	/////Async(EAsyncExecution::TaskGraphMainThread, [this]() {
	//bool alwaysRenderOcean = false;
	//double seaMeshTolerance = 10.0;

	//RtMesh->UpdateSectionGroup(LandGroupKeyInner, LandMeshStreamInner);
	//RtMesh->UpdateSectionConfig(LandSectionKeyInner, RtMesh->GetSectionConfig(LandSectionKeyInner), GetDepth() == MaxDepth);
	//	
	//if (alwaysRenderOcean || MinLandRadius + seaMeshTolerance <= SeaLevel) {
	//	RenderSea = true;
	//	RtMesh->UpdateSectionGroup(SeaGroupKeyInner, SeaMeshStreamInner);
	//}
	////});
}