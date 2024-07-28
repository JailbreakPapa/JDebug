#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Resources/BufferVulkan.h>
#include <RendererVulkan/Resources/TextureVulkan.h>
#include <RendererVulkan/Resources/UnorderedAccessViewVulkan.h>
#include <RendererVulkan/Utils/ConversionUtilsVulkan.h>

bool IsArrayView(const nsGALTextureCreationDescription& texDesc, const nsGALTextureUnorderedAccessViewCreationDescription& viewDesc)
{
  return texDesc.m_uiArraySize > 1 || viewDesc.m_uiFirstArraySlice > 0;
}

nsGALTextureUnorderedAccessViewVulkan::nsGALTextureUnorderedAccessViewVulkan(
  nsGALTexture* pResource, const nsGALTextureUnorderedAccessViewCreationDescription& Description)
  : nsGALTextureUnorderedAccessView(pResource, Description)
{
}

nsGALTextureUnorderedAccessViewVulkan::~nsGALTextureUnorderedAccessViewVulkan() = default;

nsResult nsGALTextureUnorderedAccessViewVulkan::InitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);

  const nsGALTexture* pTexture = nullptr;
  if (!m_Description.m_hTexture.IsInvalidated())
    pTexture = pDevice->GetTexture(m_Description.m_hTexture);

  if (pTexture == nullptr)
  {
    nsLog::Error("No valid texture handle given for resource view creation!");
    return NS_FAILURE;
  }

  auto pParentTexture = static_cast<const nsGALTextureVulkan*>(pTexture->GetParentResource());
  auto image = pParentTexture->GetImage();
  const nsGALTextureCreationDescription& texDesc = pTexture->GetDescription();

  const bool bIsArrayView = IsArrayView(texDesc, m_Description);

  nsGALResourceFormat::Enum viewFormat = m_Description.m_OverrideViewFormat == nsGALResourceFormat::Invalid ? texDesc.m_Format : m_Description.m_OverrideViewFormat;
  vk::ImageViewCreateInfo viewCreateInfo;
  viewCreateInfo.format = pVulkanDevice->GetFormatLookupTable().GetFormatInfo(viewFormat).m_format;
  viewCreateInfo.image = image;
  viewCreateInfo.subresourceRange = nsConversionUtilsVulkan::GetSubresourceRange(texDesc, m_Description);
  viewCreateInfo.viewType = nsConversionUtilsVulkan::GetImageViewType(texDesc.m_Type, bIsArrayView);
  if (texDesc.m_Type == nsGALTextureType::TextureCube)
    viewCreateInfo.viewType = vk::ImageViewType::e2DArray; // There is no RWTextureCube / RWTextureCubeArray in HLSL

  m_resourceImageInfo.imageLayout = vk::ImageLayout::eGeneral;

  m_range = viewCreateInfo.subresourceRange;
  VK_SUCCEED_OR_RETURN_NS_FAILURE(pVulkanDevice->GetVulkanDevice().createImageView(&viewCreateInfo, nullptr, &m_resourceImageInfo.imageView));
  pVulkanDevice->SetDebugName("UAV-Texture", m_resourceImageInfo.imageView);

  return NS_SUCCESS;
}

nsResult nsGALTextureUnorderedAccessViewVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);
  pVulkanDevice->DeleteLater(m_resourceImageInfo.imageView);
  m_resourceImageInfo = vk::DescriptorImageInfo();
  return NS_SUCCESS;
}

/////////////////////////////////////////////////////

const vk::DescriptorBufferInfo& nsGALBufferUnorderedAccessViewVulkan::GetBufferInfo() const
{
  // Vulkan buffers get constantly swapped out for new ones so the vk::Buffer pointer is not persistent.
  // We need to acquire the latest one on every request for rendering.
  m_resourceBufferInfo.buffer = static_cast<const nsGALBufferVulkan*>(GetResource())->GetVkBuffer();
  return m_resourceBufferInfo;
}

nsGALBufferUnorderedAccessViewVulkan::nsGALBufferUnorderedAccessViewVulkan(
  nsGALBuffer* pResource, const nsGALBufferUnorderedAccessViewCreationDescription& Description)
  : nsGALBufferUnorderedAccessView(pResource, Description)
{
}

nsGALBufferUnorderedAccessViewVulkan::~nsGALBufferUnorderedAccessViewVulkan() = default;

nsResult nsGALBufferUnorderedAccessViewVulkan::InitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);

  const nsGALBuffer* pBuffer = nullptr;
  if (!m_Description.m_hBuffer.IsInvalidated())
    pBuffer = pDevice->GetBuffer(m_Description.m_hBuffer);

  if (pBuffer == nullptr)
  {
    nsLog::Error("No valid buffer handle given for resource view creation!");
    return NS_FAILURE;
  }

  if (!pBuffer->GetDescription().m_BufferFlags.IsSet(nsGALBufferUsageFlags::ByteAddressBuffer) && m_Description.m_bRawView)
  {
    nsLog::Error("Trying to create a raw view for a buffer with no raw view flag is invalid!");
    return NS_FAILURE;
  }

  auto pParentBuffer = static_cast<const nsGALBufferVulkan*>(pBuffer);
  if (pBuffer->GetDescription().m_BufferFlags.IsSet(nsGALBufferUsageFlags::StructuredBuffer))
  {
    m_resourceBufferInfo.offset = pBuffer->GetDescription().m_uiStructSize * m_Description.m_uiFirstElement;
    m_resourceBufferInfo.range = pBuffer->GetDescription().m_uiStructSize * m_Description.m_uiNumElements;
  }
  else if (m_Description.m_bRawView)
  {
    m_resourceBufferInfo.offset = sizeof(nsUInt32) * m_Description.m_uiFirstElement;
    m_resourceBufferInfo.range = sizeof(nsUInt32) * m_Description.m_uiNumElements;
  }
  else
  {
    m_resourceBufferInfo.offset = pBuffer->GetDescription().m_uiStructSize * m_Description.m_uiFirstElement;
    m_resourceBufferInfo.range = pBuffer->GetDescription().m_uiStructSize * m_Description.m_uiNumElements;

    nsGALResourceFormat::Enum viewFormat = m_Description.m_OverrideViewFormat;
    if (viewFormat == nsGALResourceFormat::Invalid)
      viewFormat = nsGALResourceFormat::RUInt;

    vk::BufferViewCreateInfo viewCreateInfo;
    viewCreateInfo.buffer = pParentBuffer->GetVkBuffer();
    viewCreateInfo.format = pVulkanDevice->GetFormatLookupTable().GetFormatInfo(viewFormat).m_format;
    viewCreateInfo.offset = pBuffer->GetDescription().m_uiStructSize * m_Description.m_uiFirstElement;
    viewCreateInfo.range = pBuffer->GetDescription().m_uiStructSize * m_Description.m_uiNumElements;

    VK_SUCCEED_OR_RETURN_NS_FAILURE(pVulkanDevice->GetVulkanDevice().createBufferView(&viewCreateInfo, nullptr, &m_bufferView));
  }

  return NS_SUCCESS;
}

nsResult nsGALBufferUnorderedAccessViewVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);
  m_resourceBufferInfo = vk::DescriptorBufferInfo();
  pVulkanDevice->DeleteLater(m_bufferView);
  return NS_SUCCESS;
}