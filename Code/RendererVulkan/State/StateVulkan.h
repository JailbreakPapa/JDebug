
#pragma once

#include <RendererFoundation/State/State.h>

#include <vulkan/vulkan.hpp>

class NS_RENDERERVULKAN_DLL nsGALBlendStateVulkan : public nsGALBlendState
{
public:
  NS_ALWAYS_INLINE const vk::PipelineColorBlendStateCreateInfo* GetBlendState() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALBlendStateVulkan(const nsGALBlendStateCreationDescription& Description);

  ~nsGALBlendStateVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  vk::PipelineColorBlendStateCreateInfo m_blendState = {};
  vk::PipelineColorBlendAttachmentState m_blendAttachmentState[8] = {};
};

class NS_RENDERERVULKAN_DLL nsGALDepthStencilStateVulkan : public nsGALDepthStencilState
{
public:
  NS_ALWAYS_INLINE const vk::PipelineDepthStencilStateCreateInfo* GetDepthStencilState() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALDepthStencilStateVulkan(const nsGALDepthStencilStateCreationDescription& Description);

  ~nsGALDepthStencilStateVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  vk::PipelineDepthStencilStateCreateInfo m_depthStencilState = {};
};

class NS_RENDERERVULKAN_DLL nsGALRasterizerStateVulkan : public nsGALRasterizerState
{
public:
  NS_ALWAYS_INLINE const vk::PipelineRasterizationStateCreateInfo* GetRasterizerState() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALRasterizerStateVulkan(const nsGALRasterizerStateCreationDescription& Description);

  ~nsGALRasterizerStateVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  vk::PipelineRasterizationStateCreateInfo m_rasterizerState = {};
};

class NS_RENDERERVULKAN_DLL nsGALSamplerStateVulkan : public nsGALSamplerState
{
public:
  NS_ALWAYS_INLINE const vk::DescriptorImageInfo& GetImageInfo() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALSamplerStateVulkan(const nsGALSamplerStateCreationDescription& Description);
  ~nsGALSamplerStateVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  vk::DescriptorImageInfo m_resourceImageInfo;
};


#include <RendererVulkan/State/Implementation/StateVulkan_inl.h>
