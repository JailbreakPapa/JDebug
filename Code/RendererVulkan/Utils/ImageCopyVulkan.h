#pragma once

#include <RendererVulkan/RendererVulkanDLL.h>

#include <RendererFoundation/Shader/ShaderUtils.h>
#include <RendererVulkan/Cache/ResourceCacheVulkan.h>

#include <vulkan/vulkan.hpp>

class nsGALBufferVulkan;
class nsGALTextureVulkan;
class nsGALRenderTargetViewVulkan;
class nsGALTextureResourceViewVulkan;
class nsGALBufferResourceViewVulkan;
class nsGALTextureUnorderedAccessViewVulkan;
class nsGALBufferUnorderedAccessViewVulkan;


/// \brief
class NS_RENDERERVULKAN_DLL nsImageCopyVulkan
{
public:
  nsImageCopyVulkan(nsGALDeviceVulkan& GALDeviceVulkan);
  ~nsImageCopyVulkan();
  void Init(const nsGALTextureVulkan* pSource, const nsGALTextureVulkan* pTarget, nsShaderUtils::nsBuiltinShaderType type);

  void Copy(const nsVec3U32& sourceOffset, const vk::ImageSubresourceLayers& sourceLayers, const nsVec3U32& targetOffset, const vk::ImageSubresourceLayers& targetLayers, const nsVec3U32& extends);

  static void Initialize(nsGALDeviceVulkan& GALDeviceVulkan);
  static void DeInitialize(nsGALDeviceVulkan& GALDeviceVulkan);

  struct RenderPassCacheKey
  {
    NS_DECLARE_POD_TYPE();

    vk::Format targetFormat;
    vk::SampleCountFlagBits targetSamples;
  };

  struct FramebufferCacheKey
  {
    NS_DECLARE_POD_TYPE();

    vk::RenderPass m_renderpass;
    vk::ImageView m_targetView;
    nsVec3U32 m_extends;
    uint32_t m_layerCount;
  };

  struct ImageViewCacheKey
  {
    NS_DECLARE_POD_TYPE();

    vk::Image m_image;
    vk::ImageSubresourceLayers m_subresourceLayers;
  };

  struct ImageViewCacheValue
  {
    NS_DECLARE_POD_TYPE();

    vk::ImageSubresourceLayers m_subresourceLayers;
    vk::ImageView m_imageView;
  };

private:
  void RenderInternal(const nsVec3U32& sourceOffset, const vk::ImageSubresourceLayers& sourceLayers, const nsVec3U32& targetOffset, const vk::ImageSubresourceLayers& targetLayers, const nsVec3U32& extends);

  static void OnBeforeImageDestroyed(nsGALDeviceVulkan::OnBeforeImageDestroyedData data);


private:
  nsGALDeviceVulkan& m_GALDeviceVulkan;

  // Init input
  const nsGALTextureVulkan* m_pSource = nullptr;
  const nsGALTextureVulkan* m_pTarget = nullptr;
  nsShaderUtils::nsBuiltinShaderType m_type = nsShaderUtils::nsBuiltinShaderType::CopyImage;

  // Init derived Vulkan objects
  vk::RenderPass m_renderPass;
  nsShaderUtils::nsBuiltinShader m_shader;
  nsGALVertexDeclarationHandle m_hVertexDecl;
  nsResourceCacheVulkan::PipelineLayoutDesc m_LayoutDesc;
  nsResourceCacheVulkan::GraphicsPipelineDesc m_PipelineDesc;
  vk::Pipeline m_pipeline;

  // Cache to keep important resources alive
  // This avoids recreating them every frame
  struct Cache
  {
    Cache(nsAllocator* pAllocator);
    ~Cache();

    nsHashTable<nsGALShaderHandle, nsGALVertexDeclarationHandle> m_vertexDeclarations;
    nsHashTable<RenderPassCacheKey, vk::RenderPass> m_renderPasses;
    nsHashTable<ImageViewCacheKey, vk::ImageView> m_sourceImageViews;
    nsHashTable<vk::Image, ImageViewCacheValue> m_imageToSourceImageViewCacheKey;
    nsHashTable<ImageViewCacheKey, vk::ImageView> m_targetImageViews;
    nsHashTable<vk::Image, ImageViewCacheValue> m_imageToTargetImageViewCacheKey;
    nsHashTable<FramebufferCacheKey, vk::Framebuffer> m_framebuffers;
    nsHashTable<nsShaderUtils::nsBuiltinShaderType, nsShaderUtils::nsBuiltinShader> m_shaders;

    nsEventSubscriptionID m_onBeforeImageDeletedSubscription;
  };

  static nsUniquePtr<Cache> s_cache;
};
