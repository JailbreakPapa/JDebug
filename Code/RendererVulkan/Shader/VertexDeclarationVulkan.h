
#pragma once

#include <RendererFoundation/RendererFoundationDLL.h>
#include <RendererFoundation/Shader/VertexDeclaration.h>
#include <RendererVulkan/RendererVulkanDLL.h>

#include <vulkan/vulkan.hpp>

class nsGALVertexDeclarationVulkan : public nsGALVertexDeclaration
{
public:
  NS_ALWAYS_INLINE nsArrayPtr<const vk::VertexInputAttributeDescription> GetAttributes() const;
  NS_ALWAYS_INLINE nsArrayPtr<const vk::VertexInputBindingDescription> GetBindings() const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  virtual nsResult InitPlatform(nsGALDevice* pDevice) override;

  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;

  nsGALVertexDeclarationVulkan(const nsGALVertexDeclarationCreationDescription& Description);

  virtual ~nsGALVertexDeclarationVulkan();

  nsHybridArray<vk::VertexInputAttributeDescription, NS_GAL_MAX_VERTEX_BUFFER_COUNT> m_attributes;
  nsHybridArray<vk::VertexInputBindingDescription, NS_GAL_MAX_VERTEX_BUFFER_COUNT> m_bindings;
};

#include <RendererVulkan/Shader/Implementation/VertexDeclarationVulkan_inl.h>
