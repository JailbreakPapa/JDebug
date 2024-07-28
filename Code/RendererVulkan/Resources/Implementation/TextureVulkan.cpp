#include <RendererVulkan/RendererVulkanPCH.h>

#include <Foundation/Memory/MemoryUtils.h>

#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Device/InitContext.h>
#include <RendererVulkan/Resources/BufferVulkan.h>
#include <RendererVulkan/Resources/TextureVulkan.h>
#include <RendererVulkan/Utils/ConversionUtilsVulkan.h>
#include <RendererVulkan/Utils/PipelineBarrierVulkan.h>

vk::Extent3D nsGALTextureVulkan::GetMipLevelSize(nsUInt32 uiMipLevel) const
{
  vk::Extent3D size = {m_Description.m_uiWidth, m_Description.m_uiHeight, m_Description.m_uiDepth};
  size.width = nsMath::Max(1u, size.width >> uiMipLevel);
  size.height = nsMath::Max(1u, size.height >> uiMipLevel);
  size.depth = nsMath::Max(1u, size.depth >> uiMipLevel);
  return size;
}

vk::ImageSubresourceRange nsGALTextureVulkan::GetFullRange() const
{
  vk::ImageSubresourceRange range;
  range.aspectMask = GetAspectMask();
  range.baseArrayLayer = 0;
  range.baseMipLevel = 0;
  range.layerCount = m_Description.m_Type == nsGALTextureType::TextureCube ? m_Description.m_uiArraySize * 6 : m_Description.m_uiArraySize;
  range.levelCount = m_Description.m_uiMipLevelCount;
  return range;
}

vk::ImageAspectFlags nsGALTextureVulkan::GetAspectMask() const
{
  vk::ImageAspectFlags mask = nsConversionUtilsVulkan::IsDepthFormat(m_imageFormat) ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
  if (nsConversionUtilsVulkan::IsStencilFormat(m_imageFormat))
    mask |= vk::ImageAspectFlagBits::eStencil;
  return mask;
}

nsGALTextureVulkan::nsGALTextureVulkan(const nsGALTextureCreationDescription& Description, bool bLinearCPU, bool bStaging)
  : nsGALTexture(Description)
  , m_image(nullptr)
  , m_bLinearCPU(bLinearCPU)
  , m_bStaging(bStaging)
{
}

nsGALTextureVulkan::~nsGALTextureVulkan() = default;

nsResult nsGALTextureVulkan::InitPlatform(nsGALDevice* pDevice, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData)
{
  m_pDevice = static_cast<nsGALDeviceVulkan*>(pDevice);

  vk::ImageFormatListCreateInfo imageFormats;
  vk::ImageCreateInfo createInfo = {};

  m_imageFormat = ComputeImageFormat(m_pDevice, m_Description.m_Format, createInfo, imageFormats, m_bStaging);
  ComputeCreateInfo(m_pDevice, m_Description, createInfo, m_stages, m_access, m_preferredLayout);
  if (m_bLinearCPU)
  {
    ComputeCreateInfoLinear(createInfo, m_stages, m_access);
  }

  if (m_Description.m_pExisitingNativeObject == nullptr)
  {
    nsVulkanAllocationCreateInfo allocInfo;
    ComputeAllocInfo(m_bLinearCPU, allocInfo);

    vk::ImageFormatProperties props2;
    VK_ASSERT_DEBUG(m_pDevice->GetVulkanPhysicalDevice().getImageFormatProperties(createInfo.format, createInfo.imageType, createInfo.tiling, createInfo.usage, createInfo.flags, &props2));
    VK_SUCCEED_OR_RETURN_NS_FAILURE(nsMemoryAllocatorVulkan::CreateImage(createInfo, allocInfo, m_image, m_alloc, &m_allocInfo));
  }
  else
  {
    m_image = static_cast<VkImage>(m_Description.m_pExisitingNativeObject);
  }
  m_pDevice->GetInitContext().InitTexture(this, createInfo, pInitialData);

  if (m_Description.m_ResourceAccess.m_bReadBack)
  {
    return CreateStagingBuffer(createInfo);
  }

  return NS_SUCCESS;
}


vk::Format nsGALTextureVulkan::ComputeImageFormat(nsGALDeviceVulkan* pDevice, nsEnum<nsGALResourceFormat> galFormat, vk::ImageCreateInfo& ref_createInfo, vk::ImageFormatListCreateInfo& ref_imageFormats, bool bStaging)
{
  const nsGALFormatLookupEntryVulkan& format = pDevice->GetFormatLookupTable().GetFormatInfo(galFormat);

  ref_createInfo.flags |= vk::ImageCreateFlagBits::eMutableFormat;
  if (pDevice->GetExtensions().m_bImageFormatList && !format.m_mutableFormats.IsEmpty())
  {
    ref_createInfo.pNext = &ref_imageFormats;

    ref_imageFormats.viewFormatCount = format.m_mutableFormats.GetCount();
    ref_imageFormats.pViewFormats = format.m_mutableFormats.GetData();
  }

  ref_createInfo.format = bStaging ? format.m_readback : format.m_format;
  return ref_createInfo.format;
}

void nsGALTextureVulkan::ComputeCreateInfo(nsGALDeviceVulkan* m_pDevice, const nsGALTextureCreationDescription& m_Description, vk::ImageCreateInfo& createInfo, vk::PipelineStageFlags& m_stages, vk::AccessFlags& m_access, vk::ImageLayout& m_preferredLayout)
{
  NS_ASSERT_DEBUG(createInfo.format != vk::Format::eUndefined, "No storage format available for given format: {0}", m_Description.m_Format);

  const bool bIsDepth = nsConversionUtilsVulkan::IsDepthFormat(createInfo.format);

  m_stages = vk::PipelineStageFlagBits::eTransfer;
  m_access = vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite;
  m_preferredLayout = vk::ImageLayout::eGeneral;

  createInfo.initialLayout = vk::ImageLayout::eUndefined;
  createInfo.sharingMode = vk::SharingMode::eExclusive;
  createInfo.pQueueFamilyIndices = nullptr;
  createInfo.queueFamilyIndexCount = 0;
  createInfo.tiling = vk::ImageTiling::eOptimal;
  createInfo.usage |= vk::ImageUsageFlagBits::eTransferDst;
  if (m_Description.m_ResourceAccess.m_bReadBack)
    createInfo.usage |= vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;

  createInfo.extent.width = m_Description.m_uiWidth;
  createInfo.extent.height = m_Description.m_uiHeight;
  createInfo.extent.depth = m_Description.m_uiDepth;
  createInfo.mipLevels = m_Description.m_uiMipLevelCount;

  createInfo.samples = static_cast<vk::SampleCountFlagBits>(m_Description.m_SampleCount.GetValue());

  // m_bAllowDynamicMipGeneration has to be emulated via a shader so we need to enable shader resource view and render target support.
  if (m_Description.m_bAllowShaderResourceView || m_Description.m_bAllowDynamicMipGeneration)
  {
    // Needed for blit-based generation
    createInfo.usage |= vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc;
    // Needed for shader-based generation
    createInfo.usage |= vk::ImageUsageFlagBits::eSampled;
    m_stages |= m_pDevice->GetSupportedStages();
    m_access |= vk::AccessFlagBits::eShaderRead;
    m_preferredLayout = nsConversionUtilsVulkan::GetDefaultLayout(createInfo.format);
  }
  // VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT
  if (bIsDepth)
  {
    // This looks wrong but apparently, even if you don't intend to render to a depth texture, you need to set the eDepthStencilAttachment flag.
    // VUID-VkImageMemoryBarrier-oldLayout-01210: https://vulkan.lunarg.com/doc/view/1.3.275.0/windows/1.3-extensions/vkspec.html#VUID-VkImageMemoryBarrier-oldLayout-01210
    // If srcQueueFamilyIndex and dstQueueFamilyIndex define a queue family ownership transfer or oldLayout and newLayout define an image layout transition, and oldLayout or newLayout is VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL then image must have been created with VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    createInfo.usage |= vk::ImageUsageFlagBits::eDepthStencilAttachment;
  }

  if (m_Description.m_bCreateRenderTarget || m_Description.m_bAllowDynamicMipGeneration)
  {
    if (bIsDepth)
    {
      m_stages |= vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
      m_access |= vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
      m_preferredLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }
    else
    {
      createInfo.usage |= vk::ImageUsageFlagBits::eColorAttachment;
      m_stages |= vk::PipelineStageFlagBits::eColorAttachmentOutput;
      m_access |= vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
      m_preferredLayout = vk::ImageLayout::eColorAttachmentOptimal;
    }
  }

  if (m_Description.m_bAllowUAV)
  {
    createInfo.usage |= vk::ImageUsageFlagBits::eStorage;
    m_stages |= m_pDevice->GetSupportedStages();
    m_access |= vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
    m_preferredLayout = vk::ImageLayout::eGeneral;
  }

  switch (m_Description.m_Type)
  {
    case nsGALTextureType::Texture2D:
    case nsGALTextureType::Texture2DShared:
    case nsGALTextureType::TextureCube:
    {
      createInfo.imageType = vk::ImageType::e2D;
      const bool bTexture2D = m_Description.m_Type == nsGALTextureType::Texture2D || m_Description.m_Type == nsGALTextureType::Texture2DShared;
      createInfo.arrayLayers = bTexture2D ? m_Description.m_uiArraySize : (m_Description.m_uiArraySize * 6);

      if (m_Description.m_Type == nsGALTextureType::TextureCube)
        createInfo.flags |= vk::ImageCreateFlagBits::eCubeCompatible;
    }
    break;

    case nsGALTextureType::Texture3D:
    {
      createInfo.arrayLayers = 1;
      createInfo.imageType = vk::ImageType::e3D;
      if (m_Description.m_bCreateRenderTarget)
      {
        createInfo.flags |= vk::ImageCreateFlagBits::e2DArrayCompatible;
      }
    }
    break;

    default:
      NS_ASSERT_NOT_IMPLEMENTED;
  }
}

void nsGALTextureVulkan::ComputeCreateInfoLinear(vk::ImageCreateInfo& createInfo, vk::PipelineStageFlags& m_stages, vk::AccessFlags& m_access)
{
  createInfo.tiling = vk::ImageTiling::eLinear;
  createInfo.usage |= vk::ImageUsageFlagBits::eTransferSrc;
  m_stages |= vk::PipelineStageFlagBits::eHost;
  m_access |= vk::AccessFlagBits::eHostRead;
  createInfo.flags = {}; // Clear all flags as we don't need them and they usually are not supported on NVidia in linear mode.
}

void nsGALTextureVulkan::ComputeAllocInfo(bool m_bLinearCPU, nsVulkanAllocationCreateInfo& allocInfo)
{
  allocInfo.m_usage = nsVulkanMemoryUsage::Auto;
  if (m_bLinearCPU)
  {
    allocInfo.m_flags = nsVulkanAllocationCreateFlags::HostAccessRandom;
  }
}

nsGALTextureVulkan::StagingMode nsGALTextureVulkan::ComputeStagingMode(nsGALDeviceVulkan* m_pDevice, const nsGALTextureCreationDescription& m_Description, const vk::ImageCreateInfo& createInfo)
{
  if (!m_Description.m_ResourceAccess.m_bReadBack)
    return StagingMode::None;

  // We want the staging texture to always have the intended format and not the override format given by the parent texture.
  vk::Format stagingFormat = m_pDevice->GetFormatLookupTable().GetFormatInfo(m_Description.m_Format).m_readback;

  NS_ASSERT_DEV(!nsConversionUtilsVulkan::IsStencilFormat(createInfo.format), "Stencil read-back not implemented.");
  NS_ASSERT_DEV(!nsConversionUtilsVulkan::IsDepthFormat(stagingFormat), "Depth read-back should use a color format for CPU staging.");

  const vk::FormatProperties srcFormatProps = m_pDevice->GetVulkanPhysicalDevice().getFormatProperties(createInfo.format);
  const vk::FormatProperties dstFormatProps = m_pDevice->GetVulkanPhysicalDevice().getFormatProperties(stagingFormat);

  const bool bFormatsEqual = createInfo.format == stagingFormat && createInfo.samples == vk::SampleCountFlagBits::e1;
  const bool bSupportsCopy = bFormatsEqual && (srcFormatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eTransferSrc) && (dstFormatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eTransferDst);
  if (bFormatsEqual)
  {
    NS_ASSERT_DEV(srcFormatProps.optimalTilingFeatures & vk::FormatFeatureFlagBits::eTransferSrc, "Source format can't be read, readback impossible.");
    return StagingMode::Buffer;
  }
  else
  {
    vk::ImageFormatProperties props;
    vk::Result res = m_pDevice->GetVulkanPhysicalDevice().getImageFormatProperties(stagingFormat, createInfo.imageType, vk::ImageTiling::eLinear, vk::ImageUsageFlagBits::eColorAttachment, {}, &props);

    // #TODO_VULKAN Note that on Nvidia driver 531.68 the above call succeeds even though linear rendering is not supported by the driver. Thus, we need to check explicitly here that vk::FormatFeatureFlagBits::eColorAttachment is supported again via the vk::FormatProperties.
    const bool bCanUseDirectTexture = (dstFormatProps.linearTilingFeatures & vk::FormatFeatureFlagBits::eColorAttachment) && (res == vk::Result::eSuccess) && createInfo.arrayLayers <= props.maxArrayLayers && createInfo.mipLevels <= props.maxMipLevels && createInfo.extent.depth <= props.maxExtent.depth && createInfo.extent.width <= props.maxExtent.width && createInfo.extent.height <= props.maxExtent.height && (createInfo.samples & props.sampleCounts);
    return bCanUseDirectTexture ? StagingMode::Texture : StagingMode::TextureAndBuffer;
  }
}

nsUInt32 nsGALTextureVulkan::ComputeSubResourceOffsets(nsDynamicArray<SubResourceOffset>& subResourceSizes) const
{
  const nsUInt32 alignment = (nsUInt32)nsGALBufferVulkan::GetAlignment(m_pDevice, vk::BufferUsageFlagBits::eTransferDst);
  const vk::Format stagingFormat = m_pDevice->GetFormatLookupTable().GetFormatInfo(m_Description.m_Format).m_readback;
  const nsUInt8 uiBlockSize = vk::blockSize(stagingFormat);
  const auto blockExtent = vk::blockExtent(stagingFormat);
  const nsUInt32 arrayLayers = (m_Description.m_Type == nsGALTextureType::TextureCube ? (m_Description.m_uiArraySize * 6) : m_Description.m_uiArraySize);
  const nsUInt32 mipLevels = m_Description.m_uiMipLevelCount;

  subResourceSizes.Reserve(arrayLayers * mipLevels);
  nsUInt32 uiOffset = 0;
  for (nsUInt32 uiLayer = 0; uiLayer < arrayLayers; uiLayer++)
  {
    for (nsUInt32 uiMipLevel = 0; uiMipLevel < mipLevels; uiMipLevel++)
    {
      const nsUInt32 uiSubresourceIndex = uiMipLevel + uiLayer * mipLevels;
      NS_ASSERT_DEBUG(subResourceSizes.GetCount() == uiSubresourceIndex, "");

      const vk::Extent3D imageExtent = GetMipLevelSize(uiMipLevel);
      const VkExtent3D blockCount = {
        (imageExtent.width + blockExtent[0] - 1) / blockExtent[0],
        (imageExtent.height + blockExtent[1] - 1) / blockExtent[1],
        (imageExtent.depth + blockExtent[2] - 1) / blockExtent[2]};

      const nsUInt32 uiTotalSize = uiBlockSize * blockCount.width * blockCount.height * blockCount.depth;
      subResourceSizes.PushBack({uiOffset, uiTotalSize, blockCount.width / blockExtent[0], blockCount.height / blockExtent[1]});
      uiOffset += nsMemoryUtils::AlignSize(uiTotalSize, alignment);
    }
  }
  return uiOffset;
}

nsResult nsGALTextureVulkan::CreateStagingBuffer(const vk::ImageCreateInfo& createInfo)
{
  m_stagingMode = nsGALTextureVulkan::ComputeStagingMode(m_pDevice, m_Description, createInfo);
  if (m_stagingMode == StagingMode::Texture || m_stagingMode == StagingMode::TextureAndBuffer)
  {
    nsGALTextureCreationDescription stagingDesc = m_Description;
    stagingDesc.m_SampleCount = nsGALMSAASampleCount::None;
    stagingDesc.m_bAllowShaderResourceView = false;
    stagingDesc.m_bAllowUAV = false;
    stagingDesc.m_bCreateRenderTarget = true;
    stagingDesc.m_bAllowDynamicMipGeneration = false;
    stagingDesc.m_ResourceAccess.m_bReadBack = false;
    stagingDesc.m_ResourceAccess.m_bImmutable = false;
    stagingDesc.m_pExisitingNativeObject = nullptr;

    const bool bLinearCPU = m_stagingMode == StagingMode::Texture;

    m_hStagingTexture = m_pDevice->CreateTextureInternal(stagingDesc, {}, bLinearCPU, true);
    if (m_hStagingTexture.IsInvalidated())
    {
      nsLog::Error("Failed to create staging texture for read-back");
      return NS_FAILURE;
    }
  }
  if (m_stagingMode == StagingMode::Buffer || m_stagingMode == StagingMode::TextureAndBuffer)
  {
    nsGALBufferCreationDescription stagingBuffer;
    stagingBuffer.m_BufferFlags = nsGALBufferUsageFlags::ByteAddressBuffer;
    nsHybridArray<SubResourceOffset, 8> subResourceSizes;
    stagingBuffer.m_uiTotalSize = ComputeSubResourceOffsets(subResourceSizes);
    stagingBuffer.m_uiStructSize = 1;

    stagingBuffer.m_ResourceAccess.m_bImmutable = false;

    m_hStagingBuffer = m_pDevice->CreateBufferInternal(stagingBuffer, {}, true);
    if (m_hStagingBuffer.IsInvalidated())
    {
      nsLog::Error("Failed to create staging buffer for read-back");
      return NS_FAILURE;
    }
  }

  return NS_SUCCESS;
}
nsResult nsGALTextureVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);
  if (m_image && !m_Description.m_pExisitingNativeObject)
  {
    pVulkanDevice->DeleteLater(m_image, m_alloc);
  }
  m_image = nullptr;

  if (!m_hStagingTexture.IsInvalidated())
  {
    pDevice->DestroyTexture(m_hStagingTexture);
    m_hStagingTexture.Invalidate();
  }
  if (!m_hStagingBuffer.IsInvalidated())
  {
    pDevice->DestroyBuffer(m_hStagingBuffer);
    m_hStagingBuffer.Invalidate();
  }

  return NS_SUCCESS;
}

void nsGALTextureVulkan::SetDebugNamePlatform(const char* szName) const
{
  m_pDevice->SetDebugName(szName, m_image, m_alloc);
  if (!m_hStagingTexture.IsInvalidated())
  {
    auto pStagingTexture = static_cast<const nsGALTextureVulkan*>(m_pDevice->GetTexture(m_hStagingTexture));
    pStagingTexture->SetDebugName(szName);
  }
  if (!m_hStagingBuffer.IsInvalidated())
  {
    auto pStagingBuffer = static_cast<const nsGALBufferVulkan*>(m_pDevice->GetBuffer(m_hStagingBuffer));
    pStagingBuffer->SetDebugName(szName);
  }
}


