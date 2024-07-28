#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Lights/ReflectionProbeComponentBase.h>

#include <Core/Graphics/Camera.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/View.h>

namespace
{
  static nsVariantArray GetDefaultExcludeTags()
  {
    nsVariantArray value(nsStaticsAllocatorWrapper::GetAllocator());
    value.PushBack(nsStringView("SkyLight"));
    return value;
  }
} // namespace


// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsReflectionProbeComponentBase, 2, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ENUM_ACCESSOR_PROPERTY("ReflectionProbeMode", nsReflectionProbeMode, GetReflectionProbeMode, SetReflectionProbeMode)->AddAttributes(new nsDefaultValueAttribute(nsReflectionProbeMode::Static), new nsGroupAttribute("Capture Description")),
    NS_SET_ACCESSOR_PROPERTY("IncludeTags", GetIncludeTags, InsertIncludeTag, RemoveIncludeTag)->AddAttributes(new nsTagSetWidgetAttribute("Default")),
    NS_SET_ACCESSOR_PROPERTY("ExcludeTags", GetExcludeTags, InsertExcludeTag, RemoveExcludeTag)->AddAttributes(new nsTagSetWidgetAttribute("Default"), new nsDefaultValueAttribute(GetDefaultExcludeTags())),
    NS_ACCESSOR_PROPERTY("NearPlane", GetNearPlane, SetNearPlane)->AddAttributes(new nsDefaultValueAttribute(0.0f), new nsClampValueAttribute(0.0f, {}), new nsMinValueTextAttribute("Auto")),
    NS_ACCESSOR_PROPERTY("FarPlane", GetFarPlane, SetFarPlane)->AddAttributes(new nsDefaultValueAttribute(100.0f), new nsClampValueAttribute(0.01f, 10000.0f)),
    NS_ACCESSOR_PROPERTY("CaptureOffset", GetCaptureOffset, SetCaptureOffset),
    NS_ACCESSOR_PROPERTY("ShowDebugInfo", GetShowDebugInfo, SetShowDebugInfo),
    NS_ACCESSOR_PROPERTY("ShowMipMaps", GetShowMipMaps, SetShowMipMaps),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering/Reflections"),
    new nsTransformManipulatorAttribute("CaptureOffset"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsReflectionProbeComponentBase::nsReflectionProbeComponentBase()
{
  m_Desc.m_uniqueID = nsUuid::MakeUuid();
}

nsReflectionProbeComponentBase::~nsReflectionProbeComponentBase() = default;

void nsReflectionProbeComponentBase::SetReflectionProbeMode(nsEnum<nsReflectionProbeMode> mode)
{
  m_Desc.m_Mode = mode;
  m_bStatesDirty = true;
}

nsEnum<nsReflectionProbeMode> nsReflectionProbeComponentBase::GetReflectionProbeMode() const
{
  return m_Desc.m_Mode;
}

const nsTagSet& nsReflectionProbeComponentBase::GetIncludeTags() const
{
  return m_Desc.m_IncludeTags;
}

void nsReflectionProbeComponentBase::InsertIncludeTag(const char* szTag)
{
  m_Desc.m_IncludeTags.SetByName(szTag);
  m_bStatesDirty = true;
}

void nsReflectionProbeComponentBase::RemoveIncludeTag(const char* szTag)
{
  m_Desc.m_IncludeTags.RemoveByName(szTag);
  m_bStatesDirty = true;
}


const nsTagSet& nsReflectionProbeComponentBase::GetExcludeTags() const
{
  return m_Desc.m_ExcludeTags;
}

void nsReflectionProbeComponentBase::InsertExcludeTag(const char* szTag)
{
  m_Desc.m_ExcludeTags.SetByName(szTag);
  m_bStatesDirty = true;
}

void nsReflectionProbeComponentBase::RemoveExcludeTag(const char* szTag)
{
  m_Desc.m_ExcludeTags.RemoveByName(szTag);
  m_bStatesDirty = true;
}

void nsReflectionProbeComponentBase::SetNearPlane(float fNearPlane)
{
  m_Desc.m_fNearPlane = fNearPlane;
  m_bStatesDirty = true;
}

void nsReflectionProbeComponentBase::SetFarPlane(float fFarPlane)
{
  m_Desc.m_fFarPlane = fFarPlane;
  m_bStatesDirty = true;
}

void nsReflectionProbeComponentBase::SetCaptureOffset(const nsVec3& vOffset)
{
  m_Desc.m_vCaptureOffset = vOffset;
  m_bStatesDirty = true;
}

void nsReflectionProbeComponentBase::SetShowDebugInfo(bool bShowDebugInfo)
{
  m_Desc.m_bShowDebugInfo = bShowDebugInfo;
  m_bStatesDirty = true;
}

bool nsReflectionProbeComponentBase::GetShowDebugInfo() const
{
  return m_Desc.m_bShowDebugInfo;
}

void nsReflectionProbeComponentBase::SetShowMipMaps(bool bShowMipMaps)
{
  m_Desc.m_bShowMipMaps = bShowMipMaps;
  m_bStatesDirty = true;
}

bool nsReflectionProbeComponentBase::GetShowMipMaps() const
{
  return m_Desc.m_bShowMipMaps;
}

void nsReflectionProbeComponentBase::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);

  nsStreamWriter& s = inout_stream.GetStream();

  m_Desc.m_IncludeTags.Save(s);
  m_Desc.m_ExcludeTags.Save(s);
  s << m_Desc.m_Mode;
  s << m_Desc.m_bShowDebugInfo;
  s << m_Desc.m_uniqueID;
  s << m_Desc.m_fNearPlane;
  s << m_Desc.m_fFarPlane;
  s << m_Desc.m_vCaptureOffset;
}

void nsReflectionProbeComponentBase::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  // const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());
  nsStreamReader& s = inout_stream.GetStream();

  m_Desc.m_IncludeTags.Load(s, nsTagRegistry::GetGlobalRegistry());
  m_Desc.m_ExcludeTags.Load(s, nsTagRegistry::GetGlobalRegistry());
  s >> m_Desc.m_Mode;
  s >> m_Desc.m_bShowDebugInfo;
  s >> m_Desc.m_uniqueID;
  s >> m_Desc.m_fNearPlane;
  s >> m_Desc.m_fFarPlane;
  s >> m_Desc.m_vCaptureOffset;
}

float nsReflectionProbeComponentBase::ComputePriority(nsMsgExtractRenderData& msg, nsReflectionProbeRenderData* pRenderData, float fVolume, const nsVec3& vScale) const
{
  float fPriority = 0.0f;
  const float fLogVolume = nsMath::Log2(1.0f + fVolume); // +1 to make sure it never goes negative.
  // This sorting is only by size to make sure the probes in a cluster are iterating from smallest to largest on the GPU. Which probes are actually used is determined below by the returned priority.
  pRenderData->m_uiSortingKey = nsMath::FloatToInt(static_cast<float>(nsMath::MaxValue<nsUInt32>()) * fLogVolume / 40.0f);

  // #TODO This is a pretty poor distance / size based score.
  if (msg.m_pView)
  {
    if (auto pCamera = msg.m_pView->GetLodCamera())
    {
      float fDistance = (pCamera->GetPosition() - pRenderData->m_GlobalTransform.m_vPosition).GetLength();
      float fRadius = (nsMath::Abs(vScale.x) + nsMath::Abs(vScale.y) + nsMath::Abs(vScale.z)) / 3.0f;
      fPriority = fRadius / fDistance;
    }
  }

#ifdef NS_SHOW_REFLECTION_PROBE_PRIORITIES
  nsStringBuilder s;
  s.SetFormat("{}, {}", pRenderData->m_uiSortingKey, fPriority);
  nsDebugRenderer::Draw3DText(GetWorld(), s, pRenderData->m_GlobalTransform.m_vPosition, nsColor::Wheat);
#endif
  return fPriority;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Lights_Implementation_ReflectionProbeComponentBase);
