
#pragma once

#include <RendererFoundation/Resources/ResourceView.h>

#include <vulkan/vulkan.hpp>

class nsGALBufferVulkan;
class nsGALTextureVulkan;

class nsGALTextureResourceViewVulkan : public nsGALTextureResourceView
{
public:
  const vk::DescriptorImageInfo& GetImageInfo(bool bIsArray) const;
  vk::ImageSubresourceRange GetRange() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALTextureResourceViewVulkan(nsGALTexture* pResource, const nsGALTextureResourceViewCreationDescription& Description);
  ~nsGALTextureResourceViewVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  vk::ImageSubresourceRange m_range;
  mutable vk::DescriptorImageInfo m_resourceImageInfo;
  mutable vk::DescriptorImageInfo m_resourceImageInfoArray;
};

class nsGALBufferResourceViewVulkan : public nsGALBufferResourceView
{
public:
  const vk::DescriptorBufferInfo& GetBufferInfo() const;
  const vk::BufferView& GetBufferView() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALBufferResourceViewVulkan(nsGALBuffer* pResource, const nsGALBufferResourceViewCreationDescription& Description);
  ~nsGALBufferResourceViewVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  mutable vk::DescriptorBufferInfo m_resourceBufferInfo;
  vk::BufferView m_bufferView;
};

#include <RendererVulkan/Resources/Implementation/ResourceViewVulkan_inl.h>
