#include <RendererFoundation/RendererFoundationPCH.h>

#include <Foundation/Logging/Log.h>
#include <Foundation/Profiling/Profiling.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/SharedTextureSwapChain.h>
#include <RendererFoundation/Device/SwapChain.h>
#include <RendererFoundation/Resources/Buffer.h>
#include <RendererFoundation/Resources/ProxyTexture.h>
#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/ResourceView.h>
#include <RendererFoundation/Resources/UnorderedAccesView.h>
#include <RendererFoundation/Shader/VertexDeclaration.h>
#include <RendererFoundation/State/State.h>

namespace
{
  struct GALObjectType
  {
    enum Enum
    {
      BlendState,
      DepthStencilState,
      RasterizerState,
      SamplerState,
      Shader,
      Buffer,
      Texture,
      TextureResourceView,
      BufferResourceView,
      RenderTargetView,
      TextureUnorderedAccessView,
      BufferUnorderedAccessView,
      SwapChain,
      Query,
      VertexDeclaration
    };
  };

  NS_CHECK_AT_COMPILETIME(sizeof(nsGALBlendStateHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALDepthStencilStateHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALRasterizerStateHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALSamplerStateHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALShaderHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALBufferHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALTextureHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALTextureResourceViewHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALBufferResourceViewHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALRenderTargetViewHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALTextureUnorderedAccessViewHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALBufferUnorderedAccessViewHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALSwapChainHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALQueryHandle) == sizeof(nsUInt32));
  NS_CHECK_AT_COMPILETIME(sizeof(nsGALVertexDeclarationHandle) == sizeof(nsUInt32));
} // namespace

nsGALDevice* nsGALDevice::s_pDefaultDevice = nullptr;
nsEvent<const nsGALDeviceEvent&> nsGALDevice::s_Events;

nsGALDevice::nsGALDevice(const nsGALDeviceCreationDescription& desc)
  : m_Allocator("GALDevice", nsFoundation::GetDefaultAllocator())
  , m_AllocatorWrapper(&m_Allocator)
  , m_Description(desc)
{
}

nsGALDevice::~nsGALDevice()
{
  // Check for object leaks
  {
    NS_LOG_BLOCK("nsGALDevice object leak report");

    if (!m_Shaders.IsEmpty())
      nsLog::Warning("{0} shaders have not been cleaned up", m_Shaders.GetCount());

    if (!m_BlendStates.IsEmpty())
      nsLog::Warning("{0} blend states have not been cleaned up", m_BlendStates.GetCount());

    if (!m_DepthStencilStates.IsEmpty())
      nsLog::Warning("{0} depth stencil states have not been cleaned up", m_DepthStencilStates.GetCount());

    if (!m_RasterizerStates.IsEmpty())
      nsLog::Warning("{0} rasterizer states have not been cleaned up", m_RasterizerStates.GetCount());

    if (!m_Buffers.IsEmpty())
      nsLog::Warning("{0} buffers have not been cleaned up", m_Buffers.GetCount());

    if (!m_Textures.IsEmpty())
      nsLog::Warning("{0} textures have not been cleaned up", m_Textures.GetCount());

    if (!m_TextureResourceViews.IsEmpty())
      nsLog::Warning("{0} texture resource views have not been cleaned up", m_TextureResourceViews.GetCount());

    if (!m_BufferResourceViews.IsEmpty())
      nsLog::Warning("{0} buffer resource views have not been cleaned up", m_BufferResourceViews.GetCount());

    if (!m_RenderTargetViews.IsEmpty())
      nsLog::Warning("{0} render target views have not been cleaned up", m_RenderTargetViews.GetCount());

    if (!m_TextureUnorderedAccessViews.IsEmpty())
      nsLog::Warning("{0} texture unordered access views have not been cleaned up", m_TextureUnorderedAccessViews.GetCount());

    if (!m_BufferUnorderedAccessViews.IsEmpty())
      nsLog::Warning("{0} buffer unordered access views have not been cleaned up", m_BufferUnorderedAccessViews.GetCount());

    if (!m_SwapChains.IsEmpty())
      nsLog::Warning("{0} swap chains have not been cleaned up", m_SwapChains.GetCount());

    if (!m_Queries.IsEmpty())
      nsLog::Warning("{0} queries have not been cleaned up", m_Queries.GetCount());

    if (!m_VertexDeclarations.IsEmpty())
      nsLog::Warning("{0} vertex declarations have not been cleaned up", m_VertexDeclarations.GetCount());
  }
}

nsResult nsGALDevice::Init()
{
  NS_LOG_BLOCK("nsGALDevice::Init");

  nsResult PlatformInitResult = InitPlatform();

  if (PlatformInitResult == NS_FAILURE)
  {
    return NS_FAILURE;
  }

  nsGALSharedTextureSwapChain::SetFactoryMethod([this](const nsGALSharedTextureSwapChainCreationDescription& desc) -> nsGALSwapChainHandle
    { return CreateSwapChain([&desc](nsAllocator* pAllocator) -> nsGALSwapChain*
        { return NS_NEW(pAllocator, nsGALSharedTextureSwapChain, desc); }); });

  // Fill the capabilities
  FillCapabilitiesPlatform();

  nsLog::Info("Adapter: '{}' - {} VRAM, {} Sys RAM, {} Shared RAM", m_Capabilities.m_sAdapterName, nsArgFileSize(m_Capabilities.m_uiDedicatedVRAM),
    nsArgFileSize(m_Capabilities.m_uiDedicatedSystemRAM), nsArgFileSize(m_Capabilities.m_uiSharedSystemRAM));

  if (!m_Capabilities.m_bHardwareAccelerated)
  {
    nsLog::Warning("Selected graphics adapter has no hardware acceleration.");
  }

  NS_GALDEVICE_LOCK_AND_CHECK();

  nsProfilingSystem::InitializeGPUData();



  {
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::AfterInit;
    s_Events.Broadcast(e);
  }

  return NS_SUCCESS;
}

nsResult nsGALDevice::Shutdown()
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  NS_LOG_BLOCK("nsGALDevice::Shutdown");

  {
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::BeforeShutdown;
    s_Events.Broadcast(e);
  }

  DestroyDeadObjects();

  // make sure we are not listed as the default device anymore
  if (nsGALDevice::HasDefaultDevice() && nsGALDevice::GetDefaultDevice() == this)
  {
    nsGALDevice::SetDefaultDevice(nullptr);
  }

  return ShutdownPlatform();
}

nsStringView nsGALDevice::GetRenderer()
{
  return GetRendererPlatform();
}

void nsGALDevice::BeginPipeline(const char* szName, nsGALSwapChainHandle hSwapChain)
{
  {
    NS_PROFILE_SCOPE("BeforeBeginPipeline");
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::BeforeBeginPipeline;
    e.m_hSwapChain = hSwapChain;
    s_Events.Broadcast(e, 1);
  }

  {
    NS_GALDEVICE_LOCK_AND_CHECK();

    NS_ASSERT_DEV(!m_bBeginPipelineCalled, "Nested Pipelines are not allowed: You must call nsGALDevice::EndPipeline before you can call nsGALDevice::BeginPipeline again");
    m_bBeginPipelineCalled = true;

    nsGALSwapChain* pSwapChain = nullptr;
    m_SwapChains.TryGetValue(hSwapChain, pSwapChain);
    BeginPipelinePlatform(szName, pSwapChain);
  }

  {
    NS_PROFILE_SCOPE("AfterBeginPipeline");
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::AfterBeginPipeline;
    e.m_hSwapChain = hSwapChain;
    s_Events.Broadcast(e, 1);
  }
}

void nsGALDevice::EndPipeline(nsGALSwapChainHandle hSwapChain)
{
  {
    NS_PROFILE_SCOPE("BeforeBeginPipeline");
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::BeforeEndPipeline;
    e.m_hSwapChain = hSwapChain;
    s_Events.Broadcast(e, 1);
  }

  {
    NS_GALDEVICE_LOCK_AND_CHECK();

    NS_ASSERT_DEV(m_bBeginPipelineCalled, "You must have called nsGALDevice::BeginPipeline before you can call nsGALDevice::EndPipeline");
    m_bBeginPipelineCalled = false;

    nsGALSwapChain* pSwapChain = nullptr;
    m_SwapChains.TryGetValue(hSwapChain, pSwapChain);
    EndPipelinePlatform(pSwapChain);
  }

  {
    NS_PROFILE_SCOPE("AfterBeginPipeline");
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::AfterEndPipeline;
    e.m_hSwapChain = hSwapChain;
    s_Events.Broadcast(e, 1);
  }
}

nsGALPass* nsGALDevice::BeginPass(const char* szName)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  NS_ASSERT_DEV(!m_bBeginPassCalled, "Nested Passes are not allowed: You must call nsGALDevice::EndPass before you can call nsGALDevice::BeginPass again");
  m_bBeginPassCalled = true;

  return BeginPassPlatform(szName);
}

void nsGALDevice::EndPass(nsGALPass* pPass)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  NS_ASSERT_DEV(m_bBeginPassCalled, "You must have called nsGALDevice::BeginPass before you can call nsGALDevice::EndPass");
  m_bBeginPassCalled = false;

  EndPassPlatform(pPass);
}

nsGALBlendStateHandle nsGALDevice::CreateBlendState(const nsGALBlendStateCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  // Hash desc and return potential existing one (including inc. refcount)
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALBlendStateHandle hBlendState;
    if (m_BlendStateTable.TryGetValue(uiHash, hBlendState))
    {
      nsGALBlendState* pBlendState = m_BlendStates[hBlendState];
      if (pBlendState->GetRefCount() == 0)
      {
        ReviveDeadObject(GALObjectType::BlendState, hBlendState);
      }

      pBlendState->AddRef();
      return hBlendState;
    }
  }

  nsGALBlendState* pBlendState = CreateBlendStatePlatform(desc);

  if (pBlendState != nullptr)
  {
    NS_ASSERT_DEBUG(pBlendState->GetDescription().CalculateHash() == uiHash, "BlendState hash doesn't match");

    pBlendState->AddRef();

    nsGALBlendStateHandle hBlendState(m_BlendStates.Insert(pBlendState));
    m_BlendStateTable.Insert(uiHash, hBlendState);

    return hBlendState;
  }

  return nsGALBlendStateHandle();
}

void nsGALDevice::DestroyBlendState(nsGALBlendStateHandle hBlendState)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALBlendState* pBlendState = nullptr;

  if (m_BlendStates.TryGetValue(hBlendState, pBlendState))
  {
    pBlendState->ReleaseRef();

    if (pBlendState->GetRefCount() == 0)
    {
      AddDeadObject(GALObjectType::BlendState, hBlendState);
    }
  }
  else
  {
    nsLog::Warning("DestroyBlendState called on invalid handle (double free?)");
  }
}

nsGALDepthStencilStateHandle nsGALDevice::CreateDepthStencilState(const nsGALDepthStencilStateCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  // Hash desc and return potential existing one (including inc. refcount)
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALDepthStencilStateHandle hDepthStencilState;
    if (m_DepthStencilStateTable.TryGetValue(uiHash, hDepthStencilState))
    {
      nsGALDepthStencilState* pDepthStencilState = m_DepthStencilStates[hDepthStencilState];
      if (pDepthStencilState->GetRefCount() == 0)
      {
        ReviveDeadObject(GALObjectType::DepthStencilState, hDepthStencilState);
      }

      pDepthStencilState->AddRef();
      return hDepthStencilState;
    }
  }

  nsGALDepthStencilState* pDepthStencilState = CreateDepthStencilStatePlatform(desc);

  if (pDepthStencilState != nullptr)
  {
    NS_ASSERT_DEBUG(pDepthStencilState->GetDescription().CalculateHash() == uiHash, "DepthStencilState hash doesn't match");

    pDepthStencilState->AddRef();

    nsGALDepthStencilStateHandle hDepthStencilState(m_DepthStencilStates.Insert(pDepthStencilState));
    m_DepthStencilStateTable.Insert(uiHash, hDepthStencilState);

    return hDepthStencilState;
  }

  return nsGALDepthStencilStateHandle();
}

void nsGALDevice::DestroyDepthStencilState(nsGALDepthStencilStateHandle hDepthStencilState)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALDepthStencilState* pDepthStencilState = nullptr;

  if (m_DepthStencilStates.TryGetValue(hDepthStencilState, pDepthStencilState))
  {
    pDepthStencilState->ReleaseRef();

    if (pDepthStencilState->GetRefCount() == 0)
    {
      AddDeadObject(GALObjectType::DepthStencilState, hDepthStencilState);
    }
  }
  else
  {
    nsLog::Warning("DestroyDepthStencilState called on invalid handle (double free?)");
  }
}

nsGALRasterizerStateHandle nsGALDevice::CreateRasterizerState(const nsGALRasterizerStateCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  // Hash desc and return potential existing one (including inc. refcount)
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALRasterizerStateHandle hRasterizerState;
    if (m_RasterizerStateTable.TryGetValue(uiHash, hRasterizerState))
    {
      nsGALRasterizerState* pRasterizerState = m_RasterizerStates[hRasterizerState];
      if (pRasterizerState->GetRefCount() == 0)
      {
        ReviveDeadObject(GALObjectType::RasterizerState, hRasterizerState);
      }

      pRasterizerState->AddRef();
      return hRasterizerState;
    }
  }

  nsGALRasterizerState* pRasterizerState = CreateRasterizerStatePlatform(desc);

  if (pRasterizerState != nullptr)
  {
    NS_ASSERT_DEBUG(pRasterizerState->GetDescription().CalculateHash() == uiHash, "RasterizerState hash doesn't match");

    pRasterizerState->AddRef();

    nsGALRasterizerStateHandle hRasterizerState(m_RasterizerStates.Insert(pRasterizerState));
    m_RasterizerStateTable.Insert(uiHash, hRasterizerState);

    return hRasterizerState;
  }

  return nsGALRasterizerStateHandle();
}

void nsGALDevice::DestroyRasterizerState(nsGALRasterizerStateHandle hRasterizerState)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALRasterizerState* pRasterizerState = nullptr;

  if (m_RasterizerStates.TryGetValue(hRasterizerState, pRasterizerState))
  {
    pRasterizerState->ReleaseRef();

    if (pRasterizerState->GetRefCount() == 0)
    {
      AddDeadObject(GALObjectType::RasterizerState, hRasterizerState);
    }
  }
  else
  {
    nsLog::Warning("DestroyRasterizerState called on invalid handle (double free?)");
  }
}

nsGALSamplerStateHandle nsGALDevice::CreateSamplerState(const nsGALSamplerStateCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  /// \todo Platform independent validation

  // Hash desc and return potential existing one (including inc. refcount)
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALSamplerStateHandle hSamplerState;
    if (m_SamplerStateTable.TryGetValue(uiHash, hSamplerState))
    {
      nsGALSamplerState* pSamplerState = m_SamplerStates[hSamplerState];
      if (pSamplerState->GetRefCount() == 0)
      {
        ReviveDeadObject(GALObjectType::SamplerState, hSamplerState);
      }

      pSamplerState->AddRef();
      return hSamplerState;
    }
  }

  nsGALSamplerState* pSamplerState = CreateSamplerStatePlatform(desc);

  if (pSamplerState != nullptr)
  {
    NS_ASSERT_DEBUG(pSamplerState->GetDescription().CalculateHash() == uiHash, "SamplerState hash doesn't match");

    pSamplerState->AddRef();

    nsGALSamplerStateHandle hSamplerState(m_SamplerStates.Insert(pSamplerState));
    m_SamplerStateTable.Insert(uiHash, hSamplerState);

    return hSamplerState;
  }

  return nsGALSamplerStateHandle();
}

void nsGALDevice::DestroySamplerState(nsGALSamplerStateHandle hSamplerState)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALSamplerState* pSamplerState = nullptr;

  if (m_SamplerStates.TryGetValue(hSamplerState, pSamplerState))
  {
    pSamplerState->ReleaseRef();

    if (pSamplerState->GetRefCount() == 0)
    {
      AddDeadObject(GALObjectType::SamplerState, hSamplerState);
    }
  }
  else
  {
    nsLog::Warning("DestroySamplerState called on invalid handle (double free?)");
  }
}



nsGALShaderHandle nsGALDevice::CreateShader(const nsGALShaderCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  bool bHasByteCodes = false;

  for (nsUInt32 uiStage = 0; uiStage < nsGALShaderStage::ENUM_COUNT; uiStage++)
  {
    if (desc.HasByteCodeForStage((nsGALShaderStage::Enum)uiStage))
    {
      bHasByteCodes = true;
      break;
    }
  }

  if (!bHasByteCodes)
  {
    nsLog::Error("Can't create a shader which supplies no bytecodes at all!");
    return nsGALShaderHandle();
  }

  nsGALShader* pShader = CreateShaderPlatform(desc);

  if (pShader == nullptr)
  {
    return nsGALShaderHandle();
  }
  else
  {
    return nsGALShaderHandle(m_Shaders.Insert(pShader));
  }
}

void nsGALDevice::DestroyShader(nsGALShaderHandle hShader)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALShader* pShader = nullptr;

  if (m_Shaders.TryGetValue(hShader, pShader))
  {
    AddDeadObject(GALObjectType::Shader, hShader);
  }
  else
  {
    nsLog::Warning("DestroyShader called on invalid handle (double free?)");
  }
}


nsGALBufferHandle nsGALDevice::CreateBuffer(const nsGALBufferCreationDescription& desc, nsArrayPtr<const nsUInt8> initialData)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  if (desc.m_uiTotalSize == 0)
  {
    nsLog::Error("Trying to create a buffer with size of 0 is not possible!");
    return nsGALBufferHandle();
  }

  if (desc.m_ResourceAccess.IsImmutable())
  {
    if (initialData.IsEmpty())
    {
      nsLog::Error("Trying to create an immutable buffer but not supplying initial data is not possible!");
      return nsGALBufferHandle();
    }

    nsUInt32 uiBufferSize = desc.m_uiTotalSize;
    if (uiBufferSize != initialData.GetCount())
    {
      nsLog::Error("Trying to create a buffer with invalid initial data!");
      return nsGALBufferHandle();
    }
  }

  /// \todo Platform independent validation (buffer type supported)

  nsGALBuffer* pBuffer = CreateBufferPlatform(desc, initialData);

  return FinalizeBufferInternal(desc, pBuffer);
}

nsGALBufferHandle nsGALDevice::FinalizeBufferInternal(const nsGALBufferCreationDescription& desc, nsGALBuffer* pBuffer)
{
  if (pBuffer != nullptr)
  {
    nsGALBufferHandle hBuffer(m_Buffers.Insert(pBuffer));

    // Create default resource view
    if (desc.m_BufferFlags.IsSet(nsGALBufferUsageFlags::ShaderResource))
    {
      // #TODO_VULKAN TexelBuffer requires a format, should we store it in the buffer desc?
      if (desc.m_BufferFlags.IsAnySet(nsGALBufferUsageFlags::StructuredBuffer | nsGALBufferUsageFlags::ByteAddressBuffer))
      {
        nsGALBufferResourceViewCreationDescription viewDesc;
        viewDesc.m_hBuffer = hBuffer;
        viewDesc.m_uiFirstElement = 0;
        viewDesc.m_uiNumElements = (desc.m_uiStructSize != 0) ? (desc.m_uiTotalSize / desc.m_uiStructSize) : desc.m_uiTotalSize;

        pBuffer->m_hDefaultResourceView = CreateResourceView(viewDesc);
      }
    }
    return hBuffer;
  }

  return nsGALBufferHandle();
}

void nsGALDevice::DestroyBuffer(nsGALBufferHandle hBuffer)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALBuffer* pBuffer = nullptr;

  if (m_Buffers.TryGetValue(hBuffer, pBuffer))
  {
    AddDeadObject(GALObjectType::Buffer, hBuffer);
  }
  else
  {
    nsLog::Warning("DestroyBuffer called on invalid handle (double free?)");
  }
}

// Helper functions for buffers (for common, simple use cases)
nsGALBufferHandle nsGALDevice::CreateVertexBuffer(nsUInt32 uiVertexSize, nsUInt32 uiVertexCount, nsArrayPtr<const nsUInt8> initialData, bool bDataIsMutable /*= false */)
{
  nsGALBufferCreationDescription desc;
  desc.m_uiStructSize = uiVertexSize;
  desc.m_uiTotalSize = uiVertexSize * nsMath::Max(1u, uiVertexCount);
  desc.m_BufferFlags = nsGALBufferUsageFlags::VertexBuffer;
  desc.m_ResourceAccess.m_bImmutable = !initialData.IsEmpty() && !bDataIsMutable;

  return CreateBuffer(desc, initialData);
}

nsGALBufferHandle nsGALDevice::CreateIndexBuffer(nsGALIndexType::Enum indexType, nsUInt32 uiIndexCount, nsArrayPtr<const nsUInt8> initialData, bool bDataIsMutable /*= false*/)
{
  nsGALBufferCreationDescription desc;
  desc.m_uiStructSize = nsGALIndexType::GetSize(indexType);
  desc.m_uiTotalSize = desc.m_uiStructSize * nsMath::Max(1u, uiIndexCount);
  desc.m_BufferFlags = nsGALBufferUsageFlags::IndexBuffer;
  desc.m_ResourceAccess.m_bImmutable = !bDataIsMutable && !initialData.IsEmpty();

  return CreateBuffer(desc, initialData);
}

nsGALBufferHandle nsGALDevice::CreateConstantBuffer(nsUInt32 uiBufferSize)
{
  nsGALBufferCreationDescription desc;
  desc.m_uiStructSize = 0;
  desc.m_uiTotalSize = uiBufferSize;
  desc.m_BufferFlags = nsGALBufferUsageFlags::ConstantBuffer;
  desc.m_ResourceAccess.m_bImmutable = false;

  return CreateBuffer(desc);
}


nsGALTextureHandle nsGALDevice::CreateTexture(const nsGALTextureCreationDescription& desc, nsArrayPtr<nsGALSystemMemoryDescription> initialData)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  /// \todo Platform independent validation (desc width & height < platform maximum, format, etc.)

  if (desc.m_ResourceAccess.IsImmutable() && (initialData.IsEmpty() || initialData.GetCount() < desc.m_uiMipLevelCount) &&
      !desc.m_bCreateRenderTarget)
  {
    nsLog::Error("Trying to create an immutable texture but not supplying initial data (or not enough data pointers) is not possible!");
    return nsGALTextureHandle();
  }

  if (desc.m_uiWidth == 0 || desc.m_uiHeight == 0)
  {
    nsLog::Error("Trying to create a texture with width or height == 0 is not possible!");
    return nsGALTextureHandle();
  }

  nsGALTexture* pTexture = CreateTexturePlatform(desc, initialData);

  return FinalizeTextureInternal(desc, pTexture);
}

nsGALTextureHandle nsGALDevice::FinalizeTextureInternal(const nsGALTextureCreationDescription& desc, nsGALTexture* pTexture)
{
  if (pTexture != nullptr)
  {
    nsGALTextureHandle hTexture(m_Textures.Insert(pTexture));

    // Create default resource view
    if (desc.m_bAllowShaderResourceView)
    {
      nsGALTextureResourceViewCreationDescription viewDesc;
      viewDesc.m_hTexture = hTexture;
      viewDesc.m_uiArraySize = desc.m_uiArraySize;
      pTexture->m_hDefaultResourceView = CreateResourceView(viewDesc);
    }

    // Create default render target view
    if (desc.m_bCreateRenderTarget)
    {
      nsGALRenderTargetViewCreationDescription rtDesc;
      rtDesc.m_hTexture = hTexture;
      rtDesc.m_uiFirstSlice = 0;
      rtDesc.m_uiSliceCount = desc.m_uiArraySize;

      pTexture->m_hDefaultRenderTargetView = CreateRenderTargetView(rtDesc);
    }

    return hTexture;
  }

  return nsGALTextureHandle();
}

void nsGALDevice::DestroyTexture(nsGALTextureHandle hTexture)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTexture* pTexture = nullptr;
  if (m_Textures.TryGetValue(hTexture, pTexture))
  {
    AddDeadObject(GALObjectType::Texture, hTexture);
  }
  else
  {
    nsLog::Warning("DestroyTexture called on invalid handle (double free?)");
  }
}

nsGALTextureHandle nsGALDevice::CreateProxyTexture(nsGALTextureHandle hParentTexture, nsUInt32 uiSlice)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTexture* pParentTexture = nullptr;

  if (!hParentTexture.IsInvalidated())
  {
    pParentTexture = Get<TextureTable, nsGALTexture>(hParentTexture, m_Textures);
  }

  if (pParentTexture == nullptr)
  {
    nsLog::Error("No valid texture handle given for proxy texture creation!");
    return nsGALTextureHandle();
  }

  const auto& parentDesc = pParentTexture->GetDescription();
  NS_IGNORE_UNUSED(parentDesc);
  NS_ASSERT_DEV(parentDesc.m_Type != nsGALTextureType::Texture2DProxy, "Can't create a proxy texture of a proxy texture.");
  NS_ASSERT_DEV(parentDesc.m_Type == nsGALTextureType::TextureCube || parentDesc.m_uiArraySize > 1,
    "Proxy textures can only be created for cubemaps or array textures.");

  nsGALProxyTexture* pProxyTexture = NS_NEW(&m_Allocator, nsGALProxyTexture, *pParentTexture);
  nsGALTextureHandle hProxyTexture(m_Textures.Insert(pProxyTexture));

  const auto& desc = pProxyTexture->GetDescription();

  // Create default resource view
  if (desc.m_bAllowShaderResourceView)
  {
    nsGALTextureResourceViewCreationDescription viewDesc;
    viewDesc.m_hTexture = hProxyTexture;
    viewDesc.m_uiFirstArraySlice = uiSlice;
    viewDesc.m_uiArraySize = 1;

    pProxyTexture->m_hDefaultResourceView = CreateResourceView(viewDesc);
  }

  // Create default render target view
  if (desc.m_bCreateRenderTarget)
  {
    nsGALRenderTargetViewCreationDescription rtDesc;
    rtDesc.m_hTexture = hProxyTexture;
    rtDesc.m_uiFirstSlice = uiSlice;
    rtDesc.m_uiSliceCount = 1;

    pProxyTexture->m_hDefaultRenderTargetView = CreateRenderTargetView(rtDesc);
  }

  return hProxyTexture;
}

void nsGALDevice::DestroyProxyTexture(nsGALTextureHandle hProxyTexture)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTexture* pTexture = nullptr;
  if (m_Textures.TryGetValue(hProxyTexture, pTexture))
  {
    NS_ASSERT_DEV(pTexture->GetDescription().m_Type == nsGALTextureType::Texture2DProxy, "Given texture is not a proxy texture");

    AddDeadObject(GALObjectType::Texture, hProxyTexture);
  }
  else
  {
    nsLog::Warning("DestroyProxyTexture called on invalid handle (double free?)");
  }
}

nsGALTextureHandle nsGALDevice::CreateSharedTexture(const nsGALTextureCreationDescription& desc, nsArrayPtr<nsGALSystemMemoryDescription> initialData)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  /// \todo Platform independent validation (desc width & height < platform maximum, format, etc.)

  if (desc.m_ResourceAccess.IsImmutable() && (initialData.IsEmpty() || initialData.GetCount() < desc.m_uiMipLevelCount) &&
      !desc.m_bCreateRenderTarget)
  {
    nsLog::Error("Trying to create an immutable texture but not supplying initial data (or not enough data pointers) is not possible!");
    return nsGALTextureHandle();
  }

  if (desc.m_uiWidth == 0 || desc.m_uiHeight == 0)
  {
    nsLog::Error("Trying to create a texture with width or height == 0 is not possible!");
    return nsGALTextureHandle();
  }

  if (desc.m_pExisitingNativeObject != nullptr)
  {
    nsLog::Error("Shared textures cannot be created on exiting native objects!");
    return nsGALTextureHandle();
  }

  if (desc.m_Type != nsGALTextureType::Texture2DShared)
  {
    nsLog::Error("Only nsGALTextureType::Texture2DShared is supported for shared textures!");
    return nsGALTextureHandle();
  }

  nsGALTexture* pTexture = CreateSharedTexturePlatform(desc, initialData, nsGALSharedTextureType::Exported, {});

  return FinalizeTextureInternal(desc, pTexture);
}

nsGALTextureHandle nsGALDevice::OpenSharedTexture(const nsGALTextureCreationDescription& desc, nsGALPlatformSharedHandle hSharedHandle)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  if (desc.m_pExisitingNativeObject != nullptr)
  {
    nsLog::Error("Shared textures cannot be created on exiting native objects!");
    return nsGALTextureHandle();
  }

  if (desc.m_Type != nsGALTextureType::Texture2DShared)
  {
    nsLog::Error("Only nsGALTextureType::Texture2DShared is supported for shared textures!");
    return nsGALTextureHandle();
  }

  if (desc.m_uiWidth == 0 || desc.m_uiHeight == 0)
  {
    nsLog::Error("Trying to create a texture with width or height == 0 is not possible!");
    return nsGALTextureHandle();
  }

  nsGALTexture* pTexture = CreateSharedTexturePlatform(desc, {}, nsGALSharedTextureType::Imported, hSharedHandle);

  return FinalizeTextureInternal(desc, pTexture);
}

void nsGALDevice::DestroySharedTexture(nsGALTextureHandle hSharedTexture)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTexture* pTexture = nullptr;
  if (m_Textures.TryGetValue(hSharedTexture, pTexture))
  {
    NS_ASSERT_DEV(pTexture->GetDescription().m_Type == nsGALTextureType::Texture2DShared, "Given texture is not a shared texture texture");

    AddDeadObject(GALObjectType::Texture, hSharedTexture);
  }
  else
  {
    nsLog::Warning("DestroySharedTexture called on invalid handle (double free?)");
  }
}

nsGALTextureResourceViewHandle nsGALDevice::GetDefaultResourceView(nsGALTextureHandle hTexture)
{
  if (const nsGALTexture* pTexture = GetTexture(hTexture))
  {
    return pTexture->m_hDefaultResourceView;
  }

  return nsGALTextureResourceViewHandle();
}

nsGALBufferResourceViewHandle nsGALDevice::GetDefaultResourceView(nsGALBufferHandle hBuffer)
{
  if (const nsGALBuffer* pBuffer = GetBuffer(hBuffer))
  {
    return pBuffer->m_hDefaultResourceView;
  }

  return nsGALBufferResourceViewHandle();
}

nsGALTextureResourceViewHandle nsGALDevice::CreateResourceView(const nsGALTextureResourceViewCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTexture* pResource = nullptr;

  if (!desc.m_hTexture.IsInvalidated())
    pResource = Get<TextureTable, nsGALTexture>(desc.m_hTexture, m_Textures);

  if (pResource == nullptr)
  {
    nsLog::Error("No valid texture handle given for resource view creation!");
    return nsGALTextureResourceViewHandle();
  }

  // Hash desc and return potential existing one
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALTextureResourceViewHandle hResourceView;
    if (pResource->m_ResourceViews.TryGetValue(uiHash, hResourceView))
    {
      return hResourceView;
    }
  }

  nsGALTextureResourceView* pResourceView = CreateResourceViewPlatform(pResource, desc);

  if (pResourceView != nullptr)
  {
    nsGALTextureResourceViewHandle hResourceView(m_TextureResourceViews.Insert(pResourceView));
    pResource->m_ResourceViews.Insert(uiHash, hResourceView);

    return hResourceView;
  }

  return nsGALTextureResourceViewHandle();
}

nsGALBufferResourceViewHandle nsGALDevice::CreateResourceView(const nsGALBufferResourceViewCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALBuffer* pResource = nullptr;

  if (!desc.m_hBuffer.IsInvalidated())
    pResource = Get<BufferTable, nsGALBuffer>(desc.m_hBuffer, m_Buffers);

  if (pResource == nullptr)
  {
    nsLog::Error("No valid texture handle or buffer handle given for resource view creation!");
    return nsGALBufferResourceViewHandle();
  }

  // Hash desc and return potential existing one
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALBufferResourceViewHandle hResourceView;
    if (pResource->m_ResourceViews.TryGetValue(uiHash, hResourceView))
    {
      return hResourceView;
    }
  }

  nsGALBufferResourceView* pResourceView = CreateResourceViewPlatform(pResource, desc);

  if (pResourceView != nullptr)
  {
    nsGALBufferResourceViewHandle hResourceView(m_BufferResourceViews.Insert(pResourceView));
    pResource->m_ResourceViews.Insert(uiHash, hResourceView);

    return hResourceView;
  }

  return nsGALBufferResourceViewHandle();
}

void nsGALDevice::DestroyResourceView(nsGALTextureResourceViewHandle hResourceView)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTextureResourceView* pResourceView = nullptr;

  if (m_TextureResourceViews.TryGetValue(hResourceView, pResourceView))
  {
    AddDeadObject(GALObjectType::TextureResourceView, hResourceView);
  }
  else
  {
    nsLog::Warning("DestroyResourceView called on invalid handle (double free?)");
  }
}

void nsGALDevice::DestroyResourceView(nsGALBufferResourceViewHandle hResourceView)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALBufferResourceView* pResourceView = nullptr;

  if (m_BufferResourceViews.TryGetValue(hResourceView, pResourceView))
  {
    AddDeadObject(GALObjectType::BufferResourceView, hResourceView);
  }
  else
  {
    nsLog::Warning("DestroyResourceView called on invalid handle (double free?)");
  }
}

nsGALRenderTargetViewHandle nsGALDevice::GetDefaultRenderTargetView(nsGALTextureHandle hTexture)
{
  if (const nsGALTexture* pTexture = GetTexture(hTexture))
  {
    return pTexture->m_hDefaultRenderTargetView;
  }

  return nsGALRenderTargetViewHandle();
}

nsGALRenderTargetViewHandle nsGALDevice::CreateRenderTargetView(const nsGALRenderTargetViewCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTexture* pTexture = nullptr;

  if (!desc.m_hTexture.IsInvalidated())
    pTexture = Get<TextureTable, nsGALTexture>(desc.m_hTexture, m_Textures);

  if (pTexture == nullptr)
  {
    nsLog::Error("No valid texture handle given for render target view creation!");
    return nsGALRenderTargetViewHandle();
  }

  /// \todo Platform independent validation

  // Hash desc and return potential existing one
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALRenderTargetViewHandle hRenderTargetView;
    if (pTexture->m_RenderTargetViews.TryGetValue(uiHash, hRenderTargetView))
    {
      return hRenderTargetView;
    }
  }

  nsGALRenderTargetView* pRenderTargetView = CreateRenderTargetViewPlatform(pTexture, desc);

  if (pRenderTargetView != nullptr)
  {
    nsGALRenderTargetViewHandle hRenderTargetView(m_RenderTargetViews.Insert(pRenderTargetView));
    pTexture->m_RenderTargetViews.Insert(uiHash, hRenderTargetView);

    return hRenderTargetView;
  }

  return nsGALRenderTargetViewHandle();
}

void nsGALDevice::DestroyRenderTargetView(nsGALRenderTargetViewHandle hRenderTargetView)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALRenderTargetView* pRenderTargetView = nullptr;

  if (m_RenderTargetViews.TryGetValue(hRenderTargetView, pRenderTargetView))
  {
    AddDeadObject(GALObjectType::RenderTargetView, hRenderTargetView);
  }
  else
  {
    nsLog::Warning("DestroyRenderTargetView called on invalid handle (double free?)");
  }
}

nsGALTextureUnorderedAccessViewHandle nsGALDevice::CreateUnorderedAccessView(const nsGALTextureUnorderedAccessViewCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTexture* pTexture = nullptr;

  if (!desc.m_hTexture.IsInvalidated())
  {
    pTexture = Get<TextureTable, nsGALTexture>(desc.m_hTexture, m_Textures);
  }

  if (pTexture == nullptr)
  {
    nsLog::Error("No valid texture handle given for unordered access view creation!");
    return nsGALTextureUnorderedAccessViewHandle();
  }

  // Some platform independent validation.
  {
    // Is this really platform independent?
    if (pTexture->GetDescription().m_SampleCount != nsGALMSAASampleCount::None)
    {
      nsLog::Error("Can't create unordered access view on textures with multisampling.");
      return nsGALTextureUnorderedAccessViewHandle();
    }
  }

  // Hash desc and return potential existing one
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView;
    if (pTexture->m_UnorderedAccessViews.TryGetValue(uiHash, hUnorderedAccessView))
    {
      return hUnorderedAccessView;
    }
  }

  nsGALTextureUnorderedAccessView* pUnorderedAccessView = CreateUnorderedAccessViewPlatform(pTexture, desc);

  if (pUnorderedAccessView != nullptr)
  {
    nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView(m_TextureUnorderedAccessViews.Insert(pUnorderedAccessView));
    pTexture->m_UnorderedAccessViews.Insert(uiHash, hUnorderedAccessView);

    return hUnorderedAccessView;
  }

  return nsGALTextureUnorderedAccessViewHandle();
}

nsGALBufferUnorderedAccessViewHandle nsGALDevice::CreateUnorderedAccessView(const nsGALBufferUnorderedAccessViewCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALBuffer* pBuffer = nullptr;

  if (!desc.m_hBuffer.IsInvalidated())
  {
    pBuffer = Get<BufferTable, nsGALBuffer>(desc.m_hBuffer, m_Buffers);
  }

  if (pBuffer == nullptr)
  {
    nsLog::Error("No valid buffer handle given for unordered access view creation!");
    return nsGALBufferUnorderedAccessViewHandle();
  }

  // Some platform independent validation.
  {
    if (desc.m_OverrideViewFormat == nsGALResourceFormat::Invalid)
    {
      nsLog::Error("Invalid resource format is not allowed for buffer unordered access views!");
      return nsGALBufferUnorderedAccessViewHandle();
    }

    if (!pBuffer->GetDescription().m_BufferFlags.IsSet(nsGALBufferUsageFlags::ByteAddressBuffer) && desc.m_bRawView)
    {
      nsLog::Error("Trying to create a raw view for a buffer with no raw view flag is invalid!");
      return nsGALBufferUnorderedAccessViewHandle();
    }
  }

  // Hash desc and return potential existing one
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView;
    if (pBuffer->m_UnorderedAccessViews.TryGetValue(uiHash, hUnorderedAccessView))
    {
      return hUnorderedAccessView;
    }
  }

  nsGALBufferUnorderedAccessView* pUnorderedAccessViewView = CreateUnorderedAccessViewPlatform(pBuffer, desc);

  if (pUnorderedAccessViewView != nullptr)
  {
    nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView(m_BufferUnorderedAccessViews.Insert(pUnorderedAccessViewView));
    pBuffer->m_UnorderedAccessViews.Insert(uiHash, hUnorderedAccessView);

    return hUnorderedAccessView;
  }

  return nsGALBufferUnorderedAccessViewHandle();
}

void nsGALDevice::DestroyUnorderedAccessView(nsGALTextureUnorderedAccessViewHandle hUnorderedAccessViewHandle)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALTextureUnorderedAccessView* pUnorderedAccesssView = nullptr;

  if (m_TextureUnorderedAccessViews.TryGetValue(hUnorderedAccessViewHandle, pUnorderedAccesssView))
  {
    AddDeadObject(GALObjectType::TextureUnorderedAccessView, hUnorderedAccessViewHandle);
  }
  else
  {
    nsLog::Warning("DestroyUnorderedAccessView called on invalid handle (double free?)");
  }
}

void nsGALDevice::DestroyUnorderedAccessView(nsGALBufferUnorderedAccessViewHandle hUnorderedAccessViewHandle)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALBufferUnorderedAccessView* pUnorderedAccesssView = nullptr;

  if (m_BufferUnorderedAccessViews.TryGetValue(hUnorderedAccessViewHandle, pUnorderedAccesssView))
  {
    AddDeadObject(GALObjectType::BufferUnorderedAccessView, hUnorderedAccessViewHandle);
  }
  else
  {
    nsLog::Warning("DestroyUnorderedAccessView called on invalid handle (double free?)");
  }
}

nsGALSwapChainHandle nsGALDevice::CreateSwapChain(const SwapChainFactoryFunction& func)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  ///// \todo Platform independent validation
  // if (desc.m_pWindow == nullptr)
  //{
  //   nsLog::Error("The desc for the swap chain creation contained an invalid (nullptr) window handle!");
  //   return nsGALSwapChainHandle();
  // }

  nsGALSwapChain* pSwapChain = func(&m_Allocator);
  // nsGALSwapChainDX11* pSwapChain = NS_NEW(&m_Allocator, nsGALSwapChainDX11, Description);

  if (!pSwapChain->InitPlatform(this).Succeeded())
  {
    NS_DELETE(&m_Allocator, pSwapChain);
    return nsGALSwapChainHandle();
  }

  return nsGALSwapChainHandle(m_SwapChains.Insert(pSwapChain));
}

nsResult nsGALDevice::UpdateSwapChain(nsGALSwapChainHandle hSwapChain, nsEnum<nsGALPresentMode> newPresentMode)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALSwapChain* pSwapChain = nullptr;

  if (m_SwapChains.TryGetValue(hSwapChain, pSwapChain))
  {
    return pSwapChain->UpdateSwapChain(this, newPresentMode);
  }
  else
  {
    nsLog::Warning("UpdateSwapChain called on invalid handle.");
    return NS_FAILURE;
  }
}

void nsGALDevice::DestroySwapChain(nsGALSwapChainHandle hSwapChain)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALSwapChain* pSwapChain = nullptr;

  if (m_SwapChains.TryGetValue(hSwapChain, pSwapChain))
  {
    AddDeadObject(GALObjectType::SwapChain, hSwapChain);
  }
  else
  {
    nsLog::Warning("DestroySwapChain called on invalid handle (double free?)");
  }
}

nsGALQueryHandle nsGALDevice::CreateQuery(const nsGALQueryCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALQuery* pQuery = CreateQueryPlatform(desc);

  if (pQuery == nullptr)
  {
    return nsGALQueryHandle();
  }
  else
  {
    return nsGALQueryHandle(m_Queries.Insert(pQuery));
  }
}

void nsGALDevice::DestroyQuery(nsGALQueryHandle hQuery)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALQuery* pQuery = nullptr;

  if (m_Queries.TryGetValue(hQuery, pQuery))
  {
    AddDeadObject(GALObjectType::Query, hQuery);
  }
  else
  {
    nsLog::Warning("DestroyQuery called on invalid handle (double free?)");
  }
}

nsGALVertexDeclarationHandle nsGALDevice::CreateVertexDeclaration(const nsGALVertexDeclarationCreationDescription& desc)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  /// \todo Platform independent validation

  // Hash desc and return potential existing one (including inc. refcount)
  nsUInt32 uiHash = desc.CalculateHash();

  {
    nsGALVertexDeclarationHandle hVertexDeclaration;
    if (m_VertexDeclarationTable.TryGetValue(uiHash, hVertexDeclaration))
    {
      nsGALVertexDeclaration* pVertexDeclaration = m_VertexDeclarations[hVertexDeclaration];
      if (pVertexDeclaration->GetRefCount() == 0)
      {
        ReviveDeadObject(GALObjectType::VertexDeclaration, hVertexDeclaration);
      }

      pVertexDeclaration->AddRef();
      return hVertexDeclaration;
    }
  }

  nsGALVertexDeclaration* pVertexDeclaration = CreateVertexDeclarationPlatform(desc);

  if (pVertexDeclaration != nullptr)
  {
    pVertexDeclaration->AddRef();

    nsGALVertexDeclarationHandle hVertexDeclaration(m_VertexDeclarations.Insert(pVertexDeclaration));
    m_VertexDeclarationTable.Insert(uiHash, hVertexDeclaration);

    return hVertexDeclaration;
  }

  return nsGALVertexDeclarationHandle();
}

void nsGALDevice::DestroyVertexDeclaration(nsGALVertexDeclarationHandle hVertexDeclaration)
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  nsGALVertexDeclaration* pVertexDeclaration = nullptr;

  if (m_VertexDeclarations.TryGetValue(hVertexDeclaration, pVertexDeclaration))
  {
    pVertexDeclaration->ReleaseRef();

    if (pVertexDeclaration->GetRefCount() == 0)
    {
      AddDeadObject(GALObjectType::VertexDeclaration, hVertexDeclaration);
    }
  }
  else
  {
    nsLog::Warning("DestroyVertexDeclaration called on invalid handle (double free?)");
  }
}

nsGALTextureHandle nsGALDevice::GetBackBufferTextureFromSwapChain(nsGALSwapChainHandle hSwapChain)
{
  nsGALSwapChain* pSwapChain = nullptr;

  if (m_SwapChains.TryGetValue(hSwapChain, pSwapChain))
  {
    return pSwapChain->GetBackBufferTexture();
  }
  else
  {
    NS_REPORT_FAILURE("Swap chain handle invalid");
    return nsGALTextureHandle();
  }
}



// Misc functions

void nsGALDevice::BeginFrame(const nsUInt64 uiRenderFrame)
{
  {
    NS_PROFILE_SCOPE("BeforeBeginFrame");
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::BeforeBeginFrame;
    s_Events.Broadcast(e);
  }

  {
    NS_GALDEVICE_LOCK_AND_CHECK();
    NS_ASSERT_DEV(!m_bBeginFrameCalled, "You must call nsGALDevice::EndFrame before you can call nsGALDevice::BeginFrame again");
    m_bBeginFrameCalled = true;

    BeginFramePlatform(uiRenderFrame);
  }

  // TODO: move to beginrendering/compute calls
  // m_pPrimaryContext->ClearStatisticsCounters();

  {
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::AfterBeginFrame;
    s_Events.Broadcast(e);
  }
}

void nsGALDevice::EndFrame()
{
  {
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::BeforeEndFrame;
    s_Events.Broadcast(e);
  }

  {
    NS_GALDEVICE_LOCK_AND_CHECK();
    NS_ASSERT_DEV(m_bBeginFrameCalled, "You must have called nsGALDevice::Begin before you can call nsGALDevice::EndFrame");

    DestroyDeadObjects();

    EndFramePlatform();

    m_bBeginFrameCalled = false;
  }

  {
    nsGALDeviceEvent e;
    e.m_pDevice = this;
    e.m_Type = nsGALDeviceEvent::AfterEndFrame;
    s_Events.Broadcast(e);
  }
}

const nsGALDeviceCapabilities& nsGALDevice::GetCapabilities() const
{
  return m_Capabilities;
}

nsUInt64 nsGALDevice::GetMemoryConsumptionForTexture(const nsGALTextureCreationDescription& desc) const
{
  // This generic implementation is only an approximation, but it can be overridden by specific devices
  // to give an accurate memory consumption figure.
  nsUInt64 uiMemory = nsUInt64(desc.m_uiWidth) * nsUInt64(desc.m_uiHeight) * nsUInt64(desc.m_uiDepth);
  uiMemory *= desc.m_uiArraySize;
  uiMemory *= nsGALResourceFormat::GetBitsPerElement(desc.m_Format);
  uiMemory /= 8; // Bits per pixel
  uiMemory *= desc.m_SampleCount;

  // Also account for mip maps
  if (desc.m_uiMipLevelCount > 1)
  {
    uiMemory += static_cast<nsUInt64>((1.0 / 3.0) * uiMemory);
  }

  return uiMemory;
}


nsUInt64 nsGALDevice::GetMemoryConsumptionForBuffer(const nsGALBufferCreationDescription& desc) const
{
  return desc.m_uiTotalSize;
}

void nsGALDevice::Flush()
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  FlushPlatform();
}

void nsGALDevice::WaitIdle()
{
  WaitIdlePlatform();
}

void nsGALDevice::DestroyViews(nsGALTexture* pResource)
{
  NS_ASSERT_DEBUG(pResource != nullptr, "Must provide valid resource");

  NS_GALDEVICE_LOCK_AND_CHECK();

  for (auto it = pResource->m_ResourceViews.GetIterator(); it.IsValid(); ++it)
  {
    nsGALTextureResourceViewHandle hResourceView = it.Value();
    nsGALTextureResourceView* pResourceView = m_TextureResourceViews[hResourceView];

    m_TextureResourceViews.Remove(hResourceView);

    DestroyResourceViewPlatform(pResourceView);
  }
  pResource->m_ResourceViews.Clear();
  pResource->m_hDefaultResourceView.Invalidate();

  for (auto it = pResource->m_RenderTargetViews.GetIterator(); it.IsValid(); ++it)
  {
    nsGALRenderTargetViewHandle hRenderTargetView = it.Value();
    nsGALRenderTargetView* pRenderTargetView = m_RenderTargetViews[hRenderTargetView];

    m_RenderTargetViews.Remove(hRenderTargetView);

    DestroyRenderTargetViewPlatform(pRenderTargetView);
  }
  pResource->m_RenderTargetViews.Clear();
  pResource->m_hDefaultRenderTargetView.Invalidate();

  for (auto it = pResource->m_UnorderedAccessViews.GetIterator(); it.IsValid(); ++it)
  {
    nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView = it.Value();
    nsGALTextureUnorderedAccessView* pUnorderedAccessView = m_TextureUnorderedAccessViews[hUnorderedAccessView];

    m_TextureUnorderedAccessViews.Remove(hUnorderedAccessView);

    DestroyUnorderedAccessViewPlatform(pUnorderedAccessView);
  }
  pResource->m_UnorderedAccessViews.Clear();
}

void nsGALDevice::DestroyViews(nsGALBuffer* pResource)
{
  NS_ASSERT_DEBUG(pResource != nullptr, "Must provide valid resource");

  NS_GALDEVICE_LOCK_AND_CHECK();

  for (auto it = pResource->m_ResourceViews.GetIterator(); it.IsValid(); ++it)
  {
    nsGALBufferResourceViewHandle hResourceView = it.Value();
    nsGALBufferResourceView* pResourceView = m_BufferResourceViews[hResourceView];

    m_BufferResourceViews.Remove(hResourceView);

    DestroyResourceViewPlatform(pResourceView);
  }
  pResource->m_ResourceViews.Clear();
  pResource->m_hDefaultResourceView.Invalidate();

  for (auto it = pResource->m_UnorderedAccessViews.GetIterator(); it.IsValid(); ++it)
  {
    nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView = it.Value();
    nsGALBufferUnorderedAccessView* pUnorderedAccessView = m_BufferUnorderedAccessViews[hUnorderedAccessView];

    m_BufferUnorderedAccessViews.Remove(hUnorderedAccessView);

    DestroyUnorderedAccessViewPlatform(pUnorderedAccessView);
  }
  pResource->m_UnorderedAccessViews.Clear();
}

void nsGALDevice::DestroyDeadObjects()
{
  // Can't use range based for here since new objects might be added during iteration
  for (nsUInt32 i = 0; i < m_DeadObjects.GetCount(); ++i)
  {
    const auto& deadObject = m_DeadObjects[i];

    switch (deadObject.m_uiType)
    {
      case GALObjectType::BlendState:
      {
        nsGALBlendStateHandle hBlendState(nsGAL::ns16_16Id(deadObject.m_uiHandle));
        nsGALBlendState* pBlendState = nullptr;

        NS_VERIFY(m_BlendStates.Remove(hBlendState, &pBlendState), "BlendState not found in idTable");
        NS_VERIFY(m_BlendStateTable.Remove(pBlendState->GetDescription().CalculateHash()), "BlendState not found in de-duplication table");

        DestroyBlendStatePlatform(pBlendState);

        break;
      }
      case GALObjectType::DepthStencilState:
      {
        nsGALDepthStencilStateHandle hDepthStencilState(nsGAL::ns16_16Id(deadObject.m_uiHandle));
        nsGALDepthStencilState* pDepthStencilState = nullptr;

        NS_VERIFY(m_DepthStencilStates.Remove(hDepthStencilState, &pDepthStencilState), "DepthStencilState not found in idTable");
        NS_VERIFY(m_DepthStencilStateTable.Remove(pDepthStencilState->GetDescription().CalculateHash()),
          "DepthStencilState not found in de-duplication table");

        DestroyDepthStencilStatePlatform(pDepthStencilState);

        break;
      }
      case GALObjectType::RasterizerState:
      {
        nsGALRasterizerStateHandle hRasterizerState(nsGAL::ns16_16Id(deadObject.m_uiHandle));
        nsGALRasterizerState* pRasterizerState = nullptr;

        NS_VERIFY(m_RasterizerStates.Remove(hRasterizerState, &pRasterizerState), "RasterizerState not found in idTable");
        NS_VERIFY(
          m_RasterizerStateTable.Remove(pRasterizerState->GetDescription().CalculateHash()), "RasterizerState not found in de-duplication table");

        DestroyRasterizerStatePlatform(pRasterizerState);

        break;
      }
      case GALObjectType::SamplerState:
      {
        nsGALSamplerStateHandle hSamplerState(nsGAL::ns16_16Id(deadObject.m_uiHandle));
        nsGALSamplerState* pSamplerState = nullptr;

        NS_VERIFY(m_SamplerStates.Remove(hSamplerState, &pSamplerState), "SamplerState not found in idTable");
        NS_VERIFY(m_SamplerStateTable.Remove(pSamplerState->GetDescription().CalculateHash()), "SamplerState not found in de-duplication table");

        DestroySamplerStatePlatform(pSamplerState);

        break;
      }
      case GALObjectType::Shader:
      {
        nsGALShaderHandle hShader(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALShader* pShader = nullptr;

        m_Shaders.Remove(hShader, &pShader);

        DestroyShaderPlatform(pShader);

        break;
      }
      case GALObjectType::Buffer:
      {
        nsGALBufferHandle hBuffer(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALBuffer* pBuffer = nullptr;

        m_Buffers.Remove(hBuffer, &pBuffer);

        DestroyViews(pBuffer);
        DestroyBufferPlatform(pBuffer);

        break;
      }
      case GALObjectType::Texture:
      {
        nsGALTextureHandle hTexture(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALTexture* pTexture = nullptr;

        NS_VERIFY(m_Textures.Remove(hTexture, &pTexture), "Unexpected invalild texture handle");

        DestroyViews(pTexture);

        switch (pTexture->GetDescription().m_Type)
        {
          case nsGALTextureType::Texture2DShared:
            DestroySharedTexturePlatform(pTexture);
            break;
          default:
            DestroyTexturePlatform(pTexture);
            break;
        }
        break;
      }
      case GALObjectType::TextureResourceView:
      {
        nsGALTextureResourceViewHandle hResourceView(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALTextureResourceView* pResourceView = nullptr;

        m_TextureResourceViews.Remove(hResourceView, &pResourceView);

        nsGALTexture* pResource = pResourceView->m_pResource;
        NS_ASSERT_DEBUG(pResource != nullptr, "");

        NS_VERIFY(pResource->m_ResourceViews.Remove(pResourceView->GetDescription().CalculateHash()), "");
        pResourceView->m_pResource = nullptr;

        DestroyResourceViewPlatform(pResourceView);

        break;
      }
      case GALObjectType::BufferResourceView:
      {
        nsGALBufferResourceViewHandle hResourceView(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALBufferResourceView* pResourceView = nullptr;

        m_BufferResourceViews.Remove(hResourceView, &pResourceView);

        nsGALBuffer* pResource = pResourceView->m_pResource;
        NS_ASSERT_DEBUG(pResource != nullptr, "");

        NS_VERIFY(pResource->m_ResourceViews.Remove(pResourceView->GetDescription().CalculateHash()), "");
        pResourceView->m_pResource = nullptr;

        DestroyResourceViewPlatform(pResourceView);

        break;
      }
      case GALObjectType::RenderTargetView:
      {
        nsGALRenderTargetViewHandle hRenderTargetView(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALRenderTargetView* pRenderTargetView = nullptr;

        m_RenderTargetViews.Remove(hRenderTargetView, &pRenderTargetView);

        nsGALTexture* pTexture = pRenderTargetView->m_pTexture;
        NS_ASSERT_DEBUG(pTexture != nullptr, "");
        NS_VERIFY(pTexture->m_RenderTargetViews.Remove(pRenderTargetView->GetDescription().CalculateHash()), "");
        pRenderTargetView->m_pTexture = nullptr;

        DestroyRenderTargetViewPlatform(pRenderTargetView);

        break;
      }
      case GALObjectType::TextureUnorderedAccessView:
      {
        nsGALTextureUnorderedAccessViewHandle hUnorderedAccessViewHandle(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALTextureUnorderedAccessView* pUnorderedAccesssView = nullptr;

        m_TextureUnorderedAccessViews.Remove(hUnorderedAccessViewHandle, &pUnorderedAccesssView);

        nsGALTexture* pResource = pUnorderedAccesssView->m_pResource;
        NS_ASSERT_DEBUG(pResource != nullptr, "");

        NS_VERIFY(pResource->m_UnorderedAccessViews.Remove(pUnorderedAccesssView->GetDescription().CalculateHash()), "");
        pUnorderedAccesssView->m_pResource = nullptr;

        DestroyUnorderedAccessViewPlatform(pUnorderedAccesssView);
        break;
      }
      case GALObjectType::BufferUnorderedAccessView:
      {
        nsGALBufferUnorderedAccessViewHandle hUnorderedAccessViewHandle(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALBufferUnorderedAccessView* pUnorderedAccesssView = nullptr;

        m_BufferUnorderedAccessViews.Remove(hUnorderedAccessViewHandle, &pUnorderedAccesssView);

        nsGALBuffer* pResource = pUnorderedAccesssView->m_pResource;
        NS_ASSERT_DEBUG(pResource != nullptr, "");

        NS_VERIFY(pResource->m_UnorderedAccessViews.Remove(pUnorderedAccesssView->GetDescription().CalculateHash()), "");
        pUnorderedAccesssView->m_pResource = nullptr;

        DestroyUnorderedAccessViewPlatform(pUnorderedAccesssView);
        break;
      }
      case GALObjectType::SwapChain:
      {
        nsGALSwapChainHandle hSwapChain(nsGAL::ns16_16Id(deadObject.m_uiHandle));
        nsGALSwapChain* pSwapChain = nullptr;

        m_SwapChains.Remove(hSwapChain, &pSwapChain);

        if (pSwapChain != nullptr)
        {
          pSwapChain->DeInitPlatform(this).IgnoreResult();
          NS_DELETE(&m_Allocator, pSwapChain);
        }

        break;
      }
      case GALObjectType::Query:
      {
        nsGALQueryHandle hQuery(nsGAL::ns20_12Id(deadObject.m_uiHandle));
        nsGALQuery* pQuery = nullptr;

        m_Queries.Remove(hQuery, &pQuery);

        DestroyQueryPlatform(pQuery);

        break;
      }
      case GALObjectType::VertexDeclaration:
      {
        nsGALVertexDeclarationHandle hVertexDeclaration(nsGAL::ns18_14Id(deadObject.m_uiHandle));
        nsGALVertexDeclaration* pVertexDeclaration = nullptr;

        NS_VERIFY(m_VertexDeclarations.Remove(hVertexDeclaration, &pVertexDeclaration), "Unexpected invalid handle");
        m_VertexDeclarationTable.Remove(pVertexDeclaration->GetDescription().CalculateHash());

        DestroyVertexDeclarationPlatform(pVertexDeclaration);

        break;
      }
      default:
        NS_ASSERT_NOT_IMPLEMENTED;
    }
  }

  m_DeadObjects.Clear();
}

const nsGALSwapChain* nsGALDevice::GetSwapChainInternal(nsGALSwapChainHandle hSwapChain, const nsRTTI* pRequestedType) const
{
  const nsGALSwapChain* pSwapChain = GetSwapChain(hSwapChain);
  if (pSwapChain)
  {
    if (!pSwapChain->GetDescription().m_pSwapChainType->IsDerivedFrom(pRequestedType))
      return nullptr;
  }
  return pSwapChain;
}
