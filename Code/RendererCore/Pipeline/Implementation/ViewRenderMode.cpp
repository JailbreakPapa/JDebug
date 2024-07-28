#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/ViewRenderMode.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/GlobalConstants.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsViewRenderMode, 1)
  NS_ENUM_CONSTANT(nsViewRenderMode::None)->AddAttributes(new nsGroupAttribute("Default")),
  NS_ENUM_CONSTANT(nsViewRenderMode::WireframeColor)->AddAttributes(new nsGroupAttribute("Wireframe")),
  NS_ENUM_CONSTANT(nsViewRenderMode::WireframeMonochrome),
  NS_ENUM_CONSTANT(nsViewRenderMode::DiffuseLitOnly)->AddAttributes(new nsGroupAttribute("Lighting")),
  NS_ENUM_CONSTANT(nsViewRenderMode::SpecularLitOnly),
  NS_ENUM_CONSTANT(nsViewRenderMode::LightCount)->AddAttributes(new nsGroupAttribute("Performance")),
  NS_ENUM_CONSTANT(nsViewRenderMode::DecalCount),
  NS_ENUM_CONSTANT(nsViewRenderMode::StaticVsDynamic),
  NS_ENUM_CONSTANT(nsViewRenderMode::TexCoordsUV0)->AddAttributes(new nsGroupAttribute("TexCoords")),
  NS_ENUM_CONSTANT(nsViewRenderMode::TexCoordsUV1),
  NS_ENUM_CONSTANT(nsViewRenderMode::VertexColors0)->AddAttributes(new nsGroupAttribute("VertexColors")),
  NS_ENUM_CONSTANT(nsViewRenderMode::VertexColors1),
  NS_ENUM_CONSTANT(nsViewRenderMode::VertexNormals)->AddAttributes(new nsGroupAttribute("Normals")),
  NS_ENUM_CONSTANT(nsViewRenderMode::VertexTangents),
  NS_ENUM_CONSTANT(nsViewRenderMode::PixelNormals),
  NS_ENUM_CONSTANT(nsViewRenderMode::DiffuseColor)->AddAttributes(new nsGroupAttribute("PixelColors")),
  NS_ENUM_CONSTANT(nsViewRenderMode::DiffuseColorRange),
  NS_ENUM_CONSTANT(nsViewRenderMode::SpecularColor),
  NS_ENUM_CONSTANT(nsViewRenderMode::EmissiveColor),
  NS_ENUM_CONSTANT(nsViewRenderMode::Roughness)->AddAttributes(new nsGroupAttribute("Surface")),
  NS_ENUM_CONSTANT(nsViewRenderMode::Occlusion),
  NS_ENUM_CONSTANT(nsViewRenderMode::Depth),
  NS_ENUM_CONSTANT(nsViewRenderMode::BoneWeights)->AddAttributes(new nsGroupAttribute("Animation")),
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

// static
nsTempHashedString nsViewRenderMode::GetPermutationValue(Enum renderMode)
{
  if (renderMode >= WireframeColor && renderMode <= WireframeMonochrome)
  {
    return "RENDER_PASS_WIREFRAME";
  }
  else if (renderMode >= DiffuseLitOnly && renderMode < ENUM_COUNT)
  {
    return "RENDER_PASS_EDITOR";
  }

  return "";
}

// static
int nsViewRenderMode::GetRenderPassForShader(Enum renderMode)
{
  switch (renderMode)
  {
    case nsViewRenderMode::None:
      return -1;

    case nsViewRenderMode::WireframeColor:
      return WIREFRAME_RENDER_PASS_COLOR;

    case nsViewRenderMode::WireframeMonochrome:
      return WIREFRAME_RENDER_PASS_MONOCHROME;

    case nsViewRenderMode::DiffuseLitOnly:
      return EDITOR_RENDER_PASS_DIFFUSE_LIT_ONLY;

    case nsViewRenderMode::SpecularLitOnly:
      return EDITOR_RENDER_PASS_SPECULAR_LIT_ONLY;

    case nsViewRenderMode::LightCount:
      return EDITOR_RENDER_PASS_LIGHT_COUNT;

    case nsViewRenderMode::DecalCount:
      return EDITOR_RENDER_PASS_DECAL_COUNT;

    case nsViewRenderMode::TexCoordsUV0:
      return EDITOR_RENDER_PASS_TEXCOORDS_UV0;

    case nsViewRenderMode::TexCoordsUV1:
      return EDITOR_RENDER_PASS_TEXCOORDS_UV1;

    case nsViewRenderMode::VertexColors0:
      return EDITOR_RENDER_PASS_VERTEX_COLORS0;

    case nsViewRenderMode::VertexColors1:
      return EDITOR_RENDER_PASS_VERTEX_COLORS1;

    case nsViewRenderMode::VertexNormals:
      return EDITOR_RENDER_PASS_VERTEX_NORMALS;

    case nsViewRenderMode::VertexTangents:
      return EDITOR_RENDER_PASS_VERTEX_TANGENTS;

    case nsViewRenderMode::PixelNormals:
      return EDITOR_RENDER_PASS_PIXEL_NORMALS;

    case nsViewRenderMode::DiffuseColor:
      return EDITOR_RENDER_PASS_DIFFUSE_COLOR;

    case nsViewRenderMode::DiffuseColorRange:
      return EDITOR_RENDER_PASS_DIFFUSE_COLOR_RANGE;

    case nsViewRenderMode::SpecularColor:
      return EDITOR_RENDER_PASS_SPECULAR_COLOR;

    case nsViewRenderMode::EmissiveColor:
      return EDITOR_RENDER_PASS_EMISSIVE_COLOR;

    case nsViewRenderMode::Roughness:
      return EDITOR_RENDER_PASS_ROUGHNESS;

    case nsViewRenderMode::Occlusion:
      return EDITOR_RENDER_PASS_OCCLUSION;

    case nsViewRenderMode::Depth:
      return EDITOR_RENDER_PASS_DEPTH;

    case nsViewRenderMode::StaticVsDynamic:
      return EDITOR_RENDER_PASS_STATIC_VS_DYNAMIC;

    case nsViewRenderMode::BoneWeights:
      return EDITOR_RENDER_PASS_BONE_WEIGHTS;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      return -1;
  }
}

// static
void nsViewRenderMode::GetDebugText(Enum renderMode, nsStringBuilder& out_sDebugText)
{
  if (renderMode == DiffuseColorRange)
  {
    out_sDebugText = "Pure magenta means the diffuse color is too dark, pure green means it is too bright.";
  }
  else if (renderMode == StaticVsDynamic)
  {
    out_sDebugText = "Static objects are shown in green, dynamic objects are shown in red.";
  }
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_ViewRenderMode);
