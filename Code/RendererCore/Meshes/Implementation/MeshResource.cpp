#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Utilities/AssetFileHeader.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/MeshResource.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMeshResource, 1, nsRTTIDefaultAllocator<nsMeshResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsMeshResource);
// clang-format on

nsUInt32 nsMeshResource::s_uiMeshBufferNameSuffix = 0;

nsMeshResource::nsMeshResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
  m_Bounds = nsBoundingBoxSphere::MakeInvalid();
}

nsResourceLoadDesc nsMeshResource::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc res;
  res.m_State = GetLoadingState();
  res.m_uiQualityLevelsDiscardable = GetNumQualityLevelsDiscardable();
  res.m_uiQualityLevelsLoadable = GetNumQualityLevelsLoadable();

  // we currently can only unload the entire mesh
  // if (WhatToUnload == Unload::AllQualityLevels)
  {
    m_SubMeshes.Clear();
    m_SubMeshes.Compact();
    m_Materials.Clear();
    m_Materials.Compact();
    m_Bones.Clear();
    m_Bones.Compact();

    m_hMeshBuffer.Invalidate();
    m_hDefaultSkeleton.Invalidate();

    res.m_uiQualityLevelsDiscardable = 0;
    res.m_uiQualityLevelsLoadable = 0;
    res.m_State = nsResourceState::Unloaded;
  }

  return res;
}

nsResourceLoadDesc nsMeshResource::UpdateContent(nsStreamReader* Stream)
{
  nsMeshResourceDescriptor desc;
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsStringBuilder sAbsFilePath;
    (*Stream) >> sAbsFilePath;
  }

  nsAssetFileHeader AssetHash;
  AssetHash.Read(*Stream).IgnoreResult();

  if (desc.Load(*Stream).Failed())
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  return CreateResource(std::move(desc));
}

void nsMeshResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsMeshResource) + (nsUInt32)m_SubMeshes.GetHeapMemoryUsage() + (nsUInt32)m_Materials.GetHeapMemoryUsage();
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsMeshResource, nsMeshResourceDescriptor)
{
  // if there is an existing mesh buffer to use, take that
  m_hMeshBuffer = descriptor.GetExistingMeshBuffer();

  m_hDefaultSkeleton = descriptor.m_hDefaultSkeleton;
  m_Bones = descriptor.m_Bones;
  m_fMaxBoneVertexOffset = descriptor.m_fMaxBoneVertexOffset;

  // otherwise create a new mesh buffer from the descriptor
  if (!m_hMeshBuffer.IsValid())
  {
    s_uiMeshBufferNameSuffix++;
    nsStringBuilder sMbName;
    sMbName.SetFormat("{0}  [MeshBuffer {1}]", GetResourceID(), nsArgU(s_uiMeshBufferNameSuffix, 4, true, 16, true));

    // note: this gets move'd, might be invalid afterwards
    nsMeshBufferResourceDescriptor& mb = descriptor.MeshBufferDesc();

    m_hMeshBuffer = nsResourceManager::CreateResource<nsMeshBufferResource>(sMbName, std::move(mb), GetResourceDescription());
  }

  m_SubMeshes = descriptor.GetSubMeshes();

  m_Materials.Clear();
  m_Materials.Reserve(descriptor.GetMaterials().GetCount());

  // copy all the material assignments and load the materials
  for (const auto& mat : descriptor.GetMaterials())
  {
    nsMaterialResourceHandle hMat;

    if (!mat.m_sPath.IsEmpty())
      hMat = nsResourceManager::LoadResource<nsMaterialResource>(mat.m_sPath);

    m_Materials.PushBack(hMat); // may be an invalid handle
  }

  m_Bounds = descriptor.GetBounds();
  NS_ASSERT_DEV(m_Bounds.IsValid(), "The mesh bounds are invalid. Make sure to call nsMeshResourceDescriptor::ComputeBounds()");

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  return res;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_MeshResource);
