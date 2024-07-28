#pragma once

#include <RendererFoundation/Resources/Texture.h>

#include <vulkan/vulkan.hpp>

class nsGALBufferVulkan;
class nsGALDeviceVulkan;

class nsGALTextureVulkan : public nsGALTexture
{
public:
  enum class StagingMode : nsUInt8
  {
    None,
    Buffer,          ///< We can use vkCopyImageToBuffer to a CPU buffer.
    Texture,         ///< Formats differ and we need to render to a linear CPU texture to do the conversion.
    TextureAndBuffer ///< Formats differ and linear texture can't be rendered to. Render to optimal layout GPU texture and then use vkCopyImageToBuffer to CPU buffer.
  };
  struct SubResourceOffset
  {
    NS_DECLARE_POD_TYPE();
    nsUInt32 m_uiOffset;
    nsUInt32 m_uiSize;
    nsUInt32 m_uiRowLength;
    nsUInt32 m_uiImageHeight;
  };

  NS_ALWAYS_INLINE vk::Image GetImage() const;
  NS_ALWAYS_INLINE vk::Format GetImageFormat() const { return m_imageFormat; }
  NS_ALWAYS_INLINE vk::ImageLayout GetPreferredLayout() const;
  NS_ALWAYS_INLINE vk::ImageLayout GetPreferredLayout(vk::ImageLayout targetLayout) const;
  NS_ALWAYS_INLINE vk::PipelineStageFlags GetUsedByPipelineStage() const;
  NS_ALWAYS_INLINE vk::AccessFlags GetAccessMask() const;

  NS_ALWAYS_INLINE nsVulkanAllocation GetAllocation() const;
  NS_ALWAYS_INLINE const nsVulkanAllocationInfo& GetAllocationInfo() const;

  NS_ALWAYS_INLINE bool IsLinearLayout() const;

  vk::Extent3D GetMipLevelSize(nsUInt32 uiMipLevel) const;
  vk::ImageSubresourceRange GetFullRange() const;
  vk::ImageAspectFlags GetAspectMask() const;

  // Read-back staging resources
  NS_ALWAYS_INLINE StagingMode GetStagingMode() const;
  NS_ALWAYS_INLINE nsGALTextureHandle GetStagingTexture() const;
  NS_ALWAYS_INLINE nsGALBufferHandle GetStagingBuffer() const;
  nsUInt32 ComputeSubResourceOffsets(nsDynamicArray<SubResourceOffset>& out_subResourceOffsets) const;

protected:
  friend class nsGALDeviceVulkan;
  friend class nsMemoryUtils;

  nsGALTextureVulkan(const nsGALTextureCreationDescription& Description, bool bLinearCPU, bool bStaging);

  ~nsGALTextureVulkan();

  virtual nsResult InitPlatform(nsGALDevice* pDevice, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData) override;
  virtual nsResult DeInitPlatform(nsGALDevice* pDevice) override;
  virtual void SetDebugNamePlatform(const char* szName) const override;

  static vk::Format ComputeImageFormat(nsGALDeviceVulkan* pDevice, nsEnum<nsGALResourceFormat> galFormat, vk::ImageCreateInfo& ref_createInfo, vk::ImageFormatListCreateInfo& ref_imageFormats, bool bStaging);
  static void ComputeCreateInfo(nsGALDeviceVulkan* pDevice, const nsGALTextureCreationDescription& description, vk::ImageCreateInfo& ref_createInfo, vk::PipelineStageFlags& ref_stages, vk::AccessFlags& ref_access, vk::ImageLayout& ref_preferredLayout);
  static void ComputeCreateInfoLinear(vk::ImageCreateInfo& ref_createInfo, vk::PipelineStageFlags& ref_stages, vk::AccessFlags& ref_access);
  static void ComputeAllocInfo(bool bLinearCPU, nsVulkanAllocationCreateInfo& ref_allocInfo);
  static StagingMode ComputeStagingMode(nsGALDeviceVulkan* pDevice, const nsGALTextureCreationDescription& description, const vk::ImageCreateInfo& createInfo);

  nsResult CreateStagingBuffer(const vk::ImageCreateInfo& createInfo);

  vk::Image m_image;
  vk::Format m_imageFormat = vk::Format::eUndefined;
  vk::ImageLayout m_preferredLayout = vk::ImageLayout::eUndefined;
  vk::PipelineStageFlags m_stages = {};
  vk::AccessFlags m_access = {};

  nsVulkanAllocation m_alloc = nullptr;
  nsVulkanAllocationInfo m_allocInfo;

  nsGALDeviceVulkan* m_pDevice = nullptr;

  bool m_bLinearCPU = false;
  bool m_bStaging = false;

  StagingMode m_stagingMode = StagingMode::None;
  nsGALTextureHandle m_hStagingTexture;
  nsGALBufferHandle m_hStagingBuffer;
};



#include <RendererVulkan/Resources/Implementation/TextureVulkan_inl.h>
