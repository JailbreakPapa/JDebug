
#pragma once

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/IdTable.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Strings/HashedString.h>
#include <RendererFoundation/Descriptors/Descriptors.h>
#include <RendererFoundation/Device/DeviceCapabilities.h>
#include <RendererFoundation/RendererFoundationDLL.h>

class nsColor;

/// \brief The nsRenderDevice class is the primary interface for interactions with rendering APIs
/// It contains a set of (non-virtual) functions to set state, create resources etc. which rely on
/// API specific implementations provided by protected virtual functions.
/// Redundant state changes are prevented at the platform independent level in the non-virtual functions.
class NS_RENDERERFOUNDATION_DLL nsGALDevice
{
public:
  static nsEvent<const nsGALDeviceEvent&> s_Events;

  // Init & shutdown functions

  nsResult Init();
  nsResult Shutdown();
  nsStringView GetRenderer();

  // Pipeline & Pass functions

  void BeginPipeline(const char* szName, nsGALSwapChainHandle hSwapChain);
  void EndPipeline(nsGALSwapChainHandle hSwapChain);

  nsGALPass* BeginPass(const char* szName);
  void EndPass(nsGALPass* pPass);

  // State creation functions

  nsGALBlendStateHandle CreateBlendState(const nsGALBlendStateCreationDescription& description);
  void DestroyBlendState(nsGALBlendStateHandle hBlendState);

  nsGALDepthStencilStateHandle CreateDepthStencilState(const nsGALDepthStencilStateCreationDescription& description);
  void DestroyDepthStencilState(nsGALDepthStencilStateHandle hDepthStencilState);

  nsGALRasterizerStateHandle CreateRasterizerState(const nsGALRasterizerStateCreationDescription& description);
  void DestroyRasterizerState(nsGALRasterizerStateHandle hRasterizerState);

  nsGALSamplerStateHandle CreateSamplerState(const nsGALSamplerStateCreationDescription& description);
  void DestroySamplerState(nsGALSamplerStateHandle hSamplerState);

  // Resource creation functions

  nsGALShaderHandle CreateShader(const nsGALShaderCreationDescription& description);
  void DestroyShader(nsGALShaderHandle hShader);

  nsGALBufferHandle CreateBuffer(const nsGALBufferCreationDescription& description, nsArrayPtr<const nsUInt8> initialData = nsArrayPtr<const nsUInt8>());
  void DestroyBuffer(nsGALBufferHandle hBuffer);

  // Helper functions for buffers (for common, simple use cases)

  nsGALBufferHandle CreateVertexBuffer(nsUInt32 uiVertexSize, nsUInt32 uiVertexCount, nsArrayPtr<const nsUInt8> initialData = nsArrayPtr<const nsUInt8>(), bool bDataIsMutable = false);
  nsGALBufferHandle CreateIndexBuffer(nsGALIndexType::Enum indexType, nsUInt32 uiIndexCount, nsArrayPtr<const nsUInt8> initialData = nsArrayPtr<const nsUInt8>(), bool bDataIsMutable = false);
  nsGALBufferHandle CreateConstantBuffer(nsUInt32 uiBufferSize);

  nsGALTextureHandle CreateTexture(const nsGALTextureCreationDescription& description, nsArrayPtr<nsGALSystemMemoryDescription> initialData = nsArrayPtr<nsGALSystemMemoryDescription>());
  void DestroyTexture(nsGALTextureHandle hTexture);

  nsGALTextureHandle CreateProxyTexture(nsGALTextureHandle hParentTexture, nsUInt32 uiSlice);
  void DestroyProxyTexture(nsGALTextureHandle hProxyTexture);

  nsGALTextureHandle CreateSharedTexture(const nsGALTextureCreationDescription& description, nsArrayPtr<nsGALSystemMemoryDescription> initialData = {});
  nsGALTextureHandle OpenSharedTexture(const nsGALTextureCreationDescription& description, nsGALPlatformSharedHandle hSharedHandle);
  void DestroySharedTexture(nsGALTextureHandle hTexture);

  // Resource views
  nsGALTextureResourceViewHandle GetDefaultResourceView(nsGALTextureHandle hTexture);
  nsGALBufferResourceViewHandle GetDefaultResourceView(nsGALBufferHandle hBuffer);

  nsGALTextureResourceViewHandle CreateResourceView(const nsGALTextureResourceViewCreationDescription& description);
  void DestroyResourceView(nsGALTextureResourceViewHandle hResourceView);

  nsGALBufferResourceViewHandle CreateResourceView(const nsGALBufferResourceViewCreationDescription& description);
  void DestroyResourceView(nsGALBufferResourceViewHandle hResourceView);

  // Render target views
  nsGALRenderTargetViewHandle GetDefaultRenderTargetView(nsGALTextureHandle hTexture);

  nsGALRenderTargetViewHandle CreateRenderTargetView(const nsGALRenderTargetViewCreationDescription& description);
  void DestroyRenderTargetView(nsGALRenderTargetViewHandle hRenderTargetView);

  // Unordered access views
  nsGALTextureUnorderedAccessViewHandle CreateUnorderedAccessView(const nsGALTextureUnorderedAccessViewCreationDescription& description);
  void DestroyUnorderedAccessView(nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView);

  nsGALBufferUnorderedAccessViewHandle CreateUnorderedAccessView(const nsGALBufferUnorderedAccessViewCreationDescription& description);
  void DestroyUnorderedAccessView(nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView);

  // Other rendering creation functions

  using SwapChainFactoryFunction = nsDelegate<nsGALSwapChain*(nsAllocator*)>;
  nsGALSwapChainHandle CreateSwapChain(const SwapChainFactoryFunction& func);
  nsResult UpdateSwapChain(nsGALSwapChainHandle hSwapChain, nsEnum<nsGALPresentMode> newPresentMode);
  void DestroySwapChain(nsGALSwapChainHandle hSwapChain);

  nsGALQueryHandle CreateQuery(const nsGALQueryCreationDescription& description);
  void DestroyQuery(nsGALQueryHandle hQuery);

  nsGALVertexDeclarationHandle CreateVertexDeclaration(const nsGALVertexDeclarationCreationDescription& description);
  void DestroyVertexDeclaration(nsGALVertexDeclarationHandle hVertexDeclaration);

  // Timestamp functions

  nsResult GetTimestampResult(nsGALTimestampHandle hTimestamp, nsTime& ref_result);

  /// \todo Map functions to save on memcpys

  // Swap chain functions

  nsGALTextureHandle GetBackBufferTextureFromSwapChain(nsGALSwapChainHandle hSwapChain);


  // Misc functions

  void BeginFrame(const nsUInt64 uiRenderFrame = 0);
  void EndFrame();

  nsGALTimestampHandle GetTimestamp();

  const nsGALDeviceCreationDescription* GetDescription() const;

  const nsGALSwapChain* GetSwapChain(nsGALSwapChainHandle hSwapChain) const;
  template <typename T>
  const T* GetSwapChain(nsGALSwapChainHandle hSwapChain) const
  {
    return static_cast<const T*>(GetSwapChainInternal(hSwapChain, nsGetStaticRTTI<T>()));
  }

  const nsGALShader* GetShader(nsGALShaderHandle hShader) const;
  const nsGALTexture* GetTexture(nsGALTextureHandle hTexture) const;
  virtual const nsGALSharedTexture* GetSharedTexture(nsGALTextureHandle hTexture) const = 0;
  const nsGALBuffer* GetBuffer(nsGALBufferHandle hBuffer) const;
  const nsGALDepthStencilState* GetDepthStencilState(nsGALDepthStencilStateHandle hDepthStencilState) const;
  const nsGALBlendState* GetBlendState(nsGALBlendStateHandle hBlendState) const;
  const nsGALRasterizerState* GetRasterizerState(nsGALRasterizerStateHandle hRasterizerState) const;
  const nsGALVertexDeclaration* GetVertexDeclaration(nsGALVertexDeclarationHandle hVertexDeclaration) const;
  const nsGALSamplerState* GetSamplerState(nsGALSamplerStateHandle hSamplerState) const;
  const nsGALTextureResourceView* GetResourceView(nsGALTextureResourceViewHandle hResourceView) const;
  const nsGALBufferResourceView* GetResourceView(nsGALBufferResourceViewHandle hResourceView) const;
  const nsGALRenderTargetView* GetRenderTargetView(nsGALRenderTargetViewHandle hRenderTargetView) const;
  const nsGALTextureUnorderedAccessView* GetUnorderedAccessView(nsGALTextureUnorderedAccessViewHandle hUnorderedAccessView) const;
  const nsGALBufferUnorderedAccessView* GetUnorderedAccessView(nsGALBufferUnorderedAccessViewHandle hUnorderedAccessView) const;
  const nsGALQuery* GetQuery(nsGALQueryHandle hQuery) const;

  const nsGALDeviceCapabilities& GetCapabilities() const;

  virtual nsUInt64 GetMemoryConsumptionForTexture(const nsGALTextureCreationDescription& description) const;
  virtual nsUInt64 GetMemoryConsumptionForBuffer(const nsGALBufferCreationDescription& description) const;

  static void SetDefaultDevice(nsGALDevice* pDefaultDevice);
  static nsGALDevice* GetDefaultDevice();
  static bool HasDefaultDevice();

  // Sends the queued up commands to the GPU
  void Flush();
  /// \brief Waits for the GPU to be idle and destroys any pending resources and GPU objects.
  void WaitIdle();

  // public in case someone external needs to lock multiple operations
  mutable nsMutex m_Mutex;

private:
  static nsGALDevice* s_pDefaultDevice;

protected:
  nsGALDevice(const nsGALDeviceCreationDescription& Description);

  virtual ~nsGALDevice();

  template <typename IdTableType, typename ReturnType>
  ReturnType* Get(typename IdTableType::TypeOfId hHandle, const IdTableType& IdTable) const;

  void DestroyViews(nsGALTexture* pResource);
  void DestroyViews(nsGALBuffer* pResource);

  template <typename HandleType>
  void AddDeadObject(nsUInt32 uiType, HandleType handle);

  template <typename HandleType>
  void ReviveDeadObject(nsUInt32 uiType, HandleType handle);

  void DestroyDeadObjects();

  /// \brief Asserts that either this device supports multi-threaded resource creation, or that this function is executed on the main thread.
  void VerifyMultithreadedAccess() const;

  const nsGALSwapChain* GetSwapChainInternal(nsGALSwapChainHandle hSwapChain, const nsRTTI* pRequestedType) const;

  nsGALTextureHandle FinalizeTextureInternal(const nsGALTextureCreationDescription& desc, nsGALTexture* pTexture);
  nsGALBufferHandle FinalizeBufferInternal(const nsGALBufferCreationDescription& desc, nsGALBuffer* pBuffer);

  nsProxyAllocator m_Allocator;
  nsLocalAllocatorWrapper m_AllocatorWrapper;

  using ShaderTable = nsIdTable<nsGALShaderHandle::IdType, nsGALShader*, nsLocalAllocatorWrapper>;
  using BlendStateTable = nsIdTable<nsGALBlendStateHandle::IdType, nsGALBlendState*, nsLocalAllocatorWrapper>;
  using DepthStencilStateTable = nsIdTable<nsGALDepthStencilStateHandle::IdType, nsGALDepthStencilState*, nsLocalAllocatorWrapper>;
  using RasterizerStateTable = nsIdTable<nsGALRasterizerStateHandle::IdType, nsGALRasterizerState*, nsLocalAllocatorWrapper>;
  using BufferTable = nsIdTable<nsGALBufferHandle::IdType, nsGALBuffer*, nsLocalAllocatorWrapper>;
  using TextureTable = nsIdTable<nsGALTextureHandle::IdType, nsGALTexture*, nsLocalAllocatorWrapper>;
  using TextureResourceViewTable = nsIdTable<nsGALTextureResourceViewHandle::IdType, nsGALTextureResourceView*, nsLocalAllocatorWrapper>;
  using BufferResourceViewTable = nsIdTable<nsGALBufferResourceViewHandle::IdType, nsGALBufferResourceView*, nsLocalAllocatorWrapper>;
  using SamplerStateTable = nsIdTable<nsGALSamplerStateHandle::IdType, nsGALSamplerState*, nsLocalAllocatorWrapper>;
  using RenderTargetViewTable = nsIdTable<nsGALRenderTargetViewHandle::IdType, nsGALRenderTargetView*, nsLocalAllocatorWrapper>;
  using TextureUnorderedAccessViewTable = nsIdTable<nsGALTextureUnorderedAccessViewHandle::IdType, nsGALTextureUnorderedAccessView*, nsLocalAllocatorWrapper>;
  using BufferUnorderedAccessViewTable = nsIdTable<nsGALBufferUnorderedAccessViewHandle::IdType, nsGALBufferUnorderedAccessView*, nsLocalAllocatorWrapper>;
  using SwapChainTable = nsIdTable<nsGALSwapChainHandle::IdType, nsGALSwapChain*, nsLocalAllocatorWrapper>;
  using QueryTable = nsIdTable<nsGALQueryHandle::IdType, nsGALQuery*, nsLocalAllocatorWrapper>;
  using VertexDeclarationTable = nsIdTable<nsGALVertexDeclarationHandle::IdType, nsGALVertexDeclaration*, nsLocalAllocatorWrapper>;

  ShaderTable m_Shaders;
  BlendStateTable m_BlendStates;
  DepthStencilStateTable m_DepthStencilStates;
  RasterizerStateTable m_RasterizerStates;
  BufferTable m_Buffers;
  TextureTable m_Textures;
  TextureResourceViewTable m_TextureResourceViews;
  BufferResourceViewTable m_BufferResourceViews;
  SamplerStateTable m_SamplerStates;
  RenderTargetViewTable m_RenderTargetViews;
  TextureUnorderedAccessViewTable m_TextureUnorderedAccessViews;
  BufferUnorderedAccessViewTable m_BufferUnorderedAccessViews;
  SwapChainTable m_SwapChains;
  QueryTable m_Queries;
  VertexDeclarationTable m_VertexDeclarations;


  // Hash tables used to prevent state object duplication
  nsHashTable<nsUInt32, nsGALBlendStateHandle, nsHashHelper<nsUInt32>, nsLocalAllocatorWrapper> m_BlendStateTable;
  nsHashTable<nsUInt32, nsGALDepthStencilStateHandle, nsHashHelper<nsUInt32>, nsLocalAllocatorWrapper> m_DepthStencilStateTable;
  nsHashTable<nsUInt32, nsGALRasterizerStateHandle, nsHashHelper<nsUInt32>, nsLocalAllocatorWrapper> m_RasterizerStateTable;
  nsHashTable<nsUInt32, nsGALSamplerStateHandle, nsHashHelper<nsUInt32>, nsLocalAllocatorWrapper> m_SamplerStateTable;
  nsHashTable<nsUInt32, nsGALVertexDeclarationHandle, nsHashHelper<nsUInt32>, nsLocalAllocatorWrapper> m_VertexDeclarationTable;

  struct DeadObject
  {
    NS_DECLARE_POD_TYPE();

    nsUInt32 m_uiType;
    nsUInt32 m_uiHandle;
  };

  nsDynamicArray<DeadObject, nsLocalAllocatorWrapper> m_DeadObjects;

  nsGALDeviceCreationDescription m_Description;

  nsGALDeviceCapabilities m_Capabilities;

  // Deactivate Doxygen document generation for the following block. (API abstraction only)
  /// \cond

  // These functions need to be implemented by a render API abstraction
protected:
  friend class nsMemoryUtils;

  // Init & shutdown functions

  virtual nsResult InitPlatform() = 0;
  virtual nsResult ShutdownPlatform() = 0;
  virtual nsStringView GetRendererPlatform() = 0;

  // Pipeline & Pass functions

  virtual void BeginPipelinePlatform(const char* szName, nsGALSwapChain* pSwapChain) = 0;
  virtual void EndPipelinePlatform(nsGALSwapChain* pSwapChain) = 0;

  virtual nsGALPass* BeginPassPlatform(const char* szName) = 0;
  virtual void EndPassPlatform(nsGALPass* pPass) = 0;

  // State creation functions

  virtual nsGALBlendState* CreateBlendStatePlatform(const nsGALBlendStateCreationDescription& Description) = 0;
  virtual void DestroyBlendStatePlatform(nsGALBlendState* pBlendState) = 0;

  virtual nsGALDepthStencilState* CreateDepthStencilStatePlatform(const nsGALDepthStencilStateCreationDescription& Description) = 0;
  virtual void DestroyDepthStencilStatePlatform(nsGALDepthStencilState* pDepthStencilState) = 0;

  virtual nsGALRasterizerState* CreateRasterizerStatePlatform(const nsGALRasterizerStateCreationDescription& Description) = 0;
  virtual void DestroyRasterizerStatePlatform(nsGALRasterizerState* pRasterizerState) = 0;

  virtual nsGALSamplerState* CreateSamplerStatePlatform(const nsGALSamplerStateCreationDescription& Description) = 0;
  virtual void DestroySamplerStatePlatform(nsGALSamplerState* pSamplerState) = 0;

  // Resource creation functions

  virtual nsGALShader* CreateShaderPlatform(const nsGALShaderCreationDescription& Description) = 0;
  virtual void DestroyShaderPlatform(nsGALShader* pShader) = 0;

  virtual nsGALBuffer* CreateBufferPlatform(const nsGALBufferCreationDescription& Description, nsArrayPtr<const nsUInt8> pInitialData) = 0;
  virtual void DestroyBufferPlatform(nsGALBuffer* pBuffer) = 0;

  virtual nsGALTexture* CreateTexturePlatform(const nsGALTextureCreationDescription& Description, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData) = 0;
  virtual void DestroyTexturePlatform(nsGALTexture* pTexture) = 0;

  virtual nsGALTexture* CreateSharedTexturePlatform(const nsGALTextureCreationDescription& Description, nsArrayPtr<nsGALSystemMemoryDescription> pInitialData, nsEnum<nsGALSharedTextureType> sharedType, nsGALPlatformSharedHandle handle) = 0;
  virtual void DestroySharedTexturePlatform(nsGALTexture* pTexture) = 0;

  virtual nsGALTextureResourceView* CreateResourceViewPlatform(nsGALTexture* pResource, const nsGALTextureResourceViewCreationDescription& Description) = 0;
  virtual void DestroyResourceViewPlatform(nsGALTextureResourceView* pResourceView) = 0;

  virtual nsGALBufferResourceView* CreateResourceViewPlatform(nsGALBuffer* pResource, const nsGALBufferResourceViewCreationDescription& Description) = 0;
  virtual void DestroyResourceViewPlatform(nsGALBufferResourceView* pResourceView) = 0;

  virtual nsGALRenderTargetView* CreateRenderTargetViewPlatform(nsGALTexture* pTexture, const nsGALRenderTargetViewCreationDescription& Description) = 0;
  virtual void DestroyRenderTargetViewPlatform(nsGALRenderTargetView* pRenderTargetView) = 0;

  virtual nsGALTextureUnorderedAccessView* CreateUnorderedAccessViewPlatform(nsGALTexture* pResource, const nsGALTextureUnorderedAccessViewCreationDescription& Description) = 0;
  virtual void DestroyUnorderedAccessViewPlatform(nsGALTextureUnorderedAccessView* pUnorderedAccessView) = 0;

  virtual nsGALBufferUnorderedAccessView* CreateUnorderedAccessViewPlatform(nsGALBuffer* pResource, const nsGALBufferUnorderedAccessViewCreationDescription& Description) = 0;
  virtual void DestroyUnorderedAccessViewPlatform(nsGALBufferUnorderedAccessView* pUnorderedAccessView) = 0;

  // Other rendering creation functions

  virtual nsGALQuery* CreateQueryPlatform(const nsGALQueryCreationDescription& Description) = 0;
  virtual void DestroyQueryPlatform(nsGALQuery* pQuery) = 0;

  virtual nsGALVertexDeclaration* CreateVertexDeclarationPlatform(const nsGALVertexDeclarationCreationDescription& Description) = 0;
  virtual void DestroyVertexDeclarationPlatform(nsGALVertexDeclaration* pVertexDeclaration) = 0;

  // Timestamp functions

  virtual nsGALTimestampHandle GetTimestampPlatform() = 0;
  virtual nsResult GetTimestampResultPlatform(nsGALTimestampHandle hTimestamp, nsTime& result) = 0;

  // Misc functions

  virtual void BeginFramePlatform(const nsUInt64 uiRenderFrame) = 0;
  virtual void EndFramePlatform() = 0;

  virtual void FillCapabilitiesPlatform() = 0;

  virtual void FlushPlatform() = 0;
  virtual void WaitIdlePlatform() = 0;


  /// \endcond

private:
  bool m_bBeginFrameCalled = false;
  bool m_bBeginPipelineCalled = false;
  bool m_bBeginPassCalled = false;
};

#include <RendererFoundation/Device/Implementation/Device_inl.h>
