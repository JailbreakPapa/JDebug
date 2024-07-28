
#pragma once

#include <RendererVulkan/RendererVulkanDLL.h>

#include <Foundation/Types/Bitflags.h>
#include <RendererFoundation/CommandEncoder/CommandEncoderPlatformInterface.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>
#include <RendererVulkan/Cache/ResourceCacheVulkan.h>

#include <vulkan/vulkan.hpp>

class nsGALBlendStateVulkan;
class nsGALBufferVulkan;
class nsGALDepthStencilStateVulkan;
class nsGALRasterizerStateVulkan;
class nsGALTextureResourceViewVulkan;
class nsGALBufferResourceViewVulkan;
class nsGALSamplerStateVulkan;
class nsGALShaderVulkan;
class nsGALTextureUnorderedAccessViewVulkan;
class nsGALBufferUnorderedAccessViewVulkan;
class nsGALDeviceVulkan;

class NS_RENDERERVULKAN_DLL nsGALCommandEncoderImplVulkan : public nsGALCommandEncoderCommonPlatformInterface, public nsGALCommandEncoderRenderPlatformInterface, public nsGALCommandEncoderComputePlatformInterface
{
public:
  nsGALCommandEncoderImplVulkan(nsGALDeviceVulkan& device);
  ~nsGALCommandEncoderImplVulkan();

  void Reset();
  void MarkDirty();
  void SetCurrentCommandBuffer(vk::CommandBuffer* commandBuffer, nsPipelineBarrierVulkan* pipelineBarrier);

  // nsGALCommandEncoderCommonPlatformInterface
  // State setting functions

  virtual void SetShaderPlatform(const nsGALShader* pShader) override;

  virtual void SetConstantBufferPlatform(const nsShaderResourceBinding& binding, const nsGALBuffer* pBuffer) override;
  virtual void SetSamplerStatePlatform(const nsShaderResourceBinding& binding, const nsGALSamplerState* pSamplerState) override;
  virtual void SetResourceViewPlatform(const nsShaderResourceBinding& binding, const nsGALTextureResourceView* pResourceView) override;
  virtual void SetResourceViewPlatform(const nsShaderResourceBinding& binding, const nsGALBufferResourceView* pResourceView) override;
  virtual void SetUnorderedAccessViewPlatform(const nsShaderResourceBinding& binding, const nsGALTextureUnorderedAccessView* pUnorderedAccessView) override;
  virtual void SetUnorderedAccessViewPlatform(const nsShaderResourceBinding& binding, const nsGALBufferUnorderedAccessView* pUnorderedAccessView) override;
  virtual void SetPushConstantsPlatform(nsArrayPtr<const nsUInt8> data) override;

  // Query functions

  virtual void BeginQueryPlatform(const nsGALQuery* pQuery) override;
  virtual void EndQueryPlatform(const nsGALQuery* pQuery) override;
  virtual nsResult GetQueryResultPlatform(const nsGALQuery* pQuery, nsUInt64& uiQueryResult) override;

  // Timestamp functions

  virtual void InsertTimestampPlatform(nsGALTimestampHandle hTimestamp) override;

  // Resource update functions

  virtual void ClearUnorderedAccessViewPlatform(const nsGALTextureUnorderedAccessView* pUnorderedAccessView, nsVec4 clearValues) override;
  virtual void ClearUnorderedAccessViewPlatform(const nsGALBufferUnorderedAccessView* pUnorderedAccessView, nsVec4 clearValues) override;

  virtual void ClearUnorderedAccessViewPlatform(const nsGALTextureUnorderedAccessView* pUnorderedAccessView, nsVec4U32 clearValues) override;
  virtual void ClearUnorderedAccessViewPlatform(const nsGALBufferUnorderedAccessView* pUnorderedAccessView, nsVec4U32 clearValues) override;

  virtual void CopyBufferPlatform(const nsGALBuffer* pDestination, const nsGALBuffer* pSource) override;
  virtual void CopyBufferRegionPlatform(const nsGALBuffer* pDestination, nsUInt32 uiDestOffset, const nsGALBuffer* pSource, nsUInt32 uiSourceOffset, nsUInt32 uiByteCount) override;

  virtual void UpdateBufferPlatform(const nsGALBuffer* pDestination, nsUInt32 uiDestOffset, nsArrayPtr<const nsUInt8> pSourceData, nsGALUpdateMode::Enum updateMode) override;

  virtual void CopyTexturePlatform(const nsGALTexture* pDestination, const nsGALTexture* pSource) override;
  virtual void CopyTextureRegionPlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& DestinationSubResource, const nsVec3U32& DestinationPoint, const nsGALTexture* pSource, const nsGALTextureSubresource& SourceSubResource, const nsBoundingBoxu32& Box) override;

  virtual void UpdateTexturePlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& DestinationSubResource, const nsBoundingBoxu32& DestinationBox, const nsGALSystemMemoryDescription& pSourceData) override;

  virtual void ResolveTexturePlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& DestinationSubResource, const nsGALTexture* pSource, const nsGALTextureSubresource& SourceSubResource) override;

  virtual void ReadbackTexturePlatform(const nsGALTexture* pTexture) override;

  virtual void CopyTextureReadbackResultPlatform(const nsGALTexture* pTexture, nsArrayPtr<nsGALTextureSubresource> SourceSubResource, nsArrayPtr<nsGALSystemMemoryDescription> TargetData) override;

  virtual void GenerateMipMapsPlatform(const nsGALTextureResourceView* pResourceView) override;

  void CopyImageToBuffer(const nsGALTextureVulkan* pSource, const nsGALBufferVulkan* pDestination);

  // Misc

  virtual void FlushPlatform() override;

  // Debug helper functions

  virtual void PushMarkerPlatform(const char* szMarker) override;
  virtual void PopMarkerPlatform() override;
  virtual void InsertEventMarkerPlatform(const char* szMarker) override;

  // nsGALCommandEncoderRenderPlatformInterface
  void BeginRendering(const nsGALRenderingSetup& renderingSetup);
  void EndRendering();

  // Draw functions

  virtual void ClearPlatform(const nsColor& ClearColor, nsUInt32 uiRenderTargetClearMask, bool bClearDepth, bool bClearStencil, float fDepthClear, nsUInt8 uiStencilClear) override;

  virtual nsResult DrawPlatform(nsUInt32 uiVertexCount, nsUInt32 uiStartVertex) override;
  virtual nsResult DrawIndexedPlatform(nsUInt32 uiIndexCount, nsUInt32 uiStartIndex) override;
  virtual nsResult DrawIndexedInstancedPlatform(nsUInt32 uiIndexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartIndex) override;
  virtual nsResult DrawIndexedInstancedIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes) override;
  virtual nsResult DrawInstancedPlatform(nsUInt32 uiVertexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartVertex) override;
  virtual nsResult DrawInstancedIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes) override;

  // State functions

  virtual void SetIndexBufferPlatform(const nsGALBuffer* pIndexBuffer) override;
  virtual void SetVertexBufferPlatform(nsUInt32 uiSlot, const nsGALBuffer* pVertexBuffer) override;
  virtual void SetVertexDeclarationPlatform(const nsGALVertexDeclaration* pVertexDeclaration) override;
  virtual void SetPrimitiveTopologyPlatform(nsGALPrimitiveTopology::Enum Topology) override;

  virtual void SetBlendStatePlatform(const nsGALBlendState* pBlendState, const nsColor& BlendFactor, nsUInt32 uiSampleMask) override;
  virtual void SetDepthStencilStatePlatform(const nsGALDepthStencilState* pDepthStencilState, nsUInt8 uiStencilRefValue) override;
  virtual void SetRasterizerStatePlatform(const nsGALRasterizerState* pRasterizerState) override;

  virtual void SetViewportPlatform(const nsRectFloat& rect, float fMinDepth, float fMaxDepth) override;
  virtual void SetScissorRectPlatform(const nsRectU32& rect) override;

  // nsGALCommandEncoderComputePlatformInterface
  // Dispatch
  void BeginCompute();
  void EndCompute();

  virtual nsResult DispatchPlatform(nsUInt32 uiThreadGroupCountX, nsUInt32 uiThreadGroupCountY, nsUInt32 uiThreadGroupCountZ) override;
  virtual nsResult DispatchIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes) override;

private:
  // Map resources from sets then slots to pointer.
  struct SetResources
  {
    nsDynamicArray<const nsGALBufferVulkan*> m_pBoundConstantBuffers;
    nsDynamicArray<const nsGALTextureResourceViewVulkan*> m_pBoundTextureResourceViews;
    nsDynamicArray<const nsGALBufferResourceViewVulkan*> m_pBoundBufferResourceViews;
    nsDynamicArray<const nsGALTextureUnorderedAccessViewVulkan*> m_pBoundTextureUnorderedAccessViews;
    nsDynamicArray<const nsGALBufferUnorderedAccessViewVulkan*> m_pBoundBufferUnorderedAccessViews;
    nsDynamicArray<const nsGALSamplerStateVulkan*> m_pBoundSamplerStates;
  };

private:
  nsResult FlushDeferredStateChanges();
  const nsGALTextureResourceViewVulkan* GetTextureResourceView(const SetResources& resources, const nsShaderResourceBinding& mapping);
  const nsGALBufferResourceViewVulkan* GetBufferResourceView(const SetResources& resources, const nsShaderResourceBinding& mapping);
  const nsGALTextureUnorderedAccessViewVulkan* GetTextureUAV(const SetResources& resources, const nsShaderResourceBinding& mapping);
  const nsGALBufferUnorderedAccessViewVulkan* GetBufferUAV(const SetResources& resources, const nsShaderResourceBinding& mapping);

private:
  nsGALDeviceVulkan& m_GALDeviceVulkan;
  vk::Device m_vkDevice;

  vk::CommandBuffer* m_pCommandBuffer = nullptr;
  nsPipelineBarrierVulkan* m_pPipelineBarrier = nullptr;


  // Cache flags.
  bool m_bPipelineStateDirty = true;
  bool m_bViewportDirty = true;
  bool m_bIndexBufferDirty = false;
  bool m_bDescriptorsDirty = false;
  nsGAL::ModifiedRange m_BoundVertexBuffersRange;
  bool m_bRenderPassActive = false; ///< #TODO_VULKAN Disabling and re-enabling the render pass is buggy as we might execute a clear twice.
  bool m_bClearSubmitted = false;   ///< Start render pass is lazy so if no draw call is executed we need to make sure the clear is executed anyways.
  bool m_bInsideCompute = false;    ///< Within BeginCompute / EndCompute block.
  bool m_bPushConstantsDirty = false;

  // Bound objects for deferred state flushes
  nsResourceCacheVulkan::PipelineLayoutDesc m_LayoutDesc;
  nsResourceCacheVulkan::GraphicsPipelineDesc m_PipelineDesc;
  nsResourceCacheVulkan::ComputePipelineDesc m_ComputeDesc;
  vk::Framebuffer m_frameBuffer;
  vk::RenderPassBeginInfo m_renderPass;
  nsHybridArray<vk::ClearValue, NS_GAL_MAX_RENDERTARGET_COUNT + 1> m_clearValues;
  vk::ImageAspectFlags m_depthMask = {};
  nsUInt32 m_uiLayers = 0;

  vk::Viewport m_viewport;
  vk::Rect2D m_scissor;
  bool m_bScissorEnabled = false;

  const nsGALRenderTargetView* m_pBoundRenderTargets[NS_GAL_MAX_RENDERTARGET_COUNT] = {};
  const nsGALRenderTargetView* m_pBoundDepthStencilTarget = nullptr;
  nsUInt32 m_uiBoundRenderTargetCount;

  const nsGALBufferVulkan* m_pIndexBuffer = nullptr;
  vk::Buffer m_pBoundVertexBuffers[NS_GAL_MAX_VERTEX_BUFFER_COUNT];
  vk::DeviceSize m_VertexBufferOffsets[NS_GAL_MAX_VERTEX_BUFFER_COUNT] = {};

  nsHybridArray<SetResources, 4> m_Resources;

  nsDeque<vk::DescriptorImageInfo> m_TextureAndSampler;
  nsHybridArray<vk::WriteDescriptorSet, 16> m_DescriptorWrites;
  nsHybridArray<vk::DescriptorSet, 4> m_DescriptorSets;

  nsDynamicArray<nsUInt8> m_PushConstants;
};
