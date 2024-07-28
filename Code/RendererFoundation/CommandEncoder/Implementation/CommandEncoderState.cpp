#include <RendererFoundation/RendererFoundationPCH.h>

#include <RendererFoundation/CommandEncoder/CommandEncoderState.h>

void nsGALCommandEncoderState::InvalidateState()
{
  m_hShader = nsGALShaderHandle();
}

void nsGALCommandEncoderRenderState::InvalidateState()
{
  nsGALCommandEncoderState::InvalidateState();

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_hVertexBuffers); ++i)
  {
    m_hVertexBuffers[i].Invalidate();
  }
  m_hIndexBuffer.Invalidate();

  m_hVertexDeclaration.Invalidate();
  m_Topology = nsGALPrimitiveTopology::ENUM_COUNT;

  m_hBlendState.Invalidate();
  m_BlendFactor = nsColor(0, 0, 0, 0);
  m_uiSampleMask = 0;

  m_hDepthStencilState.Invalidate();
  m_uiStencilRefValue = 0;

  m_hRasterizerState.Invalidate();

  m_ScissorRect = nsRectU32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  m_ViewPortRect = nsRectFloat(nsMath::MaxValue<float>(), nsMath::MaxValue<float>(), 0.0f, 0.0f);
  m_fViewPortMinDepth = nsMath::MaxValue<float>();
  m_fViewPortMaxDepth = -nsMath::MaxValue<float>();
}
