#include <RendererCore/RendererCorePCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Types/ScopeExit.h>
#include <RendererCore/Material/MaterialResource.h>
#include <RendererCore/Meshes/DynamicMeshBufferResource.h>
#include <RendererCore/Meshes/MeshBufferResource.h>
#include <RendererCore/RenderContext/RenderContext.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererCore/Shader/ShaderPermutationResource.h>
#include <RendererCore/ShaderCompiler/ShaderManager.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererCore/Textures/Texture3DResource.h>
#include <RendererCore/Textures/TextureCubeResource.h>
#include <RendererFoundation/CommandEncoder/CommandEncoder.h>
#include <RendererFoundation/Device/ImmutableSamplers.h>
#include <RendererFoundation/Resources/RenderTargetView.h>
#include <RendererFoundation/Resources/Texture.h>

nsRenderContext* nsRenderContext::s_pDefaultInstance = nullptr;
nsHybridArray<nsRenderContext*, 4> nsRenderContext::s_Instances;

nsMap<nsRenderContext::ShaderVertexDecl, nsGALVertexDeclarationHandle> nsRenderContext::s_GALVertexDeclarations;

nsMutex nsRenderContext::s_ConstantBufferStorageMutex;
nsIdTable<nsConstantBufferStorageId, nsConstantBufferStorageBase*> nsRenderContext::s_ConstantBufferStorageTable;
nsMap<nsUInt32, nsDynamicArray<nsConstantBufferStorageBase*>> nsRenderContext::s_FreeConstantBufferStorage;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, RendererContext)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation",
  "Core",
  "ImmutableSamplers"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsRenderContext::RegisterImmutableSamplers();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {

  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    nsRenderContext::OnEngineStartup();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    nsRenderContext::OnEngineShutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

//////////////////////////////////////////////////////////////////////////

nsRenderContext::Statistics::Statistics()
{
  Reset();
}

void nsRenderContext::Statistics::Reset()
{
  m_uiFailedDrawcalls = 0;
}

//////////////////////////////////////////////////////////////////////////

nsRenderContext* nsRenderContext::GetDefaultInstance()
{
  if (s_pDefaultInstance == nullptr)
    s_pDefaultInstance = CreateInstance();

  return s_pDefaultInstance;
}

nsRenderContext* nsRenderContext::CreateInstance()
{
  return NS_DEFAULT_NEW(nsRenderContext);
}

void nsRenderContext::DestroyInstance(nsRenderContext* pRenderer)
{
  NS_DEFAULT_DELETE(pRenderer);
}

nsRenderContext::nsRenderContext()
{
  if (s_pDefaultInstance == nullptr)
  {
    s_pDefaultInstance = this;
  }

  s_Instances.PushBack(this);

  m_StateFlags = nsRenderContextFlags::AllStatesInvalid;
  m_Topology = nsGALPrimitiveTopology::ENUM_COUNT; // Set to something invalid
  m_uiMeshBufferPrimitiveCount = 0;
  m_DefaultTextureFilter = nsTextureFilterSetting::FixedAnisotropic4x;
  m_bAllowAsyncShaderLoading = false;

  m_hGlobalConstantBufferStorage = CreateConstantBufferStorage<nsGlobalConstants>();

  // If no push constants are supported, they are emulated via constant buffers.
  if (nsGALDevice::GetDefaultDevice()->GetCapabilities().m_uiMaxPushConstantsSize == 0)
  {
    m_hPushConstantsStorage = CreateConstantBufferStorage(128);
  }
  ResetContextState();
}

nsRenderContext::~nsRenderContext()
{
  DeleteConstantBufferStorage(m_hGlobalConstantBufferStorage);
  if (!m_hPushConstantsStorage.IsInvalidated())
  {
    DeleteConstantBufferStorage(m_hPushConstantsStorage);
  }

  if (s_pDefaultInstance == this)
    s_pDefaultInstance = nullptr;

  s_Instances.RemoveAndSwap(this);
}

nsRenderContext::Statistics nsRenderContext::GetAndResetStatistics()
{
  nsRenderContext::Statistics ret = m_Statistics;
  ret.Reset();

  return ret;
}

nsGALRenderCommandEncoder* nsRenderContext::BeginRendering(nsGALPass* pGALPass, const nsGALRenderingSetup& renderingSetup, const nsRectFloat& viewport, const char* szName, bool bStereoSupport)
{
  nsGALMSAASampleCount::Enum msaaSampleCount = nsGALMSAASampleCount::None;

  nsGALRenderTargetViewHandle hRTV;
  if (renderingSetup.m_RenderTargetSetup.GetRenderTargetCount() > 0)
  {
    hRTV = renderingSetup.m_RenderTargetSetup.GetRenderTarget(0);
  }
  if (hRTV.IsInvalidated())
  {
    hRTV = renderingSetup.m_RenderTargetSetup.GetDepthStencilTarget();
  }

  if (const nsGALRenderTargetView* pRTV = nsGALDevice::GetDefaultDevice()->GetRenderTargetView(hRTV))
  {
    msaaSampleCount = pRTV->GetTexture()->GetDescription().m_SampleCount;
  }

  if (msaaSampleCount != nsGALMSAASampleCount::None)
  {
    SetShaderPermutationVariable("MSAA", "TRUE");
  }
  else
  {
    SetShaderPermutationVariable("MSAA", "FALSE");
  }

  auto& gc = WriteGlobalConstants();
  gc.ViewportSize = nsVec4(viewport.width, viewport.height, 1.0f / viewport.width, 1.0f / viewport.height);
  gc.NumMsaaSamples = msaaSampleCount;

  auto pGALCommandEncoder = pGALPass->BeginRendering(renderingSetup, szName);

  pGALCommandEncoder->SetViewport(viewport);

  m_pGALPass = pGALPass;
  m_pGALCommandEncoder = pGALCommandEncoder;
  m_bCompute = false;
  m_bStereoRendering = bStereoSupport;

  return pGALCommandEncoder;
}

void nsRenderContext::EndRendering()
{
  m_pGALPass->EndRendering(GetRenderCommandEncoder());

  m_pGALPass = nullptr;
  m_pGALCommandEncoder = nullptr;
  m_bStereoRendering = false;

  // TODO: The render context needs to reset its state after every encoding block if we want to record to separate command buffers.
  // Although this is currently not possible since a lot of high level code binds stuff only once per frame on the render context.
  // Resetting the state after every encoding block breaks those assumptions.
  // ResetContextState();
}

nsGALComputeCommandEncoder* nsRenderContext::BeginCompute(nsGALPass* pGALPass, const char* szName /*= ""*/)
{
  auto pGALCommandEncoder = pGALPass->BeginCompute(szName);

  m_pGALPass = pGALPass;
  m_pGALCommandEncoder = pGALCommandEncoder;
  m_bCompute = true;

  return pGALCommandEncoder;
}

void nsRenderContext::EndCompute()
{
  m_pGALPass->EndCompute(GetComputeCommandEncoder());

  m_pGALPass = nullptr;
  m_pGALCommandEncoder = nullptr;

  // TODO: See EndRendering
  // ResetContextState();
}

void nsRenderContext::SetShaderPermutationVariable(const char* szName, const nsTempHashedString& sTempValue)
{
  nsTempHashedString sHashedName(szName);

  nsHashedString sName;
  nsHashedString sValue;
  if (nsShaderManager::IsPermutationValueAllowed(szName, sHashedName, sTempValue, sName, sValue))
  {
    SetShaderPermutationVariableInternal(sName, sValue);
  }
}

void nsRenderContext::SetShaderPermutationVariable(const nsHashedString& sName, const nsHashedString& sValue)
{
  if (nsShaderManager::IsPermutationValueAllowed(sName, sValue))
  {
    SetShaderPermutationVariableInternal(sName, sValue);
  }
}


void nsRenderContext::BindMaterial(const nsMaterialResourceHandle& hMaterial)
{
  // Don't set m_hMaterial directly since we first need to check whether the material has been modified in the mean time.
  m_hNewMaterial = hMaterial;
  m_StateFlags.Add(nsRenderContextFlags::MaterialBindingChanged);
}

void nsRenderContext::BindTexture2D(const nsTempHashedString& sSlotName, const nsTexture2DResourceHandle& hTexture,
  nsResourceAcquireMode acquireMode /*= nsResourceAcquireMode::AllowLoadingFallback*/)
{
  if (hTexture.IsValid())
  {
    nsResourceLock<nsTexture2DResource> pTexture(hTexture, acquireMode);
    BindTexture2D(sSlotName, nsGALDevice::GetDefaultDevice()->GetDefaultResourceView(pTexture->GetGALTexture()));
    BindSamplerState(sSlotName, pTexture->GetGALSamplerState());
  }
  else
  {
    BindTexture2D(sSlotName, nsGALTextureResourceViewHandle());
  }
}

void nsRenderContext::BindTexture3D(const nsTempHashedString& sSlotName, const nsTexture3DResourceHandle& hTexture,
  nsResourceAcquireMode acquireMode /*= nsResourceAcquireMode::AllowLoadingFallback*/)
{
  if (hTexture.IsValid())
  {
    nsResourceLock<nsTexture3DResource> pTexture(hTexture, acquireMode);
    BindTexture3D(sSlotName, nsGALDevice::GetDefaultDevice()->GetDefaultResourceView(pTexture->GetGALTexture()));
    BindSamplerState(sSlotName, pTexture->GetGALSamplerState());
  }
  else
  {
    BindTexture3D(sSlotName, nsGALTextureResourceViewHandle());
  }
}

void nsRenderContext::BindTextureCube(const nsTempHashedString& sSlotName, const nsTextureCubeResourceHandle& hTexture,
  nsResourceAcquireMode acquireMode /*= nsResourceAcquireMode::AllowLoadingFallback*/)
{
  if (hTexture.IsValid())
  {
    nsResourceLock<nsTextureCubeResource> pTexture(hTexture, acquireMode);
    BindTextureCube(sSlotName, nsGALDevice::GetDefaultDevice()->GetDefaultResourceView(pTexture->GetGALTexture()));
    BindSamplerState(sSlotName, pTexture->GetGALSamplerState());
  }
  else
  {
    BindTextureCube(sSlotName, nsGALTextureResourceViewHandle());
  }
}

void nsRenderContext::BindTexture2D(const nsTempHashedString& sSlotName, nsGALTextureResourceViewHandle hResourceView)
{
  nsGALTextureResourceViewHandle* pOldResourceView = nullptr;
  if (m_BoundTextures2D.TryGetValue(sSlotName.GetHash(), pOldResourceView))
  {
    if (*pOldResourceView == hResourceView)
      return;

    *pOldResourceView = hResourceView;
  }
  else
  {
    m_BoundTextures2D.Insert(sSlotName.GetHash(), hResourceView);
  }

  m_StateFlags.Add(nsRenderContextFlags::TextureBindingChanged);
}

void nsRenderContext::BindTexture3D(const nsTempHashedString& sSlotName, nsGALTextureResourceViewHandle hResourceView)
{
  nsGALTextureResourceViewHandle* pOldResourceView = nullptr;
  if (m_BoundTextures3D.TryGetValue(sSlotName.GetHash(), pOldResourceView))
  {
    if (*pOldResourceView == hResourceView)
      return;

    *pOldResourceView = hResourceView;
  }
  else
  {
    m_BoundTextures3D.Insert(sSlotName.GetHash(), hResourceView);
  }

  m_StateFlags.Add(nsRenderContextFlags::TextureBindingChanged);
}

void nsRenderContext::BindTextureCube(const nsTempHashedString& sSlotName, nsGALTextureResourceViewHandle hResourceView)
{
  nsGALTextureResourceViewHandle* pOldResourceView = nullptr;
  if (m_BoundTexturesCube.TryGetValue(sSlotName.GetHash(), pOldResourceView))
  {
    if (*pOldResourceView == hResourceView)
      return;

    *pOldResourceView = hResourceView;
  }
  else
  {
    m_BoundTexturesCube.Insert(sSlotName.GetHash(), hResourceView);
  }

  m_StateFlags.Add(nsRenderContextFlags::TextureBindingChanged);
}

void nsRenderContext::BindUAV(const nsTempHashedString& sSlotName, nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView)
{
  nsGALTextureUnorderedAccessViewHandle* pOldResourceView = nullptr;
  if (m_BoundTextureUAVs.TryGetValue(sSlotName.GetHash(), pOldResourceView))
  {
    if (*pOldResourceView == hUnorderedAccessView)
      return;

    *pOldResourceView = hUnorderedAccessView;
  }
  else
  {
    m_BoundTextureUAVs.Insert(sSlotName.GetHash(), hUnorderedAccessView);
  }

  m_StateFlags.Add(nsRenderContextFlags::UAVBindingChanged);
}

void nsRenderContext::BindUAV(const nsTempHashedString& sSlotName, nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView)
{
  nsGALBufferUnorderedAccessViewHandle* pOldResourceView = nullptr;
  if (m_BoundBufferUAVs.TryGetValue(sSlotName.GetHash(), pOldResourceView))
  {
    if (*pOldResourceView == hUnorderedAccessView)
      return;

    *pOldResourceView = hUnorderedAccessView;
  }
  else
  {
    m_BoundBufferUAVs.Insert(sSlotName.GetHash(), hUnorderedAccessView);
  }

  m_StateFlags.Add(nsRenderContextFlags::UAVBindingChanged);
}


void nsRenderContext::BindSamplerState(const nsTempHashedString& sSlotName, nsGALSamplerStateHandle hSamplerSate)
{
  NS_ASSERT_DEBUG(sSlotName != "LinearSampler", "'LinearSampler' is a resevered sampler name and must not be set manually.");
  NS_ASSERT_DEBUG(sSlotName != "LinearClampSampler", "'LinearClampSampler' is a resevered sampler name and must not be set manually.");
  NS_ASSERT_DEBUG(sSlotName != "PointSampler", "'PointSampler' is a resevered sampler name and must not be set manually.");
  NS_ASSERT_DEBUG(sSlotName != "PointClampSampler", "'PointClampSampler' is a resevered sampler name and must not be set manually.");

  nsGALSamplerStateHandle* pOldSamplerState = nullptr;
  if (m_BoundSamplers.TryGetValue(sSlotName.GetHash(), pOldSamplerState))
  {
    if (*pOldSamplerState == hSamplerSate)
      return;

    *pOldSamplerState = hSamplerSate;
  }
  else
  {
    m_BoundSamplers.Insert(sSlotName.GetHash(), hSamplerSate);
  }

  m_StateFlags.Add(nsRenderContextFlags::SamplerBindingChanged);
}

void nsRenderContext::BindBuffer(const nsTempHashedString& sSlotName, nsGALBufferResourceViewHandle hResourceView)
{
  nsGALBufferResourceViewHandle* pOldResourceView = nullptr;
  if (m_BoundBuffer.TryGetValue(sSlotName.GetHash(), pOldResourceView))
  {
    if (*pOldResourceView == hResourceView)
      return;

    *pOldResourceView = hResourceView;
  }
  else
  {
    m_BoundBuffer.Insert(sSlotName.GetHash(), hResourceView);
  }

  m_StateFlags.Add(nsRenderContextFlags::BufferBindingChanged);
}

void nsRenderContext::BindConstantBuffer(const nsTempHashedString& sSlotName, nsGALBufferHandle hConstantBuffer)
{
  BoundConstantBuffer* pBoundConstantBuffer = nullptr;
  if (m_BoundConstantBuffers.TryGetValue(sSlotName.GetHash(), pBoundConstantBuffer))
  {
    if (pBoundConstantBuffer->m_hConstantBuffer == hConstantBuffer)
      return;

    pBoundConstantBuffer->m_hConstantBuffer = hConstantBuffer;
    pBoundConstantBuffer->m_hConstantBufferStorage.Invalidate();
  }
  else
  {
    m_BoundConstantBuffers.Insert(sSlotName.GetHash(), BoundConstantBuffer(hConstantBuffer));
  }

  m_StateFlags.Add(nsRenderContextFlags::ConstantBufferBindingChanged);
}

void nsRenderContext::BindConstantBuffer(const nsTempHashedString& sSlotName, nsConstantBufferStorageHandle hConstantBufferStorage)
{
  BoundConstantBuffer* pBoundConstantBuffer = nullptr;
  if (m_BoundConstantBuffers.TryGetValue(sSlotName.GetHash(), pBoundConstantBuffer))
  {
    if (pBoundConstantBuffer->m_hConstantBufferStorage == hConstantBufferStorage)
      return;

    pBoundConstantBuffer->m_hConstantBuffer.Invalidate();
    pBoundConstantBuffer->m_hConstantBufferStorage = hConstantBufferStorage;
  }
  else
  {
    m_BoundConstantBuffers.Insert(sSlotName.GetHash(), BoundConstantBuffer(hConstantBufferStorage));
  }

  m_StateFlags.Add(nsRenderContextFlags::ConstantBufferBindingChanged);
}

void nsRenderContext::SetPushConstants(const nsTempHashedString& sSlotName, nsArrayPtr<const nsUInt8> data)
{

  if (!m_hPushConstantsStorage.IsInvalidated())
  {
    NS_ASSERT_DEBUG(data.GetCount() <= 128, "Push constants are not allowed to be bigger than 128 bytes.");
    nsConstantBufferStorageBase* pStorage = nullptr;
    bool bResult = TryGetConstantBufferStorage(m_hPushConstantsStorage, pStorage);
    nsArrayPtr<nsUInt8> targetStorage = pStorage->GetRawDataForWriting();
    nsMemoryUtils::Copy(targetStorage.GetPtr(), data.GetPtr(), data.GetCount());
    BindConstantBuffer(sSlotName, m_hPushConstantsStorage);
  }
  else
  {
    NS_ASSERT_DEBUG(data.GetCount() <= nsGALDevice::GetDefaultDevice()->GetCapabilities().m_uiMaxPushConstantsSize, "Push constants are not allowed to be bigger than {} bytes.", nsGALDevice::GetDefaultDevice()->GetCapabilities().m_uiMaxPushConstantsSize);
    m_pGALCommandEncoder->SetPushConstants(data);
  }
}

void nsRenderContext::BindShader(const nsShaderResourceHandle& hShader, nsBitflags<nsShaderBindFlags> flags)
{
  m_hMaterial.Invalidate();
  m_StateFlags.Remove(nsRenderContextFlags::MaterialBindingChanged);

  BindShaderInternal(hShader, flags);
}

void nsRenderContext::BindMeshBuffer(const nsMeshBufferResourceHandle& hMeshBuffer)
{
  nsResourceLock<nsMeshBufferResource> pMeshBuffer(hMeshBuffer, nsResourceAcquireMode::AllowLoadingFallback);
  BindMeshBuffer(pMeshBuffer->GetVertexBuffer(), pMeshBuffer->GetIndexBuffer(), &(pMeshBuffer->GetVertexDeclaration()), pMeshBuffer->GetTopology(),
    pMeshBuffer->GetPrimitiveCount());
}

void nsRenderContext::BindMeshBuffer(nsGALBufferHandle hVertexBuffer, nsGALBufferHandle hIndexBuffer,
  const nsVertexDeclarationInfo* pVertexDeclarationInfo, nsGALPrimitiveTopology::Enum topology, nsUInt32 uiPrimitiveCount, nsGALBufferHandle hVertexBuffer2, nsGALBufferHandle hVertexBuffer3, nsGALBufferHandle hVertexBuffer4)
{
  if (m_hVertexBuffers[0] == hVertexBuffer && m_hVertexBuffers[1] == hVertexBuffer2 && m_hVertexBuffers[2] == hVertexBuffer3 && m_hVertexBuffers[3] == hVertexBuffer4 && m_hIndexBuffer == hIndexBuffer && m_pVertexDeclarationInfo == pVertexDeclarationInfo && m_Topology == topology && m_uiMeshBufferPrimitiveCount == uiPrimitiveCount)
  {
    return;
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  if (pVertexDeclarationInfo)
  {
    for (nsUInt32 i1 = 0; i1 < pVertexDeclarationInfo->m_VertexStreams.GetCount(); ++i1)
    {
      for (nsUInt32 i2 = 0; i2 < pVertexDeclarationInfo->m_VertexStreams.GetCount(); ++i2)
      {
        if (i1 != i2)
        {
          NS_ASSERT_DEBUG(pVertexDeclarationInfo->m_VertexStreams[i1].m_Semantic != pVertexDeclarationInfo->m_VertexStreams[i2].m_Semantic,
            "Same semantic cannot be used twice in the same vertex declaration");
        }
      }
    }
  }
#endif

  if (m_Topology != topology)
  {
    m_Topology = topology;

    nsTempHashedString sTopologies[nsGALPrimitiveTopology::ENUM_COUNT] = {
      nsTempHashedString("TOPOLOGY_POINTS"), nsTempHashedString("TOPOLOGY_LINES"), nsTempHashedString("TOPOLOGY_TRIANGLES")};

    SetShaderPermutationVariable("TOPOLOGY", sTopologies[m_Topology]);
  }

  m_hVertexBuffers[0] = hVertexBuffer;
  m_hVertexBuffers[1] = hVertexBuffer2;
  m_hVertexBuffers[2] = hVertexBuffer3;
  m_hVertexBuffers[3] = hVertexBuffer4;
  m_hIndexBuffer = hIndexBuffer;
  m_pVertexDeclarationInfo = pVertexDeclarationInfo;
  m_uiMeshBufferPrimitiveCount = uiPrimitiveCount;

  m_StateFlags.Add(nsRenderContextFlags::MeshBufferBindingChanged);
}

void nsRenderContext::BindMeshBuffer(const nsDynamicMeshBufferResourceHandle& hDynamicMeshBuffer)
{
  nsResourceLock<nsDynamicMeshBufferResource> pMeshBuffer(hDynamicMeshBuffer, nsResourceAcquireMode::AllowLoadingFallback);
  BindMeshBuffer(pMeshBuffer->GetVertexBuffer(), pMeshBuffer->GetIndexBuffer(), &(pMeshBuffer->GetVertexDeclaration()), pMeshBuffer->GetDescriptor().m_Topology, pMeshBuffer->GetDescriptor().m_uiMaxPrimitives, pMeshBuffer->GetColorBuffer());
}

nsResult nsRenderContext::DrawMeshBuffer(nsUInt32 uiPrimitiveCount, nsUInt32 uiFirstPrimitive, nsUInt32 uiInstanceCount)
{
  if (ApplyContextStates().Failed() || uiPrimitiveCount == 0 || uiInstanceCount == 0)
  {
    m_Statistics.m_uiFailedDrawcalls++;
    return NS_FAILURE;
  }

  NS_ASSERT_DEV(uiFirstPrimitive < m_uiMeshBufferPrimitiveCount, "Invalid primitive range: first primitive ({0}) can't be larger than number of primitives ({1})", uiFirstPrimitive, uiPrimitiveCount);

  uiPrimitiveCount = nsMath::Min(uiPrimitiveCount, m_uiMeshBufferPrimitiveCount - uiFirstPrimitive);
  NS_ASSERT_DEV(uiPrimitiveCount > 0, "Invalid primitive range: number of primitives can't be zero.");

  auto pCommandEncoder = GetRenderCommandEncoder();

  const nsUInt32 uiVertsPerPrimitive = nsGALPrimitiveTopology::VerticesPerPrimitive(pCommandEncoder->GetPrimitiveTopology());

  uiPrimitiveCount *= uiVertsPerPrimitive;
  uiFirstPrimitive *= uiVertsPerPrimitive;
  if (m_bStereoRendering)
  {
    uiInstanceCount *= 2;
  }

  if (uiInstanceCount > 1)
  {
    if (!m_hIndexBuffer.IsInvalidated())
    {
      return pCommandEncoder->DrawIndexedInstanced(uiPrimitiveCount, uiInstanceCount, uiFirstPrimitive);
    }
    else
    {
      return pCommandEncoder->DrawInstanced(uiPrimitiveCount, uiInstanceCount, uiFirstPrimitive);
    }
  }
  else
  {
    if (!m_hIndexBuffer.IsInvalidated())
    {
      return pCommandEncoder->DrawIndexed(uiPrimitiveCount, uiFirstPrimitive);
    }
    else
    {
      return pCommandEncoder->Draw(uiPrimitiveCount, uiFirstPrimitive);
    }
  }

  return NS_SUCCESS;
}

nsResult nsRenderContext::Dispatch(nsUInt32 uiThreadGroupCountX, nsUInt32 uiThreadGroupCountY, nsUInt32 uiThreadGroupCountZ)
{
  if (ApplyContextStates().Failed())
  {
    m_Statistics.m_uiFailedDrawcalls++;
    return NS_FAILURE;
  }

  return GetComputeCommandEncoder()->Dispatch(uiThreadGroupCountX, uiThreadGroupCountY, uiThreadGroupCountZ);
}

nsResult nsRenderContext::ApplyContextStates(bool bForce)
{
  // First apply material state since this can modify all other states.
  // Note ApplyMaterialState only returns a valid material pointer if the constant buffer of this material needs to be updated.
  // This needs to be done once we have determined the correct shader permutation.
  nsMaterialResource* pMaterial = nullptr;
  NS_SCOPE_EXIT(if (pMaterial != nullptr) { nsResourceManager::EndAcquireResource(pMaterial); });

  if (bForce || m_StateFlags.IsSet(nsRenderContextFlags::MaterialBindingChanged))
  {
    pMaterial = ApplyMaterialState();

    m_StateFlags.Remove(nsRenderContextFlags::MaterialBindingChanged);
  }

  nsShaderPermutationResource* pShaderPermutation = nullptr;
  NS_SCOPE_EXIT(if (pShaderPermutation != nullptr) { nsResourceManager::EndAcquireResource(pShaderPermutation); });

  bool bRebuildVertexDeclaration = m_StateFlags.IsAnySet(nsRenderContextFlags::ShaderStateChanged | nsRenderContextFlags::MeshBufferBindingChanged);

  if (bForce || m_StateFlags.IsSet(nsRenderContextFlags::ShaderStateChanged))
  {
    pShaderPermutation = ApplyShaderState();

    if (pShaderPermutation == nullptr)
    {
      return NS_FAILURE;
    }

    m_StateFlags.Remove(nsRenderContextFlags::ShaderStateChanged);
  }

  if (m_hActiveShaderPermutation.IsValid())
  {
    const bool bDirty = (bForce || m_StateFlags.IsAnySet(nsRenderContextFlags::TextureBindingChanged | nsRenderContextFlags::UAVBindingChanged |
                                                         nsRenderContextFlags::SamplerBindingChanged | nsRenderContextFlags::BufferBindingChanged |
                                                         nsRenderContextFlags::ConstantBufferBindingChanged));

    const nsGALShader* pShader = nullptr;
    if (bDirty)
    {
      if (pShaderPermutation == nullptr)
      {
        pShaderPermutation = nsResourceManager::BeginAcquireResource(m_hActiveShaderPermutation, nsResourceAcquireMode::BlockTillLoaded);
      }
      if (pShaderPermutation == nullptr)
      {
        return NS_FAILURE;
      }
      // #TODO_SHADER It's a bit unclean that we need to acquire the GAL shader on this level. Unfortunately, we need the resource binding on both the GAL and the high level renderer and the only alternative is some kind of duplication of the data.
      pShader = nsGALDevice::GetDefaultDevice()->GetShader(m_hActiveGALShader);
    }

    nsLogBlock applyBindingsBlock("Applying Shader Bindings", pShaderPermutation ? pShaderPermutation->GetResourceDescription().GetData() : "");

    if (bDirty)
    {
      if (bForce || m_StateFlags.IsSet(nsRenderContextFlags::UAVBindingChanged))
      {
        ApplyUAVBindings(pShader);
        m_StateFlags.Remove(nsRenderContextFlags::UAVBindingChanged);
      }

      if (bForce || m_StateFlags.IsSet(nsRenderContextFlags::TextureBindingChanged))
      {
        ApplyTextureBindings(pShader);
        m_StateFlags.Remove(nsRenderContextFlags::TextureBindingChanged);
      }

      if (bForce || m_StateFlags.IsSet(nsRenderContextFlags::SamplerBindingChanged))
      {
        ApplySamplerBindings(pShader);
        m_StateFlags.Remove(nsRenderContextFlags::SamplerBindingChanged);
      }

      if (bForce || m_StateFlags.IsSet(nsRenderContextFlags::BufferBindingChanged))
      {
        ApplyBufferBindings(pShader);
        m_StateFlags.Remove(nsRenderContextFlags::BufferBindingChanged);
      }
    }

    // Note that pMaterial is only valid, if material constants have changed, so this also always implies that ConstantBufferBindingChanged is set.
    if (pMaterial != nullptr)
    {
      pMaterial->UpdateConstantBuffer(pShaderPermutation);
      BindConstantBuffer("nsMaterialConstants", pMaterial->m_hConstantBufferStorage);
    }

    UploadConstants();

    if (bDirty)
    {
      if (bForce || m_StateFlags.IsSet(nsRenderContextFlags::ConstantBufferBindingChanged))
      {
        ApplyConstantBufferBindings(pShader);
        m_StateFlags.Remove(nsRenderContextFlags::ConstantBufferBindingChanged);
      }
    }
  }

  if ((bForce || bRebuildVertexDeclaration) && !m_bCompute)
  {
    if (m_hActiveGALShader.IsInvalidated())
      return NS_FAILURE;

    auto pCommandEncoder = GetRenderCommandEncoder();

    if (bForce || m_StateFlags.IsSet(nsRenderContextFlags::MeshBufferBindingChanged))
    {
      pCommandEncoder->SetPrimitiveTopology(m_Topology);

      for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_hVertexBuffers); ++i)
      {
        pCommandEncoder->SetVertexBuffer(i, m_hVertexBuffers[i]);
      }

      if (!m_hIndexBuffer.IsInvalidated())
        pCommandEncoder->SetIndexBuffer(m_hIndexBuffer);
    }

    nsGALVertexDeclarationHandle hVertexDeclaration;
    if (m_pVertexDeclarationInfo != nullptr && BuildVertexDeclaration(m_hActiveGALShader, *m_pVertexDeclarationInfo, hVertexDeclaration).Failed())
      return NS_FAILURE;

    // If there is a vertex buffer we need a valid vertex declaration as well.
    if ((!m_hVertexBuffers[0].IsInvalidated() || !m_hVertexBuffers[1].IsInvalidated() || !m_hVertexBuffers[2].IsInvalidated() || !m_hVertexBuffers[3].IsInvalidated()) && hVertexDeclaration.IsInvalidated())
      return NS_FAILURE;

    pCommandEncoder->SetVertexDeclaration(hVertexDeclaration);

    m_StateFlags.Remove(nsRenderContextFlags::MeshBufferBindingChanged);
  }

  return NS_SUCCESS;
}

void nsRenderContext::ResetContextState()
{
  m_StateFlags = nsRenderContextFlags::AllStatesInvalid;

  m_hActiveShader.Invalidate();
  m_hActiveGALShader.Invalidate();

  m_PermutationVariables.Clear();
  m_hNewMaterial.Invalidate();
  m_hMaterial.Invalidate();

  m_hActiveShaderPermutation.Invalidate();

  for (nsUInt32 i = 0; i < NS_ARRAY_SIZE(m_hVertexBuffers); ++i)
  {
    m_hVertexBuffers[i].Invalidate();
  }

  m_hIndexBuffer.Invalidate();
  m_pVertexDeclarationInfo = nullptr;
  m_Topology = nsGALPrimitiveTopology::ENUM_COUNT; // Set to something invalid
  m_uiMeshBufferPrimitiveCount = 0;

  m_BoundTextures2D.Clear();
  m_BoundTextures3D.Clear();
  m_BoundTexturesCube.Clear();
  m_BoundBuffer.Clear();

  m_BoundSamplers.Clear();
  // Platforms that do not support immutable samples like DX11 still need them to be bound manually, so they are bound here.
  nsTempHashedString sLinearSampler("LinearSampler");
  for (auto it : nsGALImmutableSamplers::GetImmutableSamplers())
  {
    m_BoundSamplers.Insert(it.Key().GetHash(), it.Value());

    if (it.Key() == sLinearSampler)
    {
      m_hFallbackSampler = it.Value();
    }
  }
  NS_ASSERT_DEBUG(!m_hFallbackSampler.IsInvalidated(), "'LinearSampler' should have been registered as an immutable sampler.");

  m_BoundTextureUAVs.Clear();
  m_BoundBufferUAVs.Clear();
  m_BoundConstantBuffers.Clear();
}

nsGlobalConstants& nsRenderContext::WriteGlobalConstants()
{
  nsConstantBufferStorage<nsGlobalConstants>* pStorage = nullptr;
  NS_VERIFY(TryGetConstantBufferStorage(m_hGlobalConstantBufferStorage, pStorage), "Invalid Global Constant Storage");
  return pStorage->GetDataForWriting();
}

const nsGlobalConstants& nsRenderContext::ReadGlobalConstants() const
{
  nsConstantBufferStorage<nsGlobalConstants>* pStorage = nullptr;
  NS_VERIFY(TryGetConstantBufferStorage(m_hGlobalConstantBufferStorage, pStorage), "Invalid Global Constant Storage");
  return pStorage->GetDataForReading();
}

// static
nsConstantBufferStorageHandle nsRenderContext::CreateConstantBufferStorage(nsUInt32 uiSizeInBytes, nsConstantBufferStorageBase*& out_pStorage)
{
  NS_ASSERT_DEV(nsMemoryUtils::IsSizeAligned(uiSizeInBytes, 16u), "Storage struct for constant buffer is not aligned to 16 bytes");

  NS_LOCK(s_ConstantBufferStorageMutex);

  nsConstantBufferStorageBase* pStorage = nullptr;

  auto it = s_FreeConstantBufferStorage.Find(uiSizeInBytes);
  if (it.IsValid())
  {
    nsDynamicArray<nsConstantBufferStorageBase*>& storageForSize = it.Value();
    if (!storageForSize.IsEmpty())
    {
      pStorage = storageForSize[0];
      storageForSize.RemoveAtAndSwap(0);
    }
  }

  if (pStorage == nullptr)
  {
    pStorage = NS_DEFAULT_NEW(nsConstantBufferStorageBase, uiSizeInBytes);
  }

  out_pStorage = pStorage;
  return nsConstantBufferStorageHandle(s_ConstantBufferStorageTable.Insert(pStorage));
}

// static
void nsRenderContext::DeleteConstantBufferStorage(nsConstantBufferStorageHandle hStorage)
{
  NS_LOCK(s_ConstantBufferStorageMutex);

  nsConstantBufferStorageBase* pStorage = nullptr;
  if (!s_ConstantBufferStorageTable.Remove(hStorage.m_InternalId, &pStorage))
  {
    // already deleted
    return;
  }

  nsUInt32 uiSizeInBytes = pStorage->m_Data.GetCount();

  auto it = s_FreeConstantBufferStorage.Find(uiSizeInBytes);
  if (!it.IsValid())
  {
    it = s_FreeConstantBufferStorage.Insert(uiSizeInBytes, nsDynamicArray<nsConstantBufferStorageBase*>());
  }

  it.Value().PushBack(pStorage);
}

// static
bool nsRenderContext::TryGetConstantBufferStorage(nsConstantBufferStorageHandle hStorage, nsConstantBufferStorageBase*& out_pStorage)
{
  NS_LOCK(s_ConstantBufferStorageMutex);
  return s_ConstantBufferStorageTable.TryGetValue(hStorage.m_InternalId, out_pStorage);
}

// static
nsGALSamplerStateCreationDescription nsRenderContext::GetDefaultSamplerState(nsBitflags<nsDefaultSamplerFlags> flags)
{
  nsGALSamplerStateCreationDescription desc;
  desc.m_MinFilter = flags.IsSet(nsDefaultSamplerFlags::LinearFiltering) ? nsGALTextureFilterMode::Linear : nsGALTextureFilterMode::Point;
  desc.m_MagFilter = flags.IsSet(nsDefaultSamplerFlags::LinearFiltering) ? nsGALTextureFilterMode::Linear : nsGALTextureFilterMode::Point;
  desc.m_MipFilter = flags.IsSet(nsDefaultSamplerFlags::LinearFiltering) ? nsGALTextureFilterMode::Linear : nsGALTextureFilterMode::Point;

  desc.m_AddressU = flags.IsSet(nsDefaultSamplerFlags::Clamp) ? nsImageAddressMode::Clamp : nsImageAddressMode::Repeat;
  desc.m_AddressV = flags.IsSet(nsDefaultSamplerFlags::Clamp) ? nsImageAddressMode::Clamp : nsImageAddressMode::Repeat;
  desc.m_AddressW = flags.IsSet(nsDefaultSamplerFlags::Clamp) ? nsImageAddressMode::Clamp : nsImageAddressMode::Repeat;
  return desc;
}

// private functions
//////////////////////////////////////////////////////////////////////////

// static
void nsRenderContext::LoadBuiltinShader(nsShaderUtils::nsBuiltinShaderType type, nsShaderUtils::nsBuiltinShader& out_shader)
{
  nsShaderResourceHandle hActiveShader;
  bool bStereo = false;
  switch (type)
  {
    case nsShaderUtils::nsBuiltinShaderType::CopyImageArray:
      bStereo = true;
      [[fallthrough]];
    case nsShaderUtils::nsBuiltinShaderType::CopyImage:
      hActiveShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/Copy.nsShader");
      break;
    case nsShaderUtils::nsBuiltinShaderType::DownscaleImageArray:
      bStereo = true;
      [[fallthrough]];
    case nsShaderUtils::nsBuiltinShaderType::DownscaleImage:
      hActiveShader = nsResourceManager::LoadResource<nsShaderResource>("Shaders/Pipeline/Downscale.nsShader");
      break;
  }

  NS_ASSERT_DEV(hActiveShader.IsValid(), "Could not load builtin shader!");

  nsHashTable<nsHashedString, nsHashedString> permutationVariables;
  static nsHashedString sVSRTAI = nsMakeHashedString("VERTEX_SHADER_RENDER_TARGET_ARRAY_INDEX");
  static nsHashedString sTrue = nsMakeHashedString("TRUE");
  static nsHashedString sFalse = nsMakeHashedString("FALSE");
  static nsHashedString sCameraMode = nsMakeHashedString("CAMERA_MODE");
  static nsHashedString sPerspective = nsMakeHashedString("CAMERA_MODE_PERSPECTIVE");
  static nsHashedString sStereo = nsMakeHashedString("CAMERA_MODE_STEREO");

  permutationVariables.Insert(sCameraMode, bStereo ? sStereo : sPerspective);
  if (nsGALDevice::GetDefaultDevice()->GetCapabilities().m_bVertexShaderRenderTargetArrayIndex)
    permutationVariables.Insert(sVSRTAI, sTrue);
  else
    permutationVariables.Insert(sVSRTAI, sFalse);


  nsShaderPermutationResourceHandle hActiveShaderPermutation = nsShaderManager::PreloadSinglePermutation(hActiveShader, permutationVariables, false);

  NS_ASSERT_DEV(hActiveShaderPermutation.IsValid(), "Could not load builtin shader permutation!");

  nsResourceLock<nsShaderPermutationResource> pShaderPermutation(hActiveShaderPermutation, nsResourceAcquireMode::BlockTillLoaded);

  NS_ASSERT_DEV(pShaderPermutation->IsShaderValid(), "Builtin shader permutation shader is invalid!");

  out_shader.m_hActiveGALShader = pShaderPermutation->GetGALShader();
  NS_ASSERT_DEV(!out_shader.m_hActiveGALShader.IsInvalidated(), "Invalid GAL Shader handle.");

  out_shader.m_hBlendState = pShaderPermutation->GetBlendState();
  out_shader.m_hDepthStencilState = pShaderPermutation->GetDepthStencilState();
  out_shader.m_hRasterizerState = pShaderPermutation->GetRasterizerState();
}

// static
void nsRenderContext::RegisterImmutableSamplers()
{
  nsGALImmutableSamplers::RegisterImmutableSampler(nsMakeHashedString("LinearSampler"), GetDefaultSamplerState(nsDefaultSamplerFlags::LinearFiltering)).AssertSuccess("Failed to register immutable sampler");
  nsGALImmutableSamplers::RegisterImmutableSampler(nsMakeHashedString("LinearClampSampler"), GetDefaultSamplerState(nsDefaultSamplerFlags::LinearFiltering | nsDefaultSamplerFlags::Clamp)).AssertSuccess("Failed to register immutable sampler");
  nsGALImmutableSamplers::RegisterImmutableSampler(nsMakeHashedString("PointSampler"), GetDefaultSamplerState(nsDefaultSamplerFlags::PointFiltering)).AssertSuccess("Failed to register immutable sampler");
  nsGALImmutableSamplers::RegisterImmutableSampler(nsMakeHashedString("PointClampSampler"), GetDefaultSamplerState(nsDefaultSamplerFlags::PointFiltering | nsDefaultSamplerFlags::Clamp)).AssertSuccess("Failed to register immutable sampler");
}

// static
void nsRenderContext::OnEngineStartup()
{
  nsShaderUtils::g_RequestBuiltinShaderCallback = nsMakeDelegate(nsRenderContext::LoadBuiltinShader);
}

// static
void nsRenderContext::OnEngineShutdown()
{
  nsShaderUtils::g_RequestBuiltinShaderCallback = {};
  nsShaderStageBinary::OnEngineShutdown();

  for (auto rc : s_Instances)
    NS_DEFAULT_DELETE(rc);

  s_Instances.Clear();

  // Cleanup vertex declarations
  {
    for (auto it = s_GALVertexDeclarations.GetIterator(); it.IsValid(); ++it)
    {
      nsGALDevice::GetDefaultDevice()->DestroyVertexDeclaration(it.Value());
    }

    s_GALVertexDeclarations.Clear();
  }

  // Cleanup constant buffer storage
  {
    for (auto it = s_ConstantBufferStorageTable.GetIterator(); it.IsValid(); ++it)
    {
      nsConstantBufferStorageBase* pStorage = it.Value();
      NS_DEFAULT_DELETE(pStorage);
    }

    s_ConstantBufferStorageTable.Clear();

    for (auto it = s_FreeConstantBufferStorage.GetIterator(); it.IsValid(); ++it)
    {
      nsDynamicArray<nsConstantBufferStorageBase*>& storageForSize = it.Value();
      for (auto& pStorage : storageForSize)
      {
        NS_DEFAULT_DELETE(pStorage);
      }
    }

    s_FreeConstantBufferStorage.Clear();
  }
}

// static
nsResult nsRenderContext::BuildVertexDeclaration(nsGALShaderHandle hShader, const nsVertexDeclarationInfo& decl, nsGALVertexDeclarationHandle& out_Declaration)
{
  ShaderVertexDecl svd;
  svd.m_hShader = hShader;
  svd.m_uiVertexDeclarationHash = decl.m_uiHash;

  bool bExisted = false;
  auto it = s_GALVertexDeclarations.FindOrAdd(svd, &bExisted);

  if (!bExisted)
  {
    const nsGALShader* pShader = nsGALDevice::GetDefaultDevice()->GetShader(hShader);

    auto pBytecode = pShader->GetDescription().m_ByteCodes[nsGALShaderStage::VertexShader];

    nsGALVertexDeclarationCreationDescription vd;
    vd.m_hShader = hShader;

    for (nsUInt32 slot = 0; slot < decl.m_VertexStreams.GetCount(); ++slot)
    {
      auto& stream = decl.m_VertexStreams[slot];

      // stream.m_Format
      nsGALVertexAttribute gal;
      gal.m_bInstanceData = false;
      gal.m_eFormat = stream.m_Format;
      gal.m_eSemantic = stream.m_Semantic;
      gal.m_uiOffset = stream.m_uiOffset;
      gal.m_uiVertexBufferSlot = stream.m_uiVertexBufferSlot;
      vd.m_VertexAttributes.PushBack(gal);
    }

    out_Declaration = nsGALDevice::GetDefaultDevice()->CreateVertexDeclaration(vd);

    if (out_Declaration.IsInvalidated())
    {
      /* This can happen when the resource system gives you a fallback resource, which then selects a shader that
      does not fit the mesh layout.
      E.g. when a material is not yet loaded and the fallback material is used, that fallback material may
      use another shader, that requires more data streams, than what the mesh provides.
      This problem will go away, once the proper material is loaded.

      This can be fixed by ensuring that the fallback material uses a shader that only requires data that is
      always there, e.g. only position and maybe a texcoord, and of course all meshes must provide at least those
      data streams.

      Otherwise, this is harmless, the renderer will ignore invalid drawcalls and once all the correct stuff is
      available, it will work.
      */

      nsLog::Warning("Failed to create vertex declaration");
      return NS_FAILURE;
    }

    it.Value() = out_Declaration;
  }

  out_Declaration = it.Value();
  return NS_SUCCESS;
}

void nsRenderContext::UploadConstants()
{
  BindConstantBuffer("nsGlobalConstants", m_hGlobalConstantBufferStorage);

  for (auto it = m_BoundConstantBuffers.GetIterator(); it.IsValid(); ++it)
  {
    nsConstantBufferStorageHandle hConstantBufferStorage = it.Value().m_hConstantBufferStorage;
    nsConstantBufferStorageBase* pConstantBufferStorage = nullptr;
    if (TryGetConstantBufferStorage(hConstantBufferStorage, pConstantBufferStorage))
    {
      pConstantBufferStorage->UploadData(m_pGALCommandEncoder);
    }
  }
}

void nsRenderContext::SetShaderPermutationVariableInternal(const nsHashedString& sName, const nsHashedString& sValue)
{
  nsHashedString* pOldValue = nullptr;
  m_PermutationVariables.TryGetValue(sName, pOldValue);

  if (pOldValue == nullptr || *pOldValue != sValue)
  {
    m_PermutationVariables.Insert(sName, sValue);
    m_StateFlags.Add(nsRenderContextFlags::ShaderStateChanged);
  }
}

void nsRenderContext::BindShaderInternal(const nsShaderResourceHandle& hShader, nsBitflags<nsShaderBindFlags> flags)
{
  if (flags.IsAnySet(nsShaderBindFlags::ForceRebind) || m_hActiveShader != hShader)
  {
    m_ShaderBindFlags = flags;
    m_hActiveShader = hShader;

    m_StateFlags.Add(nsRenderContextFlags::ShaderStateChanged);
  }
}

nsShaderPermutationResource* nsRenderContext::ApplyShaderState()
{
  m_hActiveGALShader.Invalidate();

  m_StateFlags.Add(nsRenderContextFlags::TextureBindingChanged | nsRenderContextFlags::SamplerBindingChanged |
                   nsRenderContextFlags::BufferBindingChanged | nsRenderContextFlags::ConstantBufferBindingChanged);

  if (!m_hActiveShader.IsValid())
    return nullptr;

  m_hActiveShaderPermutation = nsShaderManager::PreloadSinglePermutation(m_hActiveShader, m_PermutationVariables, m_bAllowAsyncShaderLoading);

  if (!m_hActiveShaderPermutation.IsValid())
    return nullptr;

  nsShaderPermutationResource* pShaderPermutation = nsResourceManager::BeginAcquireResource(
    m_hActiveShaderPermutation, m_bAllowAsyncShaderLoading ? nsResourceAcquireMode::AllowLoadingFallback : nsResourceAcquireMode::BlockTillLoaded);

  if (!pShaderPermutation->IsShaderValid())
  {
    nsResourceManager::EndAcquireResource(pShaderPermutation);
    return nullptr;
  }

  m_hActiveGALShader = pShaderPermutation->GetGALShader();
  NS_ASSERT_DEV(!m_hActiveGALShader.IsInvalidated(), "Invalid GAL Shader handle.");

  m_pGALCommandEncoder->SetShader(m_hActiveGALShader);

  // Set render state from shader
  if (!m_bCompute)
  {
    auto pCommandEncoder = GetRenderCommandEncoder();

    if (!m_ShaderBindFlags.IsSet(nsShaderBindFlags::NoBlendState))
      pCommandEncoder->SetBlendState(pShaderPermutation->GetBlendState());

    if (!m_ShaderBindFlags.IsSet(nsShaderBindFlags::NoRasterizerState))
      pCommandEncoder->SetRasterizerState(pShaderPermutation->GetRasterizerState());

    if (!m_ShaderBindFlags.IsSet(nsShaderBindFlags::NoDepthStencilState))
      pCommandEncoder->SetDepthStencilState(pShaderPermutation->GetDepthStencilState());
  }

  return pShaderPermutation;
}

nsMaterialResource* nsRenderContext::ApplyMaterialState()
{
  if (!m_hNewMaterial.IsValid())
  {
    BindShaderInternal(nsShaderResourceHandle(), nsShaderBindFlags::Default);
    return nullptr;
  }

  // check whether material has been modified
  nsMaterialResource* pMaterial = nsResourceManager::BeginAcquireResource(m_hNewMaterial, nsResourceAcquireMode::AllowLoadingFallback);

  if (m_hNewMaterial != m_hMaterial || pMaterial->IsModified())
  {
    auto pCachedValues = pMaterial->GetOrUpdateCachedValues();

    BindShaderInternal(pCachedValues->m_hShader, nsShaderBindFlags::Default);

    if (!pMaterial->m_hConstantBufferStorage.IsInvalidated())
    {
      BindConstantBuffer("nsMaterialConstants", pMaterial->m_hConstantBufferStorage);
    }

    for (auto it = pCachedValues->m_PermutationVars.GetIterator(); it.IsValid(); ++it)
    {
      SetShaderPermutationVariableInternal(it.Key(), it.Value());
    }

    for (auto it = pCachedValues->m_Texture2DBindings.GetIterator(); it.IsValid(); ++it)
    {
      BindTexture2D(it.Key(), it.Value());
    }

    for (auto it = pCachedValues->m_TextureCubeBindings.GetIterator(); it.IsValid(); ++it)
    {
      BindTextureCube(it.Key(), it.Value());
    }

    m_hMaterial = m_hNewMaterial;
  }

  // The material needs its constant buffer updated.
  // Thus, we keep it acquired until we have the correct shader permutation for the constant buffer layout.
  if (pMaterial->AreConstantsModified())
  {
    m_StateFlags.Add(nsRenderContextFlags::ConstantBufferBindingChanged);

    return pMaterial;
  }

  nsResourceManager::EndAcquireResource(pMaterial);
  return nullptr;
}

void nsRenderContext::ApplyConstantBufferBindings(const nsGALShader* pShader)
{
  const auto& bindings = pShader->GetBindingMapping();
  for (const nsShaderResourceBinding& binding : bindings)
  {
    if (binding.m_ResourceType != nsGALShaderResourceType::ConstantBuffer)
      continue;

    const nsUInt64 uiResourceHash = binding.m_sName.GetHash();

    BoundConstantBuffer boundConstantBuffer;
    if (!m_BoundConstantBuffers.TryGetValue(uiResourceHash, boundConstantBuffer))
    {
      // If the shader was compiled with debug info the shader compiler will not strip unused resources and
      // thus this error would trigger although the shader doesn't actually use the resource.
      // #TODO_SHADER if (!pBinary->GetByteCode()->m_bWasCompiledWithDebug)
      {
        nsLog::Error("No resource is bound for constant buffer slot '{0}'", binding.m_sName);
      }
      m_pGALCommandEncoder->SetConstantBuffer(binding, nsGALBufferHandle());
      continue;
    }

    if (!boundConstantBuffer.m_hConstantBuffer.IsInvalidated())
    {
      m_pGALCommandEncoder->SetConstantBuffer(binding, boundConstantBuffer.m_hConstantBuffer);
    }
    else
    {
      nsConstantBufferStorageBase* pConstantBufferStorage = nullptr;
      if (TryGetConstantBufferStorage(boundConstantBuffer.m_hConstantBufferStorage, pConstantBufferStorage))
      {
        m_pGALCommandEncoder->SetConstantBuffer(binding, pConstantBufferStorage->GetGALBufferHandle());
      }
      else
      {
        nsLog::Error("Invalid constant buffer storage is bound for slot '{0}'", binding.m_sName);
        m_pGALCommandEncoder->SetConstantBuffer(binding, nsGALBufferHandle());
      }
    }
  }
}

void nsRenderContext::ApplyTextureBindings(const nsGALShader* pShader)
{
  const auto& bindings = pShader->GetBindingMapping();
  for (const nsShaderResourceBinding& binding : bindings)
  {
    const nsUInt64 uiResourceHash = binding.m_sName.GetHash();
    nsGALTextureResourceViewHandle hResourceView;

    if (binding.m_ResourceType == nsGALShaderResourceType::Texture || binding.m_ResourceType == nsGALShaderResourceType::TextureAndSampler)
    {
      switch (binding.m_TextureType)
      {
        case nsGALShaderTextureType::Texture2D:
        case nsGALShaderTextureType::Texture2DArray:
        case nsGALShaderTextureType::Texture2DMS:
        case nsGALShaderTextureType::Texture2DMSArray:
          m_BoundTextures2D.TryGetValue(uiResourceHash, hResourceView);
          m_pGALCommandEncoder->SetResourceView(binding, hResourceView);
          break;
        case nsGALShaderTextureType::Texture3D:
          m_BoundTextures3D.TryGetValue(uiResourceHash, hResourceView);
          m_pGALCommandEncoder->SetResourceView(binding, hResourceView);
          break;
        case nsGALShaderTextureType::TextureCube:
        case nsGALShaderTextureType::TextureCubeArray:
          m_BoundTexturesCube.TryGetValue(uiResourceHash, hResourceView);
          m_pGALCommandEncoder->SetResourceView(binding, hResourceView);
          break;
        case nsGALShaderTextureType::Texture1D:
        case nsGALShaderTextureType::Texture1DArray:
        default:
          NS_ASSERT_NOT_IMPLEMENTED;
          break;
      }
    }
  }
}

void nsRenderContext::ApplyUAVBindings(const nsGALShader* pShader)
{
  const auto& bindings = pShader->GetBindingMapping();
  for (const nsShaderResourceBinding& binding : bindings)
  {
    auto type = nsGALShaderResourceCategory::MakeFromShaderDescriptorType(binding.m_ResourceType);
    if (type.IsSet(nsGALShaderResourceCategory::TextureUAV))
    {
      const nsUInt64 uiResourceHash = binding.m_sName.GetHash();
      nsGALTextureUnorderedAccessViewHandle hResourceView;
      m_BoundTextureUAVs.TryGetValue(uiResourceHash, hResourceView);
      m_pGALCommandEncoder->SetUnorderedAccessView(binding, hResourceView);
    }
    else if (type.IsSet(nsGALShaderResourceCategory::BufferUAV))
    {
      const nsUInt64 uiResourceHash = binding.m_sName.GetHash();
      nsGALBufferUnorderedAccessViewHandle hResourceView;
      m_BoundBufferUAVs.TryGetValue(uiResourceHash, hResourceView);
      m_pGALCommandEncoder->SetUnorderedAccessView(binding, hResourceView);
    }
  }
}

void nsRenderContext::ApplySamplerBindings(const nsGALShader* pShader)
{
  const auto& bindings = pShader->GetBindingMapping();
  for (const nsShaderResourceBinding& binding : bindings)
  {
    auto type = nsGALShaderResourceCategory::MakeFromShaderDescriptorType(binding.m_ResourceType);
    if (type.IsSet(nsGALShaderResourceCategory::Sampler))
    {
      const nsUInt64 uiResourceHash = binding.m_sName.GetHash();

      nsGALSamplerStateHandle hSamplerState;
      if (!m_BoundSamplers.TryGetValue(uiResourceHash, hSamplerState))
      {
        // Fallback in case no sampler was set.
        hSamplerState = m_hFallbackSampler;
      }

      m_pGALCommandEncoder->SetSamplerState(binding, hSamplerState);
    }
  }
}

void nsRenderContext::ApplyBufferBindings(const nsGALShader* pShader)
{
  const auto& bindings = pShader->GetBindingMapping();
  for (const nsShaderResourceBinding& binding : bindings)
  {
    if (binding.m_ResourceType == nsGALShaderResourceType::TexelBuffer || binding.m_ResourceType == nsGALShaderResourceType::StructuredBuffer)
    {
      const nsUInt64 uiResourceHash = binding.m_sName.GetHash();

      nsGALBufferResourceViewHandle hResourceView;
      m_BoundBuffer.TryGetValue(uiResourceHash, hResourceView);

      m_pGALCommandEncoder->SetResourceView(binding, hResourceView);
    }
  }
}

void nsRenderContext::SetDefaultTextureFilter(nsTextureFilterSetting::Enum filter)
{
  NS_ASSERT_DEBUG(
    filter >= nsTextureFilterSetting::FixedBilinear && filter <= nsTextureFilterSetting::FixedAnisotropic16x, "Invalid default texture filter");
  filter = nsMath::Clamp(filter, nsTextureFilterSetting::FixedBilinear, nsTextureFilterSetting::FixedAnisotropic16x);

  if (m_DefaultTextureFilter == filter)
    return;

  m_DefaultTextureFilter = filter;
}

nsTextureFilterSetting::Enum nsRenderContext::GetSpecificTextureFilter(nsTextureFilterSetting::Enum configuration) const
{
  if (configuration >= nsTextureFilterSetting::FixedNearest && configuration <= nsTextureFilterSetting::FixedAnisotropic16x)
    return configuration;

  int iFilter = m_DefaultTextureFilter;

  switch (configuration)
  {
    case nsTextureFilterSetting::LowestQuality:
      iFilter -= 2;
      break;
    case nsTextureFilterSetting::LowQuality:
      iFilter -= 1;
      break;
    case nsTextureFilterSetting::HighQuality:
      iFilter += 1;
      break;
    case nsTextureFilterSetting::HighestQuality:
      iFilter += 2;
      break;
    default:
      break;
  }

  iFilter = nsMath::Clamp<int>(iFilter, nsTextureFilterSetting::FixedBilinear, nsTextureFilterSetting::FixedAnisotropic16x);

  return (nsTextureFilterSetting::Enum)iFilter;
}

void nsRenderContext::SetAllowAsyncShaderLoading(bool bAllow)
{
  m_bAllowAsyncShaderLoading = bAllow;
}

bool nsRenderContext::GetAllowAsyncShaderLoading()
{
  return m_bAllowAsyncShaderLoading;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_RenderContext_Implementation_RenderContext);
