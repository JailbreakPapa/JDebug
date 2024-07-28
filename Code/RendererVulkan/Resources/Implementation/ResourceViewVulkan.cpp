#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Resources/BufferVulkan.h>
#include <RendererVulkan/Resources/ResourceViewVulkan.h>
#include <RendererVulkan/Resources/TextureVulkan.h>
#include <RendererVulkan/State/StateVulkan.h>
#include <RendererVulkan/Utils/ConversionUtilsVulkan.h>

bool IsArrayView(const nsGALTextureCreationDescription& texDesc, const nsGALTextureResourceViewCreationDescription& viewDesc)
{
  return texDesc.m_uiArraySize > 1 || viewDesc.m_uiArraySize > 1;
}

nsGALTextureResourceViewVulkan::nsGALTextureResourceViewVulkan(nsGALTexture* pResource, const nsGALTextureResourceViewCreationDescription& Description)
  : nsGALTextureResourceView(pResource, Description)
{
}

nsGALTextureResourceViewVulkan::~nsGALTextureResourceViewVulkan() = default;

nsResult nsGALTextureResourceViewVulkan::InitPlatform(nsGALDevice* pDevice)
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
  const bool bIsDepth = nsGALResourceFormat::IsDepthFormat(pTexture->GetDescription().m_Format);

  nsGALResourceFormat::Enum viewFormat = m_Description.m_OverrideViewFormat == nsGALResourceFormat::Invalid ? texDesc.m_Format : m_Description.m_OverrideViewFormat;
  vk::ImageViewCreateInfo viewCreateInfo;
  viewCreateInfo.format = pVulkanDevice->GetFormatLookupTable().GetFormatInfo(viewFormat).m_format;
  viewCreateInfo.image = image;
  viewCreateInfo.subresourceRange = nsConversionUtilsVulkan::GetSubresourceRange(texDesc, m_Description);
  viewCreateInfo.subresourceRange.aspectMask &= ~vk::ImageAspectFlagBits::eStencil;


  m_resourceImageInfo.imageLayout = nsConversionUtilsVulkan::GetDefaultLayout(pParentTexture->GetImageFormat());
  m_resourceImageInfoArray.imageLayout = m_resourceImageInfo.imageLayout;

  m_range = viewCreateInfo.subresourceRange;
  if (texDesc.m_Type == nsGALTextureType::Texture3D) // no array support
  {
    viewCreateInfo.viewType = nsConversionUtilsVulkan::GetImageViewType(texDesc.m_Type, false);
    VK_SUCCEED_OR_RETURN_NS_FAILURE(pVulkanDevice->GetVulkanDevice().createImageView(&viewCreateInfo, nullptr, &m_resourceImageInfo.imageView));
  }
  else if (m_Description.m_uiArraySize == 1) // can be array or not
  {
    viewCreateInfo.viewType = nsConversionUtilsVulkan::GetImageViewType(texDesc.m_Type, false);
    VK_SUCCEED_OR_RETURN_NS_FAILURE(pVulkanDevice->GetVulkanDevice().createImageView(&viewCreateInfo, nullptr, &m_resourceImageInfo.imageView));
    viewCreateInfo.viewType = nsConversionUtilsVulkan::GetImageViewType(texDesc.m_Type, true);
    VK_SUCCEED_OR_RETURN_NS_FAILURE(pVulkanDevice->GetVulkanDevice().createImageView(&viewCreateInfo, nullptr, &m_resourceImageInfoArray.imageView));
  }
  else // Can only be array
  {
    viewCreateInfo.viewType = nsConversionUtilsVulkan::GetImageViewType(texDesc.m_Type, true);
    VK_SUCCEED_OR_RETURN_NS_FAILURE(pVulkanDevice->GetVulkanDevice().createImageView(&viewCreateInfo, nullptr, &m_resourceImageInfoArray.imageView));
  }

  return NS_SUCCESS;
}

nsResult nsGALTextureResourceViewVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);
  pVulkanDevice->DeleteLater(m_resourceImageInfo.imageView);
  pVulkanDevice->DeleteLater(m_resourceImageInfoArray.imageView);
  m_resourceImageInfo = vk::DescriptorImageInfo();
  m_resourceImageInfoArray = vk::DescriptorImageInfo();
  return NS_SUCCESS;
}

/////////////////////////////////////////////////////////

const vk::DescriptorBufferInfo& nsGALBufferResourceViewVulkan::GetBufferInfo() const
{
  // Vulkan buffers get constantly swapped out for new ones so the vk::Buffer pointer is not persistent.
  // We need to acquire the latest one on every request for rendering.
  m_resourceBufferInfo.buffer = static_cast<const nsGALBufferVulkan*>(GetResource())->GetVkBuffer();
  return m_resourceBufferInfo;
}

nsGALBufferResourceViewVulkan::nsGALBufferResourceViewVulkan(nsGALBuffer* pResource, const nsGALBufferResourceViewCreationDescription& Description)
  : nsGALBufferResourceView(pResource, Description)
{
}

nsGALBufferResourceViewVulkan::~nsGALBufferResourceViewVulkan() = default;

nsResult nsGALBufferResourceViewVulkan::InitPlatform(nsGALDevice* pDevice)
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

nsResult nsGALBufferResourceViewVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);
  m_resourceBufferInfo = vk::DescriptorBufferInfo();
  pVulkanDevice->DeleteLater(m_bufferView);
  return NS_SUCCESS;
}


