
/// \brief Used to guard nsGALDevice functions from multi-threaded access and to verify that executing them on non-main-threads is allowed
#define NS_GALDEVICE_LOCK_AND_CHECK() \
  NS_LOCK(m_Mutex);                   \
  VerifyMultithreadedAccess()

NS_ALWAYS_INLINE const nsGALDeviceCreationDescription* nsGALDevice::GetDescription() const
{
  return &m_Description;
}

NS_ALWAYS_INLINE nsResult nsGALDevice::GetTimestampResult(nsGALTimestampHandle hTimestamp, nsTime& ref_result)
{
  return GetTimestampResultPlatform(hTimestamp, ref_result);
}

NS_ALWAYS_INLINE nsGALTimestampHandle nsGALDevice::GetTimestamp()
{
  return GetTimestampPlatform();
}

template <typename IdTableType, typename ReturnType>
NS_ALWAYS_INLINE ReturnType* nsGALDevice::Get(typename IdTableType::TypeOfId hHandle, const IdTableType& IdTable) const
{
  NS_GALDEVICE_LOCK_AND_CHECK();

  ReturnType* pObject = nullptr;
  IdTable.TryGetValue(hHandle, pObject);
  return pObject;
}

inline const nsGALSwapChain* nsGALDevice::GetSwapChain(nsGALSwapChainHandle hSwapChain) const
{
  return Get<SwapChainTable, nsGALSwapChain>(hSwapChain, m_SwapChains);
}

inline const nsGALShader* nsGALDevice::GetShader(nsGALShaderHandle hShader) const
{
  return Get<ShaderTable, nsGALShader>(hShader, m_Shaders);
}

inline const nsGALTexture* nsGALDevice::GetTexture(nsGALTextureHandle hTexture) const
{
  return Get<TextureTable, nsGALTexture>(hTexture, m_Textures);
}

inline const nsGALBuffer* nsGALDevice::GetBuffer(nsGALBufferHandle hBuffer) const
{
  return Get<BufferTable, nsGALBuffer>(hBuffer, m_Buffers);
}

inline const nsGALDepthStencilState* nsGALDevice::GetDepthStencilState(nsGALDepthStencilStateHandle hDepthStencilState) const
{
  return Get<DepthStencilStateTable, nsGALDepthStencilState>(hDepthStencilState, m_DepthStencilStates);
}

inline const nsGALBlendState* nsGALDevice::GetBlendState(nsGALBlendStateHandle hBlendState) const
{
  return Get<BlendStateTable, nsGALBlendState>(hBlendState, m_BlendStates);
}

inline const nsGALRasterizerState* nsGALDevice::GetRasterizerState(nsGALRasterizerStateHandle hRasterizerState) const
{
  return Get<RasterizerStateTable, nsGALRasterizerState>(hRasterizerState, m_RasterizerStates);
}

inline const nsGALVertexDeclaration* nsGALDevice::GetVertexDeclaration(nsGALVertexDeclarationHandle hVertexDeclaration) const
{
  return Get<VertexDeclarationTable, nsGALVertexDeclaration>(hVertexDeclaration, m_VertexDeclarations);
}

inline const nsGALSamplerState* nsGALDevice::GetSamplerState(nsGALSamplerStateHandle hSamplerState) const
{
  return Get<SamplerStateTable, nsGALSamplerState>(hSamplerState, m_SamplerStates);
}

inline const nsGALTextureResourceView* nsGALDevice::GetResourceView(nsGALTextureResourceViewHandle hResourceView) const
{
  return Get<TextureResourceViewTable, nsGALTextureResourceView>(hResourceView, m_TextureResourceViews);
}

inline const nsGALBufferResourceView* nsGALDevice::GetResourceView(nsGALBufferResourceViewHandle hResourceView) const
{
  return Get<BufferResourceViewTable, nsGALBufferResourceView>(hResourceView, m_BufferResourceViews);
}

inline const nsGALRenderTargetView* nsGALDevice::GetRenderTargetView(nsGALRenderTargetViewHandle hRenderTargetView) const
{
  return Get<RenderTargetViewTable, nsGALRenderTargetView>(hRenderTargetView, m_RenderTargetViews);
}

inline const nsGALTextureUnorderedAccessView* nsGALDevice::GetUnorderedAccessView(nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView) const
{
  return Get<TextureUnorderedAccessViewTable, nsGALTextureUnorderedAccessView>(hUnorderedAccessView, m_TextureUnorderedAccessViews);
}

inline const nsGALBufferUnorderedAccessView* nsGALDevice::GetUnorderedAccessView(nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView) const
{
  return Get<BufferUnorderedAccessViewTable, nsGALBufferUnorderedAccessView>(hUnorderedAccessView, m_BufferUnorderedAccessViews);
}

inline const nsGALQuery* nsGALDevice::GetQuery(nsGALQueryHandle hQuery) const
{
  return Get<QueryTable, nsGALQuery>(hQuery, m_Queries);
}

// static
NS_ALWAYS_INLINE void nsGALDevice::SetDefaultDevice(nsGALDevice* pDefaultDevice)
{
  s_pDefaultDevice = pDefaultDevice;
}

// static
NS_ALWAYS_INLINE nsGALDevice* nsGALDevice::GetDefaultDevice()
{
  NS_ASSERT_DEBUG(s_pDefaultDevice != nullptr, "Default device not set.");
  return s_pDefaultDevice;
}

// static
NS_ALWAYS_INLINE bool nsGALDevice::HasDefaultDevice()
{
  return s_pDefaultDevice != nullptr;
}

template <typename HandleType>
NS_FORCE_INLINE void nsGALDevice::AddDeadObject(nsUInt32 uiType, HandleType handle)
{
  auto& deadObject = m_DeadObjects.ExpandAndGetRef();
  deadObject.m_uiType = uiType;
  deadObject.m_uiHandle = handle.GetInternalID().m_Data;
}

template <typename HandleType>
void nsGALDevice::ReviveDeadObject(nsUInt32 uiType, HandleType handle)
{
  nsUInt32 uiHandle = handle.GetInternalID().m_Data;

  for (nsUInt32 i = 0; i < m_DeadObjects.GetCount(); ++i)
  {
    const auto& deadObject = m_DeadObjects[i];

    if (deadObject.m_uiType == uiType && deadObject.m_uiHandle == uiHandle)
    {
      m_DeadObjects.RemoveAtAndCopy(i);
      return;
    }
  }
}

NS_ALWAYS_INLINE void nsGALDevice::VerifyMultithreadedAccess() const
{
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  NS_ASSERT_DEV(m_Capabilities.m_bMultithreadedResourceCreation || nsThreadUtils::IsMainThread(),
    "This device does not support multi-threaded resource creation, therefore this function can only be executed on the main thread.");
#endif
}
