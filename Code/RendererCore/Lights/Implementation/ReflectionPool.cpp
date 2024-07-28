#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Camera.h>
#include <Core/Graphics/Geometry.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Math/Color16f.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/GPUResourcePool/GPUResourcePool.h>
#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Lights/Implementation/ReflectionPoolData.h>
#include <RendererCore/Lights/Implementation/ReflectionProbeData.h>
#include <RendererCore/Meshes/MeshComponentBase.h>
#include <RendererCore/Pipeline/Declarations.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Textures/TextureCubeResource.h>
#include <RendererFoundation/CommandEncoder/ComputeCommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/Pass.h>
#include <RendererFoundation/Profiling/Profiling.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, ReflectionPool)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core",
    "RenderWorld"
  END_SUBSYSTEM_DEPENDENCIES

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    nsReflectionPool::OnEngineStartup();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    nsReflectionPool::OnEngineShutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

//////////////////////////////////////////////////////////////////////////
/// nsReflectionPool

nsReflectionProbeId nsReflectionPool::RegisterReflectionProbe(const nsWorld* pWorld, const nsReflectionProbeDesc& desc, const nsReflectionProbeComponentBase* pComponent)
{
  NS_LOCK(s_pData->m_Mutex);

  Data::ProbeData probe;
  s_pData->UpdateProbeData(probe, desc, pComponent);
  return s_pData->AddProbe(pWorld, std::move(probe));
}

void nsReflectionPool::DeregisterReflectionProbe(const nsWorld* pWorld, nsReflectionProbeId id)
{
  NS_LOCK(s_pData->m_Mutex);
  s_pData->RemoveProbe(pWorld, id);
}

void nsReflectionPool::UpdateReflectionProbe(const nsWorld* pWorld, nsReflectionProbeId id, const nsReflectionProbeDesc& desc, const nsReflectionProbeComponentBase* pComponent)
{
  NS_LOCK(s_pData->m_Mutex);
  nsReflectionPool::Data::WorldReflectionData& data = s_pData->GetWorldData(pWorld);
  Data::ProbeData& probeData = data.m_Probes.GetValueUnchecked(id.m_InstanceIndex);
  s_pData->UpdateProbeData(probeData, desc, pComponent);
  data.m_mapping.UpdateProbe(id, probeData.m_Flags);
}

void nsReflectionPool::ExtractReflectionProbe(const nsComponent* pComponent, nsMsgExtractRenderData& ref_msg, nsReflectionProbeRenderData* pRenderData0, const nsWorld* pWorld, nsReflectionProbeId id, float fPriority)
{
  NS_LOCK(s_pData->m_Mutex);
  s_pData->m_ReflectionProbeUpdater.ScheduleUpdateSteps();

  const nsUInt32 uiWorldIndex = pWorld->GetIndex();
  nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[uiWorldIndex];
  data.m_mapping.AddWeight(id, fPriority);
  const nsInt32 iMappedIndex = data.m_mapping.GetReflectionIndex(id, true);

  Data::ProbeData& probeData = data.m_Probes.GetValueUnchecked(id.m_InstanceIndex);

  if (pComponent->GetOwner()->IsDynamic())
  {
    nsTransform globalTransform = pComponent->GetOwner()->GetGlobalTransform();
    if (!probeData.m_Flags.IsSet(nsProbeFlags::Dynamic) && probeData.m_GlobalTransform != globalTransform)
    {
      data.m_mapping.UpdateProbe(id, probeData.m_Flags);
    }
    probeData.m_GlobalTransform = globalTransform;
  }

  // The sky light is always active and not added to the render data (always passes in nullptr as pRenderData).
  if (pRenderData0 && iMappedIndex > 0)
  {
    // Index and flags are stored in m_uiIndex so we can't just overwrite it.
    pRenderData0->m_uiIndex |= (nsUInt32)iMappedIndex;
    ref_msg.AddRenderData(pRenderData0, nsDefaultRenderDataCategories::ReflectionProbe, nsRenderData::Caching::Never);
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  const nsUInt32 uiMipLevels = GetMipLevels();
  if (probeData.m_desc.m_bShowDebugInfo && s_pData->m_hDebugMaterial.GetCount() == uiMipLevels * s_uiNumReflectionProbeCubeMaps)
  {
    if (ref_msg.m_OverrideCategory == nsInvalidRenderDataCategory)
    {
      nsInt32 activeIndex = 0;
      if (s_pData->m_ActiveDynamicUpdate.Contains(nsReflectionProbeRef{uiWorldIndex, id}))
      {
        activeIndex = 1;
      }

      nsStringBuilder sEnum;
      nsReflectionUtils::BitflagsToString(probeData.m_Flags, sEnum, nsReflectionUtils::EnumConversionMode::ValueNameOnly);
      nsStringBuilder s;
      s.SetFormat("\n RefIdx: {}\nUpdating: {}\nFlags: {}\n", iMappedIndex, activeIndex, sEnum);
      nsDebugRenderer::Draw3DText(pWorld, s, pComponent->GetOwner()->GetGlobalPosition(), nsColorScheme::LightUI(nsColorScheme::Violet));
    }

    // Not mapped in the atlas - cannot render it.
    if (iMappedIndex < 0)
      return;

    const nsGameObject* pOwner = pComponent->GetOwner();
    const nsTransform ownerTransform = pOwner->GetGlobalTransform();

    nsUInt32 uiMipLevelsToRender = probeData.m_desc.m_bShowMipMaps ? uiMipLevels : 1;
    for (nsUInt32 i = 0; i < uiMipLevelsToRender; i++)
    {
      nsMeshRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsMeshRenderData>(pOwner);
      pRenderData->m_GlobalTransform.m_vPosition = ownerTransform * probeData.m_desc.m_vCaptureOffset;
      pRenderData->m_GlobalTransform.m_vScale = nsVec3(1.0f);
      if (!probeData.m_Flags.IsSet(nsProbeFlags::SkyLight))
      {
        pRenderData->m_GlobalTransform.m_qRotation = ownerTransform.m_qRotation;
      }
      pRenderData->m_GlobalTransform.m_vPosition.z += s_fDebugSphereRadius * i * 2;
      pRenderData->m_GlobalBounds = pOwner->GetGlobalBounds();
      pRenderData->m_hMesh = s_pData->m_hDebugSphere;
      pRenderData->m_hMaterial = s_pData->m_hDebugMaterial[iMappedIndex * uiMipLevels + i];
      pRenderData->m_Color = nsColor::White;
      pRenderData->m_uiSubMeshIndex = 0;
      pRenderData->m_uiUniqueID = nsRenderComponent::GetUniqueIdForRendering(*pComponent, 0);

      pRenderData->FillBatchIdAndSortingKey();
      ref_msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::LitOpaque, nsRenderData::Caching::Never);
    }
  }
#endif
}

//////////////////////////////////////////////////////////////////////////
/// SkyLight

nsReflectionProbeId nsReflectionPool::RegisterSkyLight(const nsWorld* pWorld, nsReflectionProbeDesc& ref_desc, const nsSkyLightComponent* pComponent)
{
  NS_LOCK(s_pData->m_Mutex);
  const nsUInt32 uiWorldIndex = pWorld->GetIndex();
  s_pData->m_uiWorldHasSkyLight |= NS_BIT(uiWorldIndex);
  s_pData->m_uiSkyIrradianceChanged |= NS_BIT(uiWorldIndex);

  Data::ProbeData probe;
  s_pData->UpdateSkyLightData(probe, ref_desc, pComponent);

  nsReflectionProbeId id = s_pData->AddProbe(pWorld, std::move(probe));
  return id;
}

void nsReflectionPool::DeregisterSkyLight(const nsWorld* pWorld, nsReflectionProbeId id)
{
  NS_LOCK(s_pData->m_Mutex);

  s_pData->RemoveProbe(pWorld, id);

  const nsUInt32 uiWorldIndex = pWorld->GetIndex();
  s_pData->m_uiWorldHasSkyLight &= ~NS_BIT(uiWorldIndex);
  s_pData->m_uiSkyIrradianceChanged |= NS_BIT(uiWorldIndex);
}

void nsReflectionPool::UpdateSkyLight(const nsWorld* pWorld, nsReflectionProbeId id, const nsReflectionProbeDesc& desc, const nsSkyLightComponent* pComponent)
{
  NS_LOCK(s_pData->m_Mutex);
  nsReflectionPool::Data::WorldReflectionData& data = s_pData->GetWorldData(pWorld);
  Data::ProbeData& probeData = data.m_Probes.GetValueUnchecked(id.m_InstanceIndex);
  if (s_pData->UpdateSkyLightData(probeData, desc, pComponent))
  {
    // s_pData->UnmapProbe(pWorld->GetIndex(), data, id);
  }
  data.m_mapping.UpdateProbe(id, probeData.m_Flags);
}

//////////////////////////////////////////////////////////////////////////
/// Misc

// static
void nsReflectionPool::SetConstantSkyIrradiance(const nsWorld* pWorld, const nsAmbientCube<nsColor>& skyIrradiance)
{
  nsUInt32 uiWorldIndex = pWorld->GetIndex();
  nsAmbientCube<nsColorLinear16f> skyIrradiance16f = skyIrradiance;

  auto& skyIrradianceStorage = s_pData->m_SkyIrradianceStorage;
  if (skyIrradianceStorage[uiWorldIndex] != skyIrradiance16f)
  {
    skyIrradianceStorage[uiWorldIndex] = skyIrradiance16f;

    s_pData->m_uiSkyIrradianceChanged |= NS_BIT(uiWorldIndex);
  }
}

void nsReflectionPool::ResetConstantSkyIrradiance(const nsWorld* pWorld)
{
  nsUInt32 uiWorldIndex = pWorld->GetIndex();

  auto& skyIrradianceStorage = s_pData->m_SkyIrradianceStorage;
  if (skyIrradianceStorage[uiWorldIndex] != nsAmbientCube<nsColorLinear16f>())
  {
    skyIrradianceStorage[uiWorldIndex] = nsAmbientCube<nsColorLinear16f>();

    s_pData->m_uiSkyIrradianceChanged |= NS_BIT(uiWorldIndex);
  }
}

// static
nsUInt32 nsReflectionPool::GetReflectionCubeMapSize()
{
  return s_uiReflectionCubeMapSize;
}

// static
nsGALTextureHandle nsReflectionPool::GetReflectionSpecularTexture(nsUInt32 uiWorldIndex, nsEnum<nsCameraUsageHint> cameraUsageHint)
{
  if (uiWorldIndex < s_pData->m_WorldReflectionData.GetCount() && cameraUsageHint != nsCameraUsageHint::Reflection)
  {
    Data::WorldReflectionData* pData = s_pData->m_WorldReflectionData[uiWorldIndex].Borrow();
    if (pData)
      return pData->m_mapping.GetTexture();
  }
  return s_pData->m_hFallbackReflectionSpecularTexture;
}

// static
nsGALTextureHandle nsReflectionPool::GetSkyIrradianceTexture()
{
  return s_pData->m_hSkyIrradianceTexture;
}

//////////////////////////////////////////////////////////////////////////
/// Private Functions

// static
void nsReflectionPool::OnEngineStartup()
{
  s_pData = NS_DEFAULT_NEW(nsReflectionPool::Data);

  nsRenderWorld::GetExtractionEvent().AddEventHandler(OnExtractionEvent);
  nsRenderWorld::GetRenderEvent().AddEventHandler(OnRenderEvent);
}

// static
void nsReflectionPool::OnEngineShutdown()
{
  nsRenderWorld::GetExtractionEvent().RemoveEventHandler(OnExtractionEvent);
  nsRenderWorld::GetRenderEvent().RemoveEventHandler(OnRenderEvent);

  NS_DEFAULT_DELETE(s_pData);
}

// static
void nsReflectionPool::OnExtractionEvent(const nsRenderWorldExtractionEvent& e)
{
  if (e.m_Type == nsRenderWorldExtractionEvent::Type::BeginExtraction)
  {
    NS_PROFILE_SCOPE("Reflection Pool BeginExtraction");
    s_pData->CreateSkyIrradianceTexture();
    s_pData->CreateReflectionViewsAndResources();
    s_pData->PreExtraction();
  }

  if (e.m_Type == nsRenderWorldExtractionEvent::Type::EndExtraction)
  {
    NS_PROFILE_SCOPE("Reflection Pool EndExtraction");
    s_pData->PostExtraction();
  }
}

// static
void nsReflectionPool::OnRenderEvent(const nsRenderWorldRenderEvent& e)
{
  if (e.m_Type != nsRenderWorldRenderEvent::Type::BeginRender)
    return;

  if (s_pData->m_hSkyIrradianceTexture.IsInvalidated())
    return;

  NS_LOCK(s_pData->m_Mutex);

  nsUInt64 uiWorldHasSkyLight = s_pData->m_uiWorldHasSkyLight;
  nsUInt64 uiSkyIrradianceChanged = s_pData->m_uiSkyIrradianceChanged;
  if ((~uiWorldHasSkyLight & uiSkyIrradianceChanged) == 0)
    return;

  auto& skyIrradianceStorage = s_pData->m_SkyIrradianceStorage;

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  auto pGALPass = pDevice->BeginPass("Sky Irradiance Texture Update");
  nsHybridArray<nsGALTextureHandle, 4> atlasToClear;

  {
    auto pGALCommandEncoder = pGALPass->BeginCompute();
    for (nsUInt32 i = 0; i < skyIrradianceStorage.GetCount(); ++i)
    {
      if ((uiWorldHasSkyLight & NS_BIT(i)) == 0 && (uiSkyIrradianceChanged & NS_BIT(i)) != 0)
      {
        nsBoundingBoxu32 destBox;
        destBox.m_vMin.Set(0, i, 0);
        destBox.m_vMax.Set(6, i + 1, 1);
        nsGALSystemMemoryDescription memDesc;
        memDesc.m_pData = &skyIrradianceStorage[i].m_Values[0];
        memDesc.m_uiRowPitch = sizeof(nsAmbientCube<nsColorLinear16f>);
        pGALCommandEncoder->UpdateTexture(s_pData->m_hSkyIrradianceTexture, nsGALTextureSubresource(), destBox, memDesc);

        uiSkyIrradianceChanged &= ~NS_BIT(i);

        if (i < s_pData->m_WorldReflectionData.GetCount() && s_pData->m_WorldReflectionData[i] != nullptr)
        {
          nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[i];
          atlasToClear.PushBack(data.m_mapping.GetTexture());
        }
      }
    }
    pGALPass->EndCompute(pGALCommandEncoder);
  }

  {
    // Clear specular sky reflection to black.
    const nsUInt32 uiNumMipMaps = GetMipLevels();
    for (nsGALTextureHandle atlas : atlasToClear)
    {
      for (nsUInt32 uiMipMapIndex = 0; uiMipMapIndex < uiNumMipMaps; ++uiMipMapIndex)
      {
        for (nsUInt32 uiFaceIndex = 0; uiFaceIndex < 6; ++uiFaceIndex)
        {
          nsGALRenderingSetup renderingSetup;
          nsGALRenderTargetViewCreationDescription desc;
          desc.m_hTexture = atlas;
          desc.m_uiMipLevel = uiMipMapIndex;
          desc.m_uiFirstSlice = uiFaceIndex;
          desc.m_uiSliceCount = 1;
          renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->CreateRenderTargetView(desc));
          renderingSetup.m_ClearColor = nsColor(0, 0, 0, 1);
          renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;

          auto pGALCommandEncoder = pGALPass->BeginRendering(renderingSetup, "ClearSkySpecular");
          pGALCommandEncoder->Clear(nsColor::Black);
          pGALPass->EndRendering(pGALCommandEncoder);
        }
      }
    }
  }

  pDevice->EndPass(pGALPass);
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_ReflectionPool);
