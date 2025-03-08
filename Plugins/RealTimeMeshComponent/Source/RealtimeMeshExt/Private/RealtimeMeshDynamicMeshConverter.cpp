// Copyright TriAxis Games, L.L.C. All Rights Reserved.


#include "RealtimeMeshDynamicMeshConverter.h"
#include "RealtimeMeshComponentModule.h"
#include "RealtimeMeshSimple.h"
#include "UDynamicMesh.h"
#include "DynamicMesh/DynamicMeshAttributeSet.h"
#include "Engine/StaticMesh.h"
#include "Mesh/RealtimeMeshBlueprintMeshBuilder.h"
#include "Mesh/RealtimeMeshBuilder.h"

using namespace UE::Geometry;
using namespace RealtimeMesh;

struct FRealtimeMeshDynamicMeshConversionVertex
{
	uint32 VertID;
	uint32 TangentXID;
	uint32 TangentYID;
	uint32 TangentZID;
	uint32 ColorID;
	TArray<uint32, TFixedAllocator<REALTIME_MESH_MAX_TEX_COORDS>> UVIDs;

	
	FRealtimeMeshDynamicMeshConversionVertex(int32 InVertID, int32 InNormID, int32 InTanXId, int32 InTanYID, int32 InColorID,
		const TArray<uint32, TFixedAllocator<REALTIME_MESH_MAX_TEX_COORDS>>& InUVIDs)
		: VertID(InVertID), TangentXID(InTanXId), TangentYID(InTanYID), TangentZID(InNormID), ColorID(InColorID), UVIDs(InUVIDs)
	{
		
	}

	bool operator==(const FRealtimeMeshDynamicMeshConversionVertex& Other) const
	{
		return VertID == Other.VertID && TangentXID == Other.TangentXID && TangentYID == Other.TangentYID &&
			TangentZID == Other.TangentZID && ColorID == Other.ColorID && UVIDs == Other.UVIDs;
	}

	bool operator!=(const FRealtimeMeshDynamicMeshConversionVertex& Other) const
	{
		return !(*this == Other);
	}

	friend uint32 GetTypeHash(const FRealtimeMeshDynamicMeshConversionVertex& Vertex)
	{
		uint32 Hash = GetTypeHash(Vertex.VertID);
		Hash = HashCombine(Hash, GetTypeHash(Vertex.TangentXID));
		Hash = HashCombine(Hash, GetTypeHash(Vertex.TangentYID));
		Hash = HashCombine(Hash, GetTypeHash(Vertex.TangentZID));
		Hash = HashCombine(Hash, GetTypeHash(Vertex.ColorID));
		for (uint32 UV : Vertex.UVIDs)
		{
			Hash = HashCombine(Hash, GetTypeHash(UV));
		}
		return Hash;
	}	
};

bool URealtimeMeshDynamicMeshConverter::CopyStreamSetToDynamicMesh(const FRealtimeMeshStreamSet& InStreamSet, FDynamicMesh3& OutDynamicMesh, FStreamSetDynamicMeshConversionOptions Options)
{
	// Do we have the minimum streams required?
	if (!ensure(InStreamSet.Contains(FRealtimeMeshStreams::Position) && InStreamSet.Contains(FRealtimeMeshStreams::Triangles)))
	{
		return false;
	}
	
	OutDynamicMesh = FDynamicMesh3();
	OutDynamicMesh.EnableTriangleGroups();
	if (Options.bWantTangents || Options.bWantUVs || Options.bWantVertexColors || Options.bWantMaterialIDs)
	{
		OutDynamicMesh.EnableAttributes();
	}

	// Copy vertices. LODMesh is dense so this should be 1-1
	const TRealtimeMeshStreamBuilder<const FVector3f> PositionData(InStreamSet.FindChecked(FRealtimeMeshStreams::Position));
	const int32 VertexCount = PositionData.Num();
	for (int32 VertID = 0; VertID < VertexCount; VertID++)
	{
		const FVector3d Position = FVector3d(PositionData.GetValue(VertID));		
		const int32 NewVertID = OutDynamicMesh.AppendVertex(Position);
		if (NewVertID != VertID)
		{
			OutDynamicMesh.Clear();
			ensure(false);
			return false;
		}
	}
	
	// Copy triangles. LODMesh is dense so this should be 1-1 unless there is a duplicate tri or non-manifold edge (currently aborting in that case)	
	const TRealtimeMeshStreamBuilder<const TIndex3<int32>, void> TriangleData(InStreamSet.FindChecked(FRealtimeMeshStreams::Triangles));
	const int32 TriangleCount = TriangleData.Num();
	for (int32 TriID = 0; TriID < TriangleCount; TriID++)
	{
		const TIndex3<int32> Tri = TriangleData.GetValue(TriID);
		const int32 NewTriID = OutDynamicMesh.AppendTriangle(Tri.V0, Tri.V1, Tri.V2);
		if (NewTriID != TriID)
		{
			OutDynamicMesh.Clear();
			ensure(false);
			return false;
		}
	}

	// Transfer polygroups
	if (const auto* PolyGroupStream = InStreamSet.Find(FRealtimeMeshStreams::PolyGroups))
	{
		const TRealtimeMeshStreamBuilder<const int32, void> PolyGroupData(*PolyGroupStream);
		for (int32 TriID = 0; TriID < TriangleCount; TriID++)
		{
			OutDynamicMesh.SetTriangleGroup(TriID, PolyGroupData.GetValue(TriID));
		}
	}

	// copy overlay normals
	const auto* TangentsStream = InStreamSet.Find(FRealtimeMeshStreams::Tangents);
	if (TangentsStream && Options.bWantNormals)
	{
		const TRealtimeMeshStreamBuilder<const TRealtimeMeshTangents<FVector4f>, void> TangentsData(*TangentsStream);		
		FDynamicMeshNormalOverlay* Normals = OutDynamicMesh.Attributes()->PrimaryNormals();
		if (Normals != nullptr)
		{
			for (int32 VertID = 0; VertID < VertexCount; VertID++)
			{
				const auto Tangents = TangentsData.GetValue(VertID);
				const int32 ElemID = Normals->AppendElement(Tangents.GetNormal());
				check(ElemID == VertID);
			}

			for (int32 TriID = 0; TriID < TriangleCount; ++TriID)
			{
				const TIndex3<int32> Tri = TriangleData.GetValue(TriID);
				Normals->SetTriangle(TriID, FIndex3i(Tri.V0, Tri.V1, Tri.V2));
			}
		}		
	}

	// copy overlay tangents
	if (TangentsStream && Options.bWantTangents)
	{
		const TRealtimeMeshStreamBuilder<const TRealtimeMeshTangents<FVector4f>, void> TangentsData(*TangentsStream);
		if (FDynamicMeshNormalOverlay* TangentsX = OutDynamicMesh.Attributes()->PrimaryTangents())
		{
			for (int32 VertID = 0; VertID < VertexCount; VertID++)
			{
				const auto Tangents = TangentsData.GetValue(VertID);
				const int32 ElemID = TangentsX->AppendElement(Tangents.GetTangent());
				check(ElemID == VertID);
			}

			for (int32 TriID = 0; TriID < TriangleCount; ++TriID)
			{
				const TIndex3<int32> Tri = TriangleData.GetValue(TriID);
				TangentsX->SetTriangle(TriID, FIndex3i(Tri.V0, Tri.V1, Tri.V2));
			}
		}		
		if (FDynamicMeshNormalOverlay* TangentsY = OutDynamicMesh.Attributes()->PrimaryBiTangents())
		{
			for (int32 VertID = 0; VertID < VertexCount; VertID++)
			{
				const auto Tangents = TangentsData.GetValue(VertID);
				const int32 ElemID = TangentsY->AppendElement(Tangents.GetBinormal());
				check(ElemID == VertID);
			}

			for (int32 TriID = 0; TriID < TriangleCount; ++TriID)
			{
				const TIndex3<int32> Tri = TriangleData.GetValue(TriID);
				TangentsY->SetTriangle(TriID, FIndex3i(Tri.V0, Tri.V1, Tri.V2));
			}
		}	
	}

	
	
	// copy UV layers
	const auto* TexCoordsStream = InStreamSet.Find(FRealtimeMeshStreams::TexCoords);
	if (TexCoordsStream && Options.bWantUVs)
	{
		int32 NumUVLayers = TexCoordsStream->GetNumElements();
		if (NumUVLayers > 0)
		{
			OutDynamicMesh.Attributes()->SetNumUVLayers(NumUVLayers);
			for (int32 UVLayerIndex = 0; UVLayerIndex < NumUVLayers; ++UVLayerIndex)
			{
				const TRealtimeMeshStridedStreamBuilder<const FVector2f, void> TexCoordsData(*TexCoordsStream, UVLayerIndex);				
				FDynamicMeshUVOverlay* UVOverlay = OutDynamicMesh.Attributes()->GetUVLayer(UVLayerIndex);
				
				for (int32 VertID = 0; VertID < VertexCount; VertID++)
				{
					const FVector2f UV = TexCoordsData.GetValue(VertID);
					const int32 ElemID = UVOverlay->AppendElement(UV);
					check(ElemID == VertID);
				}

				for (int32 TriID = 0; TriID < TriangleCount; ++TriID)
				{
					const TIndex3<int32> Tri = TriangleData.GetValue(TriID);
					UVOverlay->SetTriangle(TriID, FIndex3i(Tri.V0, Tri.V1, Tri.V2));
				}
			}
		}
	}
	
	// copy overlay colors
	const auto* ColorStream = InStreamSet.Find(FRealtimeMeshStreams::Color);
	if (ColorStream && Options.bWantVertexColors)
	{
		OutDynamicMesh.Attributes()->EnablePrimaryColors();
		FDynamicMeshColorOverlay* Colors = OutDynamicMesh.Attributes()->PrimaryColors();

		const TRealtimeMeshStreamBuilder<const FColor> ColorData(*ColorStream);
				
		for (int32 VertID = 0; VertID < VertexCount; VertID++)
		{
			const FColor Color = ColorData.GetValue(VertID);
			const int32 ElemID = Colors->AppendElement(Color.ReinterpretAsLinear());
			check(ElemID == VertID);
		}

		for (int32 TriID = 0; TriID < TriangleCount; ++TriID)
		{
			const TIndex3<int32> Tri = TriangleData.GetValue(TriID);
			Colors->SetTriangle(TriID, FIndex3i(Tri.V0, Tri.V1, Tri.V2));
		}
	}

	return true;
}

bool URealtimeMeshDynamicMeshConverter::CopyStreamSetFromDynamicMesh(const FDynamicMesh3& InDynamicMesh, FRealtimeMeshStreamSet& OutStreamSet, FStreamSetDynamicMeshConversionOptions Options)
{
	OutStreamSet = FRealtimeMeshStreamSet();
	
	TRealtimeMeshStreamBuilder<FVector3f> PositionData(OutStreamSet.AddStream(FRealtimeMeshStreams::Position, GetRealtimeMeshBufferLayout<FVector3f>()));
	TRealtimeMeshStreamBuilder<TIndex3<uint32>> TriangleData(OutStreamSet.AddStream(FRealtimeMeshStreams::Triangles,
		GetRealtimeMeshBufferLayout<TIndex3<uint32>>()));

	const FDynamicMeshNormalOverlay* Normals = InDynamicMesh.Attributes()->PrimaryNormals();
	const FDynamicMeshNormalOverlay* TangentsX = InDynamicMesh.Attributes()->PrimaryTangents();
	const FDynamicMeshNormalOverlay* TangentsY = InDynamicMesh.Attributes()->PrimaryBiTangents();
	const FDynamicMeshColorOverlay* Colors = InDynamicMesh.Attributes()->PrimaryColors();
	TArray<const FDynamicMeshUVOverlay*> UVOverlays;
	UVOverlays.SetNum(InDynamicMesh.Attributes()->NumUVLayers());
	
	for (int32 UVLayerIndex = 0; UVLayerIndex < UVOverlays.Num(); UVLayerIndex++)
	{
		UVOverlays[UVLayerIndex] = InDynamicMesh.Attributes()->GetUVLayer(UVLayerIndex);
	}
	
	const bool bCopyNormals = Options.bWantNormals && Normals != nullptr;
	const bool bCopyTangentsX = bCopyNormals && Options.bWantTangents && TangentsX != nullptr;
	const bool bCopyTangentsY = bCopyTangentsX && Options.bWantTangents && TangentsY != nullptr;
	const bool bCopyUVs = Options.bWantUVs && UVOverlays.ContainsByPredicate([](const FDynamicMeshUVOverlay* Overlay) { return Overlay != nullptr; });
	const bool bCopyColors = Options.bWantVertexColors && Colors != nullptr;

	TArray<FRealtimeMeshDynamicMeshConversionVertex> VertexList;
	TMap<FRealtimeMeshDynamicMeshConversionVertex, int32> VertexMap;

	const auto GetNewIndexForVertex = [&](const FRealtimeMeshDynamicMeshConversionVertex& VertexKey) -> int32
	{
		if (const int32* Found = VertexMap.Find(VertexKey))
		{
			return *Found;
		}

		const int32 NewIndex = VertexList.Add(VertexKey);
		VertexMap.Add(VertexKey, NewIndex);
		return NewIndex;
	};
		
	TArray<uint32, TFixedAllocator<REALTIME_MESH_MAX_TEX_COORDS>> Vert0UVs;
	TArray<uint32, TFixedAllocator<REALTIME_MESH_MAX_TEX_COORDS>> Vert1UVs;
	TArray<uint32, TFixedAllocator<REALTIME_MESH_MAX_TEX_COORDS>> Vert2UVs;		
	Vert0UVs.SetNum(UVOverlays.Num());
	Vert1UVs.SetNum(UVOverlays.Num());
	Vert2UVs.SetNum(UVOverlays.Num());
	
	const int32 TriangleCount = InDynamicMesh.TriangleCount();
	TriangleData.SetNumUninitialized(TriangleCount);
	for (int32 TriID = 0; TriID < TriangleCount; TriID++)
	{
		const FIndex3i Triangle = InDynamicMesh.GetTriangle(TriID);
		const FIndex3i NormalTriangle = bCopyNormals ? Normals->GetTriangle(TriID) : FIndex3i::Zero();
		const FIndex3i TangentXTriangle = bCopyTangentsX ? TangentsX->GetTriangle(TriID) : FIndex3i::Zero();
		const FIndex3i TangentYTriangle = bCopyTangentsY ? TangentsY->GetTriangle(TriID) : FIndex3i::Zero();
		const FIndex3i ColorTriangle = bCopyColors ? Colors->GetTriangle(TriID) : FIndex3i::Zero();
		if (bCopyUVs)
		{
			for (int32 UVLayerIndex = 0; UVLayerIndex < UVOverlays.Num(); UVLayerIndex++)
			{
				FIndex3i UVTriangle = bCopyUVs ? UVOverlays[UVLayerIndex]->GetTriangle(TriID) : FIndex3i::Zero();
				Vert0UVs[UVLayerIndex] = UVTriangle.A;
				Vert1UVs[UVLayerIndex] = UVTriangle.B;
				Vert2UVs[UVLayerIndex] = UVTriangle.C;
			}	
		}
		
		FRealtimeMeshDynamicMeshConversionVertex Vert0Key(Triangle.A, NormalTriangle.A, TangentXTriangle.A, TangentYTriangle.A, ColorTriangle.A, Vert0UVs);
		const int32 NewV0 = GetNewIndexForVertex(Vert0Key);

		FRealtimeMeshDynamicMeshConversionVertex Vert1Key(Triangle.B, NormalTriangle.B, TangentXTriangle.B, TangentYTriangle.B, ColorTriangle.B, Vert1UVs);
		const int32 NewV1 = GetNewIndexForVertex(Vert1Key);

		FRealtimeMeshDynamicMeshConversionVertex Vert2Key(Triangle.C, NormalTriangle.C, TangentXTriangle.C, TangentYTriangle.C, ColorTriangle.C, Vert2UVs);
		const int32 NewV2 = GetNewIndexForVertex(Vert2Key);

		TriangleData.Set(TriID, TIndex3<uint32>(NewV0, NewV1, NewV2));
	}

	// Copy vertex positions
	PositionData.SetNumUninitialized(VertexList.Num());
	for (int32 VertID = 0; VertID < VertexList.Num(); VertID++)
	{
		const FRealtimeMeshDynamicMeshConversionVertex& Vertex = VertexList[VertID];
		PositionData.Set(VertID, FVector3f(InDynamicMesh.GetVertex(Vertex.VertID)));
	}

	if (bCopyNormals)
	{
		TRealtimeMeshStreamBuilder<TRealtimeMeshTangents<FVector4f>, TRealtimeMeshTangents<FPackedNormal>> TangentData(OutStreamSet.AddStream(FRealtimeMeshStreams::Tangents,
		GetRealtimeMeshBufferLayout<TRealtimeMeshTangents<FPackedNormal>>()));
		TangentData.SetNumUninitialized(VertexList.Num());
		
		for (int32 VertID = 0; VertID < VertexList.Num(); VertID++)
		{
			const FRealtimeMeshDynamicMeshConversionVertex& Vertex = VertexList[VertID];
			TRealtimeMeshTangents<FVector4f> NewTangents;
			if (bCopyTangentsY)
			{
				NewTangents.SetTangents(TangentsX->GetElement(Vertex.TangentXID), TangentsY->GetElement(Vertex.TangentYID), Normals->GetElement(Vertex.TangentZID));				
			}
			else if (bCopyTangentsX)
			{				
				NewTangents.SetNormalAndTangent(Normals->GetElement(Vertex.TangentZID), TangentsX->GetElement(Vertex.TangentXID));
			}
			else
			{
				NewTangents.SetNormal(Normals->GetElement(Vertex.TangentZID));
			}
			
			TangentData.Set(VertID, NewTangents);		
		}	
	}

	if (bCopyColors)
	{
		TRealtimeMeshStreamBuilder<FColor> ColorData(OutStreamSet.AddStream(FRealtimeMeshStreams::Color,
		GetRealtimeMeshBufferLayout<FColor>()));
		ColorData.SetNumUninitialized(VertexList.Num());

		for (int32 VertID = 0; VertID < VertexList.Num(); VertID++)
		{
			const FRealtimeMeshDynamicMeshConversionVertex& Vertex = VertexList[VertID];
			ColorData.Set(VertID, FLinearColor(Colors->GetElement(Vertex.ColorID)).QuantizeRound());
		}			
	}
		
	if (bCopyUVs)
	{
		FRealtimeMeshStream& TexCoordStream = OutStreamSet.AddStream(FRealtimeMeshStreams::TexCoords, GetRealtimeMeshBufferLayout<FVector2f>(UVOverlays.Num()));
		TexCoordStream.SetNumUninitialized(VertexList.Num());

		for (int32 UVLayerIndex = 0; UVLayerIndex < UVOverlays.Num(); UVLayerIndex++)
		{
			TRealtimeMeshStridedStreamBuilder<FVector2f> TexCoordData(TexCoordStream, UVLayerIndex);
			const FDynamicMeshUVOverlay* UVOverlay = UVOverlays[UVLayerIndex];
				
			for (int32 VertID = 0; VertID < VertexList.Num(); VertID++)
			{
				const FRealtimeMeshDynamicMeshConversionVertex& Vertex = VertexList[VertID];
				TexCoordData.Set(VertID, UVOverlay->GetElement(Vertex.UVIDs[UVLayerIndex]));
			}	
		}		
	}

	if (Options.bWantMaterialIDs && InDynamicMesh.HasTriangleGroups())
	{
		TRealtimeMeshStreamBuilder<uint16> PolyGroupData(OutStreamSet.AddStream(FRealtimeMeshStreams::PolyGroups,
		GetRealtimeMeshBufferLayout<uint16>()));
		PolyGroupData.SetNumUninitialized(TriangleCount);
			
		for (int32 TriID = 0; TriID < TriangleCount; TriID++)
		{
			PolyGroupData.Set(TriID, InDynamicMesh.GetTriangleGroup(TriID));
		}
	}	
	
	return true;
}



URealtimeMeshStreamSet* URealtimeMeshDynamicMeshConverter::CopyStreamSetFromDynamicMesh(UDynamicMesh* FromDynamicMesh, URealtimeMeshStreamSet* ToStreamSet,
	FStreamSetDynamicMeshConversionOptions Options, ERealtimeMeshOutcomePins& Outcome)
{
	if (FromDynamicMesh == nullptr)
	{
		UE_LOG(RealtimeMeshLog, Warning, TEXT("RealtimeMeshWarning: CopyFromDynamicMesh failed: FromDynamicMesh is null"));
		Outcome = ERealtimeMeshOutcomePins::Failure;
		return ToStreamSet;
	}

	if (ToStreamSet == nullptr)
	{
		UE_LOG(RealtimeMeshLog, Warning, TEXT("RealtimeMeshWarning: CopyFromDynamicMesh failed: ToStreamSet is null"));
		Outcome = ERealtimeMeshOutcomePins::Failure;
		return ToStreamSet;
	}
	
	bool bSuccess = false;
	FromDynamicMesh->ProcessMesh([&](const FDynamicMesh3& Mesh)
	{
		bSuccess = CopyStreamSetFromDynamicMesh(Mesh, ToStreamSet->GetStreamSet(), Options);		
	});

	Outcome = bSuccess? ERealtimeMeshOutcomePins::Success : ERealtimeMeshOutcomePins::Failure;
	return ToStreamSet;
}

UDynamicMesh* URealtimeMeshDynamicMeshConverter::CopyStreamSetToDynamicMesh(URealtimeMeshStreamSet* FromStreamSet, UDynamicMesh* ToDynamicMesh,
	FStreamSetDynamicMeshConversionOptions Options, ERealtimeMeshOutcomePins& Outcome)
{
	if (FromStreamSet == nullptr)
	{
		UE_LOG(RealtimeMeshLog, Warning, TEXT("RealtimeMeshWarning: CopyToDynamicMesh failed: FromStreamSet is null"));
		Outcome = ERealtimeMeshOutcomePins::Failure;
		return ToDynamicMesh;
	}

	if (ToDynamicMesh == nullptr)
	{
		UE_LOG(RealtimeMeshLog, Warning, TEXT("RealtimeMeshWarning: CopyToDynamicMesh failed: ToDynamicMesh is null"));
		Outcome = ERealtimeMeshOutcomePins::Failure;
		return ToDynamicMesh;
	}

	bool bSuccess = false;
	ToDynamicMesh->EditMesh([&](FDynamicMesh3& Mesh)
	{
		bSuccess = CopyStreamSetToDynamicMesh(FromStreamSet->GetStreamSet(), Mesh, Options);
	});

	Outcome = bSuccess? ERealtimeMeshOutcomePins::Success : ERealtimeMeshOutcomePins::Failure;
	return ToDynamicMesh;
}

URealtimeMeshSimple* URealtimeMeshDynamicMeshConverter::CopyRealtimeMeshFromDynamicMesh(UDynamicMesh* FromDynamicMesh, URealtimeMeshSimple* ToRealtimeMesh,
	FRealtimeMeshDynamicMeshConversionOptions Options, ERealtimeMeshOutcomePins& Outcome)
{
	check(false);
	Outcome = ERealtimeMeshOutcomePins::Success;
	return ToRealtimeMesh;
}

UDynamicMesh* URealtimeMeshDynamicMeshConverter::CopyRealtimeMeshToDynamicMesh(URealtimeMeshSimple* FromRealtimeMesh, UDynamicMesh* ToDynamicMesh,
	FRealtimeMeshDynamicMeshConversionOptions Options, ERealtimeMeshOutcomePins& Outcome)
{
	if (FromRealtimeMesh == nullptr)
	{
		UE_LOG(RealtimeMeshLog, Warning, TEXT("RealtimeMeshWarning: CopyToDynamicMesh failed: FromRealtimeMesh is null"));
		Outcome = ERealtimeMeshOutcomePins::Failure;
		return ToDynamicMesh;
	}

	if (ToDynamicMesh == nullptr)
	{
		UE_LOG(RealtimeMeshLog, Warning, TEXT("RealtimeMeshWarning: CopyToDynamicMesh failed: ToDynamicMesh is null"));
		Outcome = ERealtimeMeshOutcomePins::Failure;
		return ToDynamicMesh;
	}
	
	bool bSuccess = false;
	FromRealtimeMesh->ProcessMesh(Options.SectionGroup, [&](const FRealtimeMeshStreamSet& StreamSet)
	{
		ToDynamicMesh->EditMesh([&](FDynamicMesh3& OutMesh)
		{
			FStreamSetDynamicMeshConversionOptions ConversionOptions;
			ConversionOptions.bWantNormals = Options.bWantNormals;
			ConversionOptions.bWantTangents = Options.bWantTangents;
			ConversionOptions.bWantUVs = Options.bWantUVs;
			ConversionOptions.bWantVertexColors = Options.bWantVertexColors;
			ConversionOptions.bWantMaterialIDs = Options.bWantPolyGroups;			
			
			bSuccess = CopyStreamSetToDynamicMesh(StreamSet, OutMesh, ConversionOptions);
		});
	});
	
	Outcome = ERealtimeMeshOutcomePins::Success;
	return ToDynamicMesh;
}
