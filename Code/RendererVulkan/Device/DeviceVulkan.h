
#pragma once

#include <Foundation/Types/Bitflags.h>
#include <Foundation/Types/UniquePtr.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererVulkan/Device/DispatchContext.h>
#include <RendererVulkan/MemoryAllocator/MemoryAllocatorVulkan.h>
#include <RendererVulkan/RendererVulkanDLL.h>

#include <vulkan/vulkan.hpp>

NS_DEFINE_AS_POD_TYPE(vk::Format);

struct nsGALFormatLookupEntryVulkan
{
  nsGALFormatLookupEntryVulkan() = default;
  nsGALFormatLookupEntryVulkan(vk::Format format)
  {
    m_format = format;
    m_readback = format;
  }

  nsGALFormatLookupEntryVulkan(vk::Format format, nsArrayPtr<vk::Format> mutableFormats)
  {
    m_format = format;
    m_readback = format;
    m_mutableFormats = mutableFormats;
  }

  inline nsGALFormatLookupEntryVulkan& R(vk::Format readbackType)
  {
    m_readback = readbackType;
    return *this;
  }

  vk::Format m_format = vk::Format::eUndefined;
  vk::Format m_readback = vk::Format::eUndefined;
  nsHybridArray<vk::Format, 6> m_mutableFormats;
};

using nsGALFormatLookupTableVulkan = nsGALFormatLookupTable<nsGALFormatLookupEntryVulkan>;

class nsGALBufferVulkan;
class nsGALTextureVulkan;
class nsGALPassVulkan;
class nsPipelineBarrierVulkan;
class nsCommandBufferPoolVulkan;
class nsStagingBufferPoolVulkan;
class nsQueryPoolVulkan;
class nsInitContextVulkan;

/// \brief The Vulkan device implementation of the graphics abstraction layer.
class NS_RENDERERVULKAN_DLL nsGALDeviceVulkan : public nsGALDevice
{
private:
  friend nsInternal::NewInstance<nsGALDevice> CreateVulkanDevice(nsAllocator* pAllocator, const nsGALDeviceCreationDescription& Description);
  nsGALDeviceVulkan(const nsGALDeviceCreationDescription& Description);

public:
  virtual ~nsGALDeviceVulkan();

public:
  struct PendingDeletionFlags
  {
    using StorageType = nsUInt32;

    enum Enum
    {
      UsesExternalMemory = NS_BIT(0),
      IsFileDescriptor = NS_BIT(1),
      Default = 0
    };

    struct Bits
    {
      StorageType UsesExternalMemory : 1;
      StorageType IsFileDescriptor : 1;
    };
  };

  struct PendingDeletion
  {
    NS_DECLARE_POD_TYPE();
    vk::ObjectType m_type;
    nsBitflags<PendingDeletionFlags> m_flags;
    void* m_pObject;
    union
    {
      nsVulkanAllocation m_allocation;
      void* m_pContext;
    };
  };

  struct ReclaimResource
  {
    NS_DECLARE_POD_TYPE();
    vk::ObjectType m_type;
    void* m_pObject;
    void* m_pContext = nullptr;
  };

  struct Extensions
  {
    bool m_bSurface = false;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    bool m_bWin32Surface = false;
#elif NS_ENABLED(NS_SUPPORTS_GLFW)
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
    bool m_bAndroidSurface = false;
#else
#  error "Vulkan Platform not supported"
#endif

    bool m_bDebugUtils = false;
    bool m_bDebugUtilsMarkers = false;
    PFN_vkCreateDebugUtilsMessengerEXT pfn_vkCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT pfn_vkDestroyDebugUtilsMessengerEXT = nullptr;
    PFN_vkSetDebugUtilsObjectNameEXT pfn_vkSetDebugUtilsObjectNameEXT = nullptr;

    bool m_bDeviceSwapChain = false;
    bool m_bShaderViewportIndexLayer = false;

    vk::PhysicalDeviceCustomBorderColorFeaturesEXT m_borderColorEXT;
    bool m_bBorderColorFloat = false;

    bool m_bImageFormatList = false;
    vk::PhysicalDeviceTimelineSemaphoreFeatures m_timelineSemaphoresEXT;
    bool m_bTimelineSemaphore = false;

    bool m_bExternalMemoryCapabilities = false;
    bool m_bExternalSemaphoreCapabilities = false;

    bool m_bExternalMemory = false;
    bool m_bExternalSemaphore = false;

    bool m_bExternalMemoryFd = false;
    bool m_bExternalSemaphoreFd = false;

    bool m_bExternalMemoryWin32 = false;
    bool m_bExternalSemaphoreWin32 = false;
  };

  struct Queue
  {
    vk::Queue m_queue;
    nsUInt32 m_uiQueueFamily = -1;
    nsUInt32 m_uiQueueIndex = 0;
  };

  nsUInt64 GetCurrentFrame() const { return m_uiFrameCounter; }
  nsUInt64 GetSafeFrame() const { return m_uiSafeFrame; }

  vk::Instance GetVulkanInstance() const;
  vk::Device GetVulkanDevice() const;
  const Queue& GetGraphicsQueue() const;
  const Queue& GetTransferQueue() const;

  vk::PhysicalDevice GetVulkanPhysicalDevice() const;
  const vk::PhysicalDeviceProperties& GetPhysicalDeviceProperties() const { return m_properties; }
  const Extensions& GetExtensions() const { return m_extensions; }
  const nsVulkanDispatchContext& GetDispatchContext() const { return m_dispatchContext; }
  vk::PipelineStageFlags GetSupportedStages() const;

  vk::CommandBuffer& GetCurrentCommandBuffer();
  nsPipelineBarrierVulkan& GetCurrentPipelineBarrier();
  nsQueryPoolVulkan& GetQueryPool() const;
  nsStagingBufferPoolVulkan& GetStagingBufferPool() const;
  nsInitContextVulkan& GetInitContext() const;
  nsProxyAllocator& GetAllocator();

  nsGALTextureHandle CreateTextureInternal(const nsGALTextureCreationDescription& Description, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData, bool bLinearCPU = false, bool bStaging = false);
  nsGALBufferHandle CreateBufferInternal(const nsGALBufferCreationDescription& Description, nsArrayPtr<const nsUInt8> pInitialData, bool bCPU = false);

  const nsGALFormatLookupTableVulkan& GetFormatLookupTable() const;

  nsInt32 GetMemoryIndex(vk::MemoryPropertyFlags properties, const vk::MemoryRequirements& requirements) const;

  vk::Fence Submit(bool bAddSignalSemaphore = true);

  void DeleteLaterImpl(const PendingDeletion& deletion);

  void DeleteLater(vk::Image& image, vk::DeviceMemory& externalMemory)
  {
    if (image)
    {
      PendingDeletion del = {vk::ObjectType::eImage, {PendingDeletionFlags::UsesExternalMemory}, (void*)image, nullptr};
      del.m_pContext = (void*)externalMemory;
      DeleteLaterImpl(del);
    }
    image = nullptr;
    externalMemory = nullptr;
  }

  template <typename T>
  void DeleteLater(T& object, nsVulkanAllocation& allocation)
  {
    if (object)
    {
      DeleteLaterImpl({object.objectType, {}, (void*)object, allocation});
    }
    object = nullptr;
    allocation = nullptr;
  }

  template <typename T>
  void DeleteLater(T& object, void* pContext)
  {
    if (object)
    {
      PendingDeletion del = {object.objectType, {}, (void*)object, nullptr};
      del.m_pContext = pContext;
      DeleteLaterImpl(static_cast<const PendingDeletion&>(del));
    }
    object = nullptr;
  }

  template <typename T>
  void DeleteLater(T& object)
  {
    if (object)
    {
      DeleteLaterImpl({object.objectType, {}, (void*)object, nullptr});
    }
    object = nullptr;
  }

  void ReclaimLater(const ReclaimResource& reclaim);

  template <typename T>
  void ReclaimLater(T& object, void* pContext = nullptr)
  {
    ReclaimLater({object.objectType, (void*)object, pContext});
    object = nullptr;
  }

  void SetDebugName(const vk::DebugUtilsObjectNameInfoEXT& info, nsVulkanAllocation allocation = nullptr);

  template <typename T>
  void SetDebugName(const char* szName, T& object, nsVulkanAllocation allocation = nullptr)
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    if (object)
    {
      vk::DebugUtilsObjectNameInfoEXT nameInfo;
      nameInfo.objectType = object.objectType;
      nameInfo.objectHandle = (uint64_t) static_cast<typename T::NativeType>(object);
      nameInfo.pObjectName = szName;

      SetDebugName(nameInfo, allocation);
    }
#endif
  }

  void ReportLiveGpuObjects();

  static void UploadBufferStaging(nsStagingBufferPoolVulkan* pStagingBufferPool, nsPipelineBarrierVulkan* pPipelineBarrier, vk::CommandBuffer commandBuffer, const nsGALBufferVulkan* pBuffer, nsArrayPtr<const nsUInt8> pInitialData, vk::DeviceSize dstOffset = 0);
  static void UploadTextureStaging(nsStagingBufferPoolVulkan* pStagingBufferPool, nsPipelineBarrierVulkan* pPipelineBarrier, vk::CommandBuffer commandBuffer, const nsGALTextureVulkan* pTexture, const vk::ImageSubresourceLayers& subResource, const nsGALSystemMemoryDescription& data);

  struct OnBeforeImageDestroyedData
  {
    vk::Image image;
    nsGALDeviceVulkan& GALDeviceVulkan;
  };
  nsEvent<OnBeforeImageDestroyedData> OnBeforeImageDestroyed;

  virtual const nsGALSharedTexture* GetSharedTexture(nsGALTextureHandle hTexture) const override;

  struct SemaphoreInfo
  {
    static SemaphoreInfo MakeWaitSemaphore(vk::Semaphore semaphore, vk::PipelineStageFlagBits waitStage = vk::PipelineStageFlagBits::eAllCommands, vk::SemaphoreType type = vk::SemaphoreType::eBinary, nsUInt64 uiValue = 0)
    {
      return SemaphoreInfo{semaphore, type, waitStage, uiValue};
    }

    static SemaphoreInfo MakeSignalSemaphore(vk::Semaphore semaphore, vk::SemaphoreType type = vk::SemaphoreType::eBinary, nsUInt64 uiValue = 0)
    {
      return SemaphoreInfo{semaphore, type, vk::PipelineStageFlagBits::eNone, uiValue};
    }

    vk::Semaphore m_semaphore;
    vk::SemaphoreType m_type = vk::SemaphoreType::eBinary;
    vk::PipelineStageFlagBits m_waitStage = vk::PipelineStageFlagBits::eAllCommands;
    nsUInt64 m_uiValue = 0;
  };
  void AddWaitSemaphore(const SemaphoreInfo& waitSemaphore);
  void AddSignalSemaphore(const SemaphoreInfo& signalSemaphore);

  // These functions need to be implemented by a render API abstraction
protected:
  // Init & shutdown functions

  vk::Result SelectInstanceExtensions(nsHybridArray<const char*, 6>& extensions);
  vk::Result SelectDeviceExtensions(vk::DeviceCreateInfo& deviceCreateInfo, nsHybridArray<const char*, 6>& extensions);

  virtual nsStringView GetRendererPlatform() override;
  virtual nsResult InitPlatform() override;
  virtual nsResult ShutdownPlatform() override;

  // Pipeline & Pass functions

  virtual void BeginPipelinePlatform(const char* szName, nsGALSwapChain* pSwapChain) override;
  virtual void EndPipelinePlatform(nsGALSwapChain* pSwapChain) override;

  virtual nsGALPass* BeginPassPlatform(const char* szName) override;
  virtual void EndPassPlatform(nsGALPass* pPass) override;

  virtual void FlushPlatform() override;


  // State creation functions

  virtual nsGALBlendState* CreateBlendStatePlatform(const nsGALBlendStateCreationDescription& Description) override;
  virtual void DestroyBlendStatePlatform(nsGALBlendState* pBlendState) override;

  virtual nsGALDepthStencilState* CreateDepthStencilStatePlatform(const nsGALDepthStencilStateCreationDescription& Description) override;
  virtual void DestroyDepthStencilStatePlatform(nsGALDepthStencilState* pDepthStencilState) override;

  virtual nsGALRasterizerState* CreateRasterizerStatePlatform(const nsGALRasterizerStateCreationDescription& Description) override;
  virtual void DestroyRasterizerStatePlatform(nsGALRasterizerState* pRasterizerState) override;

  virtual nsGALSamplerState* CreateSamplerStatePlatform(const nsGALSamplerStateCreationDescription& Description) override;
  virtual void DestroySamplerStatePlatform(nsGALSamplerState* pSamplerState) override;


  // Resource creation functions

  virtual nsGALShader* CreateShaderPlatform(const nsGALShaderCreationDescription& Description) override;
  virtual void DestroyShaderPlatform(nsGALShader* pShader) override;

  virtual nsGALBuffer* CreateBufferPlatform(const nsGALBufferCreationDescription& Description, nsArrayPtr<const nsUInt8> pInitialData) override;
  virtual void DestroyBufferPlatform(nsGALBuffer* pBuffer) override;

  virtual nsGALTexture* CreateTexturePlatform(const nsGALTextureCreationDescription& Description, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData) override;
  virtual void DestroyTexturePlatform(nsGALTexture* pTexture) override;

  virtual nsGALTexture* CreateSharedTexturePlatform(const nsGALTextureCreationDescription& Description, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData, nsEnum<nsGALSharedTextureType> sharedType, nsGALPlatformSharedHandle handle) override;
  virtual void DestroySharedTexturePlatform(nsGALTexture* pTexture) override;

  virtual nsGALTextureResourceView* CreateResourceViewPlatform(nsGALTexture* pResource, const nsGALTextureResourceViewCreationDescription& Description) override;
  virtual void DestroyResourceViewPlatform(nsGALTextureResourceView* pResourceView) override;

  virtual nsGALBufferResourceView* CreateResourceViewPlatform(nsGALBuffer* pResource, const nsGALBufferResourceViewCreationDescription& Description) override;
  virtual void DestroyResourceViewPlatform(nsGALBufferResourceView* pResourceView) override;

  virtual nsGALRenderTargetView* CreateRenderTargetViewPlatform(nsGALTexture* pTexture, const nsGALRenderTargetViewCreationDescription& Description) override;
  virtual void DestroyRenderTargetViewPlatform(nsGALRenderTargetView* pRenderTargetView) override;

  nsGALTextureUnorderedAccessView* CreateUnorderedAccessViewPlatform(nsGALTexture* pResource, const nsGALTextureUnorderedAccessViewCreationDescription& Description) override;
  virtual void DestroyUnorderedAccessViewPlatform(nsGALTextureUnorderedAccessView* pUnorderedAccessView) override;

  nsGALBufferUnorderedAccessView* CreateUnorderedAccessViewPlatform(nsGALBuffer* pResource, const nsGALBufferUnorderedAccessViewCreationDescription& Description) override;
  virtual void DestroyUnorderedAccessViewPlatform(nsGALBufferUnorderedAccessView* pUnorderedAccessView) override;

  // Other rendering creation functions

  virtual nsGALQuery* CreateQueryPlatform(const nsGALQueryCreationDescription& Description) override;
  virtual void DestroyQueryPlatform(nsGALQuery* pQuery) override;

  virtual nsGALVertexDeclaration* CreateVertexDeclarationPlatform(const nsGALVertexDeclarationCreationDescription& Description) override;
  virtual void DestroyVertexDeclarationPlatform(nsGALVertexDeclaration* pVertexDeclaration) override;

  // Timestamp functions

  virtual nsGALTimestampHandle GetTimestampPlatform() override;
  virtual nsResult GetTimestampResultPlatform(nsGALTimestampHandle hTimestamp, nsTime& result) override;

  // Misc functions

  virtual void BeginFramePlatform(const nsUInt64 uiRenderFrame) override;
  virtual void EndFramePlatform() override;

  virtual void FillCapabilitiesPlatform() override;

  virtual void WaitIdlePlatform() override;

  /// \endcond

private:
  struct PerFrameData
  {
    /// \brief These are all fences passed into submit calls. For some reason waiting for the fence of the last submit is not enough. At least I can't get it to work (neither semaphores nor barriers make it past the validation layer).
    nsHybridArray<vk::Fence, 2> m_CommandBufferFences;

    vk::CommandBuffer m_currentCommandBuffer;
    // ID3D11Query* m_pDisjointTimerQuery = nullptr;
    double m_fInvTicksPerSecond = -1.0;
    nsUInt64 m_uiFrame = -1;

    nsMutex m_pendingDeletionsMutex;
    nsDeque<PendingDeletion> m_pendingDeletions;
    nsDeque<PendingDeletion> m_pendingDeletionsPrevious;

    nsMutex m_reclaimResourcesMutex;
    nsDeque<ReclaimResource> m_reclaimResources;
    nsDeque<ReclaimResource> m_reclaimResourcesPrevious;
  };

  void DeletePendingResources(nsDeque<PendingDeletion>& pendingDeletions);
  void ReclaimResources(nsDeque<ReclaimResource>& resources);

  void FillFormatLookupTable();

  nsUInt64 m_uiFrameCounter = 1; ///< We start at 1 so m_uiFrameCounter and m_uiSafeFrame are not equal at the start.
  nsUInt64 m_uiSafeFrame = 0;
  nsUInt8 m_uiCurrentPerFrameData = 0;
  nsUInt8 m_uiNextPerFrameData = 1;

  vk::Instance m_instance;
  vk::PhysicalDevice m_physicalDevice;
  vk::PhysicalDeviceProperties m_properties;
  vk::Device m_device;
  Queue m_graphicsQueue;
  Queue m_transferQueue;

  nsGALFormatLookupTableVulkan m_FormatLookupTable;
  vk::PipelineStageFlags m_supportedStages;
  vk::PhysicalDeviceMemoryProperties m_memoryProperties;

  nsUniquePtr<nsGALPassVulkan> m_pDefaultPass;
  nsUniquePtr<nsPipelineBarrierVulkan> m_pPipelineBarrier;
  nsUniquePtr<nsCommandBufferPoolVulkan> m_pCommandBufferPool;
  nsUniquePtr<nsStagingBufferPoolVulkan> m_pStagingBufferPool;
  nsUniquePtr<nsQueryPoolVulkan> m_pQueryPool;
  nsUniquePtr<nsInitContextVulkan> m_pInitContext;

  // We daisy-chain all command buffers in a frame in sequential order via this semaphore for now.
  vk::Semaphore m_lastCommandBufferFinished;

  PerFrameData m_PerFrameData[4];

#if NS_ENABLED(NS_USE_PROFILING)
  struct GPUTimingScope* m_pFrameTimingScope = nullptr;
  struct GPUTimingScope* m_pPipelineTimingScope = nullptr;
  struct GPUTimingScope* m_pPassTimingScope = nullptr;
#endif

  Extensions m_extensions;
  nsVulkanDispatchContext m_dispatchContext;
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
#endif
  nsHybridArray<SemaphoreInfo, 3> m_waitSemaphores;
  nsHybridArray<SemaphoreInfo, 3> m_signalSemaphores;
};

#include <RendererVulkan/Device/Implementation/DeviceVulkan_inl.h>
