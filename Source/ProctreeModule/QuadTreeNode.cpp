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

	lodKey = FRealtimeMeshLODKey::FRealtimeMeshLODKey(0);
	
	NeighborLodChangeMap.Add(EdgeOrientation::LEFT, false);
	NeighborLodChangeMap.Add(EdgeOrientation::TOP, false);
	NeighborLodChangeMap.Add(EdgeOrientation::RIGHT, false);
	NeighborLodChangeMap.Add(EdgeOrientation::BOTTOM, false);

	this->SeaLevel = InRadius;
	this->RtMesh = NewObject<URealtimeMeshSimple>(this->ParentActor);
	//GenerateMeshData();
}

QuadTreeNode::~QuadTreeNode()
{
	//this->DestroyChunk();
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

FVector CalculateFinalPoint(const FVector& PointA, const FVector& PointB, const FVector& CenterPoint, float Radius) {
	// Check if the distance from B to PC is less than the radius
	if (FVector::Distance(PointB, CenterPoint) < Radius) {
		// Calculate the direction from A to B
		FVector Direction = (PointB - PointA).GetSafeNormal();

		// Solve the quadratic equation to find the intersection point
		FVector CenterToA = PointA - CenterPoint;
		float a = FVector::DotProduct(Direction, Direction);
		float b = 2.0f * FVector::DotProduct(CenterToA, Direction);
		float c = FVector::DotProduct(CenterToA, CenterToA) - Radius * Radius;

		float Discriminant = b * b - 4 * a * c;

		if (Discriminant >= 0) {
			// Calculate the two possible intersections t1 and t2
			float t1 = (-b + FMath::Sqrt(Discriminant)) / (2 * a);
			float t2 = (-b - FMath::Sqrt(Discriminant)) / (2 * a);

			// Determine the closer intersection point, assuming it's between A and B
			float t = (t1 < t2 && t1 >= 0) ? t1 : t2;

			// Calculate the final point based on the intersection
			FVector FinalPoint = PointA + Direction * t;
			return FinalPoint;
		}
	}

	// If distance from B to PC is greater than or equal to PR or no intersection, return B
	return PointB;
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
				this->Split(this->AsShared());
			}
		}
		else if ((parent.IsValid() && parent.Pin()->GetDepth() >= this->MinDepth) && k * parentSize < s(d2, fov)) {
			this->CanMerge = true;
			if (this->Index.GetQuadrant() == 3)
			parent.Pin()->TryMerge();
		}
		else {
			this->CanMerge = false;
			IsDirty = UpdateNeighborLods();
		}
	}
}

bool QuadTreeNode::ShouldSplit(FVector centroid, FVector lastCamPos, double fov, double k) {
	double d1 = FVector::Distance(lastCamPos, centroid);
	return this->GetDepth() < this->MinDepth || (this->GetDepth() < this->MaxDepth && k * this->MaxNodeRadius * this->ParentActor->GetActorScale().X > s(d1, fov));
}

void QuadTreeNode::UpdateLod()
{
	RecurseUpdateLod(this->AsShared());
}

void QuadTreeNode::CollectLeaves(TArray<TSharedPtr<QuadTreeNode>>& LeafNodes) {
	if (IsLeaf()) {
		LeafNodes.Add(this->AsShared());
	}
	else {
		for (const auto& Child : Children) {
			Child->CollectLeaves(LeafNodes);
		}
	}
}

void QuadTreeNode::Split(TSharedPtr<QuadTreeNode> inNode)
{
	if (!inNode.IsValid() || !inNode->IsLeaf())
	{
		return;
	}

	FVector2d childOffsets[4] = {
		FVector2d(-inNode->QuarterSize, -inNode->QuarterSize), // Bottom-left
		FVector2d(-inNode->QuarterSize, inNode->QuarterSize),  // Top-left
		FVector2d(inNode->QuarterSize, -inNode->QuarterSize),  // Bottom-right
		FVector2d(inNode->QuarterSize, inNode->QuarterSize)    // Top-right
	};

	// Calculate child centers using face transforms
	FVector childCenters[4];
	for (int i = 0; i < 4; i++) {
		// Start with parent center
		childCenters[i] = inNode->Center;

		// Get the local offsets
		double normX = childOffsets[i].X;
		double normY = childOffsets[i].Y;

		// Get axis mappings from face transform
		int xAxisIndex = inNode->FaceTransform.AxisMap[0];
		int yAxisIndex = inNode->FaceTransform.AxisMap[1];
		int normalAxisIndex = inNode->FaceTransform.AxisMap[2];

		int xAxisSign = inNode->FaceTransform.AxisDir[0];
		int yAxisSign = inNode->FaceTransform.AxisDir[1];
		int normalAxisSign = inNode->FaceTransform.AxisDir[2];

		// Apply offsets to correct axes
		double offsets[3] = { 0, 0, 0 };
		offsets[xAxisIndex] += xAxisSign * normX;
		offsets[yAxisIndex] += yAxisSign * normY;

		// Apply calculated offsets
		childCenters[i].X += offsets[0];
		childCenters[i].Y += offsets[1];
		childCenters[i].Z += offsets[2];
	}

	Children.Add(MakeShared<QuadTreeNode>(inNode->ParentActor, inNode->NoiseGen, inNode->Index.GetChildIndex(0), inNode->MinDepth, inNode->MaxDepth, inNode->FaceTransform, childCenters[0], inNode->HalfSize, inNode->SphereRadius));
	Children.Add(MakeShared<QuadTreeNode>(inNode->ParentActor, inNode->NoiseGen, inNode->Index.GetChildIndex(1), inNode->MinDepth, inNode->MaxDepth, inNode->FaceTransform, childCenters[1], inNode->HalfSize, inNode->SphereRadius));
	Children.Add(MakeShared<QuadTreeNode>(inNode->ParentActor, inNode->NoiseGen, inNode->Index.GetChildIndex(2), inNode->MinDepth, inNode->MaxDepth, inNode->FaceTransform, childCenters[2], inNode->HalfSize, inNode->SphereRadius));
	Children.Add(MakeShared<QuadTreeNode>(inNode->ParentActor, inNode->NoiseGen, inNode->Index.GetChildIndex(3), inNode->MinDepth, inNode->MaxDepth, inNode->FaceTransform, childCenters[3], inNode->HalfSize, inNode->SphereRadius));

	for (auto child : Children) {
		child->Parent = inNode;
		child->GenerateMeshData();
	}

	Async(EAsyncExecution::TaskGraphMainThread, [this, inNode]() {
		for (auto child : Children) {
			child->InitializeChunk();
		}
		inNode->SetChunkVisibility(false);
	});
}

void QuadTreeNode::Merge(TSharedPtr<QuadTreeNode> inNode)
{
	if (!inNode.IsValid() || inNode->IsLeaf()) {
		return;
	}

	Async(EAsyncExecution::TaskGraphMainThread, [inNode]() mutable {
		if (!inNode.IsValid() || inNode->IsLeaf()) {
			return;
		}
		inNode->SetChunkVisibility(true);
		inNode->Children[0]->DestroyChunk();
		inNode->Children[1]->DestroyChunk();
		inNode->Children[2]->DestroyChunk();
		inNode->Children[3]->DestroyChunk();
		inNode->Children.Empty();
	});
}

void QuadTreeNode::TryMerge()
{
	if (this->IsLeaf()) {
		return;
	}
	bool willMerge = true;
	//UE_LOG(LogTemp, Warning, TEXT("Checking if node %s can merge"), *FString(this->Id));
	for (TSharedPtr<QuadTreeNode> child : this->Children)
	{
		if (!child->CanMerge || !child->LastRenderedState)
		{
			willMerge = false;
		}
	}
	if (willMerge) {
		TSharedPtr<QuadTreeNode> sharedThis = this->AsShared();
		this->Merge(sharedThis);
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

void QuadTreeNode::SetChunkVisibility(bool inVisibility) {
	this->RtMesh->SetSectionVisibility(this->landSectionKeyInner, inVisibility).Then([this, inVisibility](TFuture<ERealtimeMeshProxyUpdateStatus> completedFuture) {
		this->LastRenderedState = inVisibility;
	});
	if (this->RenderSea) {
		this->RtMesh->SetSectionVisibility(this->seaSectionKeyInner, inVisibility).Then([this, inVisibility](TFuture<ERealtimeMeshProxyUpdateStatus> completedFuture) {
			this->LastRenderedState = inVisibility;
		});
	}
}

void QuadTreeNode::DestroyChunk() {
	if (this->IsInitialized && this->ChunkComponent) {
		this->ParentActor->RemoveOwnedComponent(this->ChunkComponent);
		this->ChunkComponent->DestroyComponent();
	}
}

//Must invoke on game thread
void QuadTreeNode::InitializeChunk() {
	this->ChunkComponent = NewObject<URealtimeMeshComponent>(this->ParentActor, URealtimeMeshComponent::StaticClass());
	this->ChunkComponent->RegisterComponent();

	if (this->NoiseGen) {
		this->ChunkComponent->SetRenderCustomDepth(true);
	}
	this->ChunkComponent->AttachToComponent(this->ParentActor->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	this->ChunkComponent->SetMaterial(0, this->ParentActor->LandMaterial);
	this->ChunkComponent->SetMaterial(1, this->ParentActor->SeaMaterial);
	this->ChunkComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	this->ChunkComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	this->ChunkComponent->SetRealtimeMesh(this->RtMesh);
	this->RtMesh->ClearInternalFlags(EInternalObjectFlags::Async);
	this->IsInitialized = true;
	this->LastRenderedState = true;
}

FMeshStreamBuilders QuadTreeNode::InitializeStreamBuilders(FRealtimeMeshStreamSet& inMeshStream, int inResolution){
	FMeshStreamBuilders Builders;

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
	

	//Append corners for virtual neighbor centroid calculations
	//"LEFT" Edge corners
	if ((x == 0 && y == 0) || (x == 0 && y == Resolution - 1)) {
		NeighborEdgeCorners.FindOrAdd(EdgeOrientation::LEFT).Add(normalizedPoint * SphereRadius);
	}
	//"RIGHT" Edge corners
	if ((x == Resolution - 1 && y == 0) || (x == Resolution - 1 && y == Resolution - 1)) {
		NeighborEdgeCorners.FindOrAdd(EdgeOrientation::RIGHT).Add(normalizedPoint * SphereRadius);
	}
	//"TOP" Edge corners
	if ((x == 0 && y == 0) || (x == Resolution - 1 && y == 0)) {
		NeighborEdgeCorners.FindOrAdd(EdgeOrientation::TOP).Add(normalizedPoint * SphereRadius);
	}
	//"BOTTOM" Edge corners
	if ((x == 0 && y == Resolution - 1) || (x == Resolution - 1 && y == Resolution - 1)) {
		NeighborEdgeCorners.FindOrAdd(EdgeOrientation::BOTTOM).Add(normalizedPoint * SphereRadius);
	}
	return returnIndex;
}


bool QuadTreeNode::UpdateNeighborLods() {
	double k = 8;
	double fov = this->ParentActor->GetCameraFOV();
	FVector lastCamPos = this->ParentActor->GetLastCameraPosition();
	bool edgeLodChange = false;
	for (auto& n : NeighborVirtualCentroids) {
		bool cLod = ShouldSplit(SphereCentroid * this->ParentActor->GetActorScale().X + this->ParentActor->GetActorLocation(), lastCamPos, fov, k);
		bool nLod = ShouldSplit(n.Value * this->ParentActor->GetActorScale().X + this->ParentActor->GetActorLocation(), lastCamPos, fov, k);
		if (NeighborLodChangeMap.FindOrAdd(n.Key) != nLod && nLod != cLod) {
			edgeLodChange = true;
			NeighborLodChangeMap.Add(n.Key, nLod);
		}
	}
	return edgeLodChange;
}

void QuadTreeNode::GenerateMeshData()
{
	//GenerateMeshDataFromTemplate();
	//if(true) return;
	if (this->IsInitialized || !this->NoiseGen) return;

	FVector sphereCenter = this->ParentActor->GetActorLocation();
	int Resolution = ParentActor->FaceResolution;
	int curLodLevel = this->GetDepth();
	float step = (this->Size) / (float)(Resolution - 1);

	//Land stream builders
	auto landBuilders = InitializeStreamBuilders(LandMeshStreamInner, ParentActor->FaceResolution);
	auto seaBuilders = InitializeStreamBuilders(SeaMeshStreamInner, ParentActor->FaceResolution);

	TMap<EdgeOrientation, TArray<int>> EdgeVertexIndexMap;
	EdgeVertexIndexMap.Add(EdgeOrientation::LEFT);
	EdgeVertexIndexMap.Add(EdgeOrientation::TOP);
	EdgeVertexIndexMap.Add(EdgeOrientation::RIGHT);
	EdgeVertexIndexMap.Add(EdgeOrientation::BOTTOM);

	//These values track various max/min values found during iteration
	double maxDist = 0.0;
	LandCentroid = FVector::ZeroVector;
	SeaCentroid = FVector::ZeroVector;
	SphereCentroid = FVector::ZeroVector;

	MinLandRadius = this->SphereRadius * 10.0;
	MaxLandRadius = 0.0;

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

	// Calculate estimated neighbor centroids
	for (auto& EdgeCornerPair : NeighborEdgeCorners)
	{
		EdgeOrientation Edge = EdgeCornerPair.Key;
		TArray<FVector>& Corners = EdgeCornerPair.Value;
		// We should have exactly 2 corners per edge
		if (Corners.Num() == 2)
		{
			FVector EdgeMidpoint = (Corners[0] + Corners[1]) * 0.5f;
			EdgeMidpoint = EdgeMidpoint.GetSafeNormal() * SphereRadius;
			FVector DirectionToMidpoint = (EdgeMidpoint - SphereCentroid).GetSafeNormal();
			float DistanceToEdge = FVector::Distance(SphereCentroid, EdgeMidpoint);
			FVector EstimatedNeighborCenter = EdgeMidpoint + DirectionToMidpoint * DistanceToEdge * 1.1;
			FVector NeighborVirtualCentroid = NoiseGen->GetNoiseFromPosition(EstimatedNeighborCenter.GetSafeNormal()) * SphereRadius;
			NeighborVirtualCentroids.Add(Edge, NeighborVirtualCentroid);
		}
	}

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
			bool leftLodChange = (x == 0 && *NeighborLodChangeMap.Find(EdgeOrientation::LEFT));
			bool topLodChange = (y == 0 && *NeighborLodChangeMap.Find(EdgeOrientation::TOP));
			bool rightLodChange = (x == tResolution - 1 && *NeighborLodChangeMap.Find(EdgeOrientation::RIGHT));
			bool bottomLodChange = (y == tResolution - 1 && *NeighborLodChangeMap.Find(EdgeOrientation::BOTTOM));

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
				auto triToAdd = (FaceTransform.bFlipWinding ? FIndex3UI(aTriangle.V0, aTriangle.V2, aTriangle.V1) : aTriangle);
				landBuilders.TrianglesBuilder->Add(triToAdd);
				landBuilders.PolygroupsBuilder->Add(0);
				seaBuilders.TrianglesBuilder->Add(triToAdd);
				seaBuilders.PolygroupsBuilder->Add(1);
			}
		}
	}

	LandCentroid = LandCentroid / landBuilders.PositionBuilder->Num();
	SeaCentroid = SeaCentroid / seaBuilders.PositionBuilder->Num();
	SphereCentroid = tSphereCentroid;
	//RealtimeMeshAlgo::GenerateTangents(this->LandMeshStreamInner, true);
	uint32 numPos = (unsigned int)landBuilders.PositionBuilder->Num();
	for (uint32 i = 0; i < numPos; i++)
	{
		//Calculate depth for ocean
		FVector lPoint = landBuilders.PositionBuilder->GetValue(i);
		FVector sPoint = seaBuilders.PositionBuilder->GetValue(i);
		float maxD = this->SphereRadius - MinLandRadius;
		float landR = FVector::Distance(lPoint, this->ParentActor->GetActorLocation());
		float seaR = FVector::Distance(sPoint, this->ParentActor->GetActorLocation());
		float curD = FMath::Max(0, seaR - landR);
		float normD = curD / maxD;

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

	// Loop through all the triangles
	for (int32 i = 0; i < landBuilders.TrianglesBuilder->Num(); i++)
	{
		auto tri = landBuilders.TrianglesBuilder->GetValue(i);

		const FVector& P0 = landBuilders.PositionBuilder->GetValue(tri[0]);
		const FVector& P1 = landBuilders.PositionBuilder->GetValue(tri[1]);
		const FVector& P2 = landBuilders.PositionBuilder->GetValue(tri[2]);

		const FVector2f& UV0 = landBuilders.TexCoordsBuilder->GetValue(tri[0]);
		const FVector2f& UV1 = landBuilders.TexCoordsBuilder->GetValue(tri[1]);
		const FVector2f& UV2 = landBuilders.TexCoordsBuilder->GetValue(tri[2]);

		FVector Edge1 = P1 - P0;
		FVector Edge2 = P2 - P0;

		FVector2f Edge1UV = UV1 - UV0;
		FVector2f Edge2UV = UV2 - UV0;

		float Det = Edge1UV.X * Edge2UV.Y - Edge1UV.Y * Edge2UV.X;

		FVector Tangent = Det == 0 ? FVector::RightVector : (Edge1 * Edge2UV.Y - Edge2 * Edge1UV.Y).GetSafeNormal();
		FVector3f fTangent = FVector3f(Tangent.X, Tangent.Y, Tangent.Z);

		auto t0 = landBuilders.TangentBuilder->GetValue(tri[0]);
		auto t1 = landBuilders.TangentBuilder->GetValue(tri[1]);
		auto t2 = landBuilders.TangentBuilder->GetValue(tri[2]);

		t0.SetTangent(fTangent);
		t1.SetTangent(fTangent);
		t2.SetTangent(fTangent);

		t0 = seaBuilders.TangentBuilder->GetValue(tri[0]);
		t1 = seaBuilders.TangentBuilder->GetValue(tri[1]);
		t2 = seaBuilders.TangentBuilder->GetValue(tri[2]);

		t0.SetTangent(FVector3f(0.0, 0.0, 0.0));
		t1.SetTangent(FVector3f(0.0, 0.0, 0.0));
		t2.SetTangent(FVector3f(0.0, 0.0, 0.0));
	}

	this->MaxNodeRadius = maxDist;

	//Initialize mesh object
	FRealtimeMeshCollisionConfiguration cConfig;
	cConfig.bShouldFastCookMeshes = false;
	cConfig.bUseComplexAsSimpleCollision = true;
	cConfig.bDeformableMesh = false;
	cConfig.bUseAsyncCook = true;
	this->RtMesh->SetCollisionConfig(cConfig);
	this->RtMesh->SetupMaterialSlot(0, "LandMaterial");
	this->RtMesh->SetupMaterialSlot(1, "SeaMaterial");
	
	bool alwaysRenderOcean = false;
	double seaMeshTolerance = 10.0;

	if (this->RtMesh->GetSectionGroup(this->landGroupKeyInner)) {
		this->RtMesh->UpdateSectionGroup(this->landGroupKeyInner, this->LandMeshStreamInner);
		this->RtMesh->UpdateSectionConfig(this->landSectionKeyInner, this->RtMesh->GetSectionConfig(this->landSectionKeyInner), this->GetDepth() == this->MaxDepth);
	}
	else {
		this->RtMesh->CreateSectionGroup(this->landGroupKeyInner, this->LandMeshStreamInner);
		this->RtMesh->UpdateSectionConfig(this->landSectionKeyInner, this->RtMesh->GetSectionConfig(this->landSectionKeyInner), this->GetDepth() == this->MaxDepth);
	}
	if (alwaysRenderOcean || MinLandRadius + seaMeshTolerance <= this->SeaLevel) {
		this->RenderSea = true;
		if (this->RtMesh->GetSectionGroup(seaGroupKeyInner).IsValid()) {
			//this->RtMesh->UpdateSectionGroup(this->seaGroupKeyInner, this->SeaMeshStreamInner);
		}
		else {
			//this->RtMesh->CreateSectionGroup(this->seaGroupKeyInner, this->SeaMeshStreamInner);
		}
	}
}