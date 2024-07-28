#include <RendererVulkan/RendererVulkanPCH.h>

#include <RendererVulkan/Resources/FallbackResourcesVulkan.h>

#include <Foundation/Algorithm/HashStream.h>
#include <Foundation/Configuration/Startup.h>
#include <RendererFoundation/Resources/Buffer.h>
#include <RendererVulkan/Device/DeviceVulkan.h>
#include <RendererVulkan/Resources/ResourceViewVulkan.h>
#include <RendererVulkan/Resources/TextureVulkan.h>
#include <RendererVulkan/Resources/UnorderedAccessViewVulkan.h>
#include <RendererVulkan/Utils/ConversionUtilsVulkan.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererVulkan, FallbackResourcesVulkan)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsFallbackResourcesVulkan::Initialize();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsFallbackResourcesVulkan::DeInitialize();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsGALDevice* nsFallbackResourcesVulkan::s_pDevice = nullptr;
nsEventSubscriptionID nsFallbackResourcesVulkan::s_EventID = 0;

nsHashTable<nsFallbackResourcesVulkan::Key, nsGALTextureResourceViewHandle, nsFallbackResourcesVulkan::KeyHash> nsFallbackResourcesVulkan::m_TextureResourceViews;
nsHashTable<nsEnum<nsGALShaderResourceType>, nsGALBufferResourceViewHandle, nsFallbackResourcesVulkan::KeyHash> nsFallbackResourcesVulkan::m_BufferResourceViews;
nsHashTable<nsFallbackResourcesVulkan::Key, nsGALTextureUnorderedAccessViewHandle, nsFallbackResourcesVulkan::KeyHash> nsFallbackResourcesVulkan::m_TextureUAVs;
nsHashTable<nsEnum<nsGALShaderResourceType>, nsGALBufferUnorderedAccessViewHandle, nsFallbackResourcesVulkan::KeyHash> nsFallbackResourcesVulkan::m_BufferUAVs;
nsDynamicArray<nsGALBufferHandle> nsFallbackResourcesVulkan::m_Buffers;
nsDynamicArray<nsGALTextureHandle> nsFallbackResourcesVulkan::m_Textures;

void nsFallbackResourcesVulkan::Initialize()
{
  s_EventID = nsGALDevice::s_Events.AddEventHandler(nsMakeDelegate(&nsFallbackResourcesVulkan::GALDeviceEventHandler));
}

void nsFallbackResourcesVulkan::DeInitialize()
{
  nsGALDevice::s_Events.RemoveEventHandler(s_EventID);
}
void nsFallbackResourcesVulkan::GALDeviceEventHandler(const nsGALDeviceEvent& e)
{
  switch (e.m_Type)
  {
    case nsGALDeviceEvent::AfterInit:
    {
      s_pDevice = e.m_pDevice;
      auto CreateTexture = [](nsGALTextureType::Enum type, nsGALMSAASampleCount::Enum samples, bool bDepth) -> nsGALTextureResourceViewHandle
      {
        nsGALTextureCreationDescription desc;
        desc.m_uiWidth = 4;
        desc.m_uiHeight = 4;
        if (type == nsGALTextureType::Texture3D)
          desc.m_uiDepth = 4;
        desc.m_uiMipLevelCount = 1;
        desc.m_Format = bDepth ? nsGALResourceFormat::D16 : nsGALResourceFormat::BGRAUByteNormalizedsRGB;
        desc.m_Type = type;
        desc.m_SampleCount = samples;
        desc.m_ResourceAccess.m_bImmutable = false;
        desc.m_bCreateRenderTarget = bDepth;
        nsGALTextureHandle hTexture = s_pDevice->CreateTexture(desc);
        NS_ASSERT_DEV(!hTexture.IsInvalidated(), "Failed to create fallback resource");
        // Debug device not set yet.
        s_pDevice->GetTexture(hTexture)->SetDebugName("FallbackResourceVulkan");
        m_Textures.PushBack(hTexture);
        return s_pDevice->GetDefaultResourceView(hTexture);
      };
      {
        nsGALTextureResourceViewHandle hView = CreateTexture(nsGALTextureType::Texture2D, nsGALMSAASampleCount::None, false);
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2D, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2DArray, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::Texture2D, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::Texture2DArray, false}] = hView;
      }
      {
        nsGALTextureResourceViewHandle hView = CreateTexture(nsGALTextureType::Texture2D, nsGALMSAASampleCount::None, true);
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2D, true}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2DArray, true}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::Texture2D, true}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::Texture2DArray, true}] = hView;
      }

      // Swift shader can only do 4x MSAA. Add a check anyways.
      const bool bSupported = s_pDevice->GetCapabilities().m_FormatSupport[nsGALResourceFormat::BGRAUByteNormalizedsRGB].AreAllSet(nsGALResourceFormatSupport::Texture | nsGALResourceFormatSupport::MSAA4x);

      if (bSupported)
      {
        nsGALTextureResourceViewHandle hView = CreateTexture(nsGALTextureType::Texture2D, nsGALMSAASampleCount::FourSamples, false);
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2DMS, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture2DMSArray, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::Texture2DMS, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::Texture2DMSArray, false}] = hView;
      }
      {
        nsGALTextureResourceViewHandle hView = CreateTexture(nsGALTextureType::TextureCube, nsGALMSAASampleCount::None, false);
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::TextureCube, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::TextureCubeArray, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::TextureCube, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::TextureCubeArray, false}] = hView;
      }
      {
        nsGALTextureResourceViewHandle hView = CreateTexture(nsGALTextureType::Texture3D, nsGALMSAASampleCount::None, false);
        m_TextureResourceViews[{nsGALShaderResourceType::Texture, nsGALShaderTextureType::Texture3D, false}] = hView;
        m_TextureResourceViews[{nsGALShaderResourceType::TextureAndSampler, nsGALShaderTextureType::Texture3D, false}] = hView;
      }
      {
        nsGALBufferCreationDescription desc;
        desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ByteAddressBuffer | nsGALBufferUsageFlags::ShaderResource;
        desc.m_uiStructSize = 128;
        desc.m_uiTotalSize = 1280;
        desc.m_ResourceAccess.m_bImmutable = false;
        nsGALBufferHandle hBuffer = s_pDevice->CreateBuffer(desc);
        s_pDevice->GetBuffer(hBuffer)->SetDebugName("FallbackStructuredBufferVulkan");
        m_Buffers.PushBack(hBuffer);
        nsGALBufferResourceViewHandle hView = s_pDevice->GetDefaultResourceView(hBuffer);
        m_BufferResourceViews[nsGALShaderResourceType::ConstantBuffer] = hView;
        m_BufferResourceViews[nsGALShaderResourceType::ConstantBuffer] = hView;
        m_BufferResourceViews[nsGALShaderResourceType::StructuredBuffer] = hView;
        m_BufferResourceViews[nsGALShaderResourceType::StructuredBuffer] = hView;
      }
      {
        nsGALBufferCreationDescription desc;
        desc.m_uiStructSize = sizeof(nsUInt32);
        desc.m_uiTotalSize = 1024;
        desc.m_BufferFlags = nsGALBufferUsageFlags::TexelBuffer | nsGALBufferUsageFlags::ShaderResource;
        desc.m_ResourceAccess.m_bImmutable = false;
        nsGALBufferHandle hBuffer = s_pDevice->CreateBuffer(desc);
        s_pDevice->GetBuffer(hBuffer)->SetDebugName("FallbackTexelBufferVulkan");
        m_Buffers.PushBack(hBuffer);
        nsGALBufferResourceViewHandle hView = s_pDevice->GetDefaultResourceView(hBuffer);
        m_BufferResourceViews[nsGALShaderResourceType::TexelBuffer] = hView;
      }
      {
        nsGALTextureCreationDescription desc;
        desc.m_uiWidth = 4;
        desc.m_uiHeight = 4;
        desc.m_uiMipLevelCount = 1;
        desc.m_Format = nsGALResourceFormat::RGBAHalf;
        desc.m_Type = nsGALTextureType::Texture2D;
        desc.m_SampleCount = nsGALMSAASampleCount::None;
        desc.m_ResourceAccess.m_bImmutable = false;
        desc.m_bCreateRenderTarget = false;
        desc.m_bAllowUAV = true;
        nsGALTextureHandle hTexture = s_pDevice->CreateTexture(desc);
        NS_ASSERT_DEV(!hTexture.IsInvalidated(), "Failed to create fallback resource");
        // Debug device not set yet.
        s_pDevice->GetTexture(hTexture)->SetDebugName("FallbackTextureRWVulkan");
        m_Textures.PushBack(hTexture);

        nsGALTextureUnorderedAccessViewCreationDescription descUAV;
        descUAV.m_hTexture = hTexture;
        auto hUAV = s_pDevice->CreateUnorderedAccessView(descUAV);
        m_TextureUAVs[{nsGALShaderResourceType::TextureRW, nsGALShaderTextureType::Unknown, false}] = hUAV;
      }
      {
        nsGALBufferCreationDescription desc;
        desc.m_uiStructSize = sizeof(nsUInt32);
        desc.m_uiTotalSize = 1024;
        desc.m_BufferFlags = nsGALBufferUsageFlags::TexelBuffer | nsGALBufferUsageFlags::ShaderResource | nsGALBufferUsageFlags::UnorderedAccess;
        desc.m_ResourceAccess.m_bImmutable = false;
        nsGALBufferHandle hBuffer = s_pDevice->CreateBuffer(desc);
        s_pDevice->GetBuffer(hBuffer)->SetDebugName("FallbackTexelBufferRWVulkan");
        m_Buffers.PushBack(hBuffer);
        nsGALBufferResourceViewHandle hView = s_pDevice->GetDefaultResourceView(hBuffer);
        m_BufferResourceViews[nsGALShaderResourceType::TexelBufferRW] = hView;
      }
      {
        nsGALBufferCreationDescription desc;
        desc.m_BufferFlags = nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ByteAddressBuffer | nsGALBufferUsageFlags::ShaderResource | nsGALBufferUsageFlags::UnorderedAccess;
        desc.m_uiStructSize = 128;
        desc.m_uiTotalSize = 1280;
        desc.m_ResourceAccess.m_bImmutable = false;
        nsGALBufferHandle hBuffer = s_pDevice->CreateBuffer(desc);
        s_pDevice->GetBuffer(hBuffer)->SetDebugName("FallbackStructuredBufferRWVulkan");
        m_Buffers.PushBack(hBuffer);
        nsGALBufferResourceViewHandle hView = s_pDevice->GetDefaultResourceView(hBuffer);
        m_BufferResourceViews[nsGALShaderResourceType::StructuredBufferRW] = hView;
      }
    }
    break;
    case nsGALDeviceEvent::BeforeShutdown:
    {
      m_TextureResourceViews.Clear();
      m_TextureResourceViews.Compact();
      m_BufferResourceViews.Clear();
      m_BufferResourceViews.Compact();

      m_TextureUAVs.Clear();
      m_TextureUAVs.Compact();
      m_BufferUAVs.Clear();
      m_BufferUAVs.Compact();

      for (nsGALBufferHandle hBuffer : m_Buffers)
      {
        s_pDevice->DestroyBuffer(hBuffer);
      }
      m_Buffers.Clear();
      m_Buffers.Compact();

      for (nsGALTextureHandle hTexture : m_Textures)
      {
        s_pDevice->DestroyTexture(hTexture);
      }
      m_Textures.Clear();
      m_Textures.Compact();
      s_pDevice = nullptr;
    }
    break;
    default:
      break;
  }
}

const nsGALTextureResourceViewVulkan* nsFallbackResourcesVulkan::GetFallbackTextureResourceView(nsGALShaderResourceType::Enum descriptorType, nsGALShaderTextureType::Enum textureType, bool bDepth)
{
  if (nsGALTextureResourceViewHandle* pView = m_TextureResourceViews.GetValue(Key{descriptorType, textureType, bDepth}))
  {
    return static_cast<const nsGALTextureResourceViewVulkan*>(s_pDevice->GetResourceView(*pView));
  }
  NS_REPORT_FAILURE("No fallback resource set, update nsFallbackResourcesVulkan::GALDeviceEventHandler.");
  return nullptr;
}

const nsGALBufferResourceViewVulkan* nsFallbackResourcesVulkan::GetFallbackBufferResourceView(nsGALShaderResourceType::Enum descriptorType)
{
  if (nsGALBufferResourceViewHandle* pView = m_BufferResourceViews.GetValue(descriptorType))
  {
    return static_cast<const nsGALBufferResourceViewVulkan*>(s_pDevice->GetResourceView(*pView));
  }
  NS_REPORT_FAILURE("No fallback resource set, update nsFallbackResourcesVulkan::GALDeviceEventHandler.");
  return nullptr;
}

const nsGALTextureUnorderedAccessViewVulkan* nsFallbackResourcesVulkan::GetFallbackTextureUnorderedAccessView(nsGALShaderResourceType::Enum descriptorType, nsGALShaderTextureType::Enum textureType)
{
  if (nsGALTextureUnorderedAccessViewHandle* pView = m_TextureUAVs.GetValue(Key{descriptorType, textureType, false}))
  {
    return static_cast<const nsGALTextureUnorderedAccessViewVulkan*>(s_pDevice->GetUnorderedAccessView(*pView));
  }
  NS_REPORT_FAILURE("No fallback resource set, update nsFallbackResourcesVulkan::GALDeviceEventHandler.");
  return nullptr;
}

const nsGALBufferUnorderedAccessViewVulkan* nsFallbackResourcesVulkan::GetFallbackBufferUnorderedAccessView(nsGALShaderResourceType::Enum descriptorType)
{
  if (nsGALBufferUnorderedAccessViewHandle* pView = m_BufferUAVs.GetValue(descriptorType))
  {
    return static_cast<const nsGALBufferUnorderedAccessViewVulkan*>(s_pDevice->GetUnorderedAccessView(*pView));
  }
  NS_REPORT_FAILURE("No fallback resource set, update nsFallbackResourcesVulkan::GALDeviceEventHandler.");
  return nullptr;
}

nsUInt32 nsFallbackResourcesVulkan::KeyHash::Hash(const Key& a)
{
  nsHashStreamWriter32 writer;
  writer << a.m_ResourceType.GetValue();
  writer << a.m_nsType.GetValue();
  writer << a.m_bDepth;
  return writer.GetHashValue();
}

bool nsFallbackResourcesVulkan::KeyHash::Equal(const Key& a, const Key& b)
{
  return a.m_ResourceType == b.m_ResourceType && a.m_nsType == b.m_nsType && a.m_bDepth == b.m_bDepth;
}

nsUInt32 nsFallbackResourcesVulkan::KeyHash::Hash(const nsEnum<nsGALShaderResourceType>& a)
{
  nsHashStreamWriter32 writer;
  writer << a.GetValue();
  return writer.GetHashValue();
}

bool nsFallbackResourcesVulkan::KeyHash::Equal(const nsEnum<nsGALShaderResourceType>& a, const nsEnum<nsGALShaderResourceType>& b)
{
  return a == b;
}


NS_STATICLINK_FILE(RendererVulkan, RendererVulkan_Resources_Implementation_FallbackResourcesVulkan);
