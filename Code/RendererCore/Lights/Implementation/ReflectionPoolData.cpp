#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/Implementation/ReflectionPoolData.h>

#include <Core/Graphics/Geometry.h>
#include <RendererCore/Lights/BoxReflectionProbeComponent.h>
#include <RendererCore/Lights/SkyLightComponent.h>
#include <RendererCore/Lights/SphereReflectionProbeComponent.h>
#include <RendererCore/Meshes/MeshComponentBase.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/Texture.h>

//////////////////////////////////////////////////////////////////////////
/// nsReflectionPool::Data

nsReflectionPool::Data* nsReflectionPool::s_pData;

nsReflectionPool::Data::Data()
{
  m_SkyIrradianceStorage.SetCount(64);
}

nsReflectionPool::Data::~Data()
{
  if (!m_hFallbackReflectionSpecularTexture.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroyTexture(m_hFallbackReflectionSpecularTexture);
    m_hFallbackReflectionSpecularTexture.Invalidate();
  }

  nsUInt32 uiWorldReflectionCount = m_WorldReflectionData.GetCount();
  for (nsUInt32 i = 0; i < uiWorldReflectionCount; ++i)
  {
    WorldReflectionData* pData = m_WorldReflectionData[i].Borrow();
    NS_ASSERT_DEV(!pData || pData->m_Probes.IsEmpty(), "Not all probes were deregistered.");
  }
  m_WorldReflectionData.Clear();

  if (!m_hSkyIrradianceTexture.IsInvalidated())
  {
    nsGALDevice::GetDefaultDevice()->DestroyTexture(m_hSkyIrradianceTexture);
    m_hSkyIrradianceTexture.Invalidate();
  }
}

nsReflectionProbeId nsReflectionPool::Data::AddProbe(const nsWorld* pWorld, ProbeData&& probeData)
{
  const nsUInt32 uiWorldIndex = pWorld->GetIndex();

  if (uiWorldIndex >= s_pData->m_WorldReflectionData.GetCount())
    s_pData->m_WorldReflectionData.SetCount(uiWorldIndex + 1);

  if (s_pData->m_WorldReflectionData[uiWorldIndex] == nullptr)
  {
    s_pData->m_WorldReflectionData[uiWorldIndex] = NS_DEFAULT_NEW(WorldReflectionData);
    s_pData->m_WorldReflectionData[uiWorldIndex]->m_mappingSubscriptionId = s_pData->m_WorldReflectionData[uiWorldIndex]->m_mapping.m_Events.AddEventHandler([uiWorldIndex, this](const nsReflectionProbeMappingEvent& e)
      { OnReflectionProbeMappingEvent(uiWorldIndex, e); });
  }

  nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[uiWorldIndex];

  const nsBitflags<nsProbeFlags> flags = probeData.m_Flags;
  nsReflectionProbeId id = data.m_Probes.Insert(std::move(probeData));

  if (probeData.m_Flags.IsSet(nsProbeFlags::SkyLight))
  {
    data.m_SkyLight = id;
  }
  data.m_mapping.AddProbe(id, flags);

  return id;
}

nsReflectionPool::Data::WorldReflectionData& nsReflectionPool::Data::GetWorldData(const nsWorld* pWorld)
{
  const nsUInt32 uiWorldIndex = pWorld->GetIndex();
  return *s_pData->m_WorldReflectionData[uiWorldIndex];
}

void nsReflectionPool::Data::RemoveProbe(const nsWorld* pWorld, nsReflectionProbeId id)
{
  const nsUInt32 uiWorldIndex = pWorld->GetIndex();
  nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[uiWorldIndex];

  data.m_mapping.RemoveProbe(id);

  if (data.m_SkyLight == id)
  {
    data.m_SkyLight.Invalidate();
  }

  data.m_Probes.Remove(id);

  if (data.m_Probes.IsEmpty())
  {
    s_pData->m_WorldReflectionData[uiWorldIndex]->m_mapping.m_Events.RemoveEventHandler(s_pData->m_WorldReflectionData[uiWorldIndex]->m_mappingSubscriptionId);
    s_pData->m_WorldReflectionData[uiWorldIndex].Clear();
  }
}

void nsReflectionPool::Data::UpdateProbeData(ProbeData& ref_probeData, const nsReflectionProbeDesc& desc, const nsReflectionProbeComponentBase* pComponent)
{
  ref_probeData.m_desc = desc;
  ref_probeData.m_GlobalTransform = pComponent->GetOwner()->GetGlobalTransform();

  if (const nsSphereReflectionProbeComponent* pSphere = nsDynamicCast<const nsSphereReflectionProbeComponent*>(pComponent))
  {
    ref_probeData.m_Flags = nsProbeFlags::Sphere;
  }
  else if (const nsBoxReflectionProbeComponent* pBox = nsDynamicCast<const nsBoxReflectionProbeComponent*>(pComponent))
  {
    ref_probeData.m_Flags = nsProbeFlags::Box;
  }

  if (ref_probeData.m_desc.m_Mode == nsReflectionProbeMode::Dynamic)
  {
    ref_probeData.m_Flags |= nsProbeFlags::Dynamic;
  }
  else
  {
    nsStringBuilder sComponentGuid, sCubeMapFile;
    nsConversionUtils::ToString(ref_probeData.m_desc.m_uniqueID, sComponentGuid);

    // this is where the editor will put the file for this probe
    sCubeMapFile.SetFormat(":project/AssetCache/Generated/{0}.nsTexture", sComponentGuid);

    ref_probeData.m_hCubeMap = nsResourceManager::LoadResource<nsTextureCubeResource>(sCubeMapFile);
  }
}

bool nsReflectionPool::Data::UpdateSkyLightData(ProbeData& ref_probeData, const nsReflectionProbeDesc& desc, const nsSkyLightComponent* pComponent)
{
  bool bProbeTypeChanged = false;
  if (ref_probeData.m_desc.m_Mode != desc.m_Mode)
  {
    // #TODO any other reason to unmap a probe.
    bProbeTypeChanged = true;
  }

  ref_probeData.m_desc = desc;
  ref_probeData.m_GlobalTransform = pComponent->GetOwner()->GetGlobalTransform();

  if (auto pSkyLight = nsDynamicCast<const nsSkyLightComponent*>(pComponent))
  {
    ref_probeData.m_Flags = nsProbeFlags::SkyLight;
    ref_probeData.m_hCubeMap = pSkyLight->GetCubeMap();
    if (ref_probeData.m_desc.m_Mode == nsReflectionProbeMode::Dynamic)
    {
      ref_probeData.m_Flags |= nsProbeFlags::Dynamic;
    }
    else
    {
      if (ref_probeData.m_hCubeMap.IsValid())
      {
        ref_probeData.m_Flags |= nsProbeFlags::HasCustomCubeMap;
      }
      else
      {
        nsStringBuilder sComponentGuid, sCubeMapFile;
        nsConversionUtils::ToString(ref_probeData.m_desc.m_uniqueID, sComponentGuid);

        // this is where the editor will put the file for this probe
        sCubeMapFile.SetFormat(":project/AssetCache/Generated/{0}.nsTexture", sComponentGuid);

        ref_probeData.m_hCubeMap = nsResourceManager::LoadResource<nsTextureCubeResource>(sCubeMapFile);
      }
    }
  }
  return bProbeTypeChanged;
}

void nsReflectionPool::Data::OnReflectionProbeMappingEvent(const nsUInt32 uiWorldIndex, const nsReflectionProbeMappingEvent& e)
{
  switch (e.m_Type)
  {
    case nsReflectionProbeMappingEvent::Type::ProbeMapped:
      break;
    case nsReflectionProbeMappingEvent::Type::ProbeUnmapped:
    {
      nsReflectionProbeRef probeUpdate = {uiWorldIndex, e.m_Id};
      if (m_PendingDynamicUpdate.Contains(probeUpdate))
      {
        m_PendingDynamicUpdate.Remove(probeUpdate);
        m_DynamicUpdateQueue.RemoveAndCopy(probeUpdate);
      }

      if (m_ActiveDynamicUpdate.Contains(probeUpdate))
      {
        m_ActiveDynamicUpdate.Remove(probeUpdate);
        m_ReflectionProbeUpdater.CancelUpdate(probeUpdate);
      }
    }
    break;
    case nsReflectionProbeMappingEvent::Type::ProbeUpdateRequested:
    {
      // For now, we just manage a FIFO queue of all dynamic probes that have a high enough priority.
      const nsReflectionProbeRef du = {uiWorldIndex, e.m_Id};
      nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[uiWorldIndex];
      if (!m_PendingDynamicUpdate.Contains(du))
      {
        m_PendingDynamicUpdate.Insert(du);
        m_DynamicUpdateQueue.PushBack(du);
      }
    }
    break;
  }
}

//////////////////////////////////////////////////////////////////////////
/// Dynamic Update

void nsReflectionPool::Data::PreExtraction()
{
  NS_LOCK(s_pData->m_Mutex);
  const nsUInt32 uiWorldCount = s_pData->m_WorldReflectionData.GetCount();

  for (nsUInt32 uiWorld = 0; uiWorld < uiWorldCount; uiWorld++)
  {
    if (s_pData->m_WorldReflectionData[uiWorld] == nullptr)
      continue;

    nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[uiWorld];
    data.m_mapping.PreExtraction();
  }


  // Schedule new dynamic updates
  {
    nsHybridArray<nsReflectionProbeRef, 4> updatesFinished;
    const nsUInt32 uiCount = nsMath::Min(m_ReflectionProbeUpdater.GetFreeUpdateSlots(updatesFinished), m_DynamicUpdateQueue.GetCount());
    for (const nsReflectionProbeRef& probe : updatesFinished)
    {
      m_ActiveDynamicUpdate.Remove(probe);

      if (s_pData->m_WorldReflectionData[probe.m_uiWorldIndex] == nullptr)
        continue;

      nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[probe.m_uiWorldIndex];
      data.m_mapping.ProbeUpdateFinished(probe.m_Id);
    }

    for (nsUInt32 i = 0; i < uiCount; i++)
    {
      nsReflectionProbeRef nextUpdate = m_DynamicUpdateQueue.PeekFront();
      m_DynamicUpdateQueue.PopFront();
      m_PendingDynamicUpdate.Remove(nextUpdate);

      if (s_pData->m_WorldReflectionData[nextUpdate.m_uiWorldIndex] == nullptr)
        continue;

      nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[nextUpdate.m_uiWorldIndex];
      ProbeData& probeData = data.m_Probes.GetValueUnchecked(nextUpdate.m_Id.m_InstanceIndex);

      nsReflectionProbeUpdater::TargetSlot target;
      target.m_hSpecularOutputTexture = data.m_mapping.GetTexture();
      target.m_iSpecularOutputIndex = data.m_mapping.GetReflectionIndex(nextUpdate.m_Id);

      if (probeData.m_Flags.IsSet(nsProbeFlags::SkyLight))
      {
        target.m_hIrradianceOutputTexture = m_hSkyIrradianceTexture;
        target.m_iIrradianceOutputIndex = nextUpdate.m_uiWorldIndex;
      }

      if (probeData.m_Flags.IsSet(nsProbeFlags::HasCustomCubeMap))
      {
        NS_ASSERT_DEBUG(probeData.m_hCubeMap.IsValid(), "");
        NS_VERIFY(m_ReflectionProbeUpdater.StartFilterUpdate(nextUpdate, probeData.m_desc, probeData.m_hCubeMap, target).Succeeded(), "GetFreeUpdateSlots returned incorrect result");
      }
      else
      {
        NS_VERIFY(m_ReflectionProbeUpdater.StartDynamicUpdate(nextUpdate, probeData.m_desc, probeData.m_GlobalTransform, target).Succeeded(), "GetFreeUpdateSlots returned incorrect result");
      }
      m_ActiveDynamicUpdate.Insert(nextUpdate);
    }
    m_ReflectionProbeUpdater.GenerateUpdateSteps();
  }
}

void nsReflectionPool::Data::PostExtraction()
{
  NS_LOCK(s_pData->m_Mutex);
  const nsUInt32 uiWorldCount = s_pData->m_WorldReflectionData.GetCount();
  for (nsUInt32 uiWorld = 0; uiWorld < uiWorldCount; uiWorld++)
  {
    if (s_pData->m_WorldReflectionData[uiWorld] == nullptr)
      continue;
    nsReflectionPool::Data::WorldReflectionData& data = *s_pData->m_WorldReflectionData[uiWorld];
    data.m_mapping.PostExtraction();
  }
}

//////////////////////////////////////////////////////////////////////////
/// Resource Creation

void nsReflectionPool::Data::CreateReflectionViewsAndResources()
{
  if (m_hFallbackReflectionSpecularTexture.IsInvalidated())
  {
    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

    nsGALTextureCreationDescription desc;
    desc.m_uiWidth = s_uiReflectionCubeMapSize;
    desc.m_uiHeight = s_uiReflectionCubeMapSize;
    desc.m_uiMipLevelCount = GetMipLevels();
    desc.m_uiArraySize = 1;
    desc.m_Format = nsGALResourceFormat::RGBAHalf;
    desc.m_Type = nsGALTextureType::TextureCube;
    desc.m_bCreateRenderTarget = true;
    desc.m_bAllowUAV = true;
    desc.m_ResourceAccess.m_bReadBack = true;
    desc.m_ResourceAccess.m_bImmutable = false;

    m_hFallbackReflectionSpecularTexture = pDevice->CreateTexture(desc);
    if (!m_hFallbackReflectionSpecularTexture.IsInvalidated())
    {
      pDevice->GetTexture(m_hFallbackReflectionSpecularTexture)->SetDebugName("Reflection Fallback Specular Texture");
    }
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  if (!m_hDebugSphere.IsValid())
  {
    nsGeometry geom;
    geom.AddStackedSphere(s_fDebugSphereRadius, 32, 16);

    const char* szBufferResourceName = "ReflectionProbeDebugSphereBuffer";
    nsMeshBufferResourceHandle hMeshBuffer = nsResourceManager::GetExistingResource<nsMeshBufferResource>(szBufferResourceName);
    if (!hMeshBuffer.IsValid())
    {
      nsMeshBufferResourceDescriptor desc;
      desc.AddStream(nsGALVertexAttributeSemantic::Position, nsGALResourceFormat::XYZFloat);
      desc.AddStream(nsGALVertexAttributeSemantic::Normal, nsGALResourceFormat::XYZFloat);
      desc.AllocateStreamsFromGeometry(geom, nsGALPrimitiveTopology::Triangles);

      hMeshBuffer = nsResourceManager::GetOrCreateResource<nsMeshBufferResource>(szBufferResourceName, std::move(desc), szBufferResourceName);
    }

    const char* szMeshResourceName = "ReflectionProbeDebugSphere";
    m_hDebugSphere = nsResourceManager::GetExistingResource<nsMeshResource>(szMeshResourceName);
    if (!m_hDebugSphere.IsValid())
    {
      nsMeshResourceDescriptor desc;
      desc.UseExistingMeshBuffer(hMeshBuffer);
      desc.AddSubMesh(geom.CalculateTriangleCount(), 0, 0);
      desc.ComputeBounds();

      m_hDebugSphere = nsResourceManager::GetOrCreateResource<nsMeshResource>(szMeshResourceName, std::move(desc), szMeshResourceName);
    }
  }

  if (m_hDebugMaterial.IsEmpty())
  {
    const nsUInt32 uiMipLevelCount = GetMipLevels();

    nsMaterialResourceHandle hDebugMaterial = nsResourceManager::LoadResource<nsMaterialResource>(
      "{ 6f8067d0-ece8-44e1-af46-79b49266de41 }"); // ReflectionProbeVisualization.nsMaterialAsset
    nsResourceLock<nsMaterialResource> pMaterial(hDebugMaterial, nsResourceAcquireMode::BlockTillLoaded);
    if (pMaterial->GetLoadingState() != nsResourceState::Loaded)
      return;

    nsMaterialResourceDescriptor desc = pMaterial->GetCurrentDesc();
    nsUInt32 uiMipLevel = desc.m_Parameters.GetCount();
    nsUInt32 uiReflectionProbeIndex = desc.m_Parameters.GetCount();
    nsTempHashedString sMipLevelParam = "MipLevel";
    nsTempHashedString sReflectionProbeIndexParam = "ReflectionProbeIndex";
    for (nsUInt32 i = 0; i < desc.m_Parameters.GetCount(); ++i)
    {
      if (desc.m_Parameters[i].m_Name == sMipLevelParam)
      {
        uiMipLevel = i;
      }
      if (desc.m_Parameters[i].m_Name == sReflectionProbeIndexParam)
      {
        uiReflectionProbeIndex = i;
      }
    }

    if (uiMipLevel >= desc.m_Parameters.GetCount() || uiReflectionProbeIndex >= desc.m_Parameters.GetCount())
      return;

    m_hDebugMaterial.SetCount(uiMipLevelCount * s_uiNumReflectionProbeCubeMaps);
    for (nsUInt32 iReflectionProbeIndex = 0; iReflectionProbeIndex < s_uiNumReflectionProbeCubeMaps; iReflectionProbeIndex++)
    {
      for (nsUInt32 iMipLevel = 0; iMipLevel < uiMipLevelCount; iMipLevel++)
      {
        desc.m_Parameters[uiMipLevel].m_Value = iMipLevel;
        desc.m_Parameters[uiReflectionProbeIndex].m_Value = iReflectionProbeIndex;
        nsStringBuilder sMaterialName;
        sMaterialName.SetFormat("ReflectionProbeVisualization - MipLevel {}, Index {}", iMipLevel, iReflectionProbeIndex);

        nsMaterialResourceDescriptor desc2 = desc;
        m_hDebugMaterial[iReflectionProbeIndex * uiMipLevelCount + iMipLevel] = nsResourceManager::GetOrCreateResource<nsMaterialResource>(sMaterialName, std::move(desc2));
      }
    }
  }
#endif
}

void nsReflectionPool::Data::CreateSkyIrradianceTexture()
{
  if (m_hSkyIrradianceTexture.IsInvalidated())
  {
    nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

    nsGALTextureCreationDescription desc;
    desc.m_uiWidth = 6;
    desc.m_uiHeight = 64;
    desc.m_Format = nsGALResourceFormat::RGBAHalf;
    desc.m_Type = nsGALTextureType::Texture2D;
    desc.m_bCreateRenderTarget = true;
    desc.m_bAllowUAV = true;

    m_hSkyIrradianceTexture = pDevice->CreateTexture(desc);
    pDevice->GetTexture(m_hSkyIrradianceTexture)->SetDebugName("Sky Irradiance Texture");
  }
}
