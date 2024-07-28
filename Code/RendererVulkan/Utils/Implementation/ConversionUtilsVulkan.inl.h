#include <RendererFoundation/Resources/ResourceFormats.h>
#include <RendererVulkan/Utils/ConversionUtilsVulkan.h>


namespace
{
  bool IsArrayViewInternal(const nsGALTextureCreationDescription& texDesc, const nsGALTextureResourceViewCreationDescription& viewDesc)
  {
    return texDesc.m_uiArraySize > 1 || viewDesc.m_uiFirstArraySlice > 0;
  }
  bool IsArrayViewInternal(const nsGALTextureCreationDescription& texDesc, const nsGALTextureUnorderedAccessViewCreationDescription& viewDesc)
  {
    return texDesc.m_uiArraySize > 1 || viewDesc.m_uiFirstArraySlice > 0;
  }
} // namespace

NS_ALWAYS_INLINE vk::SampleCountFlagBits nsConversionUtilsVulkan::GetSamples(nsEnum<nsGALMSAASampleCount> samples)
{
  switch (samples)
  {
    case nsGALMSAASampleCount::None:
      return vk::SampleCountFlagBits::e1;
    case nsGALMSAASampleCount::TwoSamples:
      return vk::SampleCountFlagBits::e2;
    case nsGALMSAASampleCount::FourSamples:
      return vk::SampleCountFlagBits::e4;
    case nsGALMSAASampleCount::EightSamples:
      return vk::SampleCountFlagBits::e8;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      return vk::SampleCountFlagBits::e1;
  }
}

NS_ALWAYS_INLINE vk::PresentModeKHR nsConversionUtilsVulkan::GetPresentMode(nsEnum<nsGALPresentMode> presentMode, const nsDynamicArray<vk::PresentModeKHR>& supportedModes)
{
  switch (presentMode)
  {
    case nsGALPresentMode::Immediate:
    {
      if (supportedModes.Contains(vk::PresentModeKHR::eImmediate))
        return vk::PresentModeKHR::eImmediate;
      else if (supportedModes.Contains(vk::PresentModeKHR::eMailbox))
        return vk::PresentModeKHR::eMailbox;
      else
        return vk::PresentModeKHR::eFifo;
    }
    case nsGALPresentMode::VSync:
      return vk::PresentModeKHR::eFifo; // FIFO must be supported according to the standard.
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      return vk::PresentModeKHR::eFifo;
  }
}

NS_ALWAYS_INLINE vk::ImageSubresourceRange nsConversionUtilsVulkan::GetSubresourceRange(const nsGALTextureCreationDescription& texDesc, const nsGALRenderTargetViewCreationDescription& viewDesc)
{
  vk::ImageSubresourceRange range;
  nsGALResourceFormat::Enum viewFormat = viewDesc.m_OverrideViewFormat == nsGALResourceFormat::Invalid ? texDesc.m_Format : viewDesc.m_OverrideViewFormat;
  range.aspectMask = nsGALResourceFormat::IsDepthFormat(viewFormat) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
  range.setBaseMipLevel(viewDesc.m_uiMipLevel).setLevelCount(1).setBaseArrayLayer(viewDesc.m_uiFirstSlice).setLayerCount(viewDesc.m_uiSliceCount);
  return range;
}

NS_ALWAYS_INLINE vk::ImageSubresourceRange nsConversionUtilsVulkan::GetSubresourceRange(const nsGALTextureCreationDescription& texDesc, const nsGALTextureResourceViewCreationDescription& viewDesc)
{
  vk::ImageSubresourceRange range;

  const bool bIsArrayView = IsArrayViewInternal(texDesc, viewDesc);

  nsGALResourceFormat::Enum viewFormat = viewDesc.m_OverrideViewFormat == nsGALResourceFormat::Invalid ? texDesc.m_Format : viewDesc.m_OverrideViewFormat;
  range.aspectMask = nsGALResourceFormat::IsDepthFormat(viewFormat) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
  if (viewFormat == nsGALResourceFormat::D24S8)
  {
    range.aspectMask |= vk::ImageAspectFlagBits::eStencil;
  }
  range.baseMipLevel = viewDesc.m_uiMostDetailedMipLevel;
  range.levelCount = nsMath::Min(viewDesc.m_uiMipLevelsToUse, texDesc.m_uiMipLevelCount - range.baseMipLevel);

  switch (texDesc.m_Type)
  {
    case nsGALTextureType::Texture2D:
    case nsGALTextureType::Texture2DProxy:
    case nsGALTextureType::Texture2DShared:
      range.layerCount = viewDesc.m_uiArraySize;
      range.baseArrayLayer = viewDesc.m_uiFirstArraySlice;
      break;
    case nsGALTextureType::TextureCube:
      range.baseArrayLayer = viewDesc.m_uiFirstArraySlice;
      range.layerCount = viewDesc.m_uiArraySize * 6;
      break;
    case nsGALTextureType::Texture3D:
      range.layerCount = 1;
      break;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
  }
  return range;
}


NS_ALWAYS_INLINE vk::ImageSubresourceRange nsConversionUtilsVulkan::GetSubresourceRange(const nsGALTextureCreationDescription& texDesc, const nsGALTextureUnorderedAccessViewCreationDescription& viewDesc)
{
  vk::ImageSubresourceRange range;

  const bool bIsArrayView = IsArrayViewInternal(texDesc, viewDesc);

  nsGALResourceFormat::Enum viewFormat = viewDesc.m_OverrideViewFormat == nsGALResourceFormat::Invalid ? texDesc.m_Format : viewDesc.m_OverrideViewFormat;
  range.aspectMask = nsGALResourceFormat::IsDepthFormat(viewFormat) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
  if (viewFormat == nsGALResourceFormat::D24S8)
  {
    range.aspectMask |= vk::ImageAspectFlagBits::eStencil;
  }

  range.baseMipLevel = viewDesc.m_uiMipLevelToUse;
  range.levelCount = 1;
  range.layerCount = viewDesc.m_uiArraySize;

  switch (texDesc.m_Type)
  {
    case nsGALTextureType::Texture2D:
    case nsGALTextureType::Texture2DProxy:
    case nsGALTextureType::Texture2DShared:
      range.baseArrayLayer = viewDesc.m_uiFirstArraySlice;
      break;
    case nsGALTextureType::TextureCube:
      range.baseArrayLayer = viewDesc.m_uiFirstArraySlice;
      break;
    case nsGALTextureType::Texture3D:
      if (bIsArrayView)
      {
        NS_ASSERT_NOT_IMPLEMENTED;
      }
      else
      {
        range.baseArrayLayer = viewDesc.m_uiFirstArraySlice;
      }
      break;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
  }
  return range;
}

NS_ALWAYS_INLINE vk::ImageSubresourceRange nsConversionUtilsVulkan::GetSubresourceRange(
  const vk::ImageSubresourceLayers& layers)
{
  vk::ImageSubresourceRange range;
  range.aspectMask = layers.aspectMask;
  range.baseMipLevel = layers.mipLevel;
  range.levelCount = 1;
  range.baseArrayLayer = layers.baseArrayLayer;
  range.layerCount = layers.layerCount;
  return range;
}

NS_ALWAYS_INLINE vk::ImageViewType nsConversionUtilsVulkan::GetImageViewType(nsEnum<nsGALTextureType> texType, bool bIsArrayView)
{
  switch (texType)
  {
    case nsGALTextureType::Texture2D:
    case nsGALTextureType::Texture2DProxy:
    case nsGALTextureType::Texture2DShared:
      if (!bIsArrayView)
      {
        return vk::ImageViewType::e2D;
      }
      else
      {
        return vk::ImageViewType::e2DArray;
      }
    case nsGALTextureType::TextureCube:
      if (!bIsArrayView)
      {
        return vk::ImageViewType::eCube;
      }
      else
      {
        return vk::ImageViewType::eCubeArray;
      }
    case nsGALTextureType::Texture3D:
      return vk::ImageViewType::e3D;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      return vk::ImageViewType::e1D;
  }
}

NS_ALWAYS_INLINE bool nsConversionUtilsVulkan::IsDepthFormat(vk::Format format)
{
  switch (format)
  {
    case vk::Format::eD16Unorm:
    case vk::Format::eD32Sfloat:
    case vk::Format::eD16UnormS8Uint:
    case vk::Format::eD24UnormS8Uint:
    case vk::Format::eD32SfloatS8Uint:
      return true;
    default:
      return false;
  }
}

NS_ALWAYS_INLINE bool nsConversionUtilsVulkan::IsStencilFormat(vk::Format format)
{
  switch (format)
  {
    case vk::Format::eS8Uint:
    case vk::Format::eD16UnormS8Uint:
    case vk::Format::eD24UnormS8Uint:
    case vk::Format::eD32SfloatS8Uint:
      return true;
    default:
      return false;
  }
}

NS_ALWAYS_INLINE vk::ImageLayout nsConversionUtilsVulkan::GetDefaultLayout(vk::Format format)
{
  return IsDepthFormat(format) ? vk::ImageLayout::eDepthStencilReadOnlyOptimal : vk::ImageLayout::eShaderReadOnlyOptimal;
}

NS_ALWAYS_INLINE vk::PrimitiveTopology nsConversionUtilsVulkan::GetPrimitiveTopology(nsEnum<nsGALPrimitiveTopology> topology)
{
  switch (topology)
  {
    case nsGALPrimitiveTopology::Points:
      return vk::PrimitiveTopology::ePointList;
    case nsGALPrimitiveTopology::Lines:
      return vk::PrimitiveTopology::eLineList;
    case nsGALPrimitiveTopology::Triangles:
      return vk::PrimitiveTopology::eTriangleList;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      return vk::PrimitiveTopology::ePointList;
  }
}

NS_ALWAYS_INLINE vk::ShaderStageFlagBits nsConversionUtilsVulkan::GetShaderStage(nsGALShaderStage::Enum stage)
{
  switch (stage)
  {
    case nsGALShaderStage::VertexShader:
      return vk::ShaderStageFlagBits::eVertex;
    case nsGALShaderStage::HullShader:
      return vk::ShaderStageFlagBits::eTessellationControl;
    case nsGALShaderStage::DomainShader:
      return vk::ShaderStageFlagBits::eTessellationEvaluation;
    case nsGALShaderStage::GeometryShader:
      return vk::ShaderStageFlagBits::eGeometry;
    case nsGALShaderStage::PixelShader:
      return vk::ShaderStageFlagBits::eFragment;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      [[fallthrough]];
    case nsGALShaderStage::ComputeShader:
      return vk::ShaderStageFlagBits::eCompute;
  }
}

NS_ALWAYS_INLINE vk::PipelineStageFlags nsConversionUtilsVulkan::GetPipelineStage(nsGALShaderStage::Enum stage)
{
  switch (stage)
  {
    case nsGALShaderStage::VertexShader:
      return vk::PipelineStageFlagBits::eVertexShader;
    case nsGALShaderStage::HullShader:
      return vk::PipelineStageFlagBits::eTessellationControlShader;
    case nsGALShaderStage::DomainShader:
      return vk::PipelineStageFlagBits::eTessellationEvaluationShader;
    case nsGALShaderStage::GeometryShader:
      return vk::PipelineStageFlagBits::eGeometryShader;
    case nsGALShaderStage::PixelShader:
      return vk::PipelineStageFlagBits::eFragmentShader;
    default:
      NS_ASSERT_NOT_IMPLEMENTED;
      [[fallthrough]];
    case nsGALShaderStage::ComputeShader:
      return vk::PipelineStageFlagBits::eComputeShader;
  }
}

NS_ALWAYS_INLINE vk::PipelineStageFlags nsConversionUtilsVulkan::GetPipelineStage(vk::ShaderStageFlags flags)
{
  vk::PipelineStageFlags res;
  if (flags & vk::ShaderStageFlagBits::eVertex)
    res |= vk::PipelineStageFlagBits::eVertexShader;
  if (flags & vk::ShaderStageFlagBits::eTessellationControl)
    res |= vk::PipelineStageFlagBits::eTessellationControlShader;
  if (flags & vk::ShaderStageFlagBits::eTessellationEvaluation)
    res |= vk::PipelineStageFlagBits::eTessellationEvaluationShader;
  if (flags & vk::ShaderStageFlagBits::eGeometry)
    res |= vk::PipelineStageFlagBits::eGeometryShader;
  if (flags & vk::ShaderStageFlagBits::eFragment)
    res |= vk::PipelineStageFlagBits::eFragmentShader;
  if (flags & vk::ShaderStageFlagBits::eCompute)
    res |= vk::PipelineStageFlagBits::eComputeShader;

  return res;
}

NS_ALWAYS_INLINE vk::DescriptorType nsConversionUtilsVulkan::GetDescriptorType(nsGALShaderResourceType::Enum type)
{
  switch (type)
  {
    case nsGALShaderResourceType::Unknown:
      NS_REPORT_FAILURE("Unknown descriptor type");
      break;
    case nsGALShaderResourceType::PushConstants:
      NS_REPORT_FAILURE("Push constants should never appear as shader resources");
      break;
    case nsGALShaderResourceType::Sampler:
      return vk::DescriptorType::eSampler;
    case nsGALShaderResourceType::ConstantBuffer:
      return vk::DescriptorType::eUniformBuffer;
    case nsGALShaderResourceType::Texture:
      return vk::DescriptorType::eSampledImage;
    case nsGALShaderResourceType::TextureAndSampler:
      return vk::DescriptorType::eCombinedImageSampler;
    case nsGALShaderResourceType::TexelBuffer:
      return vk::DescriptorType::eUniformTexelBuffer;
    case nsGALShaderResourceType::StructuredBuffer:
      return vk::DescriptorType::eStorageBuffer;
    case nsGALShaderResourceType::TextureRW:
      return vk::DescriptorType::eStorageImage;
    case nsGALShaderResourceType::TexelBufferRW:
      return vk::DescriptorType::eStorageTexelBuffer;
    case nsGALShaderResourceType::StructuredBufferRW:
      return vk::DescriptorType::eStorageBuffer;
  }

  return vk::DescriptorType::eMutableVALVE;
}

NS_ALWAYS_INLINE vk::ShaderStageFlagBits nsConversionUtilsVulkan::GetShaderStages(nsBitflags<nsGALShaderStageFlags> stages)
{
  return (vk::ShaderStageFlagBits)stages.GetValue();
}

NS_ALWAYS_INLINE vk::PipelineStageFlags nsConversionUtilsVulkan::GetPipelineStages(nsBitflags<nsGALShaderStageFlags> stages)
{
  vk::PipelineStageFlags res;
  for (int i = 0; i < nsGALShaderStage::ENUM_COUNT; ++i)
  {
    nsGALShaderStageFlags::Enum flag = nsGALShaderStageFlags::MakeFromShaderStage((nsGALShaderStage::Enum)i);
    if (stages.IsSet(flag))
    {
      res |= GetPipelineStage((nsGALShaderStage::Enum)i);
    }
  }
  return res;
}

NS_CHECK_AT_COMPILETIME((nsUInt32)vk::ShaderStageFlagBits::eVertex == (nsUInt32)nsGALShaderStageFlags::VertexShader);
NS_CHECK_AT_COMPILETIME((nsUInt32)vk::ShaderStageFlagBits::eTessellationControl == (nsUInt32)nsGALShaderStageFlags::HullShader);
NS_CHECK_AT_COMPILETIME((nsUInt32)vk::ShaderStageFlagBits::eTessellationEvaluation == (nsUInt32)nsGALShaderStageFlags::DomainShader);
NS_CHECK_AT_COMPILETIME((nsUInt32)vk::ShaderStageFlagBits::eGeometry == (nsUInt32)nsGALShaderStageFlags::GeometryShader);
NS_CHECK_AT_COMPILETIME((nsUInt32)vk::ShaderStageFlagBits::eFragment == (nsUInt32)nsGALShaderStageFlags::PixelShader);
NS_CHECK_AT_COMPILETIME((nsUInt32)vk::ShaderStageFlagBits::eCompute == (nsUInt32)nsGALShaderStageFlags::ComputeShader);