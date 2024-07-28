#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/Passes/TargetPass.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Resources/RenderTargetView.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsTargetPass, 1, nsRTTIDefaultAllocator<nsTargetPass>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Color0", m_PinColor0),
    NS_MEMBER_PROPERTY("Color1", m_PinColor1),
    NS_MEMBER_PROPERTY("Color2", m_PinColor2),
    NS_MEMBER_PROPERTY("Color3", m_PinColor3),
    NS_MEMBER_PROPERTY("Color4", m_PinColor4),
    NS_MEMBER_PROPERTY("Color5", m_PinColor5),
    NS_MEMBER_PROPERTY("Color6", m_PinColor6),
    NS_MEMBER_PROPERTY("Color7", m_PinColor7),
    NS_MEMBER_PROPERTY("DepthStencil", m_PinDepthStencil),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsTargetPass::nsTargetPass(const char* szName)
  : nsRenderPipelinePass(szName, true)
{
}

nsTargetPass::~nsTargetPass() = default;

const nsGALTextureHandle* nsTargetPass::GetTextureHandle(const nsGALRenderTargets& renderTargets, const nsRenderPipelineNodePin* pPin)
{
  // auto inputs = GetInputPins();
  if (pPin->m_pParent != this)
  {
    nsLog::Error("nsTargetPass::GetTextureHandle: The given pin is not part of this pass!");
    return nullptr;
  }

  nsGALTextureHandle hTarget;
  if (pPin->m_uiInputIndex == 8)
  {
    return &renderTargets.m_hDSTarget;
  }
  else
  {
    return &renderTargets.m_hRTs[pPin->m_uiInputIndex];
  }

  return nullptr;
}

bool nsTargetPass::GetRenderTargetDescriptions(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, nsArrayPtr<nsGALTextureCreationDescription> outputs)
{
  const char* pinNames[] = {
    "Color0",
    "Color1",
    "Color2",
    "Color3",
    "Color4",
    "Color5",
    "Color6",
    "Color7",
    "DepthStencil",
  };

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(pinNames); ++i)
  {
    if (!VerifyInput(view, inputs, pinNames[i]))
      return false;
  }

  return true;
}

void nsTargetPass::Execute(const nsRenderViewContext& renderViewContext, const nsArrayPtr<nsRenderPipelinePassConnection* const> inputs, const nsArrayPtr<nsRenderPipelinePassConnection* const> outputs) {}

bool nsTargetPass::VerifyInput(const nsView& view, const nsArrayPtr<nsGALTextureCreationDescription* const> inputs, const char* szPinName)
{
  nsGALDevice* pDevice = nsGALDevice::GetDefaultDevice();

  const nsRenderPipelineNodePin* pPin = GetPinByName(szPinName);
  if (inputs[pPin->m_uiInputIndex])
  {
    const nsGALTextureHandle* pHandle = GetTextureHandle(view.GetActiveRenderTargets(), pPin);
    if (pHandle)
    {
      const nsGALTexture* pTexture = pDevice->GetTexture(*pHandle);
      if (pTexture)
      {
        // TODO: Need a more sophisticated check here what is considered 'matching'
        // if (inputs[pPin->m_uiInputIndex]->CalculateHash() != pTexture->GetDescription().CalculateHash())
        //  return false;
      }
    }
  }

  return true;
}



NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Passes_TargetPass);
