#pragma once

#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Threading/Mutex.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/Resources/ResourceFormats.h>

struct nsGALDeviceEvent;

/// \brief This class serves as a pool for GPU related resources (e.g. buffers and textures required for rendering).
/// Note that the functions creating and returning render targets are thread safe (by using a mutex).
class NS_RENDERERCORE_DLL nsGPUResourcePool
{
public:
  nsGPUResourcePool();
  ~nsGPUResourcePool();

  /// \brief Returns a render target handle for the given texture description
  /// Note that you should return the handle to the pool and never destroy it directly with the device.
  nsGALTextureHandle GetRenderTarget(const nsGALTextureCreationDescription& textureDesc);

  /// \brief Convenience functions which creates a texture description fit for a 2d render target without a mip chains.
  nsGALTextureHandle GetRenderTarget(nsUInt32 uiWidth, nsUInt32 uiHeight, nsGALResourceFormat::Enum format,
    nsGALMSAASampleCount::Enum sampleCount = nsGALMSAASampleCount::None, nsUInt32 uiSliceColunt = 1);

  /// \brief Returns a render target to the pool so other consumers can use it.
  /// Note that targets which are returned to the pool are susceptible to destruction due to garbage collection.
  void ReturnRenderTarget(nsGALTextureHandle hRenderTarget);


  /// \brief Returns a buffer handle for the given buffer description
  nsGALBufferHandle GetBuffer(const nsGALBufferCreationDescription& bufferDesc);

  /// \brief Returns a buffer to the pool so other consumers can use it.
  void ReturnBuffer(nsGALBufferHandle hBuffer);


  /// \brief Tries to free resources which are currently in the pool.
  /// Triggered automatically due to allocation number / size thresholds but can be triggered manually (e.g. after editor window resize)
  ///
  /// \param uiMinimumAge How many frames at least the resource needs to have been unused before it will be GCed.
  void RunGC(nsUInt32 uiMinimumAge);


  static nsGPUResourcePool* GetDefaultInstance();
  static void SetDefaultInstance(nsGPUResourcePool* pDefaultInstance);

protected:
  void CheckAndPotentiallyRunGC();
  void UpdateMemoryStats() const;
  void GALDeviceEventHandler(const nsGALDeviceEvent& e);

  struct TextureHandleWithAge
  {
    nsGALTextureHandle m_hTexture;
    nsUInt64 m_uiLastUsed = 0;
  };

  struct BufferHandleWithAge
  {
    nsGALBufferHandle m_hBuffer;
    nsUInt64 m_uiLastUsed = 0;
  };

  nsEventSubscriptionID m_GALDeviceEventSubscriptionID = 0;
  nsUInt64 m_uiMemoryThresholdForGC = 256 * 1024 * 1024;
  nsUInt64 m_uiCurrentlyAllocatedMemory = 0;
  nsUInt16 m_uiNumAllocationsThresholdForGC = 128;
  nsUInt16 m_uiNumAllocationsSinceLastGC = 0;
  nsUInt16 m_uiFramesThresholdSinceLastGC = 60; ///< Every 60 frames resources unused for more than 10 frames in a row are GCed.
  nsUInt16 m_uiFramesSinceLastGC = 0;

  nsMap<nsUInt32, nsDynamicArray<TextureHandleWithAge>> m_AvailableTextures;
  nsSet<nsGALTextureHandle> m_TexturesInUse;

  nsMap<nsUInt32, nsDynamicArray<BufferHandleWithAge>> m_AvailableBuffers;
  nsSet<nsGALBufferHandle> m_BuffersInUse;

  nsMutex m_Lock;

  nsGALDevice* m_pDevice;

private:
  static nsGPUResourcePool* s_pDefaultInstance;
};
