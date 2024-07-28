#pragma once
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/RendererFoundationDLL.h>

#include <vulkan/vulkan.hpp>

class nsGALDeviceVulkan;
class nsGALTextureResourceViewVulkan;
class nsGALBufferResourceViewVulkan;
class nsGALTextureUnorderedAccessViewVulkan;
class nsGALBufferUnorderedAccessViewVulkan;

/// \brief Creates fallback resources in case the high-level renderer did not map a resource to a descriptor slot.
/// #TODO_VULKAN: Although the class has 'Vulkan' in the name, it could be made GAL agnostic by just returning the base class of the resource views and then it will work for any device type so it could be moved to RendererFoundation if needed for another GAL implementation.
class nsFallbackResourcesVulkan
{
public:
  /// Returns a fallback resource for the given shader resource type.
  /// \param descriptorType The shader resource descriptor for which a compatible fallback resource is requested.
  /// \param textureType In case descriptorType is a texture, this specifies the texture type.
  /// \param bDepth Whether the shader resource is using a depth sampler.
  /// \return
  static const nsGALTextureResourceViewVulkan* GetFallbackTextureResourceView(nsGALShaderResourceType::Enum descriptorType, nsGALShaderTextureType::Enum textureType, bool bDepth);
  static const nsGALBufferResourceViewVulkan* GetFallbackBufferResourceView(nsGALShaderResourceType::Enum descriptorType);
  static const nsGALTextureUnorderedAccessViewVulkan* GetFallbackTextureUnorderedAccessView(nsGALShaderResourceType::Enum descriptorType, nsGALShaderTextureType::Enum textureType);
  static const nsGALBufferUnorderedAccessViewVulkan* GetFallbackBufferUnorderedAccessView(nsGALShaderResourceType::Enum descriptorType);

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererVulkan, FallbackResourcesVulkan)
  static void GALDeviceEventHandler(const nsGALDeviceEvent& e);
  static void Initialize();
  static void DeInitialize();

  static nsGALDevice* s_pDevice;
  static nsEventSubscriptionID s_EventID;

  struct Key
  {
    NS_DECLARE_POD_TYPE();
    nsEnum<nsGALShaderResourceType> m_ResourceType;
    nsEnum<nsGALShaderTextureType> m_nsType;
    bool m_bDepth = false;
  };

  struct KeyHash
  {
    static nsUInt32 Hash(const Key& a);
    static bool Equal(const Key& a, const Key& b);

    static nsUInt32 Hash(const nsEnum<nsGALShaderResourceType>& a);
    static bool Equal(const nsEnum<nsGALShaderResourceType>& a, const nsEnum<nsGALShaderResourceType>& b);
  };

  static nsHashTable<Key, nsGALTextureResourceViewHandle, KeyHash> m_TextureResourceViews;
  static nsHashTable<nsEnum<nsGALShaderResourceType>, nsGALBufferResourceViewHandle, KeyHash> m_BufferResourceViews;
  static nsHashTable<Key, nsGALTextureUnorderedAccessViewHandle, KeyHash> m_TextureUAVs;
  static nsHashTable<nsEnum<nsGALShaderResourceType>, nsGALBufferUnorderedAccessViewHandle, KeyHash> m_BufferUAVs;

  static nsDynamicArray<nsGALBufferHandle> m_Buffers;
  static nsDynamicArray<nsGALTextureHandle> m_Textures;
};
