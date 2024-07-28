#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <RendererCore/Components/SpriteComponent.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/Textures/Texture2DResource.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsSpriteBlendMode, 1)
  NS_ENUM_CONSTANTS(nsSpriteBlendMode::Masked, nsSpriteBlendMode::Transparent, nsSpriteBlendMode::Additive)
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

// static
nsTempHashedString nsSpriteBlendMode::GetPermutationValue(Enum blendMode)
{
  switch (blendMode)
  {
    case nsSpriteBlendMode::Masked:
    case nsSpriteBlendMode::ShapeIcon:
      return "BLEND_MODE_MASKED";
    case nsSpriteBlendMode::Transparent:
      return "BLEND_MODE_TRANSPARENT";
    case nsSpriteBlendMode::Additive:
      return "BLEND_MODE_ADDITIVE";
  }

  return "";
}

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSpriteRenderData, 1, nsRTTIDefaultAllocator<nsSpriteRenderData>)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsSpriteRenderData::FillBatchIdAndSortingKey()
{
  // ignore upper 32 bit of the resource ID hash
  const nsUInt32 uiTextureIDHash = static_cast<nsUInt32>(m_hTexture.GetResourceIDHash());

  // Generate batch id from mode and texture
  nsUInt32 data[] = {(nsUInt32)m_BlendMode, uiTextureIDHash};
  m_uiBatchId = nsHashingUtils::xxHash32(data, sizeof(data));

  // Sort by mode and then by texture
  m_uiSortingKey = (m_BlendMode << 30) | (uiTextureIDHash & 0x3FFFFFFF);
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_COMPONENT_TYPE(nsSpriteComponent, 3, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Texture", GetTextureFile, SetTextureFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_2D")),
    NS_ENUM_MEMBER_PROPERTY("BlendMode", nsSpriteBlendMode, m_BlendMode),
    NS_ACCESSOR_PROPERTY("Color", GetColor, SetColor)->AddAttributes(new nsExposeColorAlphaAttribute()),
    NS_ACCESSOR_PROPERTY("Size", GetSize, SetSize)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f), new nsSuffixAttribute(" m")),
    NS_ACCESSOR_PROPERTY("MaxScreenSize", GetMaxScreenSize, SetMaxScreenSize)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(64.0f), new nsSuffixAttribute(" px")),
    NS_MEMBER_PROPERTY("AspectRatio", m_fAspectRatio)->AddAttributes(new nsClampValueAttribute(0.0f, nsVariant()), new nsDefaultValueAttribute(1.0f)),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Rendering"),
  }
  NS_END_ATTRIBUTES;
  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgExtractRenderData, OnMsgExtractRenderData),
    NS_MESSAGE_HANDLER(nsMsgSetColor, OnMsgSetColor),
  }
  NS_END_MESSAGEHANDLERS;
}
NS_END_COMPONENT_TYPE;
// clang-format on

nsSpriteComponent::nsSpriteComponent() = default;
nsSpriteComponent::~nsSpriteComponent() = default;

nsResult nsSpriteComponent::GetLocalBounds(nsBoundingBoxSphere& ref_bounds, bool& ref_bAlwaysVisible, nsMsgUpdateLocalBounds& ref_msg)
{
  ref_bounds = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3::MakeZero(), m_fSize * 0.5f);
  return NS_SUCCESS;
}

void nsSpriteComponent::OnMsgExtractRenderData(nsMsgExtractRenderData& msg) const
{
  // Don't render in shadow views
  if (msg.m_pView->GetCameraUsageHint() == nsCameraUsageHint::Shadow)
    return;

  if (!m_hTexture.IsValid())
    return;

  nsSpriteRenderData* pRenderData = nsCreateRenderDataForThisFrame<nsSpriteRenderData>(GetOwner());
  {
    pRenderData->m_GlobalTransform = GetOwner()->GetGlobalTransform();
    pRenderData->m_GlobalBounds = GetOwner()->GetGlobalBounds();
    pRenderData->m_hTexture = m_hTexture;
    pRenderData->m_fSize = m_fSize;
    pRenderData->m_fMaxScreenSize = m_fMaxScreenSize;
    pRenderData->m_fAspectRatio = m_fAspectRatio;
    pRenderData->m_BlendMode = m_BlendMode;
    pRenderData->m_color = m_Color;
    pRenderData->m_texCoordScale = nsVec2(1.0f);
    pRenderData->m_texCoordOffset = nsVec2(0.0f);
    pRenderData->m_uiUniqueID = GetUniqueIdForRendering();

    pRenderData->FillBatchIdAndSortingKey();
  }

  // Determine render data category.
  nsRenderData::Category category = nsDefaultRenderDataCategories::LitTransparent;
  if (m_BlendMode == nsSpriteBlendMode::Masked)
  {
    category = nsDefaultRenderDataCategories::LitMasked;
  }

  msg.AddRenderData(pRenderData, category, nsRenderData::Caching::IfStatic);
}

void nsSpriteComponent::SerializeComponent(nsWorldWriter& inout_stream) const
{
  SUPER::SerializeComponent(inout_stream);
  nsStreamWriter& s = inout_stream.GetStream();

  s << m_hTexture;
  s << m_fSize;
  s << m_fMaxScreenSize;

  // Version 3
  s << m_Color; // HDR now
  s << m_fAspectRatio;
  s << m_BlendMode;
}

void nsSpriteComponent::DeserializeComponent(nsWorldReader& inout_stream)
{
  SUPER::DeserializeComponent(inout_stream);
  const nsUInt32 uiVersion = inout_stream.GetComponentTypeVersion(GetStaticRTTI());

  nsStreamReader& s = inout_stream.GetStream();

  s >> m_hTexture;

  if (uiVersion < 3)
  {
    nsColorGammaUB color;
    s >> color;
    m_Color = color;
  }

  s >> m_fSize;
  s >> m_fMaxScreenSize;

  if (uiVersion >= 3)
  {
    s >> m_Color;
    s >> m_fAspectRatio;
    s >> m_BlendMode;
  }
}

void nsSpriteComponent::SetTexture(const nsTexture2DResourceHandle& hTexture)
{
  m_hTexture = hTexture;
}

const nsTexture2DResourceHandle& nsSpriteComponent::GetTexture() const
{
  return m_hTexture;
}

void nsSpriteComponent::SetTextureFile(const char* szFile)
{
  nsTexture2DResourceHandle hTexture;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hTexture = nsResourceManager::LoadResource<nsTexture2DResource>(szFile);
  }

  SetTexture(hTexture);
}

const char* nsSpriteComponent::GetTextureFile() const
{
  if (!m_hTexture.IsValid())
    return "";

  return m_hTexture.GetResourceID();
}

void nsSpriteComponent::SetColor(nsColor color)
{
  m_Color = color;
}

nsColor nsSpriteComponent::GetColor() const
{
  return m_Color;
}

void nsSpriteComponent::SetSize(float fSize)
{
  m_fSize = fSize;

  TriggerLocalBoundsUpdate();
}

float nsSpriteComponent::GetSize() const
{
  return m_fSize;
}

void nsSpriteComponent::SetMaxScreenSize(float fSize)
{
  m_fMaxScreenSize = fSize;
}

float nsSpriteComponent::GetMaxScreenSize() const
{
  return m_fMaxScreenSize;
}

void nsSpriteComponent::OnMsgSetColor(nsMsgSetColor& ref_msg)
{
  ref_msg.ModifyColor(m_Color);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsSpriteComponentPatch_1_2 : public nsGraphPatch
{
public:
  nsSpriteComponentPatch_1_2()
    : nsGraphPatch("nsSpriteComponent", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override { pNode->RenameProperty("Max Screen Size", "MaxScreenSize"); }
};

nsSpriteComponentPatch_1_2 g_nsSpriteComponentPatch_1_2;



NS_STATICLINK_FILE(RendererCore, RendererCore_Components_Implementation_SpriteComponent);
