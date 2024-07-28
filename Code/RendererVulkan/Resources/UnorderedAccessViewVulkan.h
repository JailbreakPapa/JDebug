
#pragma once

#include <RendererFoundation/Resources/UnorderedAccesView.h>

#include <vulkan/vulkan.hpp>

class nsGALBufferVulkan;

class nsGALTextureUnorderedAccessViewVulkan : public nsGALTextureUnorderedAccessView
{
public:
  NS_ALWAYS_INLINE const vk::DescriptorImageInfo& GetImageInfo() const;
  vk::ImageSubresourceRange GetRange() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALTextureUnorderedAccessViewVulkan(nsGALTexture* pResource, const nsGALTextureUnorderedAccessViewCreationDescription& Description);
  ~nsGALTextureUnorderedAccessViewVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

private:
  mutable vk::DescriptorImageInfo m_resourceImageInfo;
  // mutable vk::DescriptorBufferInfo m_resourceBufferInfo;
  // vk::BufferView m_bufferView;
  vk::ImageSubresourceRange m_range;
};

class nsGALBufferUnorderedAccessViewVulkan : public nsGALBufferUnorderedAccessView
{
public:
  const vk::DescriptorBufferInfo& GetBufferInfo() const;
  const vk::BufferView& GetBufferView() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALBufferUnorderedAccessViewVulkan(nsGALBuffer* pResource, const nsGALBufferUnorderedAccessViewCreationDescription& Description);
  ~nsGALBufferUnorderedAccessViewVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

private:
  // mutable vk::DescriptorImageInfo m_resourceImageInfo;
  mutable vk::DescriptorBufferInfo m_resourceBufferInfo;
  vk::BufferView m_bufferView;
  // vk::ImageSubresourceRange m_range;
};

#include <RendererVulkan/Resources/Implementation/UnorderedAccessViewVulkan_inl.h>
