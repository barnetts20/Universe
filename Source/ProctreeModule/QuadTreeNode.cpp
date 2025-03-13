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

	this->SeaLevel = InRadius;
}

QuadTreeNode::~QuadTreeNode()
{
	//this->DestroyChunk();
}

void QuadTreeNode::UpdateLod()
{
	RecurseUpdateLod(this->AsShared());
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
	if (this->IsInitialized && this->IsLeaf()) {
		double k = 8;
		double fov = this->ParentActor->GetCameraFOV();
		FVector lastCamPos = this->ParentActor->GetLastCameraPosition();
		auto lastCamRot = this -> ParentActor->GetLastCameraRotation();

		//Since we are doing origin rebasing frequently, the actors location can "change" arbitrarily and needs to be accounted for
		FVector planetCenter = this->ParentActor->GetActorLocation(); 
		FVector adjustedCentroid = this->LandCentroid * this->ParentActor->GetActorScale().X + planetCenter;
		auto parentCenter = adjustedCentroid;
		auto parentSize = this->MaxNodeRadius * this->ParentActor->GetActorScale().X;

		double planetRadius = FVector::Distance(this->ParentActor->GetActorLocation(), adjustedCentroid);

		auto parent = this->Parent;

		if (parent.IsValid()) {
			parentCenter = parent.Pin()->LandCentroid * this->ParentActor->GetActorScale().X + planetCenter;
			parentSize = parent.Pin()->MaxNodeRadius * this->ParentActor->GetActorScale().X;
		}

		double d1 = FVector::Distance(lastCamPos, adjustedCentroid);		
		double d2 = FVector::Distance(lastCamPos, parentCenter);
		if (ShouldSplit(adjustedCentroid, lastCamPos, fov, k)) {
			this->CanMerge = false;
			if (this->LastRenderedState) {
				QuadTreeNode::Split(this->AsShared());
			}
		}
		else if ((parent.IsValid() && parent.Pin()->GetDepth() >= this->MinDepth) && k * 1.05 * parentSize < s(d2, fov)) {
			this->CanMerge = true;
			if (this->Index.GetQuadrant() == 3)
			parent.Pin()->TryMerge();
		}
		else {
			this->CanMerge = false;
		}
	}
}

bool QuadTreeNode::ShouldSplit(FVector centroid, FVector lastCamPos, double fov, double k) {
	double d1 = FVector::Distance(lastCamPos, centroid);
	return this->GetDepth() < this->MinDepth || (this->GetDepth() < this->MaxDepth && k * this->MaxNodeRadius * this->ParentActor->GetActorScale().X > s(d1, fov));
}

void QuadTreeNode::UpdateNeighborEdge(EdgeOrientation InEdge, int InLod) {
	if (NeighborLods[(uint8)InEdge] != InLod) {
		NeighborLods[(uint8)InEdge] = InLod;
		IsDirty = true;
		//Async(EAsyncExecution::LargeThreadPool, [this]() {
			GenerateMeshData();
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

	UEnum* FaceEnum = StaticEnum<EFaceDirection>();

	if (Index.FaceId != LeftRemap.FaceId) {
		UE_LOG(LogTemp, Warning, TEXT("INDEX MAP: OG  FACE %s -> LEFT %s -> RIGHT %s"),
			*FaceEnum->GetNameStringByValue(Index.FaceId),
			*FaceEnum->GetNameStringByValue(LeftTestIndex.FaceId),
			*FaceEnum->GetNameStringByValue(LeftRemap.FaceId));
		UE_LOG(LogTemp, Warning, TEXT("INDEX MAP: ORIGINAL %s -> LEFT %s -> RIGHT %s"),
			*PathToBinary(Index.EncodedPath),
			*PathToBinary(LeftTestIndex.EncodedPath),
			*PathToBinary(LeftRemap.EncodedPath));
	}

	if (Index.FaceId != RightRemap.FaceId) {
		UE_LOG(LogTemp, Warning, TEXT("INDEX MAP: OG  FACE %s -> RIGHT %s -> LEFT %s"),
			*FaceEnum->GetNameStringByValue(Index.FaceId),
			*FaceEnum->GetNameStringByValue(RightTestIndex.FaceId),
			*FaceEnum->GetNameStringByValue(RightRemap.FaceId));
		UE_LOG(LogTemp, Warning, TEXT("INDEX MAP: ORIGINAL %s -> RIGHT %s -> LEFT %s"),
			*PathToBinary(Index.EncodedPath),
			*PathToBinary(RightTestIndex.EncodedPath),
			*PathToBinary(RightRemap.EncodedPath));
	}

	if (Index.FaceId != UpRemap.FaceId) {
		UE_LOG(LogTemp, Warning, TEXT("INDEX MAP: OG  FACE %s -> UP %s -> DOWN %s"),
			*FaceEnum->GetNameStringByValue(Index.FaceId),
			*FaceEnum->GetNameStringByValue(UpTestIndex.FaceId),
			*FaceEnum->GetNameStringByValue(UpRemap.FaceId));
		UE_LOG(LogTemp, Warning, TEXT("INDEX MAP: ORIGINAL %s -> UP %s -> DOWN %s"),
			*PathToBinary(Index.EncodedPath),
			*PathToBinary(UpTestIndex.EncodedPath),
			*PathToBinary(UpRemap.EncodedPath));
	}

	if (Index.FaceId != DownRemap.FaceId) {
		UE_LOG(LogTemp, Warning, TEXT("INDEX MAP: OG  FACE %s -> DOWN %s -> UP %s"),
			*FaceEnum->GetNameStringByValue(Index.FaceId),
			*FaceEnum->GetNameStringByValue(DownTestIndex.FaceId),
			*FaceEnum->GetNameStringByValue(DownRemap.FaceId));
		UE_LOG(LogTemp, Warning, TEXT("INDEX MAP: ORIGINAL %s -> DOWN %s -> UP %s"),
			*PathToBinary(Index.EncodedPath),
			*PathToBinary(DownTestIndex.EncodedPath),
			*PathToBinary(DownRemap.EncodedPath));
	}

	if (LeftNeighborNode) LeftNeighborNode->UpdateNeighborEdge(EdgeOrientation::RIGHT, InLod);
	if (UpNeighborNode)	UpNeighborNode->UpdateNeighborEdge(EdgeOrientation::DOWN, InLod);
	if (RightNeighborNode) RightNeighborNode->UpdateNeighborEdge(EdgeOrientation::LEFT, InLod);
	if (DownNeighborNode) DownNeighborNode->UpdateNeighborEdge(EdgeOrientation::UP, InLod);
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
	else if(IsDirty){
		GenerateMeshData();
	}
}

void QuadTreeNode::CollectLeaves(TArray<TSharedPtr<QuadTreeNode>>& OutLeafNodes) {
	if (IsLeaf()) {
		OutLeafNodes.Add(this->AsShared());
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
			Async(EAsyncExecution::TaskGraphMainThread, [inNode]() {
				inNode->UpdateNeighborLod(inNode->Index.GetDepth() + 1);
				inNode->SetChunkVisibility(false);
				inNode->IsRestructuring = false;
			}).Wait();
			for (int i = 0; i < 4; i++) {
				inNode->Children[i]->GenerateMeshData();
			}
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
		inNode->UpdateNeighborLod(inNode->Index.GetDepth());
		inNode->SetChunkVisibility(true);
		inNode->Children[0]->DestroyChunk();
		inNode->Children[1]->DestroyChunk();
		inNode->Children[2]->DestroyChunk();
		inNode->Children[3]->DestroyChunk();
		inNode->Children.Empty();

		inNode->IsRestructuring = false;
	});
}

void QuadTreeNode::TryMerge()
{
	if (this->IsLeaf()) return;

	bool willMerge = true;
	for (TSharedPtr<QuadTreeNode> child : this->Children)
	{
		if (!child->CanMerge || !child->LastRenderedState)
		{
			willMerge = false;
		}
	}
	if (willMerge) {
		QuadTreeNode::Merge(this->AsShared());
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
	return this->Children.Num() == 0;
}

int QuadTreeNode::GetDepth() const
{
	return Index.GetDepth();
}

////MESH STUFF
//Must invoke on game thread
void QuadTreeNode::InitializeChunk() {
	this->RtMesh = NewObject<URealtimeMeshSimple>(this->ParentActor);
	this->ChunkComponent = NewObject<URealtimeMeshComponent>(this->ParentActor, URealtimeMeshComponent::StaticClass());
	this->ChunkComponent->RegisterComponent();

	if (this->NoiseGen) {
		this->ChunkComponent->SetRenderCustomDepth(true);
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

	ChunkComponent->AttachToComponent(this->ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	ChunkComponent->SetMaterial(0, this->ParentActor->LandMaterial);
	ChunkComponent->SetMaterial(1, this->ParentActor->SeaMaterial);
	ChunkComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ChunkComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	ChunkComponent->SetRealtimeMesh(this->RtMesh);

	RtMesh->CreateSectionGroup(LandGroupKeyInner, LandMeshStreamInner);
	RtMesh->CreateSectionGroup(SeaGroupKeyInner, SeaMeshStreamInner);

	IsInitialized = true;
	LastRenderedState = true;
}
//Must invoke on game thread
void QuadTreeNode::DestroyChunk() {
	if (this->IsInitialized && this->ChunkComponent) {
		this->ParentActor->RemoveOwnedComponent(this->ChunkComponent);
		this->ChunkComponent->DestroyComponent();
	}
}
//Must invoke on game thread
void QuadTreeNode::SetChunkVisibility(bool inVisibility) {
	this->RtMesh->SetSectionVisibility(this->LandSectionKeyInner, inVisibility).Then([this, inVisibility](TFuture<ERealtimeMeshProxyUpdateStatus> completedFuture) {
		this->LastRenderedState = inVisibility;
		});
	if (this->RenderSea) {
		this->RtMesh->SetSectionVisibility(this->SeaSectionKeyInner, inVisibility).Then([this, inVisibility](TFuture<ERealtimeMeshProxyUpdateStatus> completedFuture) {
			this->LastRenderedState = inVisibility;
			});
	}
}

FMeshStreamBuilders QuadTreeNode::InitializeStreamBuilders(FRealtimeMeshStreamSet& inMeshStream, int inResolution){
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
	FVector result = this->Center;

	// Get normalized coordinates in face-local space
	double normX = -this->HalfSize + step * x;
	double normY = -this->HalfSize + step * y;

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

int QuadTreeNode::GenerateVertex(double x, double y, double step, FMeshStreamBuilders& landBuilders, FMeshStreamBuilders& seaBuilders) {
	int Resolution = ParentActor->FaceResolution;
	FVector facePoint = GetFacePoint(step, x, y);
	FVector normalizedPoint = facePoint.GetSafeNormal();
	FVector seaPoint = normalizedPoint * SphereRadius;//TODO: SEA LEVEL INTEGRATION
	FVector landPoint = NoiseGen->GetNoiseFromPosition(normalizedPoint) * SphereRadius;
	double landRadius = FVector::Distance(landPoint, ParentActor->GetActorLocation());
	double seaRadius = SphereRadius;
	MinLandRadius = FMath::Min(landRadius, MinLandRadius);
	MaxLandRadius = FMath::Max(landRadius, MaxLandRadius);
	//MaxNodeRadius = FMath::Max(MaxNodeRadius, FVector::Dist(CenterOnSphere, landPoint));

	SphereCentroid += normalizedPoint * SphereRadius;
	LandCentroid += landPoint;
	SeaCentroid += seaPoint;


	int returnIndex = landBuilders.PositionBuilder->Add(landPoint);
	seaBuilders.PositionBuilder->Add(seaPoint);

	landBuilders.ColorBuilder->Add(EncodeDepthColor(landRadius - seaRadius));
	seaBuilders.ColorBuilder->Add(EncodeDepthColor(seaRadius - landRadius));
	
	//Populate UVs
	FVector2f UV = FVector2f((atan2(normalizedPoint.Y, normalizedPoint.X) + PI) / (2 * PI), (acos(normalizedPoint.Z / normalizedPoint.Size()) / PI));
	landBuilders.TexCoordsBuilder->Add(UV);
	seaBuilders.TexCoordsBuilder->Add(UV);

	return returnIndex;
}

void QuadTreeNode::GenerateMeshData()
{
	if (!IsDirty || !NoiseGen || !IsInitialized) return;
	IsDirty = false;
	FVector sphereCenter = this->ParentActor->GetActorLocation();
	CenterOnSphere = Center.GetSafeNormal() * SphereRadius;

	int Resolution = ParentActor->FaceResolution;
	int curLodLevel = this->GetDepth();
	float step = (this->Size) / (float)(Resolution - 1);

	//Land stream builders
	auto landBuilders = InitializeStreamBuilders(LandMeshStreamInner, ParentActor->FaceResolution);
	auto seaBuilders = InitializeStreamBuilders(SeaMeshStreamInner, ParentActor->FaceResolution);

	TMap<EdgeOrientation, TArray<int>> EdgeVertexIndexMap;
	EdgeVertexIndexMap.Add(EdgeOrientation::LEFT);
	EdgeVertexIndexMap.Add(EdgeOrientation::UP);
	EdgeVertexIndexMap.Add(EdgeOrientation::RIGHT);
	EdgeVertexIndexMap.Add(EdgeOrientation::DOWN);

	//These values track various max/min values found during iteration
	double maxDist = 0.0;
	LandCentroid = FVector::ZeroVector;
	SeaCentroid = FVector::ZeroVector;
	SphereCentroid = FVector::ZeroVector;
	
	MinLandRadius = this->SphereRadius * 10.0;
	MaxLandRadius = 0.0;
	MaxNodeRadius = 0.0;
	float maxDepth = 0.0f;
	
	double seaLevel = this->NoiseGen->GetSeaLevel();
	double maxDisplacement = this->SphereRadius * this->NoiseGen->GetAmplitudeScale();
	double maxLandDisplacement = maxDisplacement * FMath::Abs(this->NoiseGen->GetMaxBound());
	double maxSeaDisplacement = maxDisplacement * FMath::Abs(this->NoiseGen->GetMinBound());
	FVector tSphereCentroid;
	//First build our regular grid
	for (int32 x = 0; x < Resolution; x++) {
		for (int32 y = 0; y < Resolution; y++) {
			int idx = GenerateVertex(x, y, step, landBuilders, seaBuilders);
		}
	}
	SphereCentroid = SphereCentroid / landBuilders.PositionBuilder->Num();
	tSphereCentroid = SphereCentroid;

	//Populate triangles & subdiv
	int tResolution = Resolution - 1;
	for (int32 x = 0; x < tResolution; x++) {
		for (int32 y = 0; y < tResolution; y++) {
			// Calculate base vertex indices for this quad
			int topLeft = x * ParentActor->FaceResolution + y;
			int topRight = topLeft + 1;
			int bottomLeft = topLeft + ParentActor->FaceResolution;
			int bottomRight = bottomLeft + 1;

			// Check which edges need LOD transitions
			bool leftLodChange = x == 0 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::LEFT];
			bool topLodChange = y == 0 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::UP];
			bool rightLodChange = x == tResolution - 1 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::RIGHT];
			bool bottomLodChange = y == tResolution - 1 && Index.GetDepth() < NeighborLods[(uint8)EdgeOrientation::DOWN];

			TArray<FIndex3UI> TrianglesToAdd;

			//Corner cases
			if (leftLodChange && topLodChange) {
				int leftMidPoint = GenerateVertex(x, y + 0.5, step, landBuilders, seaBuilders);
				int topMidPoint = GenerateVertex(x + 0.5, y, step, landBuilders, seaBuilders);
				// Triangulation for left+top corner
				TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomLeft, bottomRight));
				TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomRight, topLeft));
				TrianglesToAdd.Add(FIndex3UI(leftMidPoint, bottomRight, topRight));
				TrianglesToAdd.Add(FIndex3UI(leftMidPoint, topLeft, bottomRight));
			}
			else if (leftLodChange && bottomLodChange) {
				int leftMidPoint = GenerateVertex(x, y + 0.5, step, landBuilders, seaBuilders);
				int bottomMidPoint = GenerateVertex(x + 0.5, y + 1, step, landBuilders, seaBuilders);
				// Triangulation for left+bottom corner
				TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, bottomLeft, bottomRight));
				TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topRight, bottomLeft));
				TrianglesToAdd.Add(FIndex3UI(leftMidPoint, topLeft, bottomLeft));
				TrianglesToAdd.Add(FIndex3UI(leftMidPoint, bottomLeft, topRight));
			}
			else if (rightLodChange && topLodChange) {
				int rightMidPoint = GenerateVertex(x + 1, y + 0.5, step, landBuilders, seaBuilders);
				int topMidPoint = GenerateVertex(x + 0.5, y, step, landBuilders, seaBuilders);
				// Triangulation for right+top corner
				TrianglesToAdd.Add(FIndex3UI(topMidPoint, topRight, topLeft));
				TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomLeft, topRight));
				TrianglesToAdd.Add(FIndex3UI(rightMidPoint, bottomRight, topRight));//bugged
				TrianglesToAdd.Add(FIndex3UI(rightMidPoint, topRight, bottomLeft));
			}
			else if (rightLodChange && bottomLodChange) {
				int rightMidPoint = GenerateVertex(x + 1, y + 0.5, step, landBuilders, seaBuilders);
				int bottomMidPoint = GenerateVertex(x + 0.5, y + 1, step, landBuilders, seaBuilders);
				// Triangulation for right+bottom corner
				TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topRight, topLeft));
				TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topLeft, bottomRight));
				TrianglesToAdd.Add(FIndex3UI(rightMidPoint, topLeft, bottomLeft));
				TrianglesToAdd.Add(FIndex3UI(rightMidPoint, bottomRight, topLeft));
			}
			//Edge cases
			else if (leftLodChange) {
				int leftMidPoint = GenerateVertex(x, y + 0.5, step, landBuilders, seaBuilders);
				TrianglesToAdd.Add(FIndex3UI(leftMidPoint, topLeft, bottomLeft));
				TrianglesToAdd.Add(FIndex3UI(leftMidPoint, bottomLeft, bottomRight));
				TrianglesToAdd.Add(FIndex3UI(leftMidPoint, bottomRight, topRight));
			}
			else if (rightLodChange) {
				int rightMidPoint = GenerateVertex(x + 1, y + 0.5, step, landBuilders, seaBuilders);
				TrianglesToAdd.Add(FIndex3UI(rightMidPoint, bottomRight, topRight));
				TrianglesToAdd.Add(FIndex3UI(rightMidPoint, topRight, topLeft));
				TrianglesToAdd.Add(FIndex3UI(rightMidPoint, topLeft, bottomLeft));
			}
			else if (topLodChange) {
				int topMidPoint = GenerateVertex(x + 0.5, y, step, landBuilders, seaBuilders);
				TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomLeft, bottomRight));
				TrianglesToAdd.Add(FIndex3UI(topMidPoint, bottomRight, topRight));
				TrianglesToAdd.Add(FIndex3UI(topMidPoint, topRight, topLeft));
			}
			else if (bottomLodChange) {
				int bottomMidPoint = GenerateVertex(x + 0.5, y + 1, step, landBuilders, seaBuilders);
				TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, bottomLeft, bottomRight));
				TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topLeft, bottomLeft));
				TrianglesToAdd.Add(FIndex3UI(bottomMidPoint, topRight, topLeft));
			}
			//Base case
			else {
				TrianglesToAdd.Add(FIndex3UI(topLeft, bottomLeft, topRight));
				TrianglesToAdd.Add(FIndex3UI(topRight, bottomLeft, bottomRight));
			}

			for (FIndex3UI aTriangle : TrianglesToAdd) {
				landBuilders.TrianglesBuilder->Add(aTriangle);
				landBuilders.PolygroupsBuilder->Add(0);
				seaBuilders.TrianglesBuilder->Add(aTriangle);
				seaBuilders.PolygroupsBuilder->Add(1);
			}
		}
	}

	LandCentroid = LandCentroid / landBuilders.PositionBuilder->Num();
	SeaCentroid = SeaCentroid / seaBuilders.PositionBuilder->Num();
	SphereCentroid = tSphereCentroid;

	//RealtimeMeshAlgo::GenerateTangents(this->LandMeshStreamInner, true);
	//RealtimeMeshAlgo::GenerateTangents(this->SeaMeshStreamInner, true);
	uint32 numPos = (uint32)landBuilders.PositionBuilder->Num();
	for (uint32 i = 0; i < numPos; i++)
	{
		//	//Calculate depth for ocean
		FVector lPoint = landBuilders.PositionBuilder->GetValue(i);
		float curDist = FVector::Distance(lPoint, LandCentroid);
		maxDist = FMath::Max(maxDist, curDist);
		FVector vertexNormal = FVector::ZeroVector;
		// Calculate the normal by averaging the normals of neighboring triangles
		for (int32 j = 0; j < landBuilders.TrianglesBuilder->Num(); j++)
		{
			auto tri = landBuilders.TrianglesBuilder->Get(j);
			if (tri[0] == i || tri[1] == i || tri[2] == i)
			{
				const FVector& P0 = landBuilders.PositionBuilder->GetValue(tri[0]);
				const FVector& P1 = landBuilders.PositionBuilder->GetValue(tri[1]);
				const FVector& P2 = landBuilders.PositionBuilder->GetValue(tri[2]);
				// Calculate the face normal
				FVector FaceNormal = FVector::CrossProduct(P1 - P0, P2 - P0).GetSafeNormal();
				// Add the face normal to the vertex normal
				vertexNormal += FaceNormal;
			}
		}
		// Normalize the resulting vertex normal
		vertexNormal.Normalize();
		// Assuming 'center' is the center of your sphere or reference point
		FVector referenceVector = (landBuilders.PositionBuilder->GetValue(i) - sphereCenter).GetSafeNormal();
		if (FVector::DotProduct(vertexNormal, referenceVector) < 0)
		{
			vertexNormal *= -1; // Invert the normal
		}

		//normals.Add(vertexNormal);
		FRealtimeMeshTangentsHighPrecision tan;
		tan.SetNormal(FVector3f(vertexNormal.X, vertexNormal.Y, vertexNormal.Z));
		landBuilders.TangentBuilder->Add(tan);

		//Calculate sea normal and tangent
		FVector seaNormal = (seaBuilders.PositionBuilder->GetValue(i) - sphereCenter).GetSafeNormal();
		FRealtimeMeshTangentsHighPrecision seaTan;
		FVector reference = FVector(0, 0, 1);
		if (FMath::Abs(FVector::DotProduct(seaNormal, reference)) > 0.99f) {
			reference = FVector(0, 1, 0);
		}
		FVector tangent = FVector::CrossProduct(seaNormal, reference).GetSafeNormal();
		seaTan.SetNormal(FVector3f(seaNormal.X, seaNormal.Y, seaNormal.Z));
		seaTan.SetTangent(FVector3f(tangent.X, tangent.Y, tangent.Z));
		seaBuilders.TangentBuilder->Add(seaTan);
	}
	
	MaxNodeRadius = maxDist;

	///Async(EAsyncExecution::TaskGraphMainThread, [this]() {
	bool alwaysRenderOcean = false;
	double seaMeshTolerance = 10.0;

	//RtMesh->UpdateSectionGroup(this->LandGroupKeyInner, this->LandMeshStreamInner);
	//RtMesh->UpdateSectionConfig(this->LandSectionKeyInner, this->RtMesh->GetSectionConfig(this->LandSectionKeyInner), this->GetDepth() == this->MaxDepth);
		
	//if (alwaysRenderOcean || MinLandRadius + seaMeshTolerance <= this->SeaLevel) {
		RenderSea = true;
		RtMesh->UpdateSectionGroup(this->SeaGroupKeyInner, this->SeaMeshStreamInner);
	//}
	//});
}