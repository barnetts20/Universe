// Copyright TriAxis Games, L.L.C. All Rights Reserved.

#include "RenderProxy/RealtimeMeshComponentProxy.h"
#include "RenderProxy/RealtimeMeshProxy.h"
#include "RealtimeMeshComponent.h"
#include "Materials/Material.h"
#include "PhysicsEngine/BodySetup.h"
#include "PrimitiveSceneProxy.h"
#include "UnrealEngine.h"
#include "SceneManagement.h"
#include "RayTracingInstance.h"
#include "RenderProxy/RealtimeMeshLODProxy.h"
#if RMC_ENGINE_ABOVE_5_2
#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"
#include "SceneInterface.h"
#endif

DECLARE_CYCLE_STAT(TEXT("RealtimeMeshComponentSceneProxy - Create Mesh Batch"), STAT_RealtimeMeshComponentSceneProxy_CreateMeshBatch, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshComponentSceneProxy - Get Dynamic Mesh Elements"), STAT_RealtimeMeshComponentSceneProxy_GetDynamicMeshElements, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshComponentSceneProxy - Draw Static Mesh Elements"), STAT_RealtimeMeshComponentSceneProxy_DrawStaticMeshElements, STATGROUP_RealtimeMesh);
DECLARE_CYCLE_STAT(TEXT("RealtimeMeshComponentSceneProxy - Get Dynamic Ray Tracing Instances"), STAT_RealtimeMeshComponentSceneProxy_GetDynamicRayTracingInstances,
                   STATGROUP_RealtimeMesh);

#define RMC_LOG_VERBOSE(Format, ...) \
	//UE_LOG(RealtimeMeshLog, Verbose, TEXT("[RMCSP:%d Mesh:%d Thread:%d]: " Format), GetUniqueID(), (RealtimeMeshProxy? RealtimeMeshProxy->GetMeshID() : -1), FPlatformTLS::GetCurrentThreadId(), ##__VA_ARGS__);

namespace RealtimeMesh
{
	FRealtimeMeshComponentSceneProxy::FRealtimeMeshComponentSceneProxy(URealtimeMeshComponent* Component, const FRealtimeMeshProxyRef& InRealtimeMeshProxy)
		: FPrimitiveSceneProxy(Component)
		  , RealtimeMeshProxy(InRealtimeMeshProxy)
		  , BodySetup(Component->GetBodySetup())
		  , bAnyMaterialUsesDithering(false)
	{
		check(Component->GetRealtimeMesh() != nullptr);

		for (int32 MaterialIndex = 0; MaterialIndex < Component->GetNumMaterials(); MaterialIndex++)
		{
			UMaterialInterface* Mat = Component->GetMaterial(MaterialIndex);
			if (Mat == nullptr)
			{
				Mat = UMaterial::GetDefaultMaterial(MD_Surface);
			}
			Materials.Add(MaterialIndex, MakeTuple(Mat->GetRenderProxy(), Mat->IsDitheredLODTransition()));
			MaterialRelevance |= Mat->GetRelevance_Concurrent(GetScene().GetFeatureLevel());
			bAnyMaterialUsesDithering = Mat->IsDitheredLODTransition();
		}

		// Disable shadow casting if no section has it enabled.
		bCastDynamicShadow = true;
		bCastStaticShadow = true;

		const auto FeatureLevel = GetScene().GetFeatureLevel();

		// We always use local vertex factory, which gets its primitive data from GPUScene, so we can skip expensive primitive uniform buffer updates
		bVFRequiresPrimitiveUniformBuffer = !UseGPUScene(GMaxRHIShaderPlatform, FeatureLevel);
		bStaticElementsAlwaysUseProxyPrimitiveUniformBuffer = true;
		bVerifyUsedMaterials = false;
		
		bSupportsDistanceFieldRepresentation = MaterialRelevance.bOpaque && !MaterialRelevance.bUsesSingleLayerWaterMaterial && RealtimeMeshProxy->HasDistanceFieldData();
		
		//bCastsDynamicIndirectShadow = Component->bCastDynamicShadow && Component->CastShadow && Component->Mobility != EComponentMobility::Static;
		//DynamicIndirectShadowMinVisibility = 0.1f;


#if RHI_RAYTRACING
		bSupportsRayTracing = true; //InRealtimeMeshProxy->HasRayTracingGeometry();
		//bDynamicRayTracingGeometry = false;

#if RMC_ENGINE_BELOW_5_4
#if RMC_ENGINE_ABOVE_5_2
		if (IsRayTracingAllowed() && bSupportsRayTracing)
#elif RMC_ENGINE_ABOVE_5_1
		if (IsRayTracingEnabled(GetScene().GetShaderPlatform()) && bSupportsRayTracing)
#else
		if (IsRayTracingEnabled() && bSupportsRayTracing)		
#endif
		{			
			RayTracingGeometries.SetNum(InRealtimeMeshProxy->GetNumLODs());
			for (int32 LODIndex = 0; LODIndex < RealtimeMeshProxy->GetNumLODs(); LODIndex++)
			{
				if (FRayTracingGeometry* RayTracingGeo = RealtimeMeshProxy->GetLOD(LODIndex)->GetStaticRayTracingGeometry())
				{
					RayTracingGeometries[LODIndex] = RayTracingGeo;
				}
			}
			
			/*
			const bool bWantsRayTracingWPO = MaterialRelevance.bUsesWorldPositionOffset && InComponent->bEvaluateWorldPositionOffsetInRayTracing;
			
			// r.RayTracing.Geometry.StaticMeshes.WPO is handled in the following way:
			// 0 - mark ray tracing geometry as dynamic but don't create any dynamic geometries since it won't be included in ray tracing scene
			// 1 - mark ray tracing geometry as dynamic and create dynamic geometries
			// 2 - don't mark ray tracing geometry as dynamic nor create any dynamic geometries since WPO evaluation is disabled

			// if r.RayTracing.Geometry.StaticMeshes.WPO == 2, WPO evaluation is disabled so don't need to mark geometry as dynamic
			if (bWantsRayTracingWPO && CVarRayTracingStaticMeshesWPO.GetValueOnAnyThread() != 2)
			{
				bDynamicRayTracingGeometry = true;

				// only need dynamic geometries when r.RayTracing.Geometry.StaticMeshes.WPO == 1
				bNeedsDynamicRayTracingGeometries = CVarRayTracingStaticMeshesWPO.GetValueOnAnyThread() == 1;
			}
			*/
		}
#endif
#endif

#if RMC_ENGINE_ABOVE_5_2
		if (MaterialRelevance.bOpaque && !MaterialRelevance.bUsesSingleLayerWaterMaterial)
		{
			UpdateVisibleInLumenScene();
		}
#endif
		
	}

	FRealtimeMeshComponentSceneProxy::~FRealtimeMeshComponentSceneProxy()
	{
		check(true);
	}

	bool FRealtimeMeshComponentSceneProxy::CanBeOccluded() const
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

	SIZE_T FRealtimeMeshComponentSceneProxy::GetTypeHash() const
	{
		static size_t UniquePointer;
		return reinterpret_cast<size_t>(&UniquePointer);
	}

	FPrimitiveViewRelevance FRealtimeMeshComponentSceneProxy::GetViewRelevance(const FSceneView* View) const
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);

		const bool bForceDynamicPath = IsRichView(*View->Family) || IsSelected() || View->Family->EngineShowFlags.Wireframe;

		Result.bStaticRelevance = !bForceDynamicPath && RealtimeMeshProxy->GetDrawMask().IsSet(ERealtimeMeshDrawMask::DrawStatic);
		Result.bDynamicRelevance = bForceDynamicPath || RealtimeMeshProxy->GetDrawMask().IsSet(ERealtimeMeshDrawMask::DrawDynamic);

		Result.bRenderInMainPass = ShouldRenderInMainPass();
		Result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
		Result.bRenderCustomDepth = ShouldRenderCustomDepth();
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		Result.bTranslucentSelfShadow = bCastVolumetricTranslucentShadow;
		Result.bVelocityRelevance = IsMovable() && Result.bOpaque && Result.bRenderInMainPass;
		return Result;
	}

	void FRealtimeMeshComponentSceneProxy::DrawStaticElements(FStaticPrimitiveDrawInterface* PDI)
	{
		SCOPE_CYCLE_COUNTER(STAT_RealtimeMeshComponentSceneProxy_DrawStaticMeshElements);

		const auto ValidLODRange = RealtimeMeshProxy->GetValidLODRange();


		if (!ValidLODRange.IsEmpty())
		{
			FLODMask LODMask;

			for (uint8 LODIndex = ValidLODRange.GetLowerBoundValue(); LODIndex <= ValidLODRange.GetUpperBoundValue(); LODIndex++)
			{
				const auto& LOD = RealtimeMeshProxy->GetLOD(FRealtimeMeshLODKey(LODIndex));

				if (LOD.IsValid() && LOD->GetDrawMask().IsSet(ERealtimeMeshDrawMask::DrawStatic))
				{
					LODMask.SetLOD(LODIndex);

					const auto LODScreenSizes = RealtimeMeshProxy->GetScreenSizeRangeForLOD(LODIndex);

					FMeshBatch MeshBatch;
					FRealtimeMeshBatchCreationParams Params
					{
						[this](const TSharedRef<FRenderResource>& Resource) { InUseBuffers.Add(Resource); },
						[&MeshBatch]()-> FMeshBatch& {
							MeshBatch = FMeshBatch();
							return MeshBatch;
						},
#if RHI_RAYTRACING
						[&PDI](const FMeshBatch& Batch, float MinScreenSize, const FRayTracingGeometry*) { PDI->DrawMesh(Batch, MinScreenSize); },
#else
							[&PDI](const FMeshBatch& Batch, float MinScreenSize) { PDI->DrawMesh(Batch, MinScreenSize); },					
#endif
						GetUniformBuffer(),
						LODScreenSizes,
						LODMask,
						IsMovable(),
						IsLocalToWorldDeterminantNegative(),
						bCastDynamicShadow
					};

					RealtimeMeshProxy->CreateMeshBatches(LODIndex, Params, Materials, nullptr, ERealtimeMeshSectionDrawType::Static, ERealtimeMeshBatchCreationFlags::None);
				}
			}
		}
	}

	void FRealtimeMeshComponentSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap,
	                                                              FMeshElementCollector& Collector) const
	{
		SCOPE_CYCLE_COUNTER(STAT_RealtimeMeshComponentSceneProxy_GetDynamicMeshElements);

		// Set up wireframe material (if needed)
		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy* WireframeMaterialInstance = nullptr;
		if (GEngine && GEngine->WireframeMaterial && bWireframe)
		{
			WireframeMaterialInstance = new FColoredMaterialRenderProxy(GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : nullptr,
			                                                            FLinearColor(0.0f, 0.16f, 1.0f));
			if (WireframeMaterialInstance) Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);
		}

		const TRange<int8> ValidLODRange = RealtimeMeshProxy->GetValidLODRange();


		check(!RealtimeMeshProxy->GetDrawMask().HasAnyFlags() || !ValidLODRange.IsEmpty());


		check(ValidLODRange.IsEmpty() || (ValidLODRange.GetLowerBound().IsInclusive() && ValidLODRange.GetUpperBound().IsInclusive()));

		if (!ValidLODRange.IsEmpty())
		{
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				const FSceneView* View = Views[ViewIndex];
				const bool bForceDynamicPath = IsRichView(*Views[ViewIndex]->Family) || bWireframe || IsSelected();

				if (IsShown(View) && (VisibilityMap & (1 << ViewIndex)))
				{
					FFrozenSceneViewMatricesGuard FrozenMatricesGuard(*const_cast<FSceneView*>(Views[ViewIndex]));

					FLODMask LODMask = GetLODMask(View);

					for (uint8 LODIndex = ValidLODRange.GetLowerBoundValue(); LODIndex <= ValidLODRange.GetUpperBoundValue(); LODIndex++)
					{
						if (LODMask.ContainsLOD(LODIndex))
						{
							const auto& LOD = RealtimeMeshProxy->GetLOD(FRealtimeMeshLODKey(LODIndex));


							if ((LOD.IsValid() && LOD->GetDrawMask().IsSet(ERealtimeMeshDrawMask::DrawDynamic)) ||
								(bForceDynamicPath && LOD->GetDrawMask().IsSet(ERealtimeMeshDrawMask::DrawStatic)))
							{
								const auto LODScreenSizes = RealtimeMeshProxy->GetScreenSizeRangeForLOD(LODIndex);

								FRealtimeMeshBatchCreationParams Params
								{
									[](const TSharedRef<FRenderResource>&)
									{
									},
									[&Collector]()-> FMeshBatch& { return Collector.AllocateMesh(); },
	#if RHI_RAYTRACING
									[&Collector, ViewIndex](FMeshBatch& Batch, float, const FRayTracingGeometry*) { Collector.AddMesh(ViewIndex, Batch); },
	#else
									[&Collector, ViewIndex](FMeshBatch& Batch, float) { Collector.AddMesh(ViewIndex, Batch); },
	#endif
									GetUniformBuffer(),
									LODScreenSizes,
									LODMask,
									IsMovable(),
									IsLocalToWorldDeterminantNegative(),
									bCastDynamicShadow
								};

								RealtimeMeshProxy->CreateMeshBatches(LODIndex, Params, Materials, WireframeMaterialInstance, ERealtimeMeshSectionDrawType::Dynamic,
									bForceDynamicPath? ERealtimeMeshBatchCreationFlags::ForceAllDynamic : ERealtimeMeshBatchCreationFlags::None);
							}
						}
					}
				}
			}


			// Draw bounds
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					// Draw simple collision as wireframe if 'show collision', and collision is enabled, and we are not using the complex as the simple
					if (ViewFamily.EngineShowFlags.Collision && IsCollisionEnabled() && BodySetup && BodySetup->GetCollisionTraceFlag() !=
						ECollisionTraceFlag::CTF_UseComplexAsSimple)
					{
						FTransform GeomTransform(GetLocalToWorld());
						BodySetup->AggGeom.GetAggGeom(GeomTransform, GetSelectionColor(FColor(157, 149, 223, 255), IsSelected(), IsHovered()).ToFColor(true), NULL, false, false,
						                              DrawsVelocity(), ViewIndex, Collector);
					}

					// Render bounds
					RenderBounds(Collector.GetPDI(ViewIndex), ViewFamily.EngineShowFlags, GetBounds(), IsSelected());
				}
			}
#endif
		}
	}

	void FRealtimeMeshComponentSceneProxy::GetDistanceFieldAtlasData(const FDistanceFieldVolumeData*& OutDistanceFieldData, float& SelfShadowBias) const
	{
		OutDistanceFieldData = RealtimeMeshProxy->GetDistanceFieldData();
		SelfShadowBias = DistanceFieldSelfShadowBias;
	}

	void FRealtimeMeshComponentSceneProxy::GetDistanceFieldInstanceData(TArray<FRenderTransform>& InstanceLocalToPrimitiveTransforms) const
	{
		check(InstanceLocalToPrimitiveTransforms.IsEmpty());

		if (RealtimeMeshProxy->HasDistanceFieldData())
		{
			InstanceLocalToPrimitiveTransforms.Add(FRenderTransform::Identity);
		}
	}

	bool FRealtimeMeshComponentSceneProxy::HasDistanceFieldRepresentation() const
	{
		return CastsDynamicShadow() && AffectsDistanceFieldLighting() && RealtimeMeshProxy->HasDistanceFieldData();
	}

	bool FRealtimeMeshComponentSceneProxy::HasDynamicIndirectShadowCasterRepresentation() const
	{
		return bCastsDynamicIndirectShadow && HasDistanceFieldRepresentation();
	}

	const FCardRepresentationData* FRealtimeMeshComponentSceneProxy::GetMeshCardRepresentation() const
	{
		return RealtimeMeshProxy->GetCardRepresentation();
	}

	bool FRealtimeMeshComponentSceneProxy::HasRayTracingRepresentation() const
	{
#if RHI_RAYTRACING
		return bSupportsRayTracing;
#else
		return false;
#endif
	}

#if RMC_ENGINE_ABOVE_5_4
	TArray<FRayTracingGeometry*> FRealtimeMeshComponentSceneProxy::GetStaticRayTracingGeometries() const
	{
#if RHI_RAYTRACING
		if (IsRayTracingAllowed() && bSupportsRayTracing)
		{
			TArray<FRayTracingGeometry*> RayTracingGeometries;
			RayTracingGeometries.SetNum(RealtimeMeshProxy->GetNumLODs());
			for (int32 LODIndex = 0; LODIndex < RealtimeMeshProxy->GetNumLODs(); LODIndex++)
			{
				RayTracingGeometries[LODIndex] = RealtimeMeshProxy->GetLOD(LODIndex)->GetStaticRayTracingGeometry();
			}

			return MoveTemp(RayTracingGeometries);
		}
#endif // RHI_RAYTRACING
		return {};
	}
#endif // RMC_ENGINE_ABOVE_5_4

#if RHI_RAYTRACING
	void FRealtimeMeshComponentSceneProxy::GetDynamicRayTracingInstances(struct FRayTracingMaterialGatheringContext& Context,
	                                                                     TArray<struct FRayTracingInstance>& OutRayTracingInstances)
	{
		SCOPE_CYCLE_COUNTER(STAT_RealtimeMeshComponentSceneProxy_GetDynamicRayTracingInstances);

		const uint32 LODIndex = FMath::Max(GetLOD(Context.ReferenceView), (int32)GetCurrentFirstLODIdx_RenderThread());

		if (RealtimeMeshProxy->GetDrawMask().HasAnyFlags())
		{
			if (auto LOD = RealtimeMeshProxy->GetLOD(LODIndex))
			{				
				if (LOD.IsValid() && LOD->GetDrawMask().IsAnySet(ERealtimeMeshDrawMask::DrawDynamic | ERealtimeMeshDrawMask::DrawStatic))
				{
					FLODMask LODMask;
					LODMask.SetLOD(LODIndex);

					const auto LODScreenSizes = RealtimeMeshProxy->GetScreenSizeRangeForLOD(LODIndex);

					TMap<const FRayTracingGeometry*, int32> CurrentRayTracingGeometries;
					
					FMeshBatch MeshBatch;
					FRealtimeMeshBatchCreationParams Params
					{
						[](const TSharedRef<FRenderResource>&)
						{
						},
						[MeshBatch = &MeshBatch]()-> FMeshBatch& {
							*MeshBatch = FMeshBatch();
							return *MeshBatch;
						},
						[&OutRayTracingInstances, &CurrentRayTracingGeometries, LocalToWorld = GetLocalToWorld()](
								FMeshBatch& Batch, float MinScreenSize, const FRayTracingGeometry* RayTracingGeometry)
						{
							if (RayTracingGeometry->IsValid())
							{
								check(RayTracingGeometry->Initializer.TotalPrimitiveCount > 0);
								check(RayTracingGeometry->Initializer.IndexBuffer.IsValid());
								checkf(RayTracingGeometry->RayTracingGeometryRHI, TEXT("Ray tracing instance must have a valid geometry."));
								
								FRayTracingInstance* RayTracingInstance;
								if (const auto* RayTracingInstanceIndex = CurrentRayTracingGeometries.Find(RayTracingGeometry))
								{
									RayTracingInstance = &OutRayTracingInstances[*RayTracingInstanceIndex];
								}
								else
								{
									RayTracingInstance = &OutRayTracingInstances.AddDefaulted_GetRef();
									CurrentRayTracingGeometries.Add(RayTracingGeometry, OutRayTracingInstances.Num() - 1);
									
									RayTracingInstance->Geometry = RayTracingGeometry;
									RayTracingInstance->InstanceTransforms.Add(LocalToWorld);
								}
								Batch.SegmentIndex = RayTracingInstance->Materials.Num();

								RayTracingInstance->Materials.Add(Batch);
							}
						},
						GetUniformBuffer(),
						LODScreenSizes,
						LODMask,
						IsMovable(),
						IsLocalToWorldDeterminantNegative(),
						bCastDynamicShadow
					};

					RealtimeMeshProxy->CreateMeshBatches(LODIndex, Params, Materials, nullptr, ERealtimeMeshSectionDrawType::Dynamic, 
						ERealtimeMeshBatchCreationFlags::ForceAllDynamic | ERealtimeMeshBatchCreationFlags::SkipStaticRayTracedSections);

#if RMC_ENGINE_BELOW_5_2
					for (auto& RayTracingInstance : OutRayTracingInstances)
					{
						RayTracingInstance.BuildInstanceMaskAndFlags(GetScene().GetFeatureLevel());
					}
#endif
				}
			}
		}

		check(true);
	}
#endif // RHI_RAYTRACING

	int8 FRealtimeMeshComponentSceneProxy::GetCurrentFirstLOD() const
	{
		return RealtimeMeshProxy->GetValidLODRange().GetLowerBoundValue();
	}

	int8 FRealtimeMeshComponentSceneProxy::ComputeTemporalStaticMeshLOD(const FVector4& Origin, const float SphereRadius, const FSceneView& View, int32 MinLOD, float FactorScale,
	                                                                    int32 SampleIndex) const
	{
		const int32 NumLODs = REALTIME_MESH_MAX_LODS;

		const float ScreenRadiusSquared = ComputeBoundsScreenRadiusSquared(Origin, SphereRadius, View.GetTemporalLODOrigin(SampleIndex), View.ViewMatrices.GetProjectionMatrix())
			* FactorScale * FactorScale * View.LODDistanceFactor * View.LODDistanceFactor;

		// Walk backwards and return the first matching LOD
		for (int32 LODIndex = NumLODs - 1; LODIndex >= 0; --LODIndex)
		{
			const float LODSScreenSizeSquared = FMath::Square(RealtimeMeshProxy->GetScreenSizeRangeForLOD(LODIndex).GetLowerBoundValue() * 0.5f);
			if (LODSScreenSizeSquared > ScreenRadiusSquared)
			{
				return LODIndex;
			}
		}

		return MinLOD;
	}

	int8 FRealtimeMeshComponentSceneProxy::ComputeStaticMeshLOD(const FVector4& Origin, const float SphereRadius, const FSceneView& View, int32 MinLOD, float FactorScale) const
	{
		const FSceneView& LODView = GetLODView(View);
		const float ScreenRadiusSquared = ComputeBoundsScreenRadiusSquared(Origin, SphereRadius, LODView) * FactorScale * FactorScale * LODView.LODDistanceFactor * LODView.
			LODDistanceFactor;

		// Walk backwards and return the first matching LOD
		for (int32 LODIndex = RealtimeMeshProxy->GetNumLODs() - 1; LODIndex >= 0; --LODIndex)
		{
			const float LODSScreenSizeSquared = FMath::Square(RealtimeMeshProxy->GetScreenSizeRangeForLOD(LODIndex).GetLowerBoundValue() * 0.5f);
			if (LODSScreenSizeSquared > ScreenRadiusSquared)
			{
 				return FMath::Max(LODIndex, MinLOD);
			}
		}

		return MinLOD;
	}


	FLODMask FRealtimeMeshComponentSceneProxy::GetLODMask(const FSceneView* View) const
	{
		FLODMask Result;

		if (View->DrawDynamicFlags & EDrawDynamicFlags::ForceLowestLOD)
		{
			Result.SetLOD(RealtimeMeshProxy->GetValidLODRange().GetUpperBoundValue());
		}
#if WITH_EDITOR
		else if (View->Family && View->Family->EngineShowFlags.LOD == 0)
		{
			Result.SetLOD(0);
		}
#endif
		else
		{
			const FBoxSphereBounds& ProxyBounds = GetBounds();
			bool bUseDithered = RealtimeMeshProxy->GetValidLODRange().GetUpperBoundValue() != INDEX_NONE && bAnyMaterialUsesDithering;

			FCachedSystemScalabilityCVars CachedSystemScalabilityCVars = GetCachedScalabilityCVars();

			float InvScreenSizeScale = (CachedSystemScalabilityCVars.StaticMeshLODDistanceScale != 0.f) ? (1.0f / CachedSystemScalabilityCVars.StaticMeshLODDistanceScale) : 1.0f;

			int32 ClampedMinLOD = 0;

			if (bUseDithered)
			{
				for (int32 Sample = 0; Sample < 2; Sample++)
				{
					Result.SetLODSample(ComputeTemporalStaticMeshLOD(ProxyBounds.Origin, ProxyBounds.SphereRadius, *View, ClampedMinLOD, InvScreenSizeScale, Sample), Sample);
				}
			}
			else
			{
				Result.SetLOD(ComputeStaticMeshLOD(ProxyBounds.Origin, ProxyBounds.SphereRadius, *View, ClampedMinLOD, InvScreenSizeScale));
			}
		}

		return Result;
	}

	int32 FRealtimeMeshComponentSceneProxy::GetLOD(const FSceneView* View) const
	{
		const FBoxSphereBounds& ProxyBounds = GetBounds();
		FCachedSystemScalabilityCVars CachedSystemScalabilityCVars = GetCachedScalabilityCVars();

		float InvScreenSizeScale = (CachedSystemScalabilityCVars.StaticMeshLODDistanceScale != 0.f) ? (1.0f / CachedSystemScalabilityCVars.StaticMeshLODDistanceScale) : 1.0f;

		return ComputeStaticMeshLOD(ProxyBounds.Origin, ProxyBounds.SphereRadius, *View, 0, InvScreenSizeScale);
	}

	uint32 FRealtimeMeshComponentSceneProxy::GetMemoryFootprint() const
	{
		return (sizeof(*this) + GetAllocatedSize());
	}

	SIZE_T FRealtimeMeshComponentSceneProxy::GetAllocatedSize() const
	{
		return (FPrimitiveSceneProxy::GetAllocatedSize());
	}

	FMaterialRenderProxy* FRealtimeMeshComponentSceneProxy::GetMaterialSlot(int32 MaterialSlotId) const
	{
		const TTuple<FMaterialRenderProxy*, bool>* Mat = Materials.Find(MaterialSlotId);
		if (Mat != nullptr && Mat->Get<0>() != nullptr)
		{
			return Mat->Get<0>();
		}

		return UMaterial::GetDefaultMaterial(MD_Surface)->GetRenderProxy();
	}
}

#undef RMC_LOG_VERBOSE
