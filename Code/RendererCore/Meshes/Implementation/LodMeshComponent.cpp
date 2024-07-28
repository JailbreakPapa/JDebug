#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Meshes/LodMeshComponent.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/Device/Device.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsLodMeshLod, nsNoBase, 2, nsRTTIDefaultAllocator<nsLodMeshLod>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Mesh", GetMeshFile, SetMeshFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Mesh_Static")),
    NS_MEMBER_PROPERTY("Threshold", m_fThreshold)
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_COMPONENT_TYPE(nsLodMeshComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Color", GetColor, SetColor)->AddAttributes(new nsExposeColorAlphaAttribute()),
    NS_ACCESSOR_PROPERTY("CustomData", GetCustomData, SetCustomData)->AddAttributes(new nsDefaultValueAttribute(nsVec4(0, 1, 0, 1))),
    NS_ACCESSOR_PROPERTY("SortingDepthOffset", GetSortingDepthOffset, SetSortingDepthOffset),
    NS_MEMBER_PROPERTY("BoundsOffset", m_vBoundsOffset),
    NS_MEMBER_PROPERTY("BoundsRadius", m_fBoundsRadius)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.01f, 100.0f)),
    NS_ACCESSOR_PROPERTY("ShowDebugInfo", GetShowDebugInfo, SetShowDebugInfo),
    NS_ACCESSOR_PROPERTY("OverlapRanges", GetOverlapRanges, SetOverlapRanges)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_ARRAY_MEMBER_PROPERTY("Meshes", m_Meshes)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Mesh_Static")),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
    new nsSphereVisualizerAttribute("BoundsRadius", nsColor::MediumVioletRed, nullptr, nsVisualizerAnchor::Center, nsVec3(1.0f), "BoundsOffset"),
    new nsTransformManipulatorAttribute("BoundsOffset"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgSetColor, OnMsgSetColor),
    NS_MESSAGE_HANDLER(nsMsgSetCustomData, OnMsgSetCustomData),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_COMPONENT_TYPE;
// clang-format on

const char* nsLodMeshLod::GetMeshFile() const
{
  if (!m_hMesh.IsValid())
    return "";

  return m_hMesh.GetResourceID();
}

void nsLodMeshLod::SetMeshFile(const char* szFile)
{
  nsMeshResourceHandle hMesh;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hMesh = nsResourceManager::LoadResource<nsMeshResource>(szFile);
  }

  if (m_hMesh != hMesh)
  {
    m_hMesh = hMesh;

    // TriggerLocalBoundsUpdate();
    // InvalidateCachedRenderData();
  }
}

struct LodMeshCompFlags
{
  enum Enum
  {
    ShowDebugInfo = 0,
    OverlapRanges = 1,
  };
};

nsLodMeshComponent::nsLodMeshComponent() = default;
nsLodMeshComponent::~nsLodMeshComponent() = default;

void nsLodMeshComponent::SetShowDebugInfo(bool bShow)
{
  SetUserFlag(LodMeshCompFlags::ShowDebugInfo, bShow);
}

bool nsLodMeshComponent::GetShowDebugInfo() const
{
  return GetUserFlag(LodMeshCompFlags::ShowDebugInfo);
}

void nsLodMeshComponent::SetOverlapRanges(bool bShow)
{
  SetUserFlag(LodMeshCompFlags::OverlapRanges, bShow);
}

bool nsLodMeshComponent::GetOverlapRanges() const
{
  return GetUserFlag(LodMeshCompFlags::OverlapRanges);
}

void nsLodMeshComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s << m_Meshes.GetCount();
  for (const auto& mesh : m_Meshes)
  {
    s << mesh.m_hMesh;
    s << mesh.m_fThreshold;
  }

  s << m_Color;
  s << m_fSortingDepthOffset;

  s << m_vBoundsOffset;
  s << m_fBoundsRadius;

  s << m_vCustomData;
}

void nsLodMeshComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();

  nsUInt32 uiMeshes = 0;
  s >> uiMeshes;

  m_Meshes.SetCount(uiMeshes);

  for (auto& mesh : m_Meshes)
  {
    s >> mesh.m_hMesh;
    s >> mesh.m_fThreshold;
  }

  s >> m_Color;
  s >> m_fSortingDepthOffset;

  s >> m_vBoundsOffset;
  s >> m_fBoundsRadius;

  if (uiVersion >= 2)
  {
    s >> m_vCustomData;
  }
}

nsResult nsLodMeshComponent::GetLocalBounds(nsBoundingBoxSphere& out_bounds, bool& out_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  out_bounds = nsBoundingSphere::MakeFromCenterAndRadius(m_vBoundsOffset, m_fBoundsRadius);
  out_bAlwaysVisible = false;
  return NS_SUCCESS;
}

void nsLodMeshComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  if (m_Meshes.IsEmpty())
    return;

  if (msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::EditorView || msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::MainView)
  {
    UpdateSelectedLod(*msg.m_pView);
  }

  if (m_iCurLod >= m_Meshes.GetCount())
    return;

  auto hMesh = m_Meshes[m_iCurLod].m_hMesh;

  if (!hMesh.IsValid())
    return;

  nsResourceLock<nsMeshResource> pMesh(hMesh, nsResourceAcquireMode::AllowLoadingFallback);
  nsArrayPtr<const nsMeshResourceDescriptor::SubMesh> parts = pMesh->GetSubMeshes();

  for (nsUInt32 uiPartIndex = 0; uiPartIndex < parts.GetCount(); ++uiPartIndex)
  {
    const nsUInt32 uiMaterialIndex = parts[uiPartIndex].m_uiMaterialIndex;
    nsMaterialResourceHandle hMaterial;

    hMaterial = pMesh->GetMaterials()[uiMaterialIndex];

    nsMeshRenderData* pRenderData = CreateRenderData();
    {
      pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform() * pRenderData->m_GlobalTransform;
      pRenderData->m_GlobalBounds = GetOwner()->GetGlobalBounds();
      pRenderData->m_fSortingDepthOffset = m_fSortingDepthOffset;
      pRenderData->m_hMesh = hMesh;
      pRenderData->m_hMaterial = hMaterial;
      pRenderData->m_Color = m_Color;
      pRenderData->m_vCustomData = m_vCustomData;
      pRenderData->m_uiSubMeshIndex = uiPartIndex;
      pRenderData->m_uiUniqueID = GetUniqueIdForRendering(uiMaterialIndex);

      pRenderData->FillBatchIdAndSortingKey();
    }

    // Determine render data category.
    nsRenderData::Category category = nsDefaultRenderDataCategories::LitOpaque;
    if (hMaterial.IsValid())
    {
      nsResourceLock<nsMaterialResource> pMaterial(hMaterial, nsResourceAcquireMode::AllowLoadingFallback);

      category = pMaterial->GetRenderDataCategory();
    }

    msg.AddRenderData(pRenderData, category, nsRenderData::Caching::Never);
  }
}

void nsLodMeshComponent::SetColor(const nsColor& color)
{
  m_Color = color;

  InvalidateCachedRenderData();
}

const nsColor& nsLodMeshComponent::GetColor() const
{
  return m_Color;
}

void nsLodMeshComponent::SetCustomData(const nsVec4& vData)
{
  m_vCustomData = vData;

  InvalidateCachedRenderData();
}

const nsVec4& nsLodMeshComponent::GetCustomData() const
{
  return m_vCustomData;
}

void nsLodMeshComponent::SetSortingDepthOffset(float fOffset)
{
  m_fSortingDepthOffset = fOffset;

  InvalidateCachedRenderData();
}

float nsLodMeshComponent::GetSortingDepthOffset() const
{
  return m_fSortingDepthOffset;
}

void nsLodMeshComponent::OnMsgSetColor(nsMsgSetColor& ref_msg)
{
  ref_msg.ModifyColor(m_Color);

  InvalidateCachedRenderData();
}

void nsLodMeshComponent::OnMsgSetCustomData(nsMsgSetCustomData& ref_msg)
{
  m_vCustomData = ref_msg.m_vData;

  InvalidateCachedRenderData();
}

nsMeshRenderData* nsLodMeshComponent::CreateRenderData() const
{
  return nsCreateRenderDataForThisFrame<nsMeshRenderData>(GetOwner());
}

static float CalculateSphereScreenSpaceCoverage(const nsBoundingSphere& sphere, const nsCamera& camera)
{
  if (camera.IsPerspective())
  {
    return nsGraphicsUtils::CalculateSphereScreenCoverage(sphere, camera.GetCenterPosition(), camera.GetFovY(1.0f));
  }
  else
  {
    return nsGraphicsUtils::CalculateSphereScreenCoverage(sphere.m_fRadius, camera.GetDimensionY(1.0f));
  }
}

void nsLodMeshComponent::UpdateSelectedLod(const nsView& view) const
{
  const nsInt32 iNumLods = (nsInt32)m_Meshes.GetCount();

  const nsVec3 vScale = GetOwner()->GetGlobalScaling();
  const float fScale = nsMath::Max(vScale.x, vScale.y, vScale.z);
  const nsVec3 vCenter = GetOwner()->GetGlobalTransform() * m_vBoundsOffset;

  const float fCoverage = CalculateSphereScreenSpaceCoverage(nsBoundingSphere::MakeFromCenterAndRadius(vCenter, fScale * m_fBoundsRadius), *view.GetLodCamera());

  // clamp the input value, this is to prevent issues while editing the threshold array
  nsInt32 iNewLod = nsMath::Clamp<nsInt32>(m_iCurLod, 0, iNumLods);

  float fCoverageP = 1;
  float fCoverageN = 0;

  if (iNewLod > 0)
  {
    fCoverageP = m_Meshes[iNewLod - 1].m_fThreshold;
  }

  if (iNewLod < iNumLods)
  {
    fCoverageN = m_Meshes[iNewLod].m_fThreshold;
  }

  if (GetOverlapRanges())
  {
    const float fLodRangeOverlap = 0.40f;

    if (iNewLod + 1 < iNumLods)
    {
      float range = (fCoverageN - m_Meshes[iNewLod + 1].m_fThreshold);
      fCoverageN -= range * fLodRangeOverlap; // overlap into the next range
    }
    else
    {
      float range = (fCoverageN - 0.0f);
      fCoverageN -= range * fLodRangeOverlap; // overlap into the next range
    }
  }

  if (fCoverage < fCoverageN)
  {
    ++iNewLod;
  }
  else if (fCoverage > fCoverageP)
  {
    --iNewLod;
  }

  iNewLod = nsMath::Clamp(iNewLod, 0, iNumLods);
  m_iCurLod = iNewLod;

  if (GetShowDebugInfo())
  {
    nsStringBuilder sb;
    sb.SetFormat("Coverage: {}\nLOD {}\nRange: {} - {}", nsArgF(fCoverage, 3), iNewLod, nsArgF(fCoverageP, 3), nsArgF(fCoverageN, 3));
    nsDebugRenderer::Draw3DText(view.GetHandle(), sb, GetOwner()->GetGlobalPosition(), nsColor::White);
  }
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Meshes_Implementation_LodMeshComponent);
