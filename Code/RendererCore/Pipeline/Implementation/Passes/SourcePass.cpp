#include <RendererCore/RendererCorePCH.h>

#include <Core/Graphics/Camera.h>
#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Pipeline/Passes/SourcePass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererFoundation/Resources/Texture.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSourcePass, 3, nsRTTIDefaultAllocator<nsSourcePass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Output", m_PinOutput),
    NS_ENUM_MEMBER_PROPERTY("Format", nsSourceFormat, m_Format),
    NS_ENUM_MEMBER_PROPERTY("MSAA_Mode", nsGALMSAASampleCount, m_MsaaMode),
    NS_MEMBER_PROPERTY("ClearColor", m_ClearColor)->AddAttributes(new nsExposeColorAlphaAttribute()),
    NS_MEMBER_PROPERTY("Clear", m_bClear),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_STATIC_REFLECTED_ENUM(nsSourceFormat, 1)
  NS_ENUM_CONSTANTS(
    nsSourceFormat::Color4Channel8BitNormalized_sRGB,
    nsSourceFormat::Color4Channel8BitNormalized,
    nsSourceFormat::Color4Channel16BitFloat,
    nsSourceFormat::Color4Channel32BitFloat,
    nsSourceFormat::Color3Channel11_11_10BitFloat,
    nsSourceFormat::Depth16Bit,
    nsSourceFormat::Depth24BitStencil8Bit,
    nsSourceFormat::Depth32BitFloat
  )
NS_END_STATIC_REFLECTED_ENUM;
// clang-format on

nsSourcePass::nsSourcePass(const char* szName)
  : nsRenderPipelinePass(szName, true)
{
  m_Format = nsSourceFormat::Default;
  m_MsaaMode = nsGALMSAASampleCount::None;
  m_bClear = true;
  m_ClearColor = nsColor::Black;
}

nsSourcePass::~nsSourcePass() = default;

bool nsSourcePass::GetRenderTargetDescriptions(
  const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  nsUInt32 uiWidth = static_cast<nsUInt32>(view.GetViewport().width);
  nsUInt32 uiHeight = static_cast<nsUInt32>(view.GetViewport().height);

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();
  const nsGALRenderTargets& renderTargets = view.GetActiveRenderTargets();

  nsGALTextureCreationDescription desc;

  // Color
  if (m_Format == nsSourceFormat::Color4Channel8BitNormalized || m_Format == nsSourceFormat::Color4Channel8BitNormalized_sRGB)
  {
    nsGALResourceFormat::Enum preferredFormat = nsGALResourceFormat::Invalid;
    if (const nsGALTexture* pTexture = pDevice->GetTexture(renderTargets.m_hRTs[0]))
    {
      auto rendertargetDesc = pTexture->GetDescription();

      preferredFormat = rendertargetDesc.m_Format;
    }

    switch (preferredFormat)
    {
      case nsGALResourceFormat::RGBAUByteNormalized:
      case nsGALResourceFormat::RGBAUByteNormalizedsRGB:
      default:
        if (m_Format == nsSourceFormat::Color4Channel8BitNormalized_sRGB)
        {
          desc.m_Format = nsGALResourceFormat::RGBAUByteNormalizedsRGB;
        }
        else
        {
          desc.m_Format = nsGALResourceFormat::RGBAUByteNormalized;
        }
        break;
      case nsGALResourceFormat::BGRAUByteNormalized:
      case nsGALResourceFormat::BGRAUByteNormalizedsRGB:
        if (m_Format == nsSourceFormat::Color4Channel8BitNormalized_sRGB)
        {
          desc.m_Format = nsGALResourceFormat::BGRAUByteNormalizedsRGB;
        }
        else
        {
          desc.m_Format = nsGALResourceFormat::BGRAUByteNormalized;
        }
        break;
    }
  }
  else
  {
    switch (m_Format)
    {
      case nsSourceFormat::Color4Channel16BitFloat:
        desc.m_Format = nsGALResourceFormat::RGBAHalf;
        break;
      case nsSourceFormat::Color4Channel32BitFloat:
        desc.m_Format = nsGALResourceFormat::RGBAFloat;
        break;
      case nsSourceFormat::Color3Channel11_11_10BitFloat:
        desc.m_Format = nsGALResourceFormat::RG11B10Float;
        break;
      case nsSourceFormat::Depth16Bit:
        desc.m_Format = nsGALResourceFormat::D16;
        break;
      case nsSourceFormat::Depth24BitStencil8Bit:
        desc.m_Format = nsGALResourceFormat::D24S8;
        break;
      case nsSourceFormat::Depth32BitFloat:
        desc.m_Format = nsGALResourceFormat::DFloat;
        break;
      default:
        NS_ASSERT_NOT_IMPLEMENTED
    }
  }

  desc.m_uiWidth = uiWidth;
  desc.m_uiHeight = uiHeight;
  desc.m_SampleCount = m_MsaaMode;
  desc.m_bCreateRenderTarget = true;
  desc.m_uiArraySize = view.GetCamera()->IsStereoscopic() ? 2 : 1;

  outputs[m_PinOutput.m_uiOutputIndex] = desc;

  return true;
}

void nsSourcePass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs,
  const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs)
{
  if (!m_bClear)
    return;

  auto pOutput = outputs[m_PinOutput.m_uiOutputIndex];
  if (pOutput == nullptr)
  {
    return;
  }

  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  // Setup render target
  nsGALRenderingSetup renderingSetup;
  renderingSetup.m_ClearColor = m_ClearColor;
  renderingSetup.m_uiRenderTargetClearMask = 0xFFFFFFFF;
  renderingSetup.m_bClearDepth = true;
  renderingSetup.m_bClearStencil = true;

  if (nsGALResourceFormat::IsDepthFormat(pOutput->m_Desc.m_Format))
  {
    renderingSetup.m_RenderTargetSetup.SetDepthStencilTarget(pDevice->GetDefaultRenderTargetView(pOutput->m_TextureHandle));
  }
  else
  {
    renderingSetup.m_RenderTargetSetup.SetRenderTarget(0, pDevice->GetDefaultRenderTargetView(pOutput->m_TextureHandle));
  }

  auto pCommandEncoder = nsRenderContext::BeginPassAndRenderingScope(renderViewContext, renderingSetup, GetName());
}

nsResult nsSourcePass::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  inout_stream << m_Format;
  inout_stream << m_MsaaMode;
  inout_stream << m_ClearColor;
  inout_stream << m_bClear;
  return NS_SUCCESS;
}

nsResult nsSourcePass::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  inout_stream >> m_Format;
  inout_stream >> m_MsaaMode;
  inout_stream >> m_ClearColor;
  inout_stream >> m_bClear;
  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>
#include <Foundation/Serialization/GraphPatch.h>

class nsSourcePassPatch_1_2 : public nsGraphPatch
{
public:
  nsSourcePassPatch_1_2()
    : nsGraphPatch("nsSourcePass", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    pNode->RenameProperty("MSAA Mode", "MSAA_Mode");
    pNode->RenameProperty("Clear Color", "ClearColor");
  }
};

nsSourcePassPatch_1_2 g_nsSourcePassPatch_1_2;

class nsSourcePassPatch_2_3 : public nsGraphPatch
{
public:
  nsSourcePassPatch_2_3()
    : nsGraphPatch("nsSourcePass", 3)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    nsAbstractObjectNode::Property* formatProperty = pNode->FindProperty("Format");
    if (formatProperty == nullptr)
      return;

    auto formatName = formatProperty->m_Value.Get<nsString>();
    nsEnum<nsGALResourceFormat> oldFormat;
    nsReflectionUtils::StringToEnumeration<nsGALResourceFormat>(formatName.GetData(), oldFormat);

    nsEnum<nsSourceFormat> newFormat;

    switch (oldFormat)
    {
      case nsGALResourceFormat::RGBAHalf:
        newFormat = nsSourceFormat::Color4Channel16BitFloat;
        break;
      case nsGALResourceFormat::RGBAFloat:
        newFormat = nsSourceFormat::Color4Channel32BitFloat;
        break;
      case nsGALResourceFormat::RG11B10Float:
        newFormat = nsSourceFormat::Color3Channel11_11_10BitFloat;
        break;
      case nsGALResourceFormat::D16:
        newFormat = nsSourceFormat::Depth16Bit;
        break;
      case nsGALResourceFormat::D24S8:
        newFormat = nsSourceFormat::Depth24BitStencil8Bit;
        break;
      case nsGALResourceFormat::DFloat:
        newFormat = nsSourceFormat::Depth32BitFloat;
        break;
      case nsGALResourceFormat::RGBAUByteNormalized:
      case nsGALResourceFormat::BGRAUByteNormalized:
        newFormat = nsSourceFormat::Color4Channel8BitNormalized;
        break;
      case nsGALResourceFormat::RGBAUByteNormalizedsRGB:
      case nsGALResourceFormat::BGRAUByteNormalizedsRGB:
        newFormat = nsSourceFormat::Color4Channel8BitNormalized_sRGB;
        break;
      default:
        newFormat = nsSourceFormat::Default;
        break;
    }

    nsStringBuilder newFormatName;
    nsReflectionUtils::EnumerationToString(newFormat, newFormatName);
    formatProperty->m_Value = newFormatName.GetView();
  }
};

nsSourcePassPatch_2_3 g_nsSourcePassPatch_2_3;


NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_SourcePass);
