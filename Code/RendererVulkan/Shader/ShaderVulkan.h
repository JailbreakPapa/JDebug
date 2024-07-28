
#pragma once

#include <RendererVulkan/RendererVulkanDLL.h>

#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/Shader/Shader.h>

#include <RendererCore/Shader/ShaderStageBinary.h>
#include <vulkan/vulkan.hpp>

class NS_RENDERERVULKAN_DLL nsGALShaderVulkan : public nsGALShader
{
public:
  /// \brief Used as input to nsResourceCacheVulkan::RequestDescriptorSetLayout to create a vk::DescriptorSetLayout.
  struct DescriptorSetLayoutDesc
  {
    mutable nsUInt32 m_uiHash = 0;
    nsHybridArray<vk::DescriptorSetLayoutBinding, 6> m_bindings;
    void ComputeHash();
  };

  void SetDebugName(const char* szName) const override;

  NS_ALWAYS_INLINE vk::ShaderModule GetShader(nsGALShaderStage::Enum stage) const;
  NS_ALWAYS_INLINE nsUInt32 GetSetCount() const;
  NS_ALWAYS_INLINE vk::DescriptorSetLayout GetDescriptorSetLayout(nsUInt32 uiSet = 0) const;
  NS_ALWAYS_INLINE nsArrayPtr<const nsShaderResourceBinding> GetBindings(nsUInt32 uiSet = 0) const;
  NS_ALWAYS_INLINE vk::PushConstantRange GetPushConstantRange() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALShaderVulkan(const nsGALShaderCreationDescription& description);
  virtual ~nsGALShaderVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

private:
  vk::PushConstantRange m_pushConstants;
  nsHybridArray<vk::DescriptorSetLayout, 4> m_descriptorSetLayout;
  nsHybridArray<nsHybridArray<nsShaderResourceBinding, 16>, 4> m_SetBindings;
  vk::ShaderModule m_Shaders[nsGALShaderStage::ENUM_COUNT];
};

#include <RendererVulkan/Shader/Implementation/ShaderVulkan_inl.h>
