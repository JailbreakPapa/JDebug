#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Camera.h>
#include <Core/Messages/TriggerMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Math/BoundingSphere.h>
#include <Foundation/Utilities/GraphicsUtils.h>
#include <RendererCore/Components/LodComponent.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/View.h>

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

struct LodCompFlags
{
  enum Enum
  {
    ShowDebugInfo = 0,
    OverlapRanges = 1,
  };
};

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsLodComponent, 1, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("BoundsOffset", m_vBoundsOffset),
    NS_MEMBER_PROPERTY("BoundsRadius", m_fBoundsRadius)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.01f, 100.0f)),
    NS_ACCESSOR_PROPERTY("ShowDebugInfo", GetShowDebugInfo, SetShowDebugInfo),
    NS_ACCESSOR_PROPERTY("OverlapRanges", GetOverlapRanges, SetOverlapRanges)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_ARRAY_MEMBER_PROPERTY("LodThresholds", m_LodThresholds)->AddAttributes(new nsMaxArraySizeAttribute(4), new nsClampValueAttribute(0.0f, 1.0f)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Construction"),
    new nsSphereVisualizerAttribute("BoundsRadius", nsColor::MediumVioletRed, nullptr, nsVisualizerAnchor::Center, nsVec3(1.0f), "BoundsOffset"),
    new nsTransformManipulatorAttribute("BoundsOffset"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgComponentInternalTrigger, OnMsgComponentInternalTrigger),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

static const nsTempHashedString sLod0("LOD0");
static const nsTempHashedString sLod1("LOD1");
static const nsTempHashedString sLod2("LOD2");
static const nsTempHashedString sLod3("LOD3");
static const nsTempHashedString sLod4("LOD4");

nsLodComponent::nsLodComponent()
{
  SetOverlapRanges(true);
}

nsLodComponent::~nsLodComponent() = default;

void nsLodComponent::SetShowDebugInfo(bool bShow)
{
  SetUserFlag(LodCompFlags::ShowDebugInfo, bShow);
}

bool nsLodComponent::GetShowDebugInfo() const
{
  return GetUserFlag(LodCompFlags::ShowDebugInfo);
}

void nsLodComponent::SetOverlapRanges(bool bShow)
{
  SetUserFlag(LodCompFlags::OverlapRanges, bShow);
}

bool nsLodComponent::GetOverlapRanges() const
{
  return GetUserFlag(LodCompFlags::OverlapRanges);
}

void nsLodComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s << m_vBoundsOffset;
  s << m_fBoundsRadius;

  s.WriteArray(m_LodThresholds).AssertSuccess();
}

void nsLodComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  auto& s = inout_stream.GetStream();

  s >> m_vBoundsOffset;
  s >> m_fBoundsRadius;

  s.ReadArray(m_LodThresholds).AssertSuccess();
}

nsResult nsLodComponent::GetLocalBounds(nsBoundingBoxSphere& out_bounds, bool& out_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  out_bounds = nsBoundingSphere::MakeFromCenterAndRadius(m_vBoundsOffset, m_fBoundsRadius);
  out_bAlwaysVisible = false;
  return NS_SUCCESS;
}

void nsLodComponent::OnActivated()
{
  SUPER::OnActivated();

  // start with the highest LOD (lowest detail)
  m_iCurLod = m_LodThresholds.GetCount();

  nsMsgComponentInternalTrigger trig;
  trig.m_iPayload = m_iCurLod;
  OnMsgComponentInternalTrigger(trig);
}

void nsLodComponent::OnDeactivated()
{
  // when the component gets deactivated, activate all LOD children
  // this is important for editing to not behave weirdly
  // not sure whether this can have unintended side-effects at runtime
  // but there this should only be called for objects that get deleted anyway

  nsGameObject* pLod[5];
  pLod[0] = GetOwner()->FindChildByName(sLod0);
  pLod[1] = GetOwner()->FindChildByName(sLod1);
  pLod[2] = GetOwner()->FindChildByName(sLod2);
  pLod[3] = GetOwner()->FindChildByName(sLod3);
  pLod[4] = GetOwner()->FindChildByName(sLod4);

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(pLod); ++i)
  {
    if (pLod[i])
    {
      pLod[i]->SetActiveFlag(true);
    }
  }

  SUPER::OnDeactivated();
}

void nsLodComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  if (msg.m_pView->GetCameraUsageHint() != nsCameraUsageHint::EditorView &&
      msg.m_pView->GetCameraUsageHint() != nsCameraUsageHint::MainView)
  {
    return;
  }

  // Don't extract render data for selection.
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory)
    return;

  const nsInt32 iNumLods = (nsInt32)m_LodThresholds.GetCount();

  const nsVec3 vScale = GetOwner()->GetGlobalScaling();
  const float fScale = nsMath::Max(vScale.x, vScale.y, vScale.z);
  const nsVec3 vCenter = GetOwner()->GetGlobalTransform() * m_vBoundsOffset;

  const float fCoverage = CalculateSphereScreenSpaceCoverage(nsBoundingSphere::MakeFromCenterAndRadius(vCenter, fScale * m_fBoundsRadius), *msg.m_pView->GetCullingCamera());

  // clamp the input value, this is to prevent issues while editing the threshold array
  nsInt32 iNewLod = nsMath::Clamp<nsInt32>(m_iCurLod, 0, iNumLods);

  float fCoverageP = 1;
  float fCoverageN = 0;

  if (iNewLod > 0)
  {
    fCoverageP = m_LodThresholds[iNewLod - 1];
  }

  if (iNewLod < iNumLods)
  {
    fCoverageN = m_LodThresholds[iNewLod];
  }

  if (GetOverlapRanges())
  {
    const float fLodRangeOverlap = 0.40f;

    if (iNewLod + 1 < iNumLods)
    {
      float range = (fCoverageN - m_LodThresholds[iNewLod + 1]);
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

  if (GetShowDebugInfo())
  {
    nsStringBuilder sb;
    sb.SetFormat("Coverage: {}\nLOD {}\nRange: {} - {}", nsArgF(fCoverage, 3), iNewLod, nsArgF(fCoverageP, 3), nsArgF(fCoverageN, 3));
    nsDebugRenderer::Draw3DText(msg.m_pView->GetHandle(), sb, GetOwner()->GetGlobalPosition(), nsColor::White);
  }

  if (iNewLod == m_iCurLod)
    return;

  nsMsgComponentInternalTrigger trig;
  trig.m_iPayload = iNewLod;

  PostMessage(trig);
}

void nsLodComponent::OnMsgComponentInternalTrigger(nsMsgComponentInternalTrigger& msg)
{
  m_iCurLod = msg.m_iPayload;

  // search for direct children named LODn, don't waste performance searching recursively
  nsGameObject* pLod[5];
  pLod[0] = GetOwner()->FindChildByName(sLod0, false);
  pLod[1] = GetOwner()->FindChildByName(sLod1, false);
  pLod[2] = GetOwner()->FindChildByName(sLod2, false);
  pLod[3] = GetOwner()->FindChildByName(sLod3, false);
  pLod[4] = GetOwner()->FindChildByName(sLod4, false);

  // activate the selected LOD, deactivate all others
  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(pLod); ++i)
  {
    if (pLod[i])
    {
      pLod[i]->SetActiveFlag(m_iCurLod == i);
    }
  }
}


NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_LodComponent);
