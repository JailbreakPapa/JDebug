#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Meshes/MeshComponentBase.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/Device/Device.h>

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_IMPLEMENT_MESSAGE_TYPE(nsMsgSetMeshMaterial);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgSetMeshMaterial, 1, nsRTTIDefaultAllocator<nsMsgSetMeshMaterial>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Material", GetMaterialFile, SetMaterialFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Material")),
    NS_MEMBER_PROPERTY("MaterialSlot", m_uiMaterialSlot),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsMsgSetMeshMaterial::SetMaterialFile(const char* szFile)
{
  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    m_hMaterial = nsResourceManager::LoadResource<nsMaterialResource>(szFile);
  }
  else
  {
    m_hMaterial.Invalidate();
  }
}

const char* nsMsgSetMeshMaterial::GetMaterialFile() const
{
  if (!m_hMaterial.IsValid())
    return "";

  return m_hMaterial.GetResourceID();
}

void nsMsgSetMeshMaterial::Serialize(nsStreamWriter& inout_stream) const
{
  // has to be stringyfied for transfer
  inout_stream << GetMaterialFile();
  inout_stream << m_uiMaterialSlot;
}

void nsMsgSetMeshMaterial::Deserialize(nsStreamReader& inout_stream, nsUInt8 uiTypeVersion)
{
  nsStringBuilder file;
  inout_stream >> file;
  SetMaterialFile(file);

  inout_stream >> m_uiMaterialSlot;
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMeshRenderData, 1, nsRTTIDefaultAllocator<nsMeshRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsMeshRenderData::FillBatchIdAndSortingKey()
{
  FillBatchIdAndSortingKeyInternal(0);
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_ABSTRACT_COMPONENT_TYPE(nsMeshComponentBase, 4)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgSetMeshMaterial, OnMsgSetMeshMaterial),
    NS_MESSAGE_HANDLER(nsMsgSetColor, OnMsgSetColor),
    NS_MESSAGE_HANDLER(nsMsgSetCustomData, OnMsgSetCustomData),
  } NS_END_MESSAGEHANDLERS;
}
NS_END_ABSTRACT_COMPONENT_TYPE;
// clang-format on

nsMeshComponentBase::nsMeshComponentBase() = default;
nsMeshComponentBase::~nsMeshComponentBase() = default;

void nsMeshComponentBase::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  // ignore components that have created meshes (?)

  s << m_hMesh;

  s << m_Materials.GetCount();
  for (const auto& mat : m_Materials)
  {
    s << mat;
  }

  s << m_Color;
  s << m_fSortingDepthOffset;
  s << m_vCustomData;
}

void nsMeshComponentBase::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();

  s >> m_hMesh;

  if (uiVersion < 2)
  {
    nsUInt32 uiCategory = 0;
    s >> uiCategory;
  }

  nsUInt32 uiMaterials = 0;
  s >> uiMaterials;

  m_Materials.SetCount(uiMaterials);

  for (auto& mat : m_Materials)
  {
    s >> mat;
  }

  s >> m_Color;

  if (uiVersion >= 3)
  {
    s >> m_fSortingDepthOffset;
  }

  if (uiVersion >= 4)
  {
    s >> m_vCustomData;
  }
}

nsResult nsMeshComponentBase::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  if (m_hMesh.IsValid())
  {
    nsResourceLock<nsMeshResource> pMesh(m_hMesh, nsResourceAcquireMode::AllowLoadingFallback);
    ref_bounds = pMesh->GetBounds();
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

void nsMeshComponentBase::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  if (!m_hMesh.IsValid())
    return;

  nsResourceLock<nsMeshResource> pMesh(m_hMesh, nsResourceAcquireMode::AllowLoadingFallback);
  nsArrayPtr<const nsMeshResourceDescriptor::SubMesh> parts = pMesh->GetSubMeshes();

  for (nsUInt32 uiPartIndex = 0; uiPartIndex < parts.GetCount(); ++uiPartIndex)
  {
    const nsUInt32 uiMaterialIndex = parts[uiPartIndex].m_uiMaterialIndex;
    nsMaterialResourceHandle hMaterial;

    // If we have a material override, use that otherwise use the default mesh material.
    if (GetMaterial(uiMaterialIndex).IsValid())
      hMaterial = m_Materials[uiMaterialIndex];
    else
      hMaterial = pMesh->GetMaterials()[uiMaterialIndex];

    nsMeshRenderData* pRenderData = CreateRenderData();
    {
      pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform() * pRenderData->m_GlobalTransform;
      pRenderData->m_GlobalBounds = GetOwner()->GetGlobalBounds();
      pRenderData->m_fSortingDepthOffset = m_fSortingDepthOffset;
      pRenderData->m_hMesh = m_hMesh;
      pRenderData->m_hMaterial = hMaterial;
      pRenderData->m_Color = m_Color;
      pRenderData->m_vCustomData = m_vCustomData;
      pRenderData->m_uiSubMeshIndex = uiPartIndex;
      pRenderData->m_uiUniqueID = GetUniqueIdForRendering(uiMaterialIndex);

      pRenderData->FillBatchIdAndSortingKey();
    }

    bool bDontCacheYet = false;

    // Determine render data category.
    nsRenderData::Category category = nsDefaultRenderDataCategories::LitOpaque;
    if (hMaterial.IsValid())
    {
      nsResourceLock<nsMaterialResource> pMaterial(hMaterial, nsResourceAcquireMode::AllowLoadingFallback);

      if (pMaterial.GetAcquireResult() == nsResourceAcquireResult::LoadingFallback)
        bDontCacheYet = true;

      category = pMaterial->GetRenderDataCategory();
    }

    msg.AddRenderData(pRenderData, category, bDontCacheYet ? nsRenderData::Caching::Never : nsRenderData::Caching::IfStatic);
  }
}

void nsMeshComponentBase::SetMesh(const nsMeshResourceHandle& hMesh)
{
  if (m_hMesh != hMesh)
  {
    m_hMesh = hMesh;

    TriggerLocalBoundsUpdate();
    InvalidateCachedRenderData();
  }
}

void nsMeshComponentBase::SetMaterial(nsUInt32 uiIndex, const nsMaterialResourceHandle& hMaterial)
{
  m_Materials.EnsureCount(uiIndex + 1);

  if (m_Materials[uiIndex] != hMaterial)
  {
    m_Materials[uiIndex] = hMaterial;

    InvalidateCachedRenderData();
  }
}

nsMaterialResourceHandle nsMeshComponentBase::GetMaterial(nsUInt32 uiIndex) const
{
  if (uiIndex >= m_Materials.GetCount())
    return nsMaterialResourceHandle();

  return m_Materials[uiIndex];
}

void nsMeshComponentBase::SetMeshFile(const char* szFile)
{
  nsMeshResourceHandle hMesh;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hMesh = nsResourceManager::LoadResource<nsMeshResource>(szFile);
  }

  SetMesh(hMesh);
}

const char* nsMeshComponentBase::GetMeshFile() const
{
  if (!m_hMesh.IsValid())
    return "";

  return m_hMesh.GetResourceID();
}

void nsMeshComponentBase::SetColor(const nsColor& color)
{
  m_Color = color;

  InvalidateCachedRenderData();
}

const nsColor& nsMeshComponentBase::GetColor() const
{
  return m_Color;
}

void nsMeshComponentBase::SetCustomData(const nsVec4& vData)
{
  m_vCustomData = vData;

  InvalidateCachedRenderData();
}

const nsVec4& nsMeshComponentBase::GetCustomData() const
{
  return m_vCustomData;
}

void nsMeshComponentBase::SetSortingDepthOffset(float fOffset)
{
  m_fSortingDepthOffset = fOffset;

  InvalidateCachedRenderData();
}

float nsMeshComponentBase::GetSortingDepthOffset() const
{
  return m_fSortingDepthOffset;
}

void nsMeshComponentBase::OnMsgSetMeshMaterial(nsMsgSetMeshMaterial& ref_msg)
{
  SetMaterial(ref_msg.m_uiMaterialSlot, ref_msg.m_hMaterial);
}

void nsMeshComponentBase::OnMsgSetColor(nsMsgSetColor& ref_msg)
{
  ref_msg.ModifyColor(m_Color);

  InvalidateCachedRenderData();
}

void nsMeshComponentBase::OnMsgSetCustomData(nsMsgSetCustomData& ref_msg)
{
  m_vCustomData = ref_msg.m_vData;
  InvalidateCachedRenderData();
}

nsMeshRenderData* nsMeshComponentBase::CreateRenderData() const
{
  return nsCreateRenderDataForThisFrame<nsMeshRenderData>(GetOwner());
}

nsUInt32 nsMeshComponentBase::Materials_GetCount() const
{
  return m_Materials.GetCount();
}

const char* nsMeshComponentBase::Materials_GetValue(nsUInt32 uiIndex) const
{
  auto hMat = GetMaterial(uiIndex);

  if (!hMat.IsValid())
    return "";

  return hMat.GetResourceID();
}


void nsMeshComponentBase::Materials_SetValue(nsUInt32 uiIndex, const char* value)
{
  if (nsStringUtils::IsNullOrEmpty(value))
    SetMaterial(uiIndex, nsMaterialResourceHandle());
  else
  {
    auto hMat = nsResourceManager::LoadResource<nsMaterialResource>(value);
    SetMaterial(uiIndex, hMat);
  }
}


void nsMeshComponentBase::Materials_Insert(nsUInt32 uiIndex, const char* value)
{
  nsMaterialResourceHandle hMat;

  if (!nsStringUtils::IsNullOrEmpty(value))
    hMat = nsResourceManager::LoadResource<nsMaterialResource>(value);

  m_Materials.InsertAt(uiIndex, hMat);

  InvalidateCachedRenderData();
}


void nsMeshComponentBase::Materials_Remove(nsUInt32 uiIndex)
{
  m_Materials.RemoveAtAndCopy(uiIndex);

  InvalidateCachedRenderData();
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_MeshComponentBase);
