#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Cache/ResourceCacheVulkan.h>
#include <RendererVulkan/CommandEncoder/CommandEncoderImplVulkan.h>
#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Pools/DescriptorSetPoolVulkan.h>
#include <RendererVulkan/Pools/QueryPoolVulkan.h>
#include <RendererVulkan/Pools/StagingBufferPoolVulkan.h>
#include <RendererVulkan/Resources/BufferVulkan.h>
#include <RendererVulkan/Resources/FallbackResourcesVulkan.h>
#include <RendererVulkan/Resources/QueryVulkan.h>
#include <RendererVulkan/Resources/RenderTargetViewVulkan.h>
#include <RendererVulkan/Resources/ResourceViewVulkan.h>
#include <RendererVulkan/Resources/TextureVulkan.h>
#include <RendererVulkan/Resources/UnorderedAccessViewVulkan.h>
#include <RendererVulkan/Shader/ShaderVulkan.h>
#include <RendererVulkan/Shader/VertexDeclarationVulkan.h>
#include <RendererVulkan/State/StateVulkan.h>
#include <RendererVulkan/Utils/ConversionUtilsVulkan.h>
#include <RendererVulkan/Utils/ImageCopyVulkan.h>
#include <RendererVulkan/Utils/PipelineBarrierVulkan.h>

nsGALCommandEncoderImplVulkan::nsGALCommandEncoderImplVulkan(nsGALDeviceVulkan& device)
  : m_GALDeviceVulkan(device)
{
  m_vkDevice = device.GetVulkanDevice();
}

nsGALCommandEncoderImplVulkan::~nsGALCommandEncoderImplVulkan() = default;

void nsGALCommandEncoderImplVulkan::Reset()
{
  NS_ASSERT_DEBUG(!m_bRenderPassActive, "Render pass was not closed");

  m_bPipelineStateDirty = true;
  m_bViewportDirty = true;
  m_bIndexBufferDirty = true;
  m_bDescriptorsDirty = true;
  m_BoundVertexBuffersRange.Reset();

  m_LayoutDesc = {};
  m_PipelineDesc = {};
  m_frameBuffer = nullptr;

  m_viewport = vk::Viewport();
  m_scissor = vk::Rect2D();

  for (nsUInt32 i = 0; i < NS_GAL_MAX_RENDERTARGET_COUNT; i++)
  {
    m_pBoundRenderTargets[i] = nullptr;
  }
  m_pBoundDepthStencilTarget = nullptr;
  m_uiBoundRenderTargetCount = 0;

  m_pIndexBuffer = nullptr;
  for (nsUInt32 i = 0; i < NS_GAL_MAX_VERTEX_BUFFER_COUNT; i++)
  {
    m_pBoundVertexBuffers[i] = nullptr;
    m_VertexBufferOffsets[i] = 0;
  }

  // Don't clear m_Resources as otherwise we need to re-create the sub-arrays for each set on every reset.
  for (nsUInt32 i = 0; i < m_Resources.GetCount(); i++)
  {
    m_Resources[i].m_pBoundConstantBuffers.Clear();
    m_Resources[i].m_pBoundTextureResourceViews.Clear();
    m_Resources[i].m_pBoundBufferResourceViews.Clear();
    m_Resources[i].m_pBoundTextureUnorderedAccessViews.Clear();
    m_Resources[i].m_pBoundBufferUnorderedAccessViews.Clear();
    m_Resources[i].m_pBoundSamplerStates.Clear();
  }

  m_renderPass = vk::RenderPassBeginInfo();
  m_clearValues.Clear();
}

void nsGALCommandEncoderImplVulkan::MarkDirty()
{
  NS_ASSERT_DEBUG(!m_bRenderPassActive, "Render pass was not closed");

  m_bPipelineStateDirty = true;
  m_bViewportDirty = true;
  m_bIndexBufferDirty = true;
  m_bDescriptorsDirty = true;
  m_BoundVertexBuffersRange.Reset();
  for (nsUInt32 i = 0; i < NS_GAL_MAX_VERTEX_BUFFER_COUNT; i++)
  {
    if (m_pBoundVertexBuffers[i])
      m_BoundVertexBuffersRange.SetToIncludeValue(i);
  }
}

void nsGALCommandEncoderImplVulkan::SetCurrentCommandBuffer(vk::CommandBuffer* commandBuffer, nsPipelineBarrierVulkan* pipelineBarrier)
{
  m_pCommandBuffer = commandBuffer;
  m_pPipelineBarrier = pipelineBarrier;
}

// State setting functions

void nsGALCommandEncoderImplVulkan::SetShaderPlatform(const nsGALShader* pShader)
{
  if (pShader != nullptr)
  {
    m_PipelineDesc.m_pCurrentShader = static_cast<const nsGALShaderVulkan*>(pShader);
    m_ComputeDesc.m_pCurrentShader = m_PipelineDesc.m_pCurrentShader;
    m_bPipelineStateDirty = true;
  }
}

void nsGALCommandEncoderImplVulkan::SetConstantBufferPlatform(const nsShaderResourceBinding& binding, const nsGALBuffer* pBuffer)
{
  // \todo Check if the device supports the slot index?
  m_Resources.EnsureCount(binding.m_iSet + 1);
  auto& resources = m_Resources[binding.m_iSet];
  resources.m_pBoundConstantBuffers.EnsureCount(binding.m_iSlot + 1);
  resources.m_pBoundConstantBuffers[binding.m_iSlot] = pBuffer != nullptr ? static_cast<const nsGALBufferVulkan*>(pBuffer) : nullptr;
  m_bDescriptorsDirty = true;
}

void nsGALCommandEncoderImplVulkan::SetSamplerStatePlatform(const nsShaderResourceBinding& binding, const nsGALSamplerState* pSamplerState)
{
  // \todo Check if the device supports the stage / the slot index
  m_Resources.EnsureCount(binding.m_iSet + 1);
  auto& resources = m_Resources[binding.m_iSet];
  resources.m_pBoundSamplerStates.EnsureCount(binding.m_iSlot + 1);
  resources.m_pBoundSamplerStates[binding.m_iSlot] = pSamplerState != nullptr ? static_cast<const nsGALSamplerStateVulkan*>(pSamplerState) : nullptr;
  m_bDescriptorsDirty = true;
}

void nsGALCommandEncoderImplVulkan::SetResourceViewPlatform(const nsShaderResourceBinding& binding, const nsGALTextureResourceView* pResourceView)
{
  m_Resources.EnsureCount(binding.m_iSet + 1);
  auto& resources = m_Resources[binding.m_iSet];
  resources.m_pBoundTextureResourceViews.EnsureCount(binding.m_iSlot + 1);
  resources.m_pBoundTextureResourceViews[binding.m_iSlot] = pResourceView != nullptr ? static_cast<const nsGALTextureResourceViewVulkan*>(pResourceView) : nullptr;
  m_bDescriptorsDirty = true;
}

void nsGALCommandEncoderImplVulkan::SetResourceViewPlatform(const nsShaderResourceBinding& binding, const nsGALBufferResourceView* pResourceView)
{
  m_Resources.EnsureCount(binding.m_iSet + 1);
  auto& resources = m_Resources[binding.m_iSet];
  resources.m_pBoundBufferResourceViews.EnsureCount(binding.m_iSlot + 1);
  resources.m_pBoundBufferResourceViews[binding.m_iSlot] = pResourceView != nullptr ? static_cast<const nsGALBufferResourceViewVulkan*>(pResourceView) : nullptr;
  m_bDescriptorsDirty = true;
}

void nsGALCommandEncoderImplVulkan::SetUnorderedAccessViewPlatform(const nsShaderResourceBinding& binding, const nsGALTextureUnorderedAccessView* pUnorderedAccessView)
{
  m_Resources.EnsureCount(binding.m_iSet + 1);
  auto& resources = m_Resources[binding.m_iSet];
  resources.m_pBoundTextureUnorderedAccessViews.EnsureCount(binding.m_iSlot + 1);
  resources.m_pBoundTextureUnorderedAccessViews[binding.m_iSlot] = pUnorderedAccessView != nullptr ? static_cast<const nsGALTextureUnorderedAccessViewVulkan*>(pUnorderedAccessView) : nullptr;
  m_bDescriptorsDirty = true;
}

void nsGALCommandEncoderImplVulkan::SetUnorderedAccessViewPlatform(const nsShaderResourceBinding& binding, const nsGALBufferUnorderedAccessView* pUnorderedAccessView)
{
  m_Resources.EnsureCount(binding.m_iSet + 1);
  auto& resources = m_Resources[binding.m_iSet];
  resources.m_pBoundBufferUnorderedAccessViews.EnsureCount(binding.m_iSlot + 1);
  resources.m_pBoundBufferUnorderedAccessViews[binding.m_iSlot] = pUnorderedAccessView != nullptr ? static_cast<const nsGALBufferUnorderedAccessViewVulkan*>(pUnorderedAccessView) : nullptr;
  m_bDescriptorsDirty = true;
}

void nsGALCommandEncoderImplVulkan::SetPushConstantsPlatform(nsArrayPtr<const nsUInt8> data)
{
  m_bPushConstantsDirty = true;
  m_PushConstants = data;
}

// Query functions

void nsGALCommandEncoderImplVulkan::BeginQueryPlatform(const nsGALQuery* pQuery)
{
  auto pVulkanQuery = static_cast<const nsGALQueryVulkan*>(pQuery);

  // TODO how to decide the query type etc in Vulkan?

  m_pCommandBuffer->beginQuery(pVulkanQuery->GetPool(), pVulkanQuery->GetID(), {});
}

void nsGALCommandEncoderImplVulkan::EndQueryPlatform(const nsGALQuery* pQuery)
{
  auto pVulkanQuery = static_cast<const nsGALQueryVulkan*>(pQuery);

  m_pCommandBuffer->endQuery(pVulkanQuery->GetPool(), pVulkanQuery->GetID());
}

nsResult nsGALCommandEncoderImplVulkan::GetQueryResultPlatform(const nsGALQuery* pQuery, nsUInt64& uiQueryResult)
{
  auto pVulkanQuery = static_cast<const nsGALQueryVulkan*>(pQuery);
  vk::Result result = m_vkDevice.getQueryPoolResults(pVulkanQuery->GetPool(), pVulkanQuery->GetID(), 1u, sizeof(nsUInt64), &uiQueryResult, 0, vk::QueryResultFlagBits::e64);

  return result == vk::Result::eSuccess ? NS_SUCCESS : NS_FAILURE;
}

void nsGALCommandEncoderImplVulkan::InsertTimestampPlatform(nsGALTimestampHandle hTimestamp)
{
  m_GALDeviceVulkan.GetQueryPool().InsertTimestamp(m_GALDeviceVulkan.GetCurrentCommandBuffer(), hTimestamp);
}

// Resource update functions

void nsGALCommandEncoderImplVulkan::ClearUnorderedAccessViewPlatform(const nsGALTextureUnorderedAccessView* pUnorderedAccessView, nsVec4 clearValues)
{
  // this looks to require custom code, either using buffer copies or
  // clearing via a compute shader
  NS_ASSERT_NOT_IMPLEMENTED;
}

void nsGALCommandEncoderImplVulkan::ClearUnorderedAccessViewPlatform(const nsGALBufferUnorderedAccessView* pUnorderedAccessView, nsVec4 clearValues)
{
  // Same as the other clearing variant
  NS_ASSERT_NOT_IMPLEMENTED;
}

void nsGALCommandEncoderImplVulkan::ClearUnorderedAccessViewPlatform(const nsGALTextureUnorderedAccessView* pUnorderedAccessView, nsVec4U32 clearValues)
{
  // Same as the other clearing variant
  NS_ASSERT_NOT_IMPLEMENTED;
}

void nsGALCommandEncoderImplVulkan::ClearUnorderedAccessViewPlatform(const nsGALBufferUnorderedAccessView* pUnorderedAccessView, nsVec4U32 clearValues)
{
  // Same as the other clearing variant
  NS_ASSERT_NOT_IMPLEMENTED;
}

void nsGALCommandEncoderImplVulkan::CopyBufferPlatform(const nsGALBuffer* pDestination, const nsGALBuffer* pSource)
{
  auto pDestinationVulkan = static_cast<const nsGALBufferVulkan*>(pDestination);
  auto pSourceVulkan = static_cast<const nsGALBufferVulkan*>(pSource);

  NS_ASSERT_DEV(pSource->GetSize() != pDestination->GetSize(), "Source and destination buffer sizes mismatch!");

  // TODO do this in an immediate command buffer?
  vk::BufferCopy bufferCopy = {};
  bufferCopy.size = pSource->GetSize();

  // #TODO_VULKAN better barrier management of buffers.
  m_pPipelineBarrier->Flush();

  m_pCommandBuffer->copyBuffer(pSourceVulkan->GetVkBuffer(), pDestinationVulkan->GetVkBuffer(), 1, &bufferCopy);

  m_pPipelineBarrier->AccessBuffer(pSourceVulkan, 0, bufferCopy.size, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferRead, pDestinationVulkan->GetUsedByPipelineStage(), pDestinationVulkan->GetAccessMask());
  m_pPipelineBarrier->AccessBuffer(pDestinationVulkan, 0, bufferCopy.size, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite, pDestinationVulkan->GetUsedByPipelineStage(), pDestinationVulkan->GetAccessMask());
}

void nsGALCommandEncoderImplVulkan::CopyBufferRegionPlatform(const nsGALBuffer* pDestination, nsUInt32 uiDestOffset, const nsGALBuffer* pSource,
  nsUInt32 uiSourceOffset, nsUInt32 uiByteCount)
{
  auto pDestinationVulkan = static_cast<const nsGALBufferVulkan*>(pDestination);
  auto pSourceVulkan = static_cast<const nsGALBufferVulkan*>(pSource);

  vk::BufferCopy bufferCopy = {};
  bufferCopy.dstOffset = uiDestOffset;
  bufferCopy.srcOffset = uiSourceOffset;
  bufferCopy.size = uiByteCount;

  // #TODO_VULKAN better barrier management of buffers.
  m_pPipelineBarrier->Flush();

  m_pCommandBuffer->copyBuffer(pSourceVulkan->GetVkBuffer(), pDestinationVulkan->GetVkBuffer(), 1, &bufferCopy);

  m_pPipelineBarrier->AccessBuffer(pSourceVulkan, 0, bufferCopy.size, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferRead, pDestinationVulkan->GetUsedByPipelineStage(), pDestinationVulkan->GetAccessMask());
  m_pPipelineBarrier->AccessBuffer(pDestinationVulkan, 0, bufferCopy.size, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite, pDestinationVulkan->GetUsedByPipelineStage(), pDestinationVulkan->GetAccessMask());
}

void nsGALCommandEncoderImplVulkan::UpdateBufferPlatform(const nsGALBuffer* pDestination, nsUInt32 uiDestOffset, nsArrayPtr<const nsUInt8> pSourceData,
  nsGALUpdateMode::Enum updateMode)
{
  NS_CHECK_ALIGNMENT_16(pSourceData.GetPtr());

  auto pVulkanDestination = static_cast<const nsGALBufferVulkan*>(pDestination);

  switch (updateMode)
  {
    case nsGALUpdateMode::Discard:
      pVulkanDestination->DiscardBuffer();
      [[fallthrough]];
    case nsGALUpdateMode::NoOverwrite:
    {
      nsVulkanAllocation alloc = pVulkanDestination->GetAllocation();
      void* pData = nullptr;
      VK_ASSERT_DEV(nsMemoryAllocatorVulkan::MapMemory(alloc, &pData));
      NS_ASSERT_DEV(pData, "Implementation error");
      nsMemoryUtils::Copy(nsMemoryUtils::AddByteOffset((nsUInt8*)pData, uiDestOffset), pSourceData.GetPtr(), pSourceData.GetCount());
      nsMemoryAllocatorVulkan::UnmapMemory(alloc);
    }
    break;
    case nsGALUpdateMode::CopyToTempStorage:
    {
      if (m_bRenderPassActive)
      {
        m_pCommandBuffer->endRenderPass();
        m_bRenderPassActive = false;
      }

      NS_ASSERT_DEBUG(!m_bRenderPassActive, "Vulkan does not support copying buffers while a render pass is active. TODO: Fix high level render code to make this impossible.");

      m_GALDeviceVulkan.UploadBufferStaging(&m_GALDeviceVulkan.GetStagingBufferPool(), m_pPipelineBarrier, *m_pCommandBuffer, pVulkanDestination, pSourceData, uiDestOffset);
    }
    break;
    default:
      break;
  }
}

void nsGALCommandEncoderImplVulkan::CopyTexturePlatform(const nsGALTexture* pDestination, const nsGALTexture* pSource)
{
  if (m_bRenderPassActive)
  {
    m_pCommandBuffer->endRenderPass();
    m_bRenderPassActive = false;
  }

  auto destination = static_cast<const nsGALTextureVulkan*>(pDestination->GetParentResource());
  auto source = static_cast<const nsGALTextureVulkan*>(pSource->GetParentResource());

  const nsGALTextureCreationDescription& destDesc = pDestination->GetDescription();
  const nsGALTextureCreationDescription& srcDesc = pSource->GetDescription();

  NS_ASSERT_DEBUG(nsGALResourceFormat::IsDepthFormat(destDesc.m_Format) == nsGALResourceFormat::IsDepthFormat(srcDesc.m_Format), "");
  NS_ASSERT_DEBUG(destDesc.m_uiArraySize == srcDesc.m_uiArraySize, "");
  NS_ASSERT_DEBUG(destDesc.m_uiMipLevelCount == srcDesc.m_uiMipLevelCount, "");
  NS_ASSERT_DEBUG(destDesc.m_uiWidth == srcDesc.m_uiWidth, "");
  NS_ASSERT_DEBUG(destDesc.m_uiHeight == srcDesc.m_uiHeight, "");
  NS_ASSERT_DEBUG(destDesc.m_uiDepth == srcDesc.m_uiDepth, "");

  vk::ImageAspectFlagBits imageAspect = nsGALResourceFormat::IsDepthFormat(destDesc.m_Format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;

  m_pPipelineBarrier->EnsureImageLayout(source, vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferRead);
  m_pPipelineBarrier->EnsureImageLayout(destination, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite);
  m_pPipelineBarrier->Flush();


  // TODO need to copy every mip level
  nsHybridArray<vk::ImageCopy, 14> imageCopies;

  for (nsUInt32 i = 0; i < destDesc.m_uiMipLevelCount; ++i)
  {
    vk::ImageCopy& imageCopy = imageCopies.ExpandAndGetRef();
    imageCopy.dstOffset = vk::Offset3D();
    imageCopy.dstSubresource.aspectMask = imageAspect;
    imageCopy.dstSubresource.baseArrayLayer = 0;
    imageCopy.dstSubresource.layerCount = destDesc.m_uiArraySize;
    imageCopy.dstSubresource.mipLevel = i;
    imageCopy.extent.width = destDesc.m_uiWidth;
    imageCopy.extent.height = destDesc.m_uiHeight;
    imageCopy.extent.depth = destDesc.m_uiDepth;
    imageCopy.srcOffset = vk::Offset3D();
    imageCopy.srcSubresource.aspectMask = imageAspect;
    imageCopy.srcSubresource.baseArrayLayer = 0;
    imageCopy.srcSubresource.layerCount = srcDesc.m_uiArraySize;
    imageCopy.srcSubresource.mipLevel = i;
  }

  m_pCommandBuffer->copyImage(source->GetImage(), vk::ImageLayout::eTransferSrcOptimal, destination->GetImage(), vk::ImageLayout::eTransferDstOptimal, destDesc.m_uiMipLevelCount, imageCopies.GetData());
}

void nsGALCommandEncoderImplVulkan::CopyTextureRegionPlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& DestinationSubResource,
  const nsVec3U32& DestinationPoint, const nsGALTexture* pSource,
  const nsGALTextureSubresource& SourceSubResource, const nsBoundingBoxu32& Box)
{
  auto destination = static_cast<const nsGALTextureVulkan*>(pDestination->GetParentResource());
  auto source = static_cast<const nsGALTextureVulkan*>(pSource->GetParentResource());

  const nsGALTextureCreationDescription& destDesc = pDestination->GetDescription();
  const nsGALTextureCreationDescription& srcDesc = pSource->GetDescription();

  NS_ASSERT_DEBUG(nsGALResourceFormat::IsDepthFormat(destDesc.m_Format) == nsGALResourceFormat::IsDepthFormat(srcDesc.m_Format), "");

  vk::ImageAspectFlagBits imageAspect = nsGALResourceFormat::IsDepthFormat(destDesc.m_Format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;

  nsVec3U32 extent = Box.m_vMax - Box.m_vMin;

  vk::ImageCopy imageCopy = {};
  imageCopy.dstOffset.x = DestinationPoint.x;
  imageCopy.dstOffset.y = DestinationPoint.y;
  imageCopy.dstOffset.z = DestinationPoint.z;
  imageCopy.dstSubresource.aspectMask = imageAspect;
  imageCopy.dstSubresource.baseArrayLayer = DestinationSubResource.m_uiArraySlice;
  imageCopy.dstSubresource.layerCount = 1;
  imageCopy.dstSubresource.mipLevel = DestinationSubResource.m_uiMipLevel;
  imageCopy.extent.width = extent.x;
  imageCopy.extent.height = extent.y;
  imageCopy.extent.depth = extent.z;
  imageCopy.srcOffset.x = Box.m_vMin.x;
  imageCopy.srcOffset.y = Box.m_vMin.y;
  imageCopy.srcOffset.z = Box.m_vMin.z;
  imageCopy.srcSubresource.aspectMask = imageAspect;
  imageCopy.srcSubresource.baseArrayLayer = DestinationSubResource.m_uiArraySlice;
  imageCopy.srcSubresource.layerCount = 1;
  imageCopy.srcSubresource.mipLevel = SourceSubResource.m_uiMipLevel;

  m_pCommandBuffer->copyImage(source->GetImage(), vk::ImageLayout::eGeneral, destination->GetImage(), vk::ImageLayout::eGeneral, 1, &imageCopy);
}

void nsGALCommandEncoderImplVulkan::UpdateTexturePlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& DestinationSubResource,
  const nsBoundingBoxu32& DestinationBox, const nsGALSystemMemoryDescription& data)
{
  const nsGALTextureVulkan* pDestVulkan = static_cast<const nsGALTextureVulkan*>(pDestination);
  vk::ImageSubresourceRange range = pDestVulkan->GetFullRange();
  range.baseArrayLayer = DestinationSubResource.m_uiArraySlice;
  range.baseMipLevel = DestinationSubResource.m_uiMipLevel;
  range.levelCount = 1;
  range.layerCount = 1;
  m_pPipelineBarrier->EnsureImageLayout(pDestVulkan, range, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite);
  m_pPipelineBarrier->Flush();

  nsUInt32 uiWidth = nsMath::Max(DestinationBox.m_vMax.x - DestinationBox.m_vMin.x, 1u);
  nsUInt32 uiHeight = nsMath::Max(DestinationBox.m_vMax.y - DestinationBox.m_vMin.y, 1u);
  nsUInt32 uiDepth = nsMath::Max(DestinationBox.m_vMax.z - DestinationBox.m_vMin.z, 1u);

  const vk::Format format = pDestVulkan->GetImageFormat();
  const nsUInt8 uiBlockSize = vk::blockSize(format);
  const auto blockExtent = vk::blockExtent(format);
  const VkExtent3D blockCount = {
    (uiWidth + blockExtent[0] - 1) / blockExtent[0],
    (uiHeight + blockExtent[1] - 1) / blockExtent[1],
    (uiDepth + blockExtent[2] - 1) / blockExtent[2]};

  const vk::DeviceSize uiTotalSize = uiBlockSize * blockCount.width * blockCount.height * blockCount.depth;
  nsStagingBufferVulkan stagingBuffer = m_GALDeviceVulkan.GetStagingBufferPool().AllocateBuffer(0, uiTotalSize);

  const nsUInt32 uiBufferRowPitch = uiBlockSize * blockCount.width;
  const nsUInt32 uiBufferSlicePitch = uiBufferRowPitch * blockCount.height;
  NS_ASSERT_DEV(uiBufferRowPitch == data.m_uiRowPitch, "Row pitch with padding is not implemented yet.");

  void* pData = nullptr;
  nsMemoryAllocatorVulkan::MapMemory(stagingBuffer.m_alloc, &pData);
  nsMemoryUtils::RawByteCopy(pData, data.m_pData, uiTotalSize);
  nsMemoryAllocatorVulkan::UnmapMemory(stagingBuffer.m_alloc);

  vk::BufferImageCopy region = {};
  region.imageSubresource.aspectMask = range.aspectMask;
  region.imageSubresource.mipLevel = range.baseMipLevel;
  region.imageSubresource.baseArrayLayer = range.baseArrayLayer;
  region.imageSubresource.layerCount = range.layerCount;

  region.imageOffset = vk::Offset3D(DestinationBox.m_vMin.x, DestinationBox.m_vMin.y, DestinationBox.m_vMin.z);
  region.imageExtent = vk::Extent3D(uiWidth, uiHeight, uiDepth);

  region.bufferOffset = 0;
  region.bufferRowLength = blockExtent[0] * uiBufferRowPitch / uiBlockSize;
  region.bufferImageHeight = blockExtent[1] * uiBufferSlicePitch / uiBufferRowPitch;

  m_pCommandBuffer->copyBufferToImage(stagingBuffer.m_buffer, pDestVulkan->GetImage(), pDestVulkan->GetPreferredLayout(vk::ImageLayout::eTransferDstOptimal), 1, &region);
  m_GALDeviceVulkan.GetStagingBufferPool().ReclaimBuffer(stagingBuffer);
}

void nsGALCommandEncoderImplVulkan::ResolveTexturePlatform(const nsGALTexture* pDestination, const nsGALTextureSubresource& DestinationSubResource,
  const nsGALTexture* pSource, const nsGALTextureSubresource& SourceSubResource)
{
  auto pVulkanDestination = static_cast<const nsGALTextureVulkan*>(pDestination->GetParentResource());
  auto pVulkanSource = static_cast<const nsGALTextureVulkan*>(pSource->GetParentResource());

  const nsGALTextureCreationDescription& destDesc = pDestination->GetDescription();
  const nsGALTextureCreationDescription& srcDesc = pSource->GetDescription();

  NS_ASSERT_DEBUG(nsGALResourceFormat::IsDepthFormat(destDesc.m_Format) == nsGALResourceFormat::IsDepthFormat(srcDesc.m_Format), "");

  // TODO need to determine size of the subresource
  vk::ImageResolve resolveRegion = {};
  resolveRegion.dstSubresource.aspectMask = pVulkanDestination->GetFullRange().aspectMask;
  resolveRegion.dstSubresource.baseArrayLayer = DestinationSubResource.m_uiArraySlice;
  resolveRegion.dstSubresource.layerCount = 1;
  resolveRegion.dstSubresource.mipLevel = DestinationSubResource.m_uiMipLevel;
  resolveRegion.extent.width = nsMath::Min(destDesc.m_uiWidth, srcDesc.m_uiWidth);
  resolveRegion.extent.height = nsMath::Min(destDesc.m_uiHeight, srcDesc.m_uiHeight);
  resolveRegion.extent.depth = nsMath::Min(destDesc.m_uiDepth, srcDesc.m_uiDepth);
  resolveRegion.srcSubresource.aspectMask = pVulkanSource->GetFullRange().aspectMask;
  resolveRegion.srcSubresource.baseArrayLayer = SourceSubResource.m_uiArraySlice;
  resolveRegion.srcSubresource.layerCount = 1;
  resolveRegion.srcSubresource.mipLevel = SourceSubResource.m_uiMipLevel;

  m_pPipelineBarrier->EnsureImageLayout(pVulkanSource, vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferRead);
  m_pPipelineBarrier->EnsureImageLayout(pVulkanDestination, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite);
  m_pPipelineBarrier->Flush();
  if (srcDesc.m_SampleCount != nsGALMSAASampleCount::None)
  {
    m_pCommandBuffer->resolveImage(pVulkanSource->GetImage(), vk::ImageLayout::eTransferSrcOptimal, pVulkanDestination->GetImage(), vk::ImageLayout::eTransferDstOptimal, 1, &resolveRegion);
  }
  else
  {
    // DX11 allows calling resolve on a non-msaa source. For now, allow this as well in Vulkan.
    vk::Extent3D sourceMipLevelSize = pVulkanSource->GetMipLevelSize(SourceSubResource.m_uiMipLevel);
    vk::Offset3D sourceMipLevelEndOffset = {(nsInt32)sourceMipLevelSize.width, (nsInt32)sourceMipLevelSize.height, (nsInt32)sourceMipLevelSize.depth};
    vk::Extent3D dstMipLevelSize = pVulkanDestination->GetMipLevelSize(DestinationSubResource.m_uiMipLevel);
    vk::Offset3D dstMipLevelEndOffset = {(nsInt32)sourceMipLevelSize.width, (nsInt32)sourceMipLevelSize.height, (nsInt32)sourceMipLevelSize.depth};

    vk::ImageBlit imageBlitRegion;
    imageBlitRegion.srcSubresource = resolveRegion.srcSubresource;
    imageBlitRegion.srcOffsets[1] = sourceMipLevelEndOffset;
    imageBlitRegion.dstSubresource = resolveRegion.dstSubresource;
    imageBlitRegion.dstOffsets[1] = dstMipLevelEndOffset;

    m_pCommandBuffer->blitImage(pVulkanSource->GetImage(), vk::ImageLayout::eTransferSrcOptimal, pVulkanDestination->GetImage(), vk::ImageLayout::eTransferDstOptimal, 1, &imageBlitRegion, vk::Filter::eNearest);
  }

  m_pPipelineBarrier->EnsureImageLayout(pVulkanSource, pVulkanSource->GetPreferredLayout(), pVulkanSource->GetUsedByPipelineStage(), pVulkanSource->GetAccessMask());
  m_pPipelineBarrier->EnsureImageLayout(pVulkanDestination, pVulkanDestination->GetPreferredLayout(), pVulkanDestination->GetUsedByPipelineStage(), pVulkanDestination->GetAccessMask());
}

void nsGALCommandEncoderImplVulkan::CopyImageToBuffer(const nsGALTextureVulkan* pSource, const nsGALBufferVulkan* pDestination)
{
  const nsGALTextureCreationDescription& textureDesc = pSource->GetDescription();
  const vk::ImageAspectFlags imageAspect = pSource->GetAspectMask();

  nsHybridArray<nsGALTextureVulkan::SubResourceOffset, 8> subResourceOffsets;
  const nsUInt32 uiBufferSize = pSource->ComputeSubResourceOffsets(subResourceOffsets);

  nsHybridArray<vk::BufferImageCopy, 8> imageCopy;
  const nsUInt32 arraySize = textureDesc.m_Type == nsGALTextureType::TextureCube ? textureDesc.m_uiArraySize * 6 : textureDesc.m_uiArraySize;
  const nsUInt32 mipLevels = textureDesc.m_uiMipLevelCount;

  for (nsUInt32 uiLayer = 0; uiLayer < arraySize; uiLayer++)
  {
    for (nsUInt32 uiMipLevel = 0; uiMipLevel < mipLevels; uiMipLevel++)
    {
      const vk::Extent3D mipLevelSize = pSource->GetMipLevelSize(uiMipLevel);
      const nsUInt32 uiSubresourceIndex = uiMipLevel + uiLayer * mipLevels;
      const nsGALTextureVulkan::SubResourceOffset& offset = subResourceOffsets[uiSubresourceIndex];

      vk::BufferImageCopy& copy = imageCopy.ExpandAndGetRef();

      copy.bufferOffset = offset.m_uiOffset;
      copy.bufferRowLength = offset.m_uiRowLength;
      copy.bufferImageHeight = offset.m_uiImageHeight;
      copy.imageSubresource.aspectMask = imageAspect;
      copy.imageSubresource.mipLevel = uiMipLevel;
      copy.imageSubresource.baseArrayLayer = uiLayer;
      copy.imageSubresource.layerCount = 1;
      copy.imageOffset = vk::Offset3D(0, 0, 0);
      copy.imageExtent = mipLevelSize;
    }
  }

  m_pPipelineBarrier->EnsureImageLayout(pSource, vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferRead);
  m_pPipelineBarrier->Flush();

  m_pCommandBuffer->copyImageToBuffer(pSource->GetImage(), vk::ImageLayout::eTransferSrcOptimal, pDestination->GetVkBuffer(), imageCopy.GetCount(), imageCopy.GetData());

  m_pPipelineBarrier->EnsureImageLayout(pSource, pSource->GetPreferredLayout(), pSource->GetUsedByPipelineStage(), pSource->GetAccessMask());
  m_pPipelineBarrier->AccessBuffer(pDestination, 0, uiBufferSize, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eMemoryRead);
}

void nsGALCommandEncoderImplVulkan::ReadbackTexturePlatform(const nsGALTexture* pTexture)
{
  if (!m_bClearSubmitted)
  {
    m_pPipelineBarrier->Flush();

    // If we want to readback one of the render targets, we need to first flush the clear.
    // #TODO_VULKAN Check whether pTexture is one of the render targets or change the top-level api to prevent this.
    m_pCommandBuffer->beginRenderPass(m_renderPass, vk::SubpassContents::eInline);
    m_bClearSubmitted = true;
    m_bRenderPassActive = true;
  }

  if (m_bRenderPassActive)
  {
    m_pCommandBuffer->endRenderPass();
    m_bRenderPassActive = false;
  }

  NS_ASSERT_DEV(!m_bRenderPassActive, "Can't readback within a render pass");

  const nsGALTextureVulkan* pVulkanTexture = static_cast<const nsGALTextureVulkan*>(pTexture->GetParentResource());
  const nsGALTextureCreationDescription& textureDesc = pVulkanTexture->GetDescription();
  const vk::ImageAspectFlagBits imageAspect = nsGALResourceFormat::IsDepthFormat(textureDesc.m_Format) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
  const bool bMSAASourceTexture = textureDesc.m_SampleCount != nsGALMSAASampleCount::None;
  NS_ASSERT_DEV(!bMSAASourceTexture, "MSAA read-back not implemented!");
  const nsGALTextureVulkan::StagingMode stagingMode = pVulkanTexture->GetStagingMode();
  NS_ASSERT_DEV(stagingMode != nsGALTextureVulkan::StagingMode::None, "No staging resource available for read-back");

  if (stagingMode == nsGALTextureVulkan::StagingMode::Buffer)
  {
    const nsGALBufferVulkan* pStagingBuffer = static_cast<const nsGALBufferVulkan*>(m_GALDeviceVulkan.GetBuffer(pVulkanTexture->GetStagingBuffer()));
    CopyImageToBuffer(pVulkanTexture, pStagingBuffer);
  }
  else
  {
    // Render to texture
    const nsGALTextureVulkan* pStagingTexture = static_cast<const nsGALTextureVulkan*>(m_GALDeviceVulkan.GetTexture(pVulkanTexture->GetStagingTexture()));
    const bool bSourceIsDepth = nsConversionUtilsVulkan::IsDepthFormat(pVulkanTexture->GetImageFormat());

    m_pPipelineBarrier->EnsureImageLayout(pVulkanTexture, nsConversionUtilsVulkan::GetDefaultLayout(pVulkanTexture->GetImageFormat()), vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eShaderRead);
    m_pPipelineBarrier->EnsureImageLayout(pStagingTexture, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead);
    m_pPipelineBarrier->Flush();

    nsImageCopyVulkan copy(m_GALDeviceVulkan);

    const bool bStereoSupport = m_GALDeviceVulkan.GetCapabilities().m_bVertexShaderRenderTargetArrayIndex || m_GALDeviceVulkan.GetCapabilities().m_bShaderStageSupported[nsGALShaderStage::GeometryShader];
    if (bStereoSupport)
    {
      copy.Init(pVulkanTexture, pStagingTexture, nsShaderUtils::nsBuiltinShaderType::CopyImageArray);
      const nsUInt32 arraySize = textureDesc.m_Type == nsGALTextureType::TextureCube ? textureDesc.m_uiArraySize * 6 : textureDesc.m_uiArraySize;
      const nsUInt32 mipLevels = textureDesc.m_uiMipLevelCount;
      for (nsUInt32 uiMipLevel = 0; uiMipLevel < textureDesc.m_uiMipLevelCount; uiMipLevel++)
      {
        vk::ImageSubresourceLayers subresourceLayersSource;
        subresourceLayersSource.aspectMask = imageAspect;
        subresourceLayersSource.mipLevel = uiMipLevel;
        subresourceLayersSource.baseArrayLayer = 0;
        subresourceLayersSource.layerCount = arraySize;

        vk::ImageSubresourceLayers subresourceLayersTarget;
        subresourceLayersTarget.aspectMask = pStagingTexture->GetAspectMask();
        subresourceLayersTarget.mipLevel = uiMipLevel;
        subresourceLayersTarget.baseArrayLayer = 0;
        subresourceLayersTarget.layerCount = arraySize;

        vk::Extent3D mipLevelSize = pVulkanTexture->GetMipLevelSize(0);
        copy.Copy({0, 0, 0}, subresourceLayersSource, {0, 0, 0}, subresourceLayersTarget, {mipLevelSize.width, mipLevelSize.height, mipLevelSize.depth});
      }
    }
    else
    {
      copy.Init(pVulkanTexture, pStagingTexture, nsShaderUtils::nsBuiltinShaderType::CopyImage);
      const nsUInt32 arraySize = textureDesc.m_Type == nsGALTextureType::TextureCube ? textureDesc.m_uiArraySize * 6 : textureDesc.m_uiArraySize;
      const nsUInt32 mipLevels = textureDesc.m_uiMipLevelCount;

      for (nsUInt32 uiLayer = 0; uiLayer < arraySize; uiLayer++)
      {
        for (nsUInt32 uiMipLevel = 0; uiMipLevel < mipLevels; uiMipLevel++)
        {
          vk::ImageSubresourceLayers subresourceLayersSource;
          subresourceLayersSource.aspectMask = imageAspect;
          subresourceLayersSource.mipLevel = uiMipLevel;
          subresourceLayersSource.baseArrayLayer = uiLayer;
          subresourceLayersSource.layerCount = 1;

          vk::ImageSubresourceLayers subresourceLayersTarget;
          subresourceLayersTarget.aspectMask = pStagingTexture->GetAspectMask();
          subresourceLayersTarget.mipLevel = uiMipLevel;
          subresourceLayersTarget.baseArrayLayer = uiLayer;
          subresourceLayersTarget.layerCount = 1;

          vk::Extent3D mipLevelSize = pVulkanTexture->GetMipLevelSize(0);
          copy.Copy({0, 0, 0}, subresourceLayersSource, {0, 0, 0}, subresourceLayersTarget, {mipLevelSize.width, mipLevelSize.height, mipLevelSize.depth});
        }
      }
    }

    m_bPipelineStateDirty = true;
    m_bViewportDirty = true;
    m_bDescriptorsDirty = true;

    if (stagingMode == nsGALTextureVulkan::StagingMode::TextureAndBuffer)
    {
      // Copy to buffer
      const nsGALBufferVulkan* pStagingBuffer = static_cast<const nsGALBufferVulkan*>(m_GALDeviceVulkan.GetBuffer(pVulkanTexture->GetStagingBuffer()));
      CopyImageToBuffer(pVulkanTexture, pStagingBuffer);
    }
    else
    {
      // Readback texture directly
      m_pPipelineBarrier->EnsureImageLayout(pStagingTexture, vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eHost, vk::AccessFlagBits::eHostRead);
    }
  }

  // There is no need to change the layout back of this texture right now but as the next layout will most certainly not be another eTransferSrcOptimal we might as well change it back to its default state.
  m_pPipelineBarrier->EnsureImageLayout(pVulkanTexture, pVulkanTexture->GetPreferredLayout(), pVulkanTexture->GetUsedByPipelineStage(), pVulkanTexture->GetAccessMask());

  // #TODO_VULKAN readback fence
  m_GALDeviceVulkan.Submit();
  m_vkDevice.waitIdle();
  m_pPipelineBarrier = &m_GALDeviceVulkan.GetCurrentPipelineBarrier();
  m_pCommandBuffer = &m_GALDeviceVulkan.GetCurrentCommandBuffer();
}

nsUInt32 GetMipSize(nsUInt32 uiSize, nsUInt32 uiMipLevel)
{
  for (nsUInt32 i = 0; i < uiMipLevel; i++)
  {
    uiSize = uiSize / 2;
  }
  return nsMath::Max(1u, uiSize);
}

void nsGALCommandEncoderImplVulkan::CopyTextureReadbackResultPlatform(const nsGALTexture* pTexture, nsArrayPtr<nsGALTextureSubresource> SourceSubResource, nsArrayPtr<nsGALSystemMemoryDescription> TargetData)
{
  // #TODO_VULKAN readback fence
  auto pVulkanTexture = static_cast<const nsGALTextureVulkan*>(pTexture->GetParentResource());
  const nsGALTextureCreationDescription& textureDesc = pVulkanTexture->GetDescription();
  const nsGALTextureVulkan::StagingMode stagingMode = pVulkanTexture->GetStagingMode();
  NS_ASSERT_DEV(stagingMode != nsGALTextureVulkan::StagingMode::None, "No staging resource available for read-back");

  if (stagingMode == nsGALTextureVulkan::StagingMode::Texture)
  {
    const nsGALTextureVulkan* pStagingTexture = static_cast<const nsGALTextureVulkan*>(m_GALDeviceVulkan.GetTexture(pVulkanTexture->GetStagingTexture()));
    vk::ImageAspectFlags stagingAspect = pStagingTexture->GetAspectMask();

    const nsUInt32 uiSubResources = SourceSubResource.GetCount();

    void* pData = nullptr;
    nsMemoryAllocatorVulkan::MapMemory(pStagingTexture->GetAllocation(), &pData);

    for (nsUInt32 i = 0; i < uiSubResources; i++)
    {
      const nsGALTextureSubresource& subRes = SourceSubResource[i];
      const nsGALSystemMemoryDescription& memDesc = TargetData[i];

      vk::ImageSubresource subResource{stagingAspect, subRes.m_uiMipLevel, subRes.m_uiArraySlice};
      vk::SubresourceLayout subResourceLayout;
      m_vkDevice.getImageSubresourceLayout(pStagingTexture->GetImage(), &subResource, &subResourceLayout);
      nsUInt8* pSubResourceData = reinterpret_cast<nsUInt8*>(pData) + subResourceLayout.offset;

      if (subResourceLayout.rowPitch == memDesc.m_uiRowPitch)
      {
        const nsUInt32 uiMemorySize = nsGALResourceFormat::GetBitsPerElement(textureDesc.m_Format) *
                                      GetMipSize(textureDesc.m_uiWidth, subRes.m_uiMipLevel) *
                                      GetMipSize(textureDesc.m_uiHeight, subRes.m_uiMipLevel) / 8;

        memcpy(memDesc.m_pData, pSubResourceData, uiMemorySize);
      }
      else
      {
        // Copy row by row
        const nsUInt32 uiHeight = GetMipSize(textureDesc.m_uiHeight, subRes.m_uiMipLevel);
        for (nsUInt32 y = 0; y < uiHeight; ++y)
        {
          const void* pSource = nsMemoryUtils::AddByteOffset(pSubResourceData, y * subResourceLayout.rowPitch);
          void* pDest = nsMemoryUtils::AddByteOffset(memDesc.m_pData, y * memDesc.m_uiRowPitch);

          memcpy(pDest, pSource, nsGALResourceFormat::GetBitsPerElement(textureDesc.m_Format) * GetMipSize(textureDesc.m_uiWidth, subRes.m_uiMipLevel) / 8);
        }
      }
    }

    nsMemoryAllocatorVulkan::UnmapMemory(pStagingTexture->GetAllocation());
  }
  else // One of the buffer variants.
  {
    const nsGALBufferVulkan* pStagingBuffer = static_cast<const nsGALBufferVulkan*>(m_GALDeviceVulkan.GetBuffer(pVulkanTexture->GetStagingBuffer()));
    const vk::Format stagingFormat = m_GALDeviceVulkan.GetFormatLookupTable().GetFormatInfo(pVulkanTexture->GetDescription().m_Format).m_readback;

    nsHybridArray<nsGALTextureVulkan::SubResourceOffset, 8> subResourceOffsets;
    const nsUInt32 uiBufferSize = pVulkanTexture->ComputeSubResourceOffsets(subResourceOffsets);


    const nsUInt32 uiSubResources = SourceSubResource.GetCount();

    void* pData = nullptr;
    nsMemoryAllocatorVulkan::MapMemory(pStagingBuffer->GetAllocation(), &pData);

    const nsUInt32 uiMipLevels = textureDesc.m_uiMipLevelCount;
    for (nsUInt32 i = 0; i < uiSubResources; i++)
    {
      const nsGALTextureSubresource& subRes = SourceSubResource[i];
      const nsUInt32 uiSubresourceIndex = subRes.m_uiMipLevel + subRes.m_uiArraySlice * uiMipLevels;
      const nsGALTextureVulkan::SubResourceOffset offset = subResourceOffsets[uiSubresourceIndex];
      const nsGALSystemMemoryDescription& memDesc = TargetData[i];
      const auto blockExtent = vk::blockExtent(stagingFormat);
      const nsUInt8 uiBlockSize = vk::blockSize(stagingFormat);

      const nsUInt32 uiRowPitch = offset.m_uiRowLength * blockExtent[0] * uiBlockSize;

      nsUInt8* pSubResourceData = reinterpret_cast<nsUInt8*>(pData) + offset.m_uiOffset;

      if (uiRowPitch == memDesc.m_uiRowPitch)
      {
        const nsUInt32 uiMemorySize = nsGALResourceFormat::GetBitsPerElement(textureDesc.m_Format) *
                                      GetMipSize(textureDesc.m_uiWidth, subRes.m_uiMipLevel) *
                                      GetMipSize(textureDesc.m_uiHeight, subRes.m_uiMipLevel) / 8;

        memcpy(memDesc.m_pData, pSubResourceData, uiMemorySize);
      }
      else
      {
        // Copy row by row
        const nsUInt32 uiHeight = GetMipSize(textureDesc.m_uiHeight, subRes.m_uiMipLevel);
        for (nsUInt32 y = 0; y < uiHeight; ++y)
        {
          const void* pSource = nsMemoryUtils::AddByteOffset(pSubResourceData, y * uiRowPitch);
          void* pDest = nsMemoryUtils::AddByteOffset(memDesc.m_pData, y * memDesc.m_uiRowPitch);

          memcpy(pDest, pSource, nsGALResourceFormat::GetBitsPerElement(textureDesc.m_Format) * GetMipSize(textureDesc.m_uiWidth, subRes.m_uiMipLevel) / 8);
        }
      }
    }

    nsMemoryAllocatorVulkan::UnmapMemory(pStagingBuffer->GetAllocation());
  }
}

void nsGALCommandEncoderImplVulkan::GenerateMipMapsPlatform(const nsGALTextureResourceView* pResourceView)
{
  const nsGALTextureResourceViewVulkan* pVulkanResourceView = static_cast<const nsGALTextureResourceViewVulkan*>(pResourceView);
  if (m_bRenderPassActive)
  {
    m_pCommandBuffer->endRenderPass();
    m_bRenderPassActive = false;
  }


  const vk::ImageSubresourceRange viewRange = pVulkanResourceView->GetRange();
  if (viewRange.levelCount == 1)
    return;

  const nsGALTextureVulkan* pVulkanTexture = static_cast<const nsGALTextureVulkan*>(pVulkanResourceView->GetResource()->GetParentResource());
  const vk::FormatProperties formatProps = m_GALDeviceVulkan.GetVulkanPhysicalDevice().getFormatProperties(pVulkanTexture->GetImageFormat());
  const bool bSupportsBlit = ((formatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eBlitSrc) && (formatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eBlitDst));
  // MSAA textures (e.g. backbuffers) need to be converted to non MSAA versions
  const nsGALTextureCreationDescription& textureDesc = pVulkanTexture->GetDescription();
  const bool bMSAASourceTexture = textureDesc.m_SampleCount != nsGALMSAASampleCount::None;
  if (bMSAASourceTexture)
  {
    NS_ASSERT_NOT_IMPLEMENTED;
  }
  else
  {
    if (bSupportsBlit)
    {
      {
        vk::ImageSubresourceRange otherLevels = viewRange;
        otherLevels.baseMipLevel += 1;
        otherLevels.levelCount -= 1;
        m_pPipelineBarrier->EnsureImageLayout(pVulkanTexture, otherLevels, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferWrite);
      }

      for (nsUInt32 uiMipLevel = viewRange.baseMipLevel; uiMipLevel < (viewRange.baseMipLevel + viewRange.levelCount - 1); uiMipLevel++)
      {
        {
          vk::ImageSubresourceRange currentLevel = viewRange;
          currentLevel.baseMipLevel = uiMipLevel;
          currentLevel.levelCount = 1;
          m_pPipelineBarrier->EnsureImageLayout(pVulkanTexture, currentLevel, vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits::eTransfer, vk::AccessFlagBits::eTransferRead);
          m_pPipelineBarrier->Flush();
        }
        vk::Extent3D sourceMipLevelSize = pVulkanTexture->GetMipLevelSize(uiMipLevel);
        vk::Offset3D sourceMipLevelEndOffset = {(nsInt32)sourceMipLevelSize.width, (nsInt32)sourceMipLevelSize.height, (nsInt32)sourceMipLevelSize.depth};
        vk::Extent3D destinationMipLevelSize = pVulkanTexture->GetMipLevelSize(uiMipLevel + 1);
        vk::Offset3D destinationMipLevelEndOffset = {(nsInt32)destinationMipLevelSize.width, (nsInt32)destinationMipLevelSize.height, (nsInt32)destinationMipLevelSize.depth};

        vk::ImageSubresourceLayers sourceLayers;
        sourceLayers.aspectMask = viewRange.aspectMask;
        sourceLayers.mipLevel = uiMipLevel;
        sourceLayers.baseArrayLayer = viewRange.baseArrayLayer;
        sourceLayers.layerCount = viewRange.layerCount;

        vk::ImageSubresourceLayers destinationLayers = sourceLayers;
        destinationLayers.mipLevel++;

        vk::ImageBlit imageBlitRegion;
        imageBlitRegion.srcSubresource = sourceLayers;
        imageBlitRegion.srcOffsets[1] = sourceMipLevelEndOffset;
        imageBlitRegion.dstSubresource = destinationLayers;
        imageBlitRegion.dstOffsets[1] = destinationMipLevelEndOffset;

        m_pCommandBuffer->blitImage(pVulkanTexture->GetImage(), vk::ImageLayout::eTransferSrcOptimal, pVulkanTexture->GetImage(), vk::ImageLayout::eTransferDstOptimal, 1, &imageBlitRegion, vk::Filter::eLinear);
      }
      // There is no need to change the layout back of this texture right now but as the next layout will most certainly not be another eTransferSrcOptimal we might as well change it back to its default state.
      m_pPipelineBarrier->EnsureImageLayout(pVulkanResourceView, pVulkanTexture->GetPreferredLayout(), pVulkanTexture->GetUsedByPipelineStage(), pVulkanTexture->GetAccessMask());
    }
    else
    {
      {
        vk::ImageSubresourceRange otherLevels = viewRange;
        otherLevels.baseMipLevel += 1;
        otherLevels.levelCount -= 1;
        m_pPipelineBarrier->EnsureImageLayout(pVulkanTexture, otherLevels, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead);
      }

      nsImageCopyVulkan copy(m_GALDeviceVulkan);
      const bool bStereoSupport = m_GALDeviceVulkan.GetCapabilities().m_bVertexShaderRenderTargetArrayIndex || m_GALDeviceVulkan.GetCapabilities().m_bShaderStageSupported[nsGALShaderStage::GeometryShader];
      if (bStereoSupport)
      {
        copy.Init(pVulkanTexture, pVulkanTexture, nsShaderUtils::nsBuiltinShaderType::DownscaleImageArray);
        for (nsUInt32 uiMipLevel = viewRange.baseMipLevel; uiMipLevel < (viewRange.baseMipLevel + viewRange.levelCount - 1); uiMipLevel++)
        {
          vk::Extent3D sourceMipLevelSize = pVulkanTexture->GetMipLevelSize(uiMipLevel);
          vk::Offset3D sourceMipLevelEndOffset = {(nsInt32)sourceMipLevelSize.width, (nsInt32)sourceMipLevelSize.height, (nsInt32)sourceMipLevelSize.depth};
          vk::Extent3D destinationMipLevelSize = pVulkanTexture->GetMipLevelSize(uiMipLevel + 1);
          vk::Offset3D destinationMipLevelEndOffset = {(nsInt32)destinationMipLevelSize.width, (nsInt32)destinationMipLevelSize.height, (nsInt32)destinationMipLevelSize.depth};

          vk::ImageSubresourceLayers sourceLayers;
          sourceLayers.aspectMask = viewRange.aspectMask;
          sourceLayers.mipLevel = uiMipLevel;
          sourceLayers.baseArrayLayer = viewRange.baseArrayLayer;
          sourceLayers.layerCount = viewRange.layerCount;

          vk::ImageSubresourceLayers destinationLayers = sourceLayers;
          destinationLayers.mipLevel++;

          vk::Extent3D mipLevelSize = pVulkanTexture->GetMipLevelSize(0);
          copy.Copy({0, 0, 0}, sourceLayers, {0, 0, 0}, destinationLayers, {(nsUInt32)destinationMipLevelSize.width, (nsUInt32)destinationMipLevelSize.height, (nsUInt32)destinationMipLevelSize.depth});
        }
      }
      else
      {
        copy.Init(pVulkanTexture, pVulkanTexture, nsShaderUtils::nsBuiltinShaderType::DownscaleImage);
        const nsUInt32 arraySize = textureDesc.m_Type == nsGALTextureType::TextureCube ? textureDesc.m_uiArraySize * 6 : textureDesc.m_uiArraySize;
        const nsUInt32 mipLevels = textureDesc.m_uiMipLevelCount;

        for (nsUInt32 uiLayer = viewRange.baseArrayLayer; uiLayer < (viewRange.baseArrayLayer + viewRange.layerCount); uiLayer++)
        {
          for (nsUInt32 uiMipLevel = viewRange.baseMipLevel; uiMipLevel < (viewRange.baseMipLevel + viewRange.levelCount - 1); uiMipLevel++)
          {
            vk::Extent3D sourceMipLevelSize = pVulkanTexture->GetMipLevelSize(uiMipLevel);
            vk::Offset3D sourceMipLevelEndOffset = {(nsInt32)sourceMipLevelSize.width, (nsInt32)sourceMipLevelSize.height, (nsInt32)sourceMipLevelSize.depth};
            vk::Extent3D destinationMipLevelSize = pVulkanTexture->GetMipLevelSize(uiMipLevel + 1);
            vk::Offset3D destinationMipLevelEndOffset = {(nsInt32)destinationMipLevelSize.width, (nsInt32)destinationMipLevelSize.height, (nsInt32)destinationMipLevelSize.depth};

            vk::ImageSubresourceLayers sourceLayers;
            sourceLayers.aspectMask = viewRange.aspectMask;
            sourceLayers.mipLevel = uiMipLevel;
            sourceLayers.baseArrayLayer = viewRange.baseArrayLayer;
            sourceLayers.layerCount = 1;

            vk::ImageSubresourceLayers destinationLayers = sourceLayers;
            destinationLayers.mipLevel++;

            vk::Extent3D mipLevelSize = pVulkanTexture->GetMipLevelSize(0);
            copy.Copy({0, 0, 0}, sourceLayers, {0, 0, 0}, destinationLayers, {(nsUInt32)destinationMipLevelSize.width, (nsUInt32)destinationMipLevelSize.height, (nsUInt32)destinationMipLevelSize.depth});
          }
        }
      }

      m_pPipelineBarrier->EnsureImageLayout(pVulkanResourceView, pVulkanTexture->GetPreferredLayout(), pVulkanTexture->GetUsedByPipelineStage(), pVulkanTexture->GetAccessMask());

      m_bPipelineStateDirty = true;
      m_bViewportDirty = true;
      m_bDescriptorsDirty = true;
    }
  }
}

void nsGALCommandEncoderImplVulkan::FlushPlatform()
{
}

// Debug helper functions

void nsGALCommandEncoderImplVulkan::PushMarkerPlatform(const char* szMarker)
{
  if (m_GALDeviceVulkan.GetExtensions().m_bDebugUtilsMarkers)
  {
    constexpr float markerColor[4] = {1, 1, 1, 1};
    vk::DebugUtilsLabelEXT markerInfo = {};
    nsMemoryUtils::Copy(markerInfo.color.data(), markerColor, NS_ARRAY_SIZE(markerColor));
    markerInfo.pLabelName = szMarker;

    m_pCommandBuffer->beginDebugUtilsLabelEXT(markerInfo);
  }
}

void nsGALCommandEncoderImplVulkan::PopMarkerPlatform()
{
  if (m_GALDeviceVulkan.GetExtensions().m_bDebugUtilsMarkers)
  {
    m_pCommandBuffer->endDebugUtilsLabelEXT();
  }
}

void nsGALCommandEncoderImplVulkan::InsertEventMarkerPlatform(const char* szMarker)
{
  if (m_GALDeviceVulkan.GetExtensions().m_bDebugUtilsMarkers)
  {
    constexpr float markerColor[4] = {1, 1, 1, 1};
    vk::DebugUtilsLabelEXT markerInfo = {};
    nsMemoryUtils::Copy(markerInfo.color.data(), markerColor, NS_ARRAY_SIZE(markerColor));
    markerInfo.pLabelName = szMarker;
    m_pCommandBuffer->insertDebugUtilsLabelEXT(markerInfo);
  }
}

//////////////////////////////////////////////////////////////////////////

void nsGALCommandEncoderImplVulkan::BeginRendering(const nsGALRenderingSetup& renderingSetup)
{
  m_PipelineDesc.m_renderPass = nsResourceCacheVulkan::RequestRenderPass(renderingSetup);
  m_PipelineDesc.m_uiAttachmentCount = renderingSetup.m_RenderTargetSetup.GetRenderTargetCount();
  nsSizeU32 size;
  m_frameBuffer = nsResourceCacheVulkan::RequestFrameBuffer(m_PipelineDesc.m_renderPass, renderingSetup.m_RenderTargetSetup, size, m_PipelineDesc.m_msaa, m_uiLayers);

  SetScissorRectPlatform(nsRectU32(size.width, size.height));

  {
    m_renderPass.renderPass = m_PipelineDesc.m_renderPass;
    m_renderPass.framebuffer = m_frameBuffer;
    m_renderPass.renderArea.offset.setX(0).setY(0);
    m_renderPass.renderArea.extent.setHeight(size.height).setWidth(size.width);

    m_clearValues.Clear();

    const bool m_bHasDepth = !renderingSetup.m_RenderTargetSetup.GetDepthStencilTarget().IsInvalidated();
    const nsUInt32 uiColorCount = renderingSetup.m_RenderTargetSetup.GetRenderTargetCount();
    m_bClearSubmitted = !(renderingSetup.m_bClearDepth || renderingSetup.m_bClearStencil || renderingSetup.m_uiRenderTargetClearMask);

    if (m_bHasDepth)
    {
      vk::ClearValue& depthClear = m_clearValues.ExpandAndGetRef();
      depthClear.depthStencil.setDepth(1.0f).setStencil(0);

      const nsGALRenderTargetViewVulkan* pRenderTargetView = static_cast<const nsGALRenderTargetViewVulkan*>(m_GALDeviceVulkan.GetRenderTargetView(renderingSetup.m_RenderTargetSetup.GetDepthStencilTarget()));
      m_depthMask = pRenderTargetView->GetRange().aspectMask;
      m_pPipelineBarrier->EnsureImageLayout(pRenderTargetView, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests, vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead);
    }
    for (nsUInt32 i = 0; i < uiColorCount; i++)
    {
      vk::ClearValue& colorClear = m_clearValues.ExpandAndGetRef();
      nsColor col = renderingSetup.m_ClearColor;
      colorClear.color.setFloat32({col.r, col.g, col.b, col.a});

      const nsGALRenderTargetViewVulkan* pRenderTargetView = static_cast<const nsGALRenderTargetViewVulkan*>(m_GALDeviceVulkan.GetRenderTargetView(renderingSetup.m_RenderTargetSetup.GetRenderTarget(i)));
      m_pPipelineBarrier->EnsureImageLayout(pRenderTargetView, vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead);
    }

    m_renderPass.clearValueCount = m_clearValues.GetCount();
    m_renderPass.pClearValues = m_clearValues.GetData();
  }

  m_bPipelineStateDirty = true;
  m_bViewportDirty = true;
}

void nsGALCommandEncoderImplVulkan::EndRendering()
{
  if (!m_bClearSubmitted)
  {
    m_pPipelineBarrier->Flush();
    // If we end rendering without having flushed the clear, just begin and immediately end rendering.
    m_pCommandBuffer->beginRenderPass(m_renderPass, vk::SubpassContents::eInline);
    m_bClearSubmitted = true;
    m_bRenderPassActive = true;
  }

  if (m_bRenderPassActive)
  {
    m_pCommandBuffer->endRenderPass();
    m_bRenderPassActive = false;
  }

  m_depthMask = {};
  m_uiLayers = 0;
  m_PipelineDesc.m_msaa = nsGALMSAASampleCount::None;
  m_PipelineDesc.m_renderPass = nullptr;
  m_frameBuffer = nullptr;
}

void nsGALCommandEncoderImplVulkan::ClearPlatform(const nsColor& ClearColor, nsUInt32 uiRenderTargetClearMask, bool bClearDepth, bool bClearStencil, float fDepthClear, nsUInt8 uiStencilClear)
{
  if (!m_bRenderPassActive && !m_bInsideCompute)
  {
    if (m_pPipelineBarrier->IsDirty())
    {
      m_pPipelineBarrier->Flush();
    }

    m_pCommandBuffer->beginRenderPass(m_renderPass, vk::SubpassContents::eInline);
    m_bClearSubmitted = true;
    m_bRenderPassActive = true;
  }
  // #TODO_VULKAN Not sure if we need barriers here.
  nsHybridArray<vk::ClearAttachment, 8> attachments;

  // Clear color
  if (uiRenderTargetClearMask != 0)
  {
    for (nsUInt32 i = 0; i < NS_GAL_MAX_RENDERTARGET_COUNT; i++)
    {
      if (uiRenderTargetClearMask & (1u << i) && i < m_PipelineDesc.m_uiAttachmentCount)
      {
        vk::ClearAttachment& attachment = attachments.ExpandAndGetRef();
        attachment.aspectMask = vk::ImageAspectFlagBits::eColor;
        attachment.clearValue.color.setFloat32({ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a});
        attachment.colorAttachment = i;
      }
    }
  }
  // Clear depth / stencil
  if ((bClearDepth || bClearStencil) && m_depthMask != vk::ImageAspectFlagBits::eNone)
  {
    vk::ClearAttachment& attachment = attachments.ExpandAndGetRef();
    if (bClearDepth && (m_depthMask & vk::ImageAspectFlagBits::eDepth))
    {
      attachment.aspectMask |= vk::ImageAspectFlagBits::eDepth;
      attachment.clearValue.depthStencil.setDepth(fDepthClear);
    }
    if (bClearStencil && (m_depthMask & vk::ImageAspectFlagBits::eStencil))
    {
      attachment.aspectMask |= vk::ImageAspectFlagBits::eStencil;
      attachment.clearValue.depthStencil.setStencil(uiStencilClear);
    }
  }

  vk::ClearRect rect;
  rect.baseArrayLayer = 0;
  rect.layerCount = m_uiLayers;
  rect.rect = m_renderPass.renderArea;
  m_pCommandBuffer->clearAttachments(attachments.GetCount(), attachments.GetData(), 1, &rect);
}

// Draw functions

nsResult nsGALCommandEncoderImplVulkan::DrawPlatform(nsUInt32 uiVertexCount, nsUInt32 uiStartVertex)
{
  NS_SUCCEED_OR_RETURN(FlushDeferredStateChanges());

  m_pCommandBuffer->draw(uiVertexCount, 1, uiStartVertex, 0);
  return NS_SUCCESS;
}

nsResult nsGALCommandEncoderImplVulkan::DrawIndexedPlatform(nsUInt32 uiIndexCount, nsUInt32 uiStartIndex)
{
  NS_SUCCEED_OR_RETURN(FlushDeferredStateChanges());

  m_pCommandBuffer->drawIndexed(uiIndexCount, 1, uiStartIndex, 0, 0);
  return NS_SUCCESS;
}

nsResult nsGALCommandEncoderImplVulkan::DrawIndexedInstancedPlatform(nsUInt32 uiIndexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartIndex)
{
  NS_SUCCEED_OR_RETURN(FlushDeferredStateChanges());

  m_pCommandBuffer->drawIndexed(uiIndexCountPerInstance, uiInstanceCount, uiStartIndex, 0, 0);
  return NS_SUCCESS;
}

nsResult nsGALCommandEncoderImplVulkan::DrawIndexedInstancedIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes)
{
  NS_SUCCEED_OR_RETURN(FlushDeferredStateChanges());

  m_pCommandBuffer->drawIndexedIndirect(static_cast<const nsGALBufferVulkan*>(pIndirectArgumentBuffer)->GetVkBuffer(), uiArgumentOffsetInBytes, 1, 0);
  return NS_SUCCESS;
}

nsResult nsGALCommandEncoderImplVulkan::DrawInstancedPlatform(nsUInt32 uiVertexCountPerInstance, nsUInt32 uiInstanceCount, nsUInt32 uiStartVertex)
{
  NS_SUCCEED_OR_RETURN(FlushDeferredStateChanges());

  m_pCommandBuffer->draw(uiVertexCountPerInstance, uiInstanceCount, uiStartVertex, 0);
  return NS_SUCCESS;
}

nsResult nsGALCommandEncoderImplVulkan::DrawInstancedIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes)
{
  NS_SUCCEED_OR_RETURN(FlushDeferredStateChanges());

  m_pCommandBuffer->drawIndirect(static_cast<const nsGALBufferVulkan*>(pIndirectArgumentBuffer)->GetVkBuffer(), uiArgumentOffsetInBytes, 1, 0);
  return NS_SUCCESS;
}

void nsGALCommandEncoderImplVulkan::SetIndexBufferPlatform(const nsGALBuffer* pIndexBuffer)
{
  if (m_pIndexBuffer != pIndexBuffer)
  {
    m_pIndexBuffer = static_cast<const nsGALBufferVulkan*>(pIndexBuffer);
    m_bIndexBufferDirty = true;
  }
}

void nsGALCommandEncoderImplVulkan::SetVertexBufferPlatform(nsUInt32 uiSlot, const nsGALBuffer* pVertexBuffer)
{
  NS_ASSERT_DEV(uiSlot < NS_GAL_MAX_VERTEX_BUFFER_COUNT, "Invalid slot index");
  vk::Buffer buffer = pVertexBuffer != nullptr ? static_cast<const nsGALBufferVulkan*>(pVertexBuffer)->GetVkBuffer() : nullptr;
  nsUInt32 stride = pVertexBuffer != nullptr ? pVertexBuffer->GetDescription().m_uiStructSize : 0;

  if (buffer != m_pBoundVertexBuffers[uiSlot])
  {
    m_pBoundVertexBuffers[uiSlot] = buffer;
    m_BoundVertexBuffersRange.SetToIncludeValue(uiSlot);

    if (m_PipelineDesc.m_VertexBufferStrides[uiSlot] != stride)
    {
      m_PipelineDesc.m_VertexBufferStrides[uiSlot] = stride;
      m_bPipelineStateDirty = true;
    }
  }
}

void nsGALCommandEncoderImplVulkan::SetVertexDeclarationPlatform(const nsGALVertexDeclaration* pVertexDeclaration)
{
  if (m_PipelineDesc.m_pCurrentVertexDecl != pVertexDeclaration)
  {
    m_PipelineDesc.m_pCurrentVertexDecl = static_cast<const nsGALVertexDeclarationVulkan*>(pVertexDeclaration);
    m_bPipelineStateDirty = true;
  }
}

void nsGALCommandEncoderImplVulkan::SetPrimitiveTopologyPlatform(nsGALPrimitiveTopology::Enum Topology)
{
  if (m_PipelineDesc.m_topology != Topology)
  {
    m_PipelineDesc.m_topology = Topology;
    m_bPipelineStateDirty = true;
  }
}

void nsGALCommandEncoderImplVulkan::SetBlendStatePlatform(const nsGALBlendState* pBlendState, const nsColor& BlendFactor, nsUInt32 uiSampleMask)
{
  // #TODO_VULKAN BlendFactor / uiSampleMask ?
  if (m_PipelineDesc.m_pCurrentBlendState != pBlendState)
  {
    m_PipelineDesc.m_pCurrentBlendState = pBlendState != nullptr ? static_cast<const nsGALBlendStateVulkan*>(pBlendState) : nullptr;
    m_bPipelineStateDirty = true;
  }
}

void nsGALCommandEncoderImplVulkan::SetDepthStencilStatePlatform(const nsGALDepthStencilState* pDepthStencilState, nsUInt8 uiStencilRefValue)
{
  // #TODO_VULKAN uiStencilRefValue ?
  if (m_PipelineDesc.m_pCurrentDepthStencilState != pDepthStencilState)
  {
    m_PipelineDesc.m_pCurrentDepthStencilState = pDepthStencilState != nullptr ? static_cast<const nsGALDepthStencilStateVulkan*>(pDepthStencilState) : nullptr;
    m_bPipelineStateDirty = true;
  }
}

void nsGALCommandEncoderImplVulkan::SetRasterizerStatePlatform(const nsGALRasterizerState* pRasterizerState)
{
  if (m_PipelineDesc.m_pCurrentRasterizerState != pRasterizerState)
  {
    m_PipelineDesc.m_pCurrentRasterizerState = pRasterizerState != nullptr ? static_cast<const nsGALRasterizerStateVulkan*>(pRasterizerState) : nullptr;
    if (m_PipelineDesc.m_pCurrentRasterizerState->GetDescription().m_bScissorTest != m_bScissorEnabled)
    {
      m_bScissorEnabled = m_PipelineDesc.m_pCurrentRasterizerState->GetDescription().m_bScissorTest;
      m_bViewportDirty = true;
    }
    m_bPipelineStateDirty = true;
  }
}

void nsGALCommandEncoderImplVulkan::SetViewportPlatform(const nsRectFloat& rect, float fMinDepth, float fMaxDepth)
{
  // We use nsClipSpaceYMode::Regular and rely in the Vulkan 1.1 feature that a negative height performs y-inversion of the clip-space to framebuffer-space transform.
  // https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VK_KHR_maintenance1.html
  vk::Viewport viewport = {rect.x, rect.height + rect.y, rect.width, -rect.height, fMinDepth, fMaxDepth};
  if (m_viewport != viewport)
  {
    // Viewport is marked as dynamic in the pipeline layout and thus does not mark m_bPipelineStateDirty.
    m_viewport = viewport;
    m_bViewportDirty = true;
  }
}

void nsGALCommandEncoderImplVulkan::SetScissorRectPlatform(const nsRectU32& rect)
{
  vk::Rect2D scissor(vk::Offset2D(rect.x, rect.y), vk::Extent2D(rect.width, rect.height));
  if (m_scissor != scissor)
  {
    // viewport is marked as dynamic in the pipeline layout and thus does not mark m_bPipelineStateDirty.
    m_scissor = scissor;
    m_bViewportDirty = true;
  }
}

//////////////////////////////////////////////////////////////////////////

void nsGALCommandEncoderImplVulkan::BeginCompute()
{
  m_bClearSubmitted = true;
  m_bInsideCompute = true;
  m_bPipelineStateDirty = true;
}

void nsGALCommandEncoderImplVulkan::EndCompute()
{
  m_bInsideCompute = false;
}

nsResult nsGALCommandEncoderImplVulkan::DispatchPlatform(nsUInt32 uiThreadGroupCountX, nsUInt32 uiThreadGroupCountY, nsUInt32 uiThreadGroupCountZ)
{
  NS_SUCCEED_OR_RETURN(FlushDeferredStateChanges());
  m_pCommandBuffer->dispatch(uiThreadGroupCountX, uiThreadGroupCountY, uiThreadGroupCountZ);
  return NS_SUCCESS;
}

nsResult nsGALCommandEncoderImplVulkan::DispatchIndirectPlatform(const nsGALBuffer* pIndirectArgumentBuffer, nsUInt32 uiArgumentOffsetInBytes)
{
  NS_SUCCEED_OR_RETURN(FlushDeferredStateChanges());
  m_pCommandBuffer->dispatchIndirect(static_cast<const nsGALBufferVulkan*>(pIndirectArgumentBuffer)->GetVkBuffer(), uiArgumentOffsetInBytes);
  return NS_SUCCESS;
}

#define NS_VULKAN_CHECK_STATE(bCondition, szErrorMsg, ...)                                  \
  do                                                                                        \
  {                                                                                         \
    /*NS_ASSERT_DEBUG(bCondition, szErrorMsg, ##__VA_ARGS__); */                            \
    NS_MSVC_ANALYSIS_WARNING_PUSH                                                           \
    NS_MSVC_ANALYSIS_WARNING_DISABLE(6326) /* disable static analysis for the comparison */ \
    if (!!(bCondition) == false)                                                            \
    {                                                                                       \
      nsLog::Error(szErrorMsg, ##__VA_ARGS__);                                              \
      return NS_FAILURE;                                                                    \
    }                                                                                       \
    NS_MSVC_ANALYSIS_WARNING_POP                                                            \
  } while (false)

nsResult nsGALCommandEncoderImplVulkan::FlushDeferredStateChanges()
{
  if (m_bPipelineStateDirty)
  {
    if (!m_PipelineDesc.m_pCurrentShader)
    {
      nsLog::Error("No shader set");
      return NS_FAILURE;
    }

    const nsUInt32 uiSets = m_PipelineDesc.m_pCurrentShader->GetSetCount();
    m_LayoutDesc.m_layout.SetCount(uiSets);
    m_LayoutDesc.m_pushConstants = m_PipelineDesc.m_pCurrentShader->GetPushConstantRange();
    for (nsUInt32 uiSet = 0; uiSet < uiSets; ++uiSet)
    {
      m_LayoutDesc.m_layout[uiSet] = m_PipelineDesc.m_pCurrentShader->GetDescriptorSetLayout(uiSet);
    }

    m_PipelineDesc.m_layout = nsResourceCacheVulkan::RequestPipelineLayout(m_LayoutDesc);
    m_ComputeDesc.m_layout = m_PipelineDesc.m_layout;

    vk::Pipeline pipeline;
    if (m_bInsideCompute)
    {
      pipeline = nsResourceCacheVulkan::RequestComputePipeline(m_ComputeDesc);
    }
    else
    {
      pipeline = nsResourceCacheVulkan::RequestGraphicsPipeline(m_PipelineDesc);
    }

    m_pCommandBuffer->bindPipeline(m_bInsideCompute ? vk::PipelineBindPoint::eCompute : vk::PipelineBindPoint::eGraphics, pipeline);
    m_bPipelineStateDirty = false;
    // Changes to the descriptor layout always require the descriptor set to be re-created.
    m_bDescriptorsDirty = true;
  }

  if (!m_bInsideCompute && m_bViewportDirty)
  {
    m_pCommandBuffer->setViewport(0, 1, &m_viewport);
    if (m_bScissorEnabled)
      m_pCommandBuffer->setScissor(0, 1, &m_scissor);
    else
    {
      vk::Rect2D noScissor({int(m_viewport.x), int(m_viewport.y + m_viewport.height)}, {nsUInt32(m_viewport.width), nsUInt32(-m_viewport.height)});
      m_pCommandBuffer->setScissor(0, 1, &noScissor);
    }
    m_bViewportDirty = false;
  }

  if (!m_bInsideCompute && m_BoundVertexBuffersRange.IsValid())
  {
    const nsUInt32 uiStartSlot = m_BoundVertexBuffersRange.m_uiMin;
    const nsUInt32 uiNumSlots = m_BoundVertexBuffersRange.GetCount();

    nsUInt32 uiCurrentStartSlot = uiStartSlot;
    // Finding valid ranges.
    for (nsUInt32 i = uiStartSlot; i < (uiStartSlot + uiNumSlots); i++)
    {
      if (!m_pBoundVertexBuffers[i])
      {
        if (i - uiCurrentStartSlot > 0)
        {
          // There are some null elements in the array. We can't submit these to Vulkan and need to skip them so flush everything before it.
          m_pCommandBuffer->bindVertexBuffers(uiCurrentStartSlot, i - uiCurrentStartSlot, m_pBoundVertexBuffers + uiCurrentStartSlot, m_VertexBufferOffsets + uiCurrentStartSlot);
        }
        uiCurrentStartSlot = i + 1;
      }
    }
    // The last element in the buffer range must always be valid so we can simply flush the rest.
    if (m_pBoundVertexBuffers[uiCurrentStartSlot])
      m_pCommandBuffer->bindVertexBuffers(uiCurrentStartSlot, m_BoundVertexBuffersRange.m_uiMax - uiCurrentStartSlot + 1, m_pBoundVertexBuffers + uiCurrentStartSlot, m_VertexBufferOffsets + uiCurrentStartSlot);

    m_BoundVertexBuffersRange.Reset();
  }

  if (!m_bInsideCompute && m_bIndexBufferDirty)
  {
    if (m_pIndexBuffer)
      m_pCommandBuffer->bindIndexBuffer(m_pIndexBuffer->GetVkBuffer(), 0, m_pIndexBuffer->GetIndexType());
    m_bIndexBufferDirty = false;
  }

  if (true /*m_bDescriptorsDirty*/)
  {
    // #TODO_VULKAN we always create a new descriptor set as we don't know if a buffer was modified since the last draw call (nsGALBufferVulkan::DiscardBuffer).
    //  Need to figure out a fast check if any buffer or buffer of a resource view was discarded.
    m_bDescriptorsDirty = false;

    m_DescriptorWrites.Clear();
    m_TextureAndSampler.Clear();
    const nsUInt32 uiSets = m_PipelineDesc.m_pCurrentShader->GetSetCount();
    m_DescriptorSets.SetCount(uiSets);
    for (nsUInt32 uiSet = 0; uiSet < uiSets; ++uiSet)
    {
      m_DescriptorSets[uiSet] = nsDescriptorSetPoolVulkan::CreateDescriptorSet(m_LayoutDesc.m_layout[uiSet]);

      nsArrayPtr<const nsShaderResourceBinding> bindingMapping = m_PipelineDesc.m_pCurrentShader->GetBindings(uiSet);
      const nsUInt32 uiCount = bindingMapping.GetCount();

      m_Resources.EnsureCount(uiSet + 1);
      auto& resources = m_Resources[uiSet];

      for (nsUInt32 i = 0; i < uiCount; i++)
      {
        const nsShaderResourceBinding& mapping = bindingMapping[i];
        vk::WriteDescriptorSet& write = m_DescriptorWrites.ExpandAndGetRef();
        write.dstArrayElement = 0;
        write.descriptorType = nsConversionUtilsVulkan::GetDescriptorType(mapping.m_ResourceType);
        write.dstBinding = mapping.m_iSlot; // #TODO_VULKAN this should be i + arrayIndex or something?
        write.dstSet = m_DescriptorSets[uiSet];
        write.descriptorCount = mapping.m_uiArraySize;
        switch (mapping.m_ResourceType)
        {
          case nsGALShaderResourceType::ConstantBuffer:
          {
            const nsGALBufferVulkan* pBuffer = mapping.m_iSlot < resources.m_pBoundConstantBuffers.GetCount() ? resources.m_pBoundConstantBuffers[mapping.m_iSlot] : nullptr;
            NS_VULKAN_CHECK_STATE(pBuffer != nullptr, "No CB bound at '{}'", mapping.m_sName.GetView());
            write.pBufferInfo = &pBuffer->GetBufferInfo();
          }
          break;
          case nsGALShaderResourceType::Texture:
          case nsGALShaderResourceType::TextureAndSampler:
          {
            const nsGALTextureResourceViewVulkan* pResourceView = GetTextureResourceView(resources, mapping);
            write.pImageInfo = &pResourceView->GetImageInfo(nsGALShaderTextureType::IsArray(mapping.m_TextureType));

            const auto* pTexture = static_cast<const nsGALTextureVulkan*>(pResourceView->GetResource()->GetParentResource());
            const bool bIsDepth = nsGALResourceFormat::IsDepthFormat(pTexture->GetDescription().m_Format);

            m_pPipelineBarrier->EnsureImageLayout(pResourceView, pTexture->GetPreferredLayout(nsConversionUtilsVulkan::GetDefaultLayout(pTexture->GetImageFormat())), nsConversionUtilsVulkan::GetPipelineStages(mapping.m_Stages), vk::AccessFlagBits::eShaderRead);

            if (mapping.m_ResourceType == nsGALShaderResourceType::TextureAndSampler)
            {
              // TextureAndSampler is the only one where we have to combine two resources, requiring us to create a dynamic entry for the pImageInfo field.
              const nsGALSamplerStateVulkan* pSampler = mapping.m_iSlot < resources.m_pBoundSamplerStates.GetCount() ? resources.m_pBoundSamplerStates[mapping.m_iSlot] : nullptr;
              NS_VULKAN_CHECK_STATE(pSampler != nullptr, "No sampler bound at '{}'", mapping.m_sName.GetView());
              m_TextureAndSampler.PushBack(*write.pImageInfo);
              m_TextureAndSampler.PeekBack().sampler = pSampler->GetImageInfo().sampler;
              write.pImageInfo = &m_TextureAndSampler.PeekBack();
            }
          }
          break;
          case nsGALShaderResourceType::TexelBuffer:
          {
            const nsGALBufferResourceViewVulkan* pResourceView = GetBufferResourceView(resources, mapping);
            NS_VULKAN_CHECK_STATE(pResourceView != nullptr, "No SRV bound at '{}'", mapping.m_sName.GetView());
            write.pTexelBufferView = &pResourceView->GetBufferView();
          }
          break;
          case nsGALShaderResourceType::StructuredBuffer:
          {
            const nsGALBufferResourceViewVulkan* pResourceView = GetBufferResourceView(resources, mapping);
            NS_VULKAN_CHECK_STATE(pResourceView != nullptr, "No SRV bound at '{}'", mapping.m_sName.GetView());
            write.pBufferInfo = &pResourceView->GetBufferInfo();
          }
          break;
          case nsGALShaderResourceType::TextureRW:
          {
            const nsGALTextureUnorderedAccessViewVulkan* pUAV = GetTextureUAV(resources, mapping);
            write.pImageInfo = &pUAV->GetImageInfo();

            const auto* pTexture = static_cast<const nsGALTextureVulkan*>(pUAV->GetResource()->GetParentResource());
            m_pPipelineBarrier->EnsureImageLayout(pUAV, pTexture->GetPreferredLayout(vk::ImageLayout::eGeneral), nsConversionUtilsVulkan::GetPipelineStages(mapping.m_Stages), vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite);
          }
          break;
          case nsGALShaderResourceType::TexelBufferRW:
          {
            const nsGALBufferUnorderedAccessViewVulkan* pUAV = GetBufferUAV(resources, mapping);
            NS_VULKAN_CHECK_STATE(pUAV != nullptr, "No UAV bound at '{}'", mapping.m_sName.GetView());
            write.pTexelBufferView = &pUAV->GetBufferView();
          }
          break;
          case nsGALShaderResourceType::StructuredBufferRW:
          {
            const nsGALBufferUnorderedAccessViewVulkan* pUAV = GetBufferUAV(resources, mapping);
            NS_VULKAN_CHECK_STATE(pUAV != nullptr, "No UAV bound at '{}'", mapping.m_sName.GetView());
            write.pBufferInfo = &pUAV->GetBufferInfo();
          }
          break;
          case nsGALShaderResourceType::Sampler:
          {
            const nsGALSamplerStateVulkan* pSampler = mapping.m_iSlot < resources.m_pBoundSamplerStates.GetCount() ? resources.m_pBoundSamplerStates[mapping.m_iSlot] : nullptr;
            NS_VULKAN_CHECK_STATE(pSampler != nullptr, "No sampler bound at '{}'", mapping.m_sName.GetView());
            write.pImageInfo = &pSampler->GetImageInfo();
          }
          break;
          default:
            break;
        }
      }

      nsDescriptorSetPoolVulkan::UpdateDescriptorSet(m_DescriptorSets[uiSet], m_DescriptorWrites);
    }
    m_pCommandBuffer->bindDescriptorSets(m_bInsideCompute ? vk::PipelineBindPoint::eCompute : vk::PipelineBindPoint::eGraphics, m_PipelineDesc.m_layout, 0, m_DescriptorSets.GetCount(), m_DescriptorSets.GetData(), 0, nullptr);
  }

  if (m_bPushConstantsDirty && m_LayoutDesc.m_pushConstants.size > 0)
  {
    NS_ASSERT_DEBUG(m_LayoutDesc.m_pushConstants.size == m_PushConstants.GetCount(), "");

    m_pCommandBuffer->pushConstants(m_PipelineDesc.m_layout, m_LayoutDesc.m_pushConstants.stageFlags, m_LayoutDesc.m_pushConstants.offset, m_PushConstants.GetCount(), m_PushConstants.GetData());
  }

  if (m_bRenderPassActive && m_pPipelineBarrier->IsDirty())
  {
    m_pCommandBuffer->endRenderPass();
    m_bRenderPassActive = false;

    m_pPipelineBarrier->FullBarrier();
  }

  m_pPipelineBarrier->Flush();

  if (!m_bRenderPassActive && !m_bInsideCompute)
  {
    m_pCommandBuffer->beginRenderPass(m_renderPass, vk::SubpassContents::eInline);
    m_bClearSubmitted = true;
    m_bRenderPassActive = true;
  }
  return NS_SUCCESS;
}

const nsGALTextureResourceViewVulkan* nsGALCommandEncoderImplVulkan::GetTextureResourceView(const SetResources& resources, const nsShaderResourceBinding& mapping)
{
  const nsGALTextureResourceViewVulkan* pResourceView = nullptr;
  if (mapping.m_iSlot < resources.m_pBoundTextureResourceViews.GetCount())
  {
    pResourceView = resources.m_pBoundTextureResourceViews[mapping.m_iSlot];
  }

  if (!pResourceView)
  {
    nsStringBuilder sName = mapping.m_sName.GetData();
    bool bDepth = sName.FindSubString_NoCase("shadow") != nullptr || sName.FindSubString_NoCase("depth");
    pResourceView = nsFallbackResourcesVulkan::GetFallbackTextureResourceView(mapping.m_ResourceType, mapping.m_TextureType, bDepth);
  }
  return pResourceView;
}

const nsGALBufferResourceViewVulkan* nsGALCommandEncoderImplVulkan::GetBufferResourceView(const SetResources& resources, const nsShaderResourceBinding& mapping)
{
  const nsGALBufferResourceViewVulkan* pResourceView = nullptr;
  if (mapping.m_iSlot < resources.m_pBoundBufferResourceViews.GetCount())
  {
    pResourceView = resources.m_pBoundBufferResourceViews[mapping.m_iSlot];
  }

  if (!pResourceView)
  {
    nsStringBuilder sName = mapping.m_sName.GetData();
    bool bDepth = sName.FindSubString_NoCase("shadow") != nullptr || sName.FindSubString_NoCase("depth");
    pResourceView = nsFallbackResourcesVulkan::GetFallbackBufferResourceView(mapping.m_ResourceType);
  }
  return pResourceView;
}

const nsGALTextureUnorderedAccessViewVulkan* nsGALCommandEncoderImplVulkan::GetTextureUAV(const SetResources& resources, const nsShaderResourceBinding& mapping)
{
  const nsGALTextureUnorderedAccessViewVulkan* pUAV = nullptr;
  if (mapping.m_iSlot < resources.m_pBoundTextureUnorderedAccessViews.GetCount())
  {
    pUAV = resources.m_pBoundTextureUnorderedAccessViews[mapping.m_iSlot];
  }

  if (!pUAV)
  {
    pUAV = nsFallbackResourcesVulkan::GetFallbackTextureUnorderedAccessView(mapping.m_ResourceType, mapping.m_TextureType);
  }
  return pUAV;
}

const nsGALBufferUnorderedAccessViewVulkan* nsGALCommandEncoderImplVulkan::GetBufferUAV(const SetResources& resources, const nsShaderResourceBinding& mapping)
{
  const nsGALBufferUnorderedAccessViewVulkan* pUAV = nullptr;
  if (mapping.m_iSlot < resources.m_pBoundBufferUnorderedAccessViews.GetCount())
  {
    pUAV = resources.m_pBoundBufferUnorderedAccessViews[mapping.m_iSlot];
  }

  if (!pUAV)
  {
    pUAV = nsFallbackResourcesVulkan::GetFallbackBufferUnorderedAccessView(mapping.m_ResourceType);
  }
  return pUAV;
}
