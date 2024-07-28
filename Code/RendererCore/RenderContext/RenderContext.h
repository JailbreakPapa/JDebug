#pragma once

#include <Core/ResourceManager/Resource.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Math/Rect.h>
#include <Foundation/Strings/String.h>
#include <RendererCore/Declarations.h>
#include <RendererCore/Pipeline/ViewData.h>
#include <RendererCore/RenderContext/Implementation/RenderContextStructs.h>
#include <RendererCore/Shader/ConstantBufferStorage.h>
#include <RendererCore/Shader/ShaderStageBinary.h>
#include <RendererCore/ShaderCompiler/PermutationGenerator.h>
#include <RendererCore/Textures/Texture2DResource.h>
#include <RendererCore/Textures/Texture3DResource.h>
#include <RendererCore/Textures/TextureCubeResource.h>
#include <RendererFoundation/CommandEncoder/ComputeCommandEncoder.h>
#include <RendererFoundation/CommandEncoder/RenderCommandEncoder.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Device/Pass.h>
#include <RendererFoundation/Shader/Shader.h>
#include <RendererFoundation/Shader/ShaderUtils.h>

#include <RendererCore/../../../Data/Base/Shaders/Common/GlobalConstants.h>

struct nsRenderWorldRenderEvent;

//////////////////////////////////////////////////////////////////////////
// nsRenderContext
//////////////////////////////////////////////////////////////////////////

class NS_RENDERERCORE_DLL nsRenderContext
{
private:
  nsRenderContext();
  ~nsRenderContext();
  friend class nsMemoryUtils;

  static nsRenderContext* s_pDefaultInstance;
  static nsHybridArray<nsRenderContext*, 4> s_Instances;

public:
  static nsRenderContext* GetDefaultInstance();
  static nsRenderContext* CreateInstance();
  static void DestroyInstance(nsRenderContext* pRenderer);

public:
  struct Statistics
  {
    Statistics();
    void Reset();

    nsUInt32 m_uiFailedDrawcalls;
  };

  Statistics GetAndResetStatistics();

  nsGALRenderCommandEncoder* BeginRendering(nsGALPass* pGALPass, const nsGALRenderingSetup& renderingSetup, const nsRectFloat& viewport, const char* szName = "", bool bStereoRendering = false);
  void EndRendering();

  nsGALComputeCommandEncoder* BeginCompute(nsGALPass* pGALPass, const char* szName = "");
  void EndCompute();

  // Helper class to automatically end rendering or compute on scope exit
  template <typename T>
  class CommandEncoderScope
  {
    NS_DISALLOW_COPY_AND_ASSIGN(CommandEncoderScope);

  public:
    NS_ALWAYS_INLINE ~CommandEncoderScope()
    {
      m_RenderContext.EndCommandEncoder(m_pGALCommandEncoder);

      if (m_pGALPass != nullptr)
      {
        nsGALDevice::GetDefaultDevice()->EndPass(m_pGALPass);
      }
    }

    NS_ALWAYS_INLINE T* operator->() { return m_pGALCommandEncoder; }
    NS_ALWAYS_INLINE operator const T*() { return m_pGALCommandEncoder; }

  private:
    friend class nsRenderContext;

    NS_ALWAYS_INLINE CommandEncoderScope(nsRenderContext& renderContext, nsGALPass* pGALPass, T* pGALCommandEncoder)
      : m_RenderContext(renderContext)
      , m_pGALPass(pGALPass)
      , m_pGALCommandEncoder(pGALCommandEncoder)
    {
    }

    nsRenderContext& m_RenderContext;
    nsGALPass* m_pGALPass;
    T* m_pGALCommandEncoder;
  };

  using RenderingScope = CommandEncoderScope<nsGALRenderCommandEncoder>;
  NS_ALWAYS_INLINE static RenderingScope BeginRenderingScope(nsGALPass* pGALPass, const nsRenderViewContext& viewContext, const nsGALRenderingSetup& renderingSetup, const char* szName = "", bool bStereoRendering = false)
  {
    return RenderingScope(*viewContext.m_pRenderContext, nullptr, viewContext.m_pRenderContext->BeginRendering(pGALPass, renderingSetup, viewContext.m_pViewData->m_ViewPortRect, szName, bStereoRendering));
  }

  NS_ALWAYS_INLINE static RenderingScope BeginPassAndRenderingScope(const nsRenderViewContext& viewContext, const nsGALRenderingSetup& renderingSetup, const char* szName, bool bStereoRendering = false)
  {
    nsGALPass* pGALPass = nsGALDevice::GetDefaultDevice()->BeginPass(szName);

    return RenderingScope(*viewContext.m_pRenderContext, pGALPass, viewContext.m_pRenderContext->BeginRendering(pGALPass, renderingSetup, viewContext.m_pViewData->m_ViewPortRect, "", bStereoRendering));
  }

  using ComputeScope = CommandEncoderScope<nsGALComputeCommandEncoder>;
  NS_ALWAYS_INLINE static ComputeScope BeginComputeScope(nsGALPass* pGALPass, const nsRenderViewContext& viewContext, const char* szName = "")
  {
    return ComputeScope(*viewContext.m_pRenderContext, nullptr, viewContext.m_pRenderContext->BeginCompute(pGALPass, szName));
  }

  NS_ALWAYS_INLINE static ComputeScope BeginPassAndComputeScope(const nsRenderViewContext& viewContext, const char* szName)
  {
    nsGALPass* pGALPass = nsGALDevice::GetDefaultDevice()->BeginPass(szName);

    return ComputeScope(*viewContext.m_pRenderContext, pGALPass, viewContext.m_pRenderContext->BeginCompute(pGALPass));
  }

  NS_ALWAYS_INLINE nsGALCommandEncoder* GetCommandEncoder()
  {
    NS_ASSERT_DEBUG(m_pGALCommandEncoder != nullptr, "BeginRendering/Compute has not been called");
    return m_pGALCommandEncoder;
  }

  NS_ALWAYS_INLINE nsGALRenderCommandEncoder* GetRenderCommandEncoder()
  {
    NS_ASSERT_DEBUG(m_pGALCommandEncoder != nullptr && !m_bCompute, "BeginRendering has not been called");
    return static_cast<nsGALRenderCommandEncoder*>(m_pGALCommandEncoder);
  }

  NS_ALWAYS_INLINE nsGALComputeCommandEncoder* GetComputeCommandEncoder()
  {
    NS_ASSERT_DEBUG(m_pGALCommandEncoder != nullptr && m_bCompute, "BeginCompute has not been called");
    return static_cast<nsGALComputeCommandEncoder*>(m_pGALCommandEncoder);
  }


  // Member Functions
  void SetShaderPermutationVariable(const char* szName, const nsTempHashedString& sValue);
  void SetShaderPermutationVariable(const nsHashedString& sName, const nsHashedString& sValue);

  void BindMaterial(const nsMaterialResourceHandle& hMaterial);

  void BindTexture2D(const nsTempHashedString& sSlotName, const nsTexture2DResourceHandle& hTexture, nsResourceAcquireMode acquireMode = nsResourceAcquireMode::AllowLoadingFallback);
  void BindTexture3D(const nsTempHashedString& sSlotName, const nsTexture3DResourceHandle& hTexture, nsResourceAcquireMode acquireMode = nsResourceAcquireMode::AllowLoadingFallback);
  void BindTextureCube(const nsTempHashedString& sSlotName, const nsTextureCubeResourceHandle& hTexture, nsResourceAcquireMode acquireMode = nsResourceAcquireMode::AllowLoadingFallback);

  void BindTexture2D(const nsTempHashedString& sSlotName, nsGALTextureResourceViewHandle hResourceView);
  void BindTexture3D(const nsTempHashedString& sSlotName, nsGALTextureResourceViewHandle hResourceView);
  void BindTextureCube(const nsTempHashedString& sSlotName, nsGALTextureResourceViewHandle hResourceView);

  /// Binds a read+write texture or buffer
  void BindUAV(const nsTempHashedString& sSlotName, nsGALTextureUnorderedAccessViewHandle hUnorderedAccessViewHandle);
  void BindUAV(const nsTempHashedString& sSlotName, nsGALBufferUnorderedAccessViewHandle hUnorderedAccessViewHandle);

  void BindSamplerState(const nsTempHashedString& sSlotName, nsGALSamplerStateHandle hSamplerSate);

  void BindBuffer(const nsTempHashedString& sSlotName, nsGALBufferResourceViewHandle hResourceView);

  void BindConstantBuffer(const nsTempHashedString& sSlotName, nsGALBufferHandle hConstantBuffer);
  void BindConstantBuffer(const nsTempHashedString& sSlotName, nsConstantBufferStorageHandle hConstantBufferStorage);

  /// \brief Sets push constants to the given data block.
  /// Note that for platforms that don't support push constants, this is emulated via a constant buffer. Thus, a slot name must be provided as well which matches the name of the BEGIN_PUSH_CONSTANTS block in the shader.
  /// \param sSlotName Name of the BEGIN_PUSH_CONSTANTS block in the shader.
  /// \param data Data of the push constants. If more than 128 bytes, nsGALDeviceCapabilities::m_uiMaxPushConstantsSize should be checked to ensure the data block is not too big for the platform.
  void SetPushConstants(const nsTempHashedString& sSlotName, nsArrayPtr<const nsUInt8> data);

  /// Templated version of SetPushConstants.
  /// \tparam T Type of the push constants struct.
  /// \param sSlotName Name of the BEGIN_PUSH_CONSTANTS block in the shader.
  /// \param constants Instance of type T that contains the push constants.
  template <typename T>
  NS_ALWAYS_INLINE void SetPushConstants(const nsTempHashedString& sSlotName, const T& constants)
  {
    SetPushConstants(sSlotName, nsArrayPtr<const nsUInt8>(reinterpret_cast<const nsUInt8*>(&constants), sizeof(T)));
  }

  /// \brief Sets the currently active shader on the given render context.
  ///
  /// This function has no effect until the next draw or dispatch call on the context.
  void BindShader(const nsShaderResourceHandle& hShader, nsBitflags<nsShaderBindFlags> flags = nsShaderBindFlags::Default);

  void BindMeshBuffer(const nsDynamicMeshBufferResourceHandle& hDynamicMeshBuffer);
  void BindMeshBuffer(const nsMeshBufferResourceHandle& hMeshBuffer);
  void BindMeshBuffer(nsGALBufferHandle hVertexBuffer, nsGALBufferHandle hIndexBuffer, const nsVertexDeclarationInfo* pVertexDeclarationInfo, nsGALPrimitiveTopology::Enum topology, nsUInt32 uiPrimitiveCount, nsGALBufferHandle hVertexBuffer2 = {}, nsGALBufferHandle hVertexBuffer3 = {}, nsGALBufferHandle hVertexBuffer4 = {});
  NS_ALWAYS_INLINE void BindNullMeshBuffer(nsGALPrimitiveTopology::Enum topology, nsUInt32 uiPrimitiveCount)
  {
    BindMeshBuffer(nsGALBufferHandle(), nsGALBufferHandle(), nullptr, topology, uiPrimitiveCount);
  }

  nsResult DrawMeshBuffer(nsUInt32 uiPrimitiveCount = 0xFFFFFFFF, nsUInt32 uiFirstPrimitive = 0, nsUInt32 uiInstanceCount = 1);

  nsResult Dispatch(nsUInt32 uiThreadGroupCountX, nsUInt32 uiThreadGroupCountY = 1, nsUInt32 uiThreadGroupCountZ = 1);

  nsResult ApplyContextStates(bool bForce = false);
  void ResetContextState();

  nsGlobalConstants& WriteGlobalConstants();
  const nsGlobalConstants& ReadGlobalConstants() const;

  /// \brief Sets the texture filter mode that is used by default for texture resources.
  ///
  /// The built in default is Anisotropic 4x.
  /// If the default setting is changed, already loaded textures might not adjust.
  /// Nearest filtering is not allowed as a default filter.
  void SetDefaultTextureFilter(nsTextureFilterSetting::Enum filter);

  /// \brief Returns the texture filter mode that is used by default for textures.
  nsTextureFilterSetting::Enum GetDefaultTextureFilter() const { return m_DefaultTextureFilter; }

  /// \brief Returns the 'fixed' texture filter setting that the combination of default texture filter and given \a configuration defines.
  ///
  /// If \a configuration is set to a fixed filter, that setting is returned.
  /// If it is one of LowestQuality to HighestQuality, the adjusted default filter is returned.
  /// When the default filter is used (with adjustments), the allowed range is Bilinear to Aniso16x, the Nearest filter is never used.
  nsTextureFilterSetting::Enum GetSpecificTextureFilter(nsTextureFilterSetting::Enum configuration) const;

  /// \brief Set async shader loading. During runtime all shaders should be preloaded so this is off by default.
  void SetAllowAsyncShaderLoading(bool bAllow);

  /// \brief Returns async shader loading. During runtime all shaders should be preloaded so this is off by default.
  bool GetAllowAsyncShaderLoading();


  // Static Functions
public:
  // Constant buffer storage handling
  template <typename T>
  NS_ALWAYS_INLINE static nsConstantBufferStorageHandle CreateConstantBufferStorage()
  {
    return CreateConstantBufferStorage(sizeof(T));
  }

  template <typename T>
  NS_FORCE_INLINE static nsConstantBufferStorageHandle CreateConstantBufferStorage(nsConstantBufferStorage<T>*& out_pStorage)
  {
    nsConstantBufferStorageBase* pStorage;
    nsConstantBufferStorageHandle hStorage = CreateConstantBufferStorage(sizeof(T), pStorage);
    out_pStorage = static_cast<nsConstantBufferStorage<T>*>(pStorage);
    return hStorage;
  }

  NS_FORCE_INLINE static nsConstantBufferStorageHandle CreateConstantBufferStorage(nsUInt32 uiSizeInBytes)
  {
    nsConstantBufferStorageBase* pStorage;
    return CreateConstantBufferStorage(uiSizeInBytes, pStorage);
  }

  static nsConstantBufferStorageHandle CreateConstantBufferStorage(nsUInt32 uiSizeInBytes, nsConstantBufferStorageBase*& out_pStorage);
  static void DeleteConstantBufferStorage(nsConstantBufferStorageHandle hStorage);

  template <typename T>
  NS_FORCE_INLINE static bool TryGetConstantBufferStorage(nsConstantBufferStorageHandle hStorage, nsConstantBufferStorage<T>*& out_pStorage)
  {
    nsConstantBufferStorageBase* pStorage = nullptr;
    bool bResult = TryGetConstantBufferStorage(hStorage, pStorage);
    out_pStorage = static_cast<nsConstantBufferStorage<T>*>(pStorage);
    return bResult;
  }

  static bool TryGetConstantBufferStorage(nsConstantBufferStorageHandle hStorage, nsConstantBufferStorageBase*& out_pStorage);

  template <typename T>
  NS_FORCE_INLINE static T* GetConstantBufferData(nsConstantBufferStorageHandle hStorage)
  {
    nsConstantBufferStorage<T>* pStorage = nullptr;
    if (TryGetConstantBufferStorage(hStorage, pStorage))
    {
      return &(pStorage->GetDataForWriting());
    }

    return nullptr;
  }

  // Default sampler state
  static nsGALSamplerStateCreationDescription GetDefaultSamplerState(nsBitflags<nsDefaultSamplerFlags> flags);

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, RendererContext);

  static void LoadBuiltinShader(nsShaderUtils::nsBuiltinShaderType type, nsShaderUtils::nsBuiltinShader& out_shader);
  static void RegisterImmutableSamplers();
  static void OnEngineStartup();
  static void OnEngineShutdown();

private:
  Statistics m_Statistics;
  nsBitflags<nsRenderContextFlags> m_StateFlags;
  nsShaderResourceHandle m_hActiveShader;
  nsGALShaderHandle m_hActiveGALShader;

  nsHashTable<nsHashedString, nsHashedString> m_PermutationVariables;
  nsMaterialResourceHandle m_hNewMaterial;
  nsMaterialResourceHandle m_hMaterial;

  nsShaderPermutationResourceHandle m_hActiveShaderPermutation;

  nsBitflags<nsShaderBindFlags> m_ShaderBindFlags;

  nsGALBufferHandle m_hVertexBuffers[4];
  nsGALBufferHandle m_hIndexBuffer;
  const nsVertexDeclarationInfo* m_pVertexDeclarationInfo;
  nsGALPrimitiveTopology::Enum m_Topology;
  nsUInt32 m_uiMeshBufferPrimitiveCount;
  nsEnum<nsTextureFilterSetting> m_DefaultTextureFilter;
  bool m_bAllowAsyncShaderLoading;
  bool m_bStereoRendering = false;

  nsHashTable<nsUInt64, nsGALTextureResourceViewHandle> m_BoundTextures2D;
  nsHashTable<nsUInt64, nsGALTextureResourceViewHandle> m_BoundTextures3D;
  nsHashTable<nsUInt64, nsGALTextureResourceViewHandle> m_BoundTexturesCube;
  nsHashTable<nsUInt64, nsGALTextureUnorderedAccessViewHandle> m_BoundTextureUAVs;

  nsHashTable<nsUInt64, nsGALSamplerStateHandle> m_BoundSamplers;

  nsHashTable<nsUInt64, nsGALBufferResourceViewHandle> m_BoundBuffer;
  nsHashTable<nsUInt64, nsGALBufferUnorderedAccessViewHandle> m_BoundBufferUAVs;
  nsGALSamplerStateHandle m_hFallbackSampler;

  struct BoundConstantBuffer
  {
    NS_DECLARE_POD_TYPE();

    BoundConstantBuffer() = default;
    BoundConstantBuffer(nsGALBufferHandle hConstantBuffer)
      : m_hConstantBuffer(hConstantBuffer)
    {
    }
    BoundConstantBuffer(nsConstantBufferStorageHandle hConstantBufferStorage)
      : m_hConstantBufferStorage(hConstantBufferStorage)
    {
    }

    nsGALBufferHandle m_hConstantBuffer;
    nsConstantBufferStorageHandle m_hConstantBufferStorage;
  };

  nsHashTable<nsUInt64, BoundConstantBuffer> m_BoundConstantBuffers;

  nsConstantBufferStorageHandle m_hGlobalConstantBufferStorage;
  nsConstantBufferStorageHandle m_hPushConstantsStorage;

  struct ShaderVertexDecl
  {
    nsGALShaderHandle m_hShader;
    nsUInt32 m_uiVertexDeclarationHash;

    NS_FORCE_INLINE bool operator<(const ShaderVertexDecl& rhs) const
    {
      if (m_hShader < rhs.m_hShader)
        return true;
      if (rhs.m_hShader < m_hShader)
        return false;
      return m_uiVertexDeclarationHash < rhs.m_uiVertexDeclarationHash;
    }

    NS_FORCE_INLINE bool operator==(const ShaderVertexDecl& rhs) const
    {
      return (m_hShader == rhs.m_hShader && m_uiVertexDeclarationHash == rhs.m_uiVertexDeclarationHash);
    }
  };

  static nsResult BuildVertexDeclaration(nsGALShaderHandle hShader, const nsVertexDeclarationInfo& decl, nsGALVertexDeclarationHandle& out_Declaration);

  static nsMap<ShaderVertexDecl, nsGALVertexDeclarationHandle> s_GALVertexDeclarations;

  static nsMutex s_ConstantBufferStorageMutex;
  static nsIdTable<nsConstantBufferStorageId, nsConstantBufferStorageBase*> s_ConstantBufferStorageTable;
  static nsMap<nsUInt32, nsDynamicArray<nsConstantBufferStorageBase*>> s_FreeConstantBufferStorage;

private: // Per Renderer States
  friend RenderingScope;
  friend ComputeScope;
  NS_ALWAYS_INLINE void EndCommandEncoder(nsGALRenderCommandEncoder*) { EndRendering(); }
  NS_ALWAYS_INLINE void EndCommandEncoder(nsGALComputeCommandEncoder*) { EndCompute(); }

  nsGALPass* m_pGALPass = nullptr;
  nsGALCommandEncoder* m_pGALCommandEncoder = nullptr;
  bool m_bCompute = false;

  // Member Functions
  void UploadConstants();

  void SetShaderPermutationVariableInternal(const nsHashedString& sName, const nsHashedString& sValue);
  void BindShaderInternal(const nsShaderResourceHandle& hShader, nsBitflags<nsShaderBindFlags> flags);
  nsShaderPermutationResource* ApplyShaderState();
  nsMaterialResource* ApplyMaterialState();
  void ApplyConstantBufferBindings(const nsGALShader* pShader);
  void ApplyTextureBindings(const nsGALShader* pShader);
  void ApplyUAVBindings(const nsGALShader* pShader);
  void ApplySamplerBindings(const nsGALShader* pShader);
  void ApplyBufferBindings(const nsGALShader* pShader);
};
