#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Resources/RenderTargetViewVulkan.h>
#include <RendererVulkan/Resources/TextureVulkan.h>
#include <RendererVulkan/Utils/ConversionUtilsVulkan.h>

bool IsArrayView(const nsGALTextureCreationDescription& texDesc, const nsGALRenderTargetViewCreationDescription& viewDesc)
{
  return texDesc.m_uiArraySize > 1 || viewDesc.m_uiFirstSlice > 0;
}

nsGALRenderTargetViewVulkan::nsGALRenderTargetViewVulkan(nsGALTexture* pTexture, const nsGALRenderTargetViewCreationDescription& Description)
  : nsGALRenderTargetView(pTexture, Description)
{
}

nsGALRenderTargetViewVulkan::~nsGALRenderTargetViewVulkan() {}

nsResult nsGALRenderTargetViewVulkan::InitPlatform(nsGALDevice* pDevice)
{
  const nsGALTexture* pTexture = nullptr;
  if (!m_Description.m_hTexture.IsInvalidated())
    pTexture = pDevice->GetTexture(m_Description.m_hTexture);

  if (pTexture == nullptr)
  {
    nsLog::Error("No valid texture handle given for render target view creation!");
    return NS_FAILURE;
  }

  const nsGALTextureCreationDescription& texDesc = pTexture->GetDescription();
  nsGALResourceFormat::Enum viewFormat = texDesc.m_Format;

  if (m_Description.m_OverrideViewFormat != nsGALResourceFormat::Invalid)
    viewFormat = m_Description.m_OverrideViewFormat;

  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);
  auto pTextureVulkan = static_cast<const nsGALTextureVulkan*>(pTexture->GetParentResource());
  vk::Format vkViewFormat = pTextureVulkan->GetImageFormat();

  const bool bIsDepthFormat = nsConversionUtilsVulkan::IsDepthFormat(vkViewFormat);

  if (vkViewFormat == vk::Format::eUndefined)
  {
    nsLog::Error("Couldn't get Vulkan format for view!");
    return NS_FAILURE;
  }


  vk::Image vkImage = pTextureVulkan->GetImage();
  const bool bIsArrayView = IsArrayView(texDesc, m_Description);

  vk::ImageViewCreateInfo imageViewCreationInfo;
  if (bIsDepthFormat)
  {
    imageViewCreationInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;
    if (texDesc.m_Format == nsGALResourceFormat::D24S8)
    {
      imageViewCreationInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
    }
  }
  else
  {
    imageViewCreationInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
  }

  imageViewCreationInfo.image = vkImage;
  imageViewCreationInfo.format = vkViewFormat;

  if (!bIsArrayView)
  {
    imageViewCreationInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreationInfo.subresourceRange.baseMipLevel = m_Description.m_uiMipLevel;
    imageViewCreationInfo.subresourceRange.levelCount = 1;
    imageViewCreationInfo.subresourceRange.layerCount = 1;
  }
  else
  {
    imageViewCreationInfo.viewType = vk::ImageViewType::e2DArray;
    imageViewCreationInfo.subresourceRange.baseMipLevel = m_Description.m_uiMipLevel;
    imageViewCreationInfo.subresourceRange.levelCount = 1;
    imageViewCreationInfo.subresourceRange.baseArrayLayer = m_Description.m_uiFirstSlice;
    imageViewCreationInfo.subresourceRange.layerCount = m_Description.m_uiSliceCount;
  }
  m_range = imageViewCreationInfo.subresourceRange;
  m_bfullRange = m_range == pTextureVulkan->GetFullRange();

  VK_SUCCEED_OR_RETURN_NS_FAILURE(pVulkanDevice->GetVulkanDevice().createImageView(&imageViewCreationInfo, nullptr, &m_imageView));
  pVulkanDevice->SetDebugName("RTV", m_imageView);
  return NS_SUCCESS;
}

nsResult nsGALRenderTargetViewVulkan::DeInitPlatform(nsGALDevice* pDevice)
{
  nsGALDeviceVulkan* pVulkanDevice = static_cast<nsGALDeviceVulkan*>(pDevice);
  pVulkanDevice->DeleteLater(m_imageView);
  return NS_SUCCESS;
}


