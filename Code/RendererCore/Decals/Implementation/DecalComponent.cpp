#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Camera.h>
#include <Core/Messages/ApplyOnlyToMessage.h>
#include <Core/Messages/DeleteObjectMessage.h>
#include <Core/Messages/SetColorMessage.h>
#include <Core/Messages/TriggerMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <RendererCore/Decals/DecalAtlasResource.h>
#include <RendererCore/Decals/DecalComponent.h>
#include <RendererCore/Decals/DecalResource.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/Shader/ShaderUtils.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/LightData.h>

nsDecalComponentManager::nsDecalComponentManager(nsWorld* pWorld)
  : nsComponentManager<nsDecalComponent, nsBlockStorageType::Compact>(pWorld)
{
}

void nsDecalComponentManager::Initialize()
{
  m_hDecalAtlas = nsDecalAtlasResource::GetDecalAtlasResource();
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDecalRenderData, 1, nsRTTIDefaultAllocator<nsDecalRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_COMPONENT_TYPE(nsDecalComponent, 8, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ARRAY_ACCESSOR_PROPERTY("Decals", DecalFile_GetCount, DecalFile_Get, DecalFile_Set, DecalFile_Insert, DecalFile_Remove)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Decal")),
    NS_ENUM_ACCESSOR_PROPERTY("ProjectionAxis", nsBasisAxis, GetProjectionAxis, SetProjectionAxis),
    NS_ACCESSOR_PROPERTY("Extents", GetExtents, SetExtents)->AddAttributes(new nsDefaultValueAttribute(nsVec3(1.0f)), new nsClampValueAttribute(nsVec3(0.01f), nsVariant(25.0f))),
    NS_ACCESSOR_PROPERTY("SizeVariance", GetSizeVariance, SetSizeVariance)->AddAttributes(new nsClampValueAttribute(0.0f, 1.0f)),
    NS_ACCESSOR_PROPERTY("Color", GetColor, SetColor)->AddAttributes(new nsExposeColorAlphaAttribute()),
    NS_ACCESSOR_PROPERTY("EmissiveColor", GetEmissiveColor, SetEmissiveColor)->AddAttributes(new nsDefaultValueAttribute(nsColor::Black)),
    NS_ACCESSOR_PROPERTY("SortOrder", GetSortOrder, SetSortOrder)->AddAttributes(new nsClampValueAttribute(-64.0f, 64.0f)),
    NS_ACCESSOR_PROPERTY("WrapAround", GetWrapAround, SetWrapAround),
    NS_ACCESSOR_PROPERTY("MapNormalToGeometry", GetMapNormalToGeometry, SetMapNormalToGeometry)->AddAttributes(new nsDefaultValueAttribute(true)),
    NS_ACCESSOR_PROPERTY("InnerFadeAngle", GetInnerFadeAngle, SetInnerFadeAngle)->AddAttributes(new nsClampValueAttribute(nsAngle::MakeFromDegree(0.0f), nsAngle::MakeFromDegree(89.0f)), new nsDefaultValueAttribute(nsAngle::MakeFromDegree(50.0f))),
    NS_ACCESSOR_PROPERTY("OuterFadeAngle", GetOuterFadeAngle, SetOuterFadeAngle)->AddAttributes(new nsClampValueAttribute(nsAngle::MakeFromDegree(0.0f), nsAngle::MakeFromDegree(89.0f)), new nsDefaultValueAttribute(nsAngle::MakeFromDegree(80.0f))),
    NS_MEMBER_PROPERTY("FadeOutDelay", m_FadeOutDelay),
    NS_MEMBER_PROPERTY("FadeOutDuration", m_FadeOutDuration),
    NS_ENUM_MEMBER_PROPERTY("OnFinishedAction", nsOnComponentFinishedAction, m_OnFinishedAction),
    NS_ACCESSOR_PROPERTY("ApplyToDynamic", DummyGetter, SetApplyToRef)->AddAttributes(new nsGameObjectReferenceAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Effects"),
    new nsDirectionVisualizerAttribute("ProjectionAxis", 0.5f, nsColorScheme::LightUI(nsColorScheme::Blue)),
    new nsBoxManipulatorAttribute("Extents", 1.0f, true),
    new nsBoxVisualizerAttribute("Extents"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgComponentInternalTrigger, OnTriggered),
    NS_MESSAGE_HANDLER(nsMsgDeleteGameObject, OnMsgDeleteGameObject),
    NS_MESSAGE_HANDLER(nsMsgOnlyApplyToObject, OnMsgOnlyApplyToObject),
    NS_MESSAGE_HANDLER(nsMsgSetColor, OnMsgSetColor),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_COMPONENT_TYPE
// clang-format on

nsDecalComponent::nsDecalComponent() = default;

nsDecalComponent::~nsDecalComponent() = default;

void nsDecalComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s << m_vExtents;
  s << m_Color;
  s << m_EmissiveColor;
  s << m_InnerFadeAngle;
  s << m_OuterFadeAngle;
  s << m_fSortOrder;
  s << m_FadeOutDelay.m_Value;
  s << m_FadeOutDelay.m_fVariance;
  s << m_FadeOutDuration;
  s << m_StartFadeOutTime;
  s << m_fSizeVariance;
  s << m_OnFinishedAction;
  s << m_bWrapAround;
  s << m_bMapNormalToGeometry;

  // version 5
  s << m_ProjectionAxis;

  // version 6
  inout_stream.WriteGameObjectHandle(m_hApplyOnlyToObject);

  // version 7
  s << m_uiRandomDecalIdx;
  s.WriteArray(m_Decals).IgnoreResult();
}

void nsDecalComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();

  s >> m_vExtents;

  if (uiVersion >= 4)
  {
    s >> m_Color;
    s >> m_EmissiveColor;
  }
  else
  {
    nsColor tmp;
    s >> tmp;
    m_Color = tmp;
  }

  s >> m_InnerFadeAngle;
  s >> m_OuterFadeAngle;
  s >> m_fSortOrder;

  if (uiVersion <= 7)
  {
    nsUInt32 dummy;
    s >> dummy;
  }

  m_uiInternalSortKey = GetOwner()->GetStableRandomSeed();
  m_uiInternalSortKey = (m_uiInternalSortKey >> 16) ^ (m_uiInternalSortKey & 0xFFFF);

  if (uiVersion < 7)
  {
    m_Decals.SetCount(1);
    s >> m_Decals[0];
  }

  s >> m_FadeOutDelay.m_Value;
  s >> m_FadeOutDelay.m_fVariance;
  s >> m_FadeOutDuration;
  s >> m_StartFadeOutTime;
  s >> m_fSizeVariance;
  s >> m_OnFinishedAction;

  if (uiVersion >= 3)
  {
    s >> m_bWrapAround;
  }

  if (uiVersion >= 4)
  {
    s >> m_bMapNormalToGeometry;
  }

  if (uiVersion >= 5)
  {
    s >> m_ProjectionAxis;
  }

  if (uiVersion >= 6)
  {
    SetApplyOnlyTo(inout_stream.ReadGameObjectHandle());
  }

  if (uiVersion >= 7)
  {
    s >> m_uiRandomDecalIdx;
    s.ReadArray(m_Decals).IgnoreResult();
  }
}

nsResult nsDecalComponent::GetLocalBounds(nsBoundingBoxSphere& bounds, bool& bAlwaysVisible, nsMsgUpdateLocalBounds& msg)
{
  if (m_Decals.IsEmpty())
    return NS_FAILURE;

  m_uiRandomDecalIdx = (GetOwner()->GetStableRandomSeed() % m_Decals.GetCount()) & 0xFF;

  const nsUInt32 uiDecalIndex = nsMath::Min<nsUInt32>(m_uiRandomDecalIdx, m_Decals.GetCount() - 1);

  if (!m_Decals[uiDecalIndex].IsValid() || m_vExtents.IsZero())
    return NS_FAILURE;

  float fAspectRatio = 1.0f;

  {
    auto hDecalAtlas = GetWorld()->GetComponentManager<nsDecalComponentManager>()->m_hDecalAtlas;
    nsResourceLock<nsDecalAtlasResource> pDecalAtlas(hDecalAtlas, nsResourceAcquireMode::BlockTillLoaded);

    const auto& atlas = pDecalAtlas->GetAtlas();
    const nsUInt32 decalIdx = atlas.m_Items.Find(nsHashingUtils::StringHashTo32(m_Decals[uiDecalIndex].GetResourceIDHash()));

    if (decalIdx != nsInvalidIndex)
    {
      const auto& item = atlas.m_Items.GetValue(decalIdx);
      fAspectRatio = (float)item.m_LayerRects[0].width / item.m_LayerRects[0].height;
    }
  }

  nsVec3 vAspectCorrection = nsVec3(1.0f);
  if (!nsMath::IsEqual(fAspectRatio, 1.0f, 0.001f))
  {
    if (fAspectRatio > 1.0f)
    {
      vAspectCorrection.z /= fAspectRatio;
    }
    else
    {
      vAspectCorrection.y *= fAspectRatio;
    }
  }

  const nsQuat axisRotation = nsBasisAxis::GetBasisRotation_PosX(m_ProjectionAxis);
  nsVec3 vHalfExtents = (axisRotation * vAspectCorrection).Abs().CompMul(m_vExtents * 0.5f);

  bounds = nsBoundingBoxSphere::MakeFromBox(nsBoundingBox::MakeFromMinMax(-vHalfExtents, vHalfExtents));
  return NS_SUCCESS;
}

void nsDecalComponent::SetExtents(const nsVec3& value)
{
  m_vExtents = value.CompMax(nsVec3::MakeZero());

  TriggerLocalBoundsUpdate();
}

const nsVec3& nsDecalComponent::GetExtents() const
{
  return m_vExtents;
}

void nsDecalComponent::SetSizeVariance(float fVariance)
{
  m_fSizeVariance = nsMath::Clamp(fVariance, 0.0f, 1.0f);
}

float nsDecalComponent::GetSizeVariance() const
{
  return m_fSizeVariance;
}

void nsDecalComponent::SetColor(nsColorGammaUB color)
{
  m_Color = color;
}

nsColorGammaUB nsDecalComponent::GetColor() const
{
  return m_Color;
}

void nsDecalComponent::SetEmissiveColor(nsColor color)
{
  m_EmissiveColor = color;
}

nsColor nsDecalComponent::GetEmissiveColor() const
{
  return m_EmissiveColor;
}

void nsDecalComponent::SetInnerFadeAngle(nsAngle spotAngle)
{
  m_InnerFadeAngle = nsMath::Clamp(spotAngle, nsAngle::MakeFromDegree(0.0f), m_OuterFadeAngle);
}

nsAngle nsDecalComponent::GetInnerFadeAngle() const
{
  return m_InnerFadeAngle;
}

void nsDecalComponent::SetOuterFadeAngle(nsAngle spotAngle)
{
  m_OuterFadeAngle = nsMath::Clamp(spotAngle, m_InnerFadeAngle, nsAngle::MakeFromDegree(90.0f));
}

nsAngle nsDecalComponent::GetOuterFadeAngle() const
{
  return m_OuterFadeAngle;
}

void nsDecalComponent::SetSortOrder(float fOrder)
{
  m_fSortOrder = fOrder;
}

float nsDecalComponent::GetSortOrder() const
{
  return m_fSortOrder;
}

void nsDecalComponent::SetWrapAround(bool bWrapAround)
{
  m_bWrapAround = bWrapAround;
}

bool nsDecalComponent::GetWrapAround() const
{
  return m_bWrapAround;
}

void nsDecalComponent::SetMapNormalToGeometry(bool bMapNormal)
{
  m_bMapNormalToGeometry = bMapNormal;
}

bool nsDecalComponent::GetMapNormalToGeometry() const
{
  return m_bMapNormalToGeometry;
}

void nsDecalComponent::SetDecal(nsUInt32 uiIndex, const nsDecalResourceHandle& hDecal)
{
  m_Decals.EnsureCount(uiIndex + 1);
  m_Decals[uiIndex] = hDecal;

  TriggerLocalBoundsUpdate();
}

const nsDecalResourceHandle& nsDecalComponent::GetDecal(nsUInt32 uiIndex) const
{
  return m_Decals[uiIndex];
}

void nsDecalComponent::SetProjectionAxis(nsEnum<nsBasisAxis> projectionAxis)
{
  m_ProjectionAxis = projectionAxis;

  TriggerLocalBoundsUpdate();
}

nsEnum<nsBasisAxis> nsDecalComponent::GetProjectionAxis() const
{
  return m_ProjectionAxis;
}

void nsDecalComponent::SetApplyOnlyTo(nsGameObjectHandle hObject)
{
  if (m_hApplyOnlyToObject != hObject)
  {
    m_hApplyOnlyToObject = hObject;
    UpdateApplyTo();
  }
}

nsGameObjectHandle nsDecalComponent::GetApplyOnlyTo() const
{
  return m_hApplyOnlyToObject;
}

void nsDecalComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't extract decal render data for selection.
  if (msg.m_OverrideCategory != nsInvalidRenderDataCategory)
    return;

  if (m_Decals.IsEmpty())
    return;

  const nsUInt32 uiDecalIndex = nsMath::Min<nsUInt32>(m_uiRandomDecalIdx, m_Decals.GetCount() - 1);

  if (!m_Decals[uiDecalIndex].IsValid() || m_vExtents.IsZero() || GetOwner()->GetLocalScaling().IsZero())
    return;

  float fFade = 1.0f;

  const nsTime tNow = GetWorld()->GetClock().GetAccumulatedTime();
  if (tNow > m_StartFadeOutTime)
  {
    fFade -= nsMath::Min<float>(1.0f, (float)((tNow - m_StartFadeOutTime).GetSeconds() / m_FadeOutDuration.GetSeconds()));
  }

  nsColor finalColor = m_Color;
  finalColor.a *= fFade;

  if (finalColor.a <= 0.0f)
    return;

  const bool bNoFade = m_InnerFadeAngle == nsAngle::MakeFromRadian(0.0f) && m_OuterFadeAngle == nsAngle::MakeFromRadian(0.0f);
  const float fCosInner = nsMath::Cos(m_InnerFadeAngle);
  const float fCosOuter = nsMath::Cos(m_OuterFadeAngle);
  const float fFadeParamScale = bNoFade ? 0.0f : (1.0f / nsMath::Max(0.001f, (fCosInner - fCosOuter)));
  const float fFadeParamOffset = bNoFade ? 1.0f : (-fCosOuter * fFadeParamScale);

  auto hDecalAtlas = GetWorld()->GetComponentManager<nsDecalComponentManager>()->m_hDecalAtlas;
  nsVec4 baseAtlasScaleOffset = nsVec4(0.5f);
  nsVec4 normalAtlasScaleOffset = nsVec4(0.5f);
  nsVec4 ormAtlasScaleOffset = nsVec4(0.5f);
  nsUInt32 uiDecalFlags = 0;

  float fAspectRatio = 1.0f;

  {
    nsResourceLock<nsDecalAtlasResource> pDecalAtlas(hDecalAtlas, nsResourceAcquireMode::BlockTillLoaded);

    const auto& atlas = pDecalAtlas->GetAtlas();
    const nsUInt32 decalIdx = atlas.m_Items.Find(nsHashingUtils::StringHashTo32(m_Decals[uiDecalIndex].GetResourceIDHash()));

    if (decalIdx != nsInvalidIndex)
    {
      const auto& item = atlas.m_Items.GetValue(decalIdx);
      uiDecalFlags = item.m_uiFlags;

      auto layerRectToScaleOffset = [](nsRectU32 layerRect, nsVec2U32 vTextureSize)
      {
        nsVec4 result;
        result.x = (float)layerRect.width / vTextureSize.x * 0.5f;
        result.y = (float)layerRect.height / vTextureSize.y * 0.5f;
        result.z = (float)layerRect.x / vTextureSize.x + result.x;
        result.w = (float)layerRect.y / vTextureSize.y + result.y;
        return result;
      };

      baseAtlasScaleOffset = layerRectToScaleOffset(item.m_LayerRects[0], pDecalAtlas->GetBaseColorTextureSize());
      normalAtlasScaleOffset = layerRectToScaleOffset(item.m_LayerRects[1], pDecalAtlas->GetNormalTextureSize());
      ormAtlasScaleOffset = layerRectToScaleOffset(item.m_LayerRects[2], pDecalAtlas->GetORMTextureSize());

      fAspectRatio = (float)item.m_LayerRects[0].width / item.m_LayerRects[0].height;
    }
  }

  auto pRenderData = nsCreateRenderDataForThisFrame<nsDecalRenderData>(GetOwner());

  nsUInt32 uiSortingId = (nsUInt32)(nsMath::Min(m_fSortOrder * 512.0f, 32767.0f) + 32768.0f);
  pRenderData->m_uiSortingKey = (uiSortingId << 16) | (m_uiInternalSortKey & 0xFFFF);

  const nsQuat axisRotation = nsBasisAxis::GetBasisRotation_PosX(m_ProjectionAxis);

  pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
  pRenderData->m_GlobalTransform.m_vScale = (axisRotation * (pRenderData->m_GlobalTransform.m_vScale.CompMul(m_vExtents * 0.5f))).Abs();
  pRenderData->m_GlobalTransform.m_qRotation = pRenderData->m_GlobalTransform.m_qRotation * axisRotation;

  if (!nsMath::IsEqual(fAspectRatio, 1.0f, 0.001f))
  {
    if (fAspectRatio > 1.0f)
    {
      pRenderData->m_GlobalTransform.m_vScale.z /= fAspectRatio;
    }
    else
    {
      pRenderData->m_GlobalTransform.m_vScale.y *= fAspectRatio;
    }
  }

  pRenderData->m_uiApplyOnlyToId = m_uiApplyOnlyToId;
  pRenderData->m_uiFlags = uiDecalFlags;
  pRenderData->m_uiFlags |= (m_bWrapAround ? DECAL_WRAP_AROUND : 0);
  pRenderData->m_uiFlags |= (m_bMapNormalToGeometry ? DECAL_MAP_NORMAL_TO_GEOMETRY : 0);
  pRenderData->m_uiAngleFadeParams = nsShaderUtils::Float2ToRG16F(nsVec2(fFadeParamScale, fFadeParamOffset));
  pRenderData->m_BaseColor = finalColor;
  pRenderData->m_EmissiveColor = m_EmissiveColor;
  nsShaderUtils::Float4ToRGBA16F(baseAtlasScaleOffset, pRenderData->m_uiBaseColorAtlasScale, pRenderData->m_uiBaseColorAtlasOffset);
  nsShaderUtils::Float4ToRGBA16F(normalAtlasScaleOffset, pRenderData->m_uiNormalAtlasScale, pRenderData->m_uiNormalAtlasOffset);
  nsShaderUtils::Float4ToRGBA16F(ormAtlasScaleOffset, pRenderData->m_uiORMAtlasScale, pRenderData->m_uiORMAtlasOffset);

  nsRenderData::Caching::Enum caching = (m_FadeOutDelay.m_Value.GetSeconds() > 0.0 || m_FadeOutDuration.GetSeconds() > 0.0) ? nsRenderData::Caching::Never : nsRenderData::Caching::IfStatic;
  msg.AddRenderData(pRenderData, nsDefaultRenderDataCategories::Decal, caching);
}

void nsDecalComponent::SetApplyToRef(const char* szReference)
{
  auto resolver = GetWorld()->GetGameObjectReferenceResolver();

  if (!resolver.IsValid())
    return;

  nsGameObjectHandle hTarget = resolver(szReference, GetHandle(), "ApplyTo");

  if (m_hApplyOnlyToObject == hTarget)
    return;

  m_hApplyOnlyToObject = hTarget;

  if (IsActiveAndInitialized())
  {
    UpdateApplyTo();
  }
}

void nsDecalComponent::UpdateApplyTo()
{
  nsUInt32 uiPrevId = m_uiApplyOnlyToId;

  m_uiApplyOnlyToId = 0;

  if (!m_hApplyOnlyToObject.IsInvalidated())
  {
    m_uiApplyOnlyToId = nsInvalidIndex;

    nsGameObject* pObject = nullptr;
    if (GetWorld()->TryGetObject(m_hApplyOnlyToObject, pObject))
    {
      nsRenderComponent* pRenderComponent = nullptr;
      if (pObject->TryGetComponentOfBaseType(pRenderComponent))
      {
        // this only works for dynamic objects, for static ones we must use ID 0
        if (pRenderComponent->GetOwner()->IsDynamic())
        {
          m_uiApplyOnlyToId = pRenderComponent->GetUniqueIdForRendering();
        }
      }
    }
  }

  if (uiPrevId != m_uiApplyOnlyToId && GetOwner()->IsStatic())
  {
    InvalidateCachedRenderData();
  }
}

static nsHashedString s_sSuicide = nsMakeHashedString("Suicide");

void nsDecalComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  nsWorld* pWorld = GetWorld();

  // no fade out -> fade out pretty late
  m_StartFadeOutTime = nsTime::MakeFromHours(24.0 * 365.0 * 100.0); // 100 years should be enough for everybody (ignoring leap years)

  if (m_FadeOutDelay.m_Value.GetSeconds() > 0.0 || m_FadeOutDuration.GetSeconds() > 0.0)
  {
    const nsTime tFadeOutDelay = nsTime::MakeFromSeconds(pWorld->GetRandomNumberGenerator().DoubleVariance(m_FadeOutDelay.m_Value.GetSeconds(), m_FadeOutDelay.m_fVariance));
    m_StartFadeOutTime = pWorld->GetClock().GetAccumulatedTime() + tFadeOutDelay;

    if (m_OnFinishedAction != nsOnComponentFinishedAction::None)
    {
      nsMsgComponentInternalTrigger msg;
      msg.m_sMessage = s_sSuicide;

      const nsTime tKill = tFadeOutDelay + m_FadeOutDuration;

      PostMessage(msg, tKill);
    }
  }

  if (m_fSizeVariance > 0)
  {
    const float scale = (float)pWorld->GetRandomNumberGenerator().DoubleVariance(1.0, m_fSizeVariance);
    m_vExtents *= scale;

    TriggerLocalBoundsUpdate();

    InvalidateCachedRenderData();
  }
}

void nsDecalComponent::OnActivated()
{
  SUPER::OnActivated();

  m_uiInternalSortKey = GetOwner()->GetStableRandomSeed();
  m_uiInternalSortKey = (m_uiInternalSortKey >> 16) ^ (m_uiInternalSortKey & 0xFFFF);

  UpdateApplyTo();
}

void nsDecalComponent::OnTriggered(nsMsgComponentInternalTrigger& msg)
{
  if (msg.m_sMessage != s_sSuicide)
    return;

  nsOnComponentFinishedAction::HandleFinishedAction(this, m_OnFinishedAction);
}

void nsDecalComponent::OnMsgDeleteGameObject(nsMsgDeleteGameObject& msg)
{
  nsOnComponentFinishedAction::HandleDeleteObjectMsg(msg, m_OnFinishedAction);
}

void nsDecalComponent::OnMsgOnlyApplyToObject(nsMsgOnlyApplyToObject& msg)
{
  SetApplyOnlyTo(msg.m_hObject);
}

void nsDecalComponent::OnMsgSetColor(nsMsgSetColor& msg)
{
  msg.ModifyColor(m_Color);
}

nsUInt32 nsDecalComponent::DecalFile_GetCount() const
{
  return m_Decals.GetCount();
}

const char* nsDecalComponent::DecalFile_Get(nsUInt32 uiIndex) const
{
  if (!m_Decals[uiIndex].IsValid())
    return "";

  return m_Decals[uiIndex].GetResourceID();
}

void nsDecalComponent::DecalFile_Set(nsUInt32 uiIndex, const char* szFile)
{
  nsDecalResourceHandle hResource;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hResource = nsResourceManager::LoadResource<nsDecalResource>(szFile);
  }

  SetDecal(uiIndex, hResource);
}

void nsDecalComponent::DecalFile_Insert(nsUInt32 uiIndex, const char* szFile)
{
  m_Decals.InsertAt(uiIndex, nsDecalResourceHandle());
  DecalFile_Set(uiIndex, szFile);
}

void nsDecalComponent::DecalFile_Remove(nsUInt32 uiIndex)
{
  m_Decals.RemoveAtAndCopy(uiIndex);
}

//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/GraphPatch.h>

class nsDecalComponent_6_7 : public nsGraphPatch
{
public:
  nsDecalComponent_6_7()
    : nsGraphPatch("nsDecalComponent", 7)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    auto* pDecal = pNode->FindProperty("Decal");
    if (pDecal && pDecal->m_Value.IsA<nsString>())
    {
      nsVariantArray ar;
      ar.PushBack(pDecal->m_Value.Get<nsString>());
      pNode->AddProperty("Decals", ar);
    }
  }
};

nsDecalComponent_6_7 g_nsDecalComponent_6_7;

NS_STATICLINK_FILE(RendererCore, RendererCore_Decals_Implementation_DecalComponent);
