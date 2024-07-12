/*
 *   Copyright (c) 2023 WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <AniDriveProtoPlugin/AniDriveProtoPluginPCH.h>

#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <AniDriveProtoPlugin/Components/SampleRenderComponent.h>
#include <Foundation/Math/Rect.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Textures/Texture2DResource.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_BITFLAGS(SampleRenderComponentMask, 1)
  NS_BITFLAGS_CONSTANT(SampleRenderComponentMask::Box),
  NS_BITFLAGS_CONSTANT(SampleRenderComponentMask::Sphere),
  NS_BITFLAGS_CONSTANT(SampleRenderComponentMask::Cross),
  NS_BITFLAGS_CONSTANT(SampleRenderComponentMask::Quad)
NS_END_STATIC_REFLECTED_BITFLAGS;

NS_BEGIN_COMPONENT_TYPE(SampleRenderComponent, 1 /* version */, nsComponentMode::Static)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Size", m_fSize)->AddAttributes(new nsDefaultValueAttribute(1), new nsClampValueAttribute(0, 10)),
    NS_MEMBER_PROPERTY("Color", m_Color)->AddAttributes(new nsDefaultValueAttribute(nsColor::White)),
    NS_ACCESSOR_PROPERTY("Texture", GetTextureFile, SetTextureFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Texture_2D")),
    NS_BITFLAGS_MEMBER_PROPERTY("Render", SampleRenderComponentMask, m_RenderTypes)->AddAttributes(new nsDefaultValueAttribute(SampleRenderComponentMask::Box)),
  }
  NS_END_PROPERTIES;

  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("AniDriveProto"), // Component menu group
  }
  NS_END_ATTRIBUTES;

  NS_BEGIN_MESSAGEHANDLERS
  {
    NS_MESSAGE_HANDLER(nsMsgSetColor, OnSetColor)
  }
  NS_END_MESSAGEHANDLERS;

  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(SetRandomColor)
  }
  NS_END_FUNCTIONS;
}
NS_END_COMPONENT_TYPE
// clang-format on

SampleRenderComponent::SampleRenderComponent() = default;
SampleRenderComponent::~SampleRenderComponent() = default;

void SampleRenderComponent::SerializeComponent(nsWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  if (OWNTYPE::GetStaticRTTI()->GetTypeVersion() == 1)
  {
    // this automatically serializes all properties
    // if you need more control, increase the component 'version' at the top of this file
    // and then use the code path below for manual serialization
    nsReflectionSerializer::WriteObjectToBinary(s, GetDynamicRTTI(), this);
  }
  else
  {
    // do custom serialization, for example:
    // s << m_fSize;
    // s << m_Color;
    // s << m_hTexture;
    // s << m_RenderTypes;
  }
}

void SampleRenderComponent::DeserializeComponent(nsWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const nsUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  if (uiVersion == 1)
  {
    // this automatically de-serializes all properties
    // if you need more control, increase the component 'version' at the top of this file
    // and then use the code path below for manual de-serialization
    nsReflectionSerializer::ReadObjectPropertiesFromBinary(s, *GetDynamicRTTI(), this);
  }
  else
  {
    // do custom de-serialization, for example:
    // s >> m_fSize;
    // s >> m_Color;
    // s >> m_hTexture;
    // s >> m_RenderTypes;
  }
}

void SampleRenderComponent::SetTexture(const nsTexture2DResourceHandle& hTexture)
{
  m_hTexture = hTexture;
}

const nsTexture2DResourceHandle& SampleRenderComponent::GetTexture() const
{
  return m_hTexture;
}

void SampleRenderComponent::SetTextureFile(const char* szFile)
{
  nsTexture2DResourceHandle hTexture;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hTexture = nsResourceManager::LoadResource<nsTexture2DResource>(szFile);
  }

  SetTexture(hTexture);
}

const char* SampleRenderComponent::GetTextureFile(void) const
{
  if (m_hTexture.IsValid())
    return m_hTexture.GetResourceID();

  return nullptr;
}

void SampleRenderComponent::OnSetColor(nsMsgSetColor& msg)
{
  m_Color = msg.m_Color;
}

void SampleRenderComponent::SetRandomColor()
{
  nsRandom& rng = GetWorld()->GetRandomNumberGenerator();

  m_Color.r = static_cast<float>(rng.DoubleMinMax(0.2f, 1.0f));
  m_Color.g = static_cast<float>(rng.DoubleMinMax(0.2f, 1.0f));
  m_Color.b = static_cast<float>(rng.DoubleMinMax(0.2f, 1.0f));
}

void SampleRenderComponent::Update()
{
  const nsTransform ownerTransform = GetOwner()->GetGlobalTransform();

  if (m_RenderTypes.IsSet(SampleRenderComponentMask::Box))
  {
    nsBoundingBox bbox = nsBoundingBox::MakeFromCenterAndHalfExtents(nsVec3::MakeZero(), nsVec3(m_fSize));

    nsDebugRenderer::DrawLineBox(GetWorld(), bbox, m_Color, ownerTransform);
  }

  if (m_RenderTypes.IsSet(SampleRenderComponentMask::Cross))
  {
    nsDebugRenderer::DrawCross(GetWorld(), nsVec3::MakeZero(), m_fSize, m_Color, ownerTransform);
  }

  if (m_RenderTypes.IsSet(SampleRenderComponentMask::Sphere))
  {
    nsBoundingSphere sphere = nsBoundingSphere::MakeFromCenterAndRadius(nsVec3::MakeZero(), m_fSize);
    nsDebugRenderer::DrawLineSphere(GetWorld(), sphere, m_Color, ownerTransform);
  }

  if (m_RenderTypes.IsSet(SampleRenderComponentMask::Quad) && m_hTexture.IsValid())
  {
    nsHybridArray<nsDebugRenderer::TexturedTriangle, 16> triangles;

    {
      auto& t0 = triangles.ExpandAndGetRef();

      t0.m_position[0].Set(0, -m_fSize, +m_fSize);
      t0.m_position[1].Set(0, +m_fSize, -m_fSize);
      t0.m_position[2].Set(0, -m_fSize, -m_fSize);

      t0.m_texcoord[0].Set(0.0f, 0.0f);
      t0.m_texcoord[1].Set(1.0f, 1.0f);
      t0.m_texcoord[2].Set(0.0f, 1.0f);
    }

    {
      auto& t1 = triangles.ExpandAndGetRef();

      t1.m_position[0].Set(0, -m_fSize, +m_fSize);
      t1.m_position[1].Set(0, +m_fSize, +m_fSize);
      t1.m_position[2].Set(0, +m_fSize, -m_fSize);

      t1.m_texcoord[0].Set(0.0f, 0.0f);
      t1.m_texcoord[1].Set(1.0f, 0.0f);
      t1.m_texcoord[2].Set(1.0f, 1.0f);
    }

    // move the triangles into our object space
    for (auto& tri : triangles)
    {
      tri.m_position[0] = ownerTransform.TransformPosition(tri.m_position[0]);
      tri.m_position[1] = ownerTransform.TransformPosition(tri.m_position[1]);
      tri.m_position[2] = ownerTransform.TransformPosition(tri.m_position[2]);
    }

    nsDebugRenderer::DrawTexturedTriangles(GetWorld(), triangles, m_Color, m_hTexture);
  }
}
