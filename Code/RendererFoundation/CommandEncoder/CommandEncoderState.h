
#pragma once

#include <Foundation/Math/Color.h>
#include <Foundation/Math/Rect.h>
#include <RendererFoundation/RendererFoundationDLL.h>

struct NS_RENDERERFOUNDATION_DLL nsGALCommandEncoderState
{
  virtual void InvalidateState();

  nsGALShaderHandle m_hShader;
};

struct NS_RENDERERFOUNDATION_DLL nsGALCommandEncoderRenderState final : public nsGALCommandEncoderState
{
  virtual void InvalidateState() override;

  nsGALBufferHandle m_hVertexBuffers[NS_GAL_MAX_VERTEX_BUFFER_COUNT];
  nsGALBufferHandle m_hIndexBuffer;

  nsGALVertexDeclarationHandle m_hVertexDeclaration;
  nsGALPrimitiveTopology::Enum m_Topology = nsGALPrimitiveTopology::ENUM_COUNT;

  nsGALBlendStateHandle m_hBlendState;
  nsColor m_BlendFactor = nsColor(0, 0, 0, 0);
  nsUInt32 m_uiSampleMask = 0;

  nsGALDepthStencilStateHandle m_hDepthStencilState;
  nsUInt8 m_uiStencilRefValue = 0;

  nsGALRasterizerStateHandle m_hRasterizerState;

  nsRectU32 m_ScissorRect = nsRectU32(0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  nsRectFloat m_ViewPortRect = nsRectFloat(nsMath::MaxValue<float>(), nsMath::MaxValue<float>(), 0.0f, 0.0f);
  float m_fViewPortMinDepth = nsMath::MaxValue<float>();
  float m_fViewPortMaxDepth = -nsMath::MaxValue<float>();
};
