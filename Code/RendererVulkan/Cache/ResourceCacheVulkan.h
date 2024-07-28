#pragma once

#include <RendererVulkan/RendererVulkanDLL.h>

#include <Foundation/Algorithm/HashStream.h>
#include <Foundation/Math/Size.h>
#include <RendererFoundation/Resources/RenderTargetSetup.h>
#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Resources/RenderTargetViewVulkan.h>
#include <RendererVulkan/Shader/ShaderVulkan.h>

#include <vulkan/vulkan.hpp>

class nsGALRasterizerStateVulkan;
class nsGALBlendStateVulkan;
class nsGALDepthStencilStateVulkan;
class nsGALShaderVulkan;
class nsGALVertexDeclarationVulkan;
class nsRefCounted;

NS_DEFINE_AS_POD_TYPE(vk::DynamicState);

/// \brief Creates and caches persistent Vulkan resources. Resources are never freed until the device is shut down.
class NS_RENDERERVULKAN_DLL nsResourceCacheVulkan
{
public:
  static void Initialize(nsGALDeviceVulkan* pDevice, vk::Device device);
  static void DeInitialize();

  static vk::RenderPass RequestRenderPass(const nsGALRenderingSetup& renderingSetup);
  static vk::Framebuffer RequestFrameBuffer(vk::RenderPass renderPass, const nsGALRenderTargetSetup& renderTargetSetup, nsSizeU32& out_Size, nsEnum<nsGALMSAASampleCount>& out_msaa, nsUInt32& out_uiLayers);

  struct PipelineLayoutDesc
  {
    nsHybridArray<vk::DescriptorSetLayout, 4> m_layout;
    vk::PushConstantRange m_pushConstants;
  };

  struct GraphicsPipelineDesc
  {
    NS_DECLARE_POD_TYPE();
    vk::RenderPass m_renderPass;     // Created from nsGALRenderingSetup
    vk::PipelineLayout m_layout;     // Created from shader
    nsEnum<nsGALPrimitiveTopology> m_topology;
    nsEnum<nsGALMSAASampleCount> m_msaa;
    nsUInt8 m_uiAttachmentCount = 0; // DX12 requires format list for RT and DT
    const nsGALRasterizerStateVulkan* m_pCurrentRasterizerState = nullptr;
    const nsGALBlendStateVulkan* m_pCurrentBlendState = nullptr;
    const nsGALDepthStencilStateVulkan* m_pCurrentDepthStencilState = nullptr;
    const nsGALShaderVulkan* m_pCurrentShader = nullptr;
    const nsGALVertexDeclarationVulkan* m_pCurrentVertexDecl = nullptr;
    nsUInt32 m_VertexBufferStrides[NS_GAL_MAX_VERTEX_BUFFER_COUNT] = {};
  };

  struct ComputePipelineDesc
  {
    NS_DECLARE_POD_TYPE();
    vk::PipelineLayout m_layout;
    const nsGALShaderVulkan* m_pCurrentShader = nullptr;
  };

  static vk::PipelineLayout RequestPipelineLayout(const PipelineLayoutDesc& desc);
  static vk::Pipeline RequestGraphicsPipeline(const GraphicsPipelineDesc& desc);
  static vk::Pipeline RequestComputePipeline(const ComputePipelineDesc& desc);

  struct DescriptorSetLayoutDesc
  {
    mutable nsUInt32 m_uiHash = 0;
    nsHybridArray<vk::DescriptorSetLayoutBinding, 6> m_bindings;
  };
  static vk::DescriptorSetLayout RequestDescriptorSetLayout(const nsGALShaderVulkan::DescriptorSetLayoutDesc& desc);

  /// \brief Invalidates any caches that use this resource. Basically all pointer types in GraphicsPipelineDesc except for nsGALShaderVulkan.
  static void ResourceDeleted(const nsRefCounted* pResource);
  /// \brief Invalidates any caches that use this shader resource.
  static void ShaderDeleted(const nsGALShaderVulkan* pShader);

private:
  struct FramebufferKey
  {
    vk::RenderPass m_renderPass;
    nsGALRenderTargetSetup m_renderTargetSetup;
  };

  /// \brief Hashable version without pointers of vk::FramebufferCreateInfo
  struct FramebufferDesc
  {
    VkRenderPass renderPass;
    nsSizeU32 m_size = {0, 0};
    uint32_t layers = 1;
    nsHybridArray<vk::ImageView, NS_GAL_MAX_RENDERTARGET_COUNT + 1> attachments;
    nsEnum<nsGALMSAASampleCount> m_msaa;
  };

  /// \brief Hashable version without pointers or redundant data of vk::AttachmentDescription
  struct AttachmentDesc
  {
    NS_DECLARE_POD_TYPE();
    vk::Format format = vk::Format::eUndefined;
    vk::SampleCountFlagBits samples = vk::SampleCountFlagBits::e1;
    // Not set at all right now
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eSampled;
    // Not set at all right now
    vk::ImageLayout initialLayout = vk::ImageLayout::eUndefined;
    // No support for eDontCare in NS right now
    vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
    vk::AttachmentLoadOp stencilLoadOp = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp stencilStoreOp = vk::AttachmentStoreOp::eStore;
  };

  /// \brief Hashable version without pointers of vk::RenderPassCreateInfo
  struct RenderPassDesc
  {
    nsHybridArray<AttachmentDesc, NS_GAL_MAX_RENDERTARGET_COUNT> attachments;
  };

  struct ResourceCacheHash
  {
    static nsUInt32 Hash(const RenderPassDesc& renderingSetup);
    static bool Equal(const RenderPassDesc& a, const RenderPassDesc& b);

    static nsUInt32 Hash(const nsGALRenderingSetup& renderingSetup);
    static bool Equal(const nsGALRenderingSetup& a, const nsGALRenderingSetup& b);

    static nsUInt32 Hash(const FramebufferKey& renderTargetSetup);
    static bool Equal(const FramebufferKey& a, const FramebufferKey& b);

    static nsUInt32 Hash(const PipelineLayoutDesc& desc);
    static bool Equal(const PipelineLayoutDesc& a, const PipelineLayoutDesc& b);

    static bool Less(const GraphicsPipelineDesc& a, const GraphicsPipelineDesc& b);
    static nsUInt32 Hash(const GraphicsPipelineDesc& desc);
    static bool Equal(const GraphicsPipelineDesc& a, const GraphicsPipelineDesc& b);

    static bool Less(const ComputePipelineDesc& a, const ComputePipelineDesc& b);
    static bool Equal(const ComputePipelineDesc& a, const ComputePipelineDesc& b);

    static nsUInt32 Hash(const nsGALShaderVulkan::DescriptorSetLayoutDesc& desc) { return desc.m_uiHash; }
    static bool Equal(const nsGALShaderVulkan::DescriptorSetLayoutDesc& a, const nsGALShaderVulkan::DescriptorSetLayoutDesc& b);
  };

  struct FrameBufferCache
  {
    vk::Framebuffer m_frameBuffer;
    nsSizeU32 m_size;
    nsEnum<nsGALMSAASampleCount> m_msaa;
    nsUInt32 m_layers = 0;
    NS_DECLARE_POD_TYPE();
  };

  static vk::RenderPass RequestRenderPassInternal(const RenderPassDesc& desc);
  static void GetRenderPassDesc(const nsGALRenderingSetup& renderingSetup, RenderPassDesc& out_desc);
  static void GetFrameBufferDesc(vk::RenderPass renderPass, const nsGALRenderTargetSetup& renderTargetSetup, FramebufferDesc& out_desc);

public:
  using GraphicsPipelineMap = nsMap<nsResourceCacheVulkan::GraphicsPipelineDesc, vk::Pipeline, nsResourceCacheVulkan::ResourceCacheHash>;
  using ComputePipelineMap = nsMap<nsResourceCacheVulkan::ComputePipelineDesc, vk::Pipeline, nsResourceCacheVulkan::ResourceCacheHash>;


private:
  static nsGALDeviceVulkan* s_pDevice;
  static vk::Device s_device;
  // We have a N to 1 mapping for nsGALRenderingSetup to vk::RenderPass as multiple nsGALRenderingSetup can share the same RenderPassDesc.
  // Thus, we have a two stage resolve to the vk::RenderPass. If a nsGALRenderingSetup is not present in s_shallowRenderPasses we create the RenderPassDesc which has a 1 to 1 relationship with vk::RenderPass and look that one up in s_renderPasses. Finally we add the entry to s_shallowRenderPasses to make sure a shallow lookup will work on the next query.
  static nsHashTable<nsGALRenderingSetup, vk::RenderPass, ResourceCacheHash> s_shallowRenderPasses; // #TODO_VULKAN cache invalidation
  static nsHashTable<RenderPassDesc, vk::RenderPass, ResourceCacheHash> s_renderPasses;
  static nsHashTable<FramebufferKey, FrameBufferCache, ResourceCacheHash> s_frameBuffers;           // #TODO_VULKAN cache invalidation

  static nsHashTable<PipelineLayoutDesc, vk::PipelineLayout, ResourceCacheHash> s_pipelineLayouts;
  static GraphicsPipelineMap s_graphicsPipelines;
  static ComputePipelineMap s_computePipelines;
  static nsMap<const nsRefCounted*, nsHybridArray<GraphicsPipelineMap::Iterator, 1>> s_graphicsPipelineUsedBy;
  static nsMap<const nsRefCounted*, nsHybridArray<ComputePipelineMap::Iterator, 1>> s_computePipelineUsedBy;

  static nsHashTable<nsGALShaderVulkan::DescriptorSetLayoutDesc, vk::DescriptorSetLayout, ResourceCacheHash> s_descriptorSetLayouts;
};
