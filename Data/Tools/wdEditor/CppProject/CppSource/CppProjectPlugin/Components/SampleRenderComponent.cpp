#include <CppProjectPlugin/CppProjectPluginPCH.h>

#include <Core/Messages/SetColorMessage.h>
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <CppProjectPlugin/Components/SampleRenderComponent.h>
#include <Foundation/Math/Rect.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Textures/Texture2DResource.h>

// clang-format off
WD_BEGIN_STATIC_REFLECTED_BITFLAGS(SampleRenderComponentMask, 1)
  WD_BITFLAGS_CONSTANT(SampleRenderComponentMask::Box),
  WD_BITFLAGS_CONSTANT(SampleRenderComponentMask::Sphere),
  WD_BITFLAGS_CONSTANT(SampleRenderComponentMask::Cross),
  WD_BITFLAGS_CONSTANT(SampleRenderComponentMask::Quad)
WD_END_STATIC_REFLECTED_BITFLAGS;

WD_BEGIN_COMPONENT_TYPE(SampleRenderComponent, 1 /* version */, wdComponentMode::Static)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("Size", m_fSize)->AddAttributes(new wdDefaultValueAttribute(1), new wdClampValueAttribute(0, 10)),
    WD_MEMBER_PROPERTY("Color", m_Color)->AddAttributes(new wdDefaultValueAttribute(wdColor::White)),
    WD_ACCESSOR_PROPERTY("Texture", GetTextureFile, SetTextureFile)->AddAttributes(new wdAssetBrowserAttribute("CompatibleAsset_Texture_2D")),
    WD_BITFLAGS_MEMBER_PROPERTY("Render", SampleRenderComponentMask, m_RenderTypes)->AddAttributes(new wdDefaultValueAttribute(SampleRenderComponentMask::Box)),
  }
  WD_END_PROPERTIES;

  WD_BEGIN_ATTRIBUTES
  {
    new wdCategoryAttribute("CppProject"), // Component menu group
  }
  WD_END_ATTRIBUTES;

  WD_BEGIN_MESSAGEHANDLERS
  {
    WD_MESSAGE_HANDLER(wdMsgSetColor, OnSetColor)
  }
  WD_END_MESSAGEHANDLERS;

  WD_BEGIN_FUNCTIONS
  {
    WD_SCRIPT_FUNCTION_PROPERTY(SetRandomColor)
  }
  WD_END_FUNCTIONS;
}
WD_END_COMPONENT_TYPE
// clang-format on

SampleRenderComponent::SampleRenderComponent() = default;
SampleRenderComponent::~SampleRenderComponent() = default;

void SampleRenderComponent::SerializeComponent(wdWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();

  if (OWNTYPE::GetStaticRTTI()->GetTypeVersion() == 1)
  {
    // this automatically serializes all properties
    // if you need more control, increase the component 'version' at the top of this file
    // and then use the code path below for manual serialization
    wdReflectionSerializer::WriteObjectToBinary(s, GetDynamicRTTI(), this);
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

void SampleRenderComponent::DeserializeComponent(wdWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const wdUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();

  if (uiVersion == 1)
  {
    // this automatically de-serializes all properties
    // if you need more control, increase the component 'version' at the top of this file
    // and then use the code path below for manual de-serialization
    wdReflectionSerializer::ReadObjectPropertiesFromBinary(s, *GetDynamicRTTI(), this);
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

void SampleRenderComponent::SetTexture(const wdTexture2DResourceHandle& hTexture)
{
  m_hTexture = hTexture;
}

const wdTexture2DResourceHandle& SampleRenderComponent::GetTexture() const
{
  return m_hTexture;
}

void SampleRenderComponent::SetTextureFile(const char* szFile)
{
  wdTexture2DResourceHandle hTexture;

  if (!wdStringUtils::IsNullOrEmpty(szFile))
  {
    hTexture = wdResourceManager::LoadResource<wdTexture2DResource>(szFile);
  }

  SetTexture(hTexture);
}

const char* SampleRenderComponent::GetTextureFile(void) const
{
  if (m_hTexture.IsValid())
    return m_hTexture.GetResourceID();

  return nullptr;
}

void SampleRenderComponent::OnSetColor(wdMsgSetColor& msg)
{
  m_Color = msg.m_Color;
}

void SampleRenderComponent::SetRandomColor()
{
  wdRandom& rng = GetWorld()->GetRandomNumberGenerator();

  m_Color.r = static_cast<float>(rng.DoubleMinMax(0.2f, 1.0f));
  m_Color.g = static_cast<float>(rng.DoubleMinMax(0.2f, 1.0f));
  m_Color.b = static_cast<float>(rng.DoubleMinMax(0.2f, 1.0f));
}

void SampleRenderComponent::Update()
{
  const wdTransform ownerTransform = GetOwner()->GetGlobalTransform();

  if (m_RenderTypes.IsSet(SampleRenderComponentMask::Box))
  {
    wdBoundingBox bbox;
    bbox.SetCenterAndHalfExtents(wdVec3::ZeroVector(), wdVec3(m_fSize));

    wdDebugRenderer::DrawLineBox(GetWorld(), bbox, m_Color, ownerTransform);
  }

  if (m_RenderTypes.IsSet(SampleRenderComponentMask::Cross))
  {
    wdDebugRenderer::DrawCross(GetWorld(), wdVec3::ZeroVector(), m_fSize, m_Color, ownerTransform);
  }

  if (m_RenderTypes.IsSet(SampleRenderComponentMask::Sphere))
  {
    wdBoundingSphere sphere;
    sphere.SetElements(wdVec3::ZeroVector(), m_fSize);
    wdDebugRenderer::DrawLineSphere(GetWorld(), sphere, m_Color, ownerTransform);
  }

  if (m_RenderTypes.IsSet(SampleRenderComponentMask::Quad) && m_hTexture.IsValid())
  {
    wdHybridArray<wdDebugRenderer::TexturedTriangle, 16> triangles;

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

    wdDebugRenderer::DrawTexturedTriangles(GetWorld(), triangles, m_Color, m_hTexture);
  }
}
