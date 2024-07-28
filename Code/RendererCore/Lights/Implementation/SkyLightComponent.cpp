#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/TransformChangedMessage.h>
#include <Core/Messages/UpdateLocalBoundsMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <RendererCore/Lights/Implementation/ReflectionPool.h>
#include <RendererCore/Lights/SkyLightComponent.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Textures/TextureCubeResource.h>

namespace
{
  static nsVariantArray GetDefaultTags()
  {
    nsVariantArray value(nsStaticsAllocatorWrapper::GetAllocator());
    value.PushBack("SkyLight");
    return value;
  }
} // namespace

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsSkyLightComponent, 3, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_ACCESSOR_PROPERTY("ReflectionProbeMode", nsReflectionProbeMode, GetReflectionProbeMode, SetReflectionProbeMode)->AddAttributes(new nsDefaultValueAttribute(nsReflectionProbeMode::Dynamic), new nsGroupAttribute("Capture Description")),
    NS_ACCESSOR_PROPERTY("CubeMap", GetCubeMapFile, SetCubeMapFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_Cube")),
    NS_ACCESSOR_PROPERTY("Intensity", GetIntensity, SetIntensity)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
    NS_ACCESSOR_PROPERTY("Saturation", GetSaturation, SetSaturation)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
    NS_SET_ACCESSOR_PROPERTY("IncludeTags", GetIncludeTags, InsertIncludeTag, RemoveIncludeTag)->AddAttributes(new nsTagSetWidgetAttribute("Default"), new nsDefaultValueAttribute(GetDefaultTags())),
    NS_SET_ACCESSOR_PROPERTY("ExcludeTags", GetExcludeTags, InsertExcludeTag, RemoveExcludeTag)->AddAttributes(new nsTagSetWidgetAttribute("Default")),
    NS_ACCESSOR_PROPERTY("NearPlane", GetNearPlane, SetNearPlane)->AddAttributes(new nsDefaultValueAttribute(0.0f), new nsClampValueAttribute(0.0f, {}), new nsMinValueTextAttribute("Auto")),
    NS_ACCESSOR_PROPERTY("FarPlane", GetFarPlane, SetFarPlane)->AddAttributes(new nsDefaultValueAttribute(100.0f), new nsClampValueAttribute(0.01f, 10000.0f)),
    NS_ACCESSOR_PROPERTY("ShowDebugInfo", GetShowDebugInfo, SetShowDebugInfo),
    NS_ACCESSOR_PROPERTY("ShowMipMaps", GetShowMipMaps, SetShowMipMaps),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgUpdateLocalBounds, OnUpdateLocalBounds),
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgTransformChanged, OnTransformChanged),
  }
  NS_END_MESSAGEHANDLERS;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Lighting"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsSkyLightComponent::nsSkyLightComponent()
{
  m_Desc.m_uniqueID = nsUuid::MakeUuid();
}

nsSkyLightComponent::~nsSkyLightComponent() = default;

void nsSkyLightComponent::OnActivated()
{
  GetOwner()->EnableStaticTransformChangesNotifications();
  m_Id = nsReflectionPool::RegisterSkyLight(GetWorld(), m_Desc, this);

  GetOwner()->UpdateLocalBounds();
}

void nsSkyLightComponent::OnDeactivated()
{
  nsReflectionPool::DeregisterSkyLight(GetWorld(), m_Id);
  m_Id.Invalidate();

  GetOwner()->UpdateLocalBounds();
}

void nsSkyLightComponent::SetReflectionProbeMode(nsEnum<nsReflectionProbeMode> mode)
{
  m_Desc.m_Mode = mode;
  m_bStatesDirty = true;
}

nsEnum<nsReflectionProbeMode> nsSkyLightComponent::GetReflectionProbeMode() const
{
  return m_Desc.m_Mode;
}

void nsSkyLightComponent::SetIntensity(float fIntensity)
{
  m_Desc.m_fIntensity = fIntensity;
  m_bStatesDirty = true;
}

float nsSkyLightComponent::GetIntensity() const
{
  return m_Desc.m_fIntensity;
}

void nsSkyLightComponent::SetSaturation(float fSaturation)
{
  m_Desc.m_fSaturation = fSaturation;
  m_bStatesDirty = true;
}

float nsSkyLightComponent::GetSaturation() const
{
  return m_Desc.m_fSaturation;
}

const nsTagSet& nsSkyLightComponent::GetIncludeTags() const
{
  return m_Desc.m_IncludeTags;
}

void nsSkyLightComponent::InsertIncludeTag(const char* szTag)
{
  m_Desc.m_IncludeTags.SetByName(szTag);
  m_bStatesDirty = true;
}

void nsSkyLightComponent::RemoveIncludeTag(const char* szTag)
{
  m_Desc.m_IncludeTags.RemoveByName(szTag);
  m_bStatesDirty = true;
}

const nsTagSet& nsSkyLightComponent::GetExcludeTags() const
{
  return m_Desc.m_ExcludeTags;
}

void nsSkyLightComponent::InsertExcludeTag(const char* szTag)
{
  m_Desc.m_ExcludeTags.SetByName(szTag);
  m_bStatesDirty = true;
}

void nsSkyLightComponent::RemoveExcludeTag(const char* szTag)
{
  m_Desc.m_ExcludeTags.RemoveByName(szTag);
  m_bStatesDirty = true;
}

void nsSkyLightComponent::SetShowDebugInfo(bool bShowDebugInfo)
{
  m_Desc.m_bShowDebugInfo = bShowDebugInfo;
  m_bStatesDirty = true;
}

bool nsSkyLightComponent::GetShowDebugInfo() const
{
  return m_Desc.m_bShowDebugInfo;
}

void nsSkyLightComponent::SetShowMipMaps(bool bShowMipMaps)
{
  m_Desc.m_bShowMipMaps = bShowMipMaps;
  m_bStatesDirty = true;
}

bool nsSkyLightComponent::GetShowMipMaps() const
{
  return m_Desc.m_bShowMipMaps;
}


void nsSkyLightComponent::SetCubeMapFile(const char* szFile)
{
  nsTextureCubeResourceHandle hCubeMap;
  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hCubeMap = nsResourceManager::LoadResource<nsTextureCubeResource>(szFile);
  }
  m_hCubeMap = hCubeMap;
  m_bStatesDirty = true;
}

const char* nsSkyLightComponent::GetCubeMapFile() const
{
  return m_hCubeMap.IsValid() ? m_hCubeMap.GetResourceID().GetData() : "";
}

void nsSkyLightComponent::SetNearPlane(float fNearPlane)
{
  m_Desc.m_fNearPlane = fNearPlane;
  m_bStatesDirty = true;
}

void nsSkyLightComponent::SetFarPlane(float fFarPlane)
{
  m_Desc.m_fFarPlane = fFarPlane;
  m_bStatesDirty = true;
}

void nsSkyLightComponent::OnUpdateLocalBounds(nsMsgUpdateLocalBounds& msg)
{
  msg.SetAlwaysVisible(GetOwner()->IsDynamic() ? nsDefaultSpatialDataCategories::RenderDynamic : nsDefaultSpatialDataCategories::RenderStatic);
}

void nsSkyLightComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't trigger reflection rendering in shadow or other reflection views.
  if (msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Shadow || msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Reflection)
    return;

  if (m_bStatesDirty)
  {
    m_bStatesDirty = false;
    nsReflectionPool::UpdateSkyLight(GetWorld(), m_Id, m_Desc, this);
  }

  nsReflectionPool::ExtractReflectionProbe(this, msg, nullptr, GetWorld(), m_Id, nsMath::MaxValue<float>());
}

void nsSkyLightComponent::OnTransformChanged(nsMsgTransformChanged& msg)
{
  m_bStatesDirty = true;
}

void nsSkyLightComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  m_Desc.m_IncludeTags.Save(s);
  m_Desc.m_ExcludeTags.Save(s);
  s << m_Desc.m_Mode;
  s << m_Desc.m_bShowDebugInfo;
  s << m_Desc.m_fIntensity;
  s << m_Desc.m_fSaturation;
  s << m_hCubeMap;
  s << m_Desc.m_fNearPlane;
  s << m_Desc.m_fFarPlane;
}

void nsSkyLightComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  m_Desc.m_IncludeTags.Load(s, nsTagRegistry::GetGlobalRegistry());
  m_Desc.m_ExcludeTags.Load(s, nsTagRegistry::GetGlobalRegistry());
  s >> m_Desc.m_Mode;
  s >> m_Desc.m_bShowDebugInfo;
  s >> m_Desc.m_fIntensity;
  s >> m_Desc.m_fSaturation;
  if (uiVersion >= 2)
  {
    s >> m_hCubeMap;
  }
  if (uiVersion >= 3)
  {
    s >> m_Desc.m_fNearPlane;
    s >> m_Desc.m_fFarPlane;
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/GraphPatch.h>

class nsSkyLightComponentPatch_2_3 : public nsGraphPatch
{
public:
  nsSkyLightComponentPatch_2_3()
    : nsGraphPatch("nsSkyLightComponent", 3)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    // Inline ReflectionData sub-object into the sky light itself.
    if (const nsAbstractObjectNode::Property* pProp0 = pNode->FindProperty("ReflectionData"))
    {
      if (pProp0->m_Value.IsA<nsUuid>())
      {
        if (nsAbstractObjectNode* pSubNode = pGraph->GetNode(pProp0->m_Value.Get<nsUuid>()))
        {
          for (auto pProp : pSubNode->GetProperties())
          {
            pNode->AddProperty(pProp.m_sPropertyName, pProp.m_Value);
          }
        }
      }
    }
  }
};

nsSkyLightComponentPatch_2_3 g_nsSkyLightComponentPatch_2_3;

NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_SkyLightComponent);
