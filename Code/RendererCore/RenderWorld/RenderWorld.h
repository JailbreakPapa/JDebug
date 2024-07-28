#pragma once

#include <Core/ResourceManager/ResourceHandle.h>
#include <RendererCore/Pipeline/Declarations.h>

using nsRenderPipelineResourceHandle = nsTypedResourceHandle<class nsRenderPipelineResource>;

struct nsRenderWorldExtractionEvent
{
  enum class Type
  {
    BeginExtraction,
    BeforeViewExtraction,
    AfterViewExtraction,
    EndExtraction
  };

  Type m_Type;
  nsView* m_pView = nullptr;
  nsUInt64 m_uiFrameCounter = 0;
};

struct nsRenderWorldRenderEvent
{
  enum class Type
  {
    BeginRender,
    BeforePipelineExecution,
    AfterPipelineExecution,
    EndRender,
  };

  Type m_Type;
  nsRenderPipeline* m_pPipeline = nullptr;
  const nsRenderViewContext* m_pRenderViewContext = nullptr;
  nsUInt64 m_uiFrameCounter = 0;
};

class NS_RENDERERCORE_DLL nsRenderWorld
{
public:
  static nsViewHandle CreateView(const char* szName, nsView*& out_pView);
  static void DeleteView(const nsViewHandle& hView);

  static bool TryGetView(const nsViewHandle& hView, nsView*& out_pView);

  /// \brief Searches for an nsView with the desired usage hint or alternative usage hint.
  static nsView* GetViewByUsageHint(nsCameraUsageHint::Enum usageHint, nsCameraUsageHint::Enum alternativeUsageHint = nsCameraUsageHint::None, const nsWorld* pWorld = nullptr);

  static void AddMainView(const nsViewHandle& hView);
  static void RemoveMainView(const nsViewHandle& hView);
  static void ClearMainViews();
  static nsArrayPtr<nsViewHandle> GetMainViews();

  static void CacheRenderData(const nsView& view, const nsGameObjectHandle& hOwnerObject, const nsComponentHandle& hOwnerComponent, nsUInt16 uiComponentVersion, nsArrayPtr<nsInternal::RenderDataCacheEntry> cacheEntries);

  static void DeleteAllCachedRenderData();
  static void DeleteCachedRenderData(const nsGameObjectHandle& hOwnerObject, const nsComponentHandle& hOwnerComponent);
  static void DeleteCachedRenderDataForObject(const nsGameObject* pOwnerObject);
  static void DeleteCachedRenderDataForObjectRecursive(const nsGameObject* pOwnerObject);
  static void ResetRenderDataCache(nsView& ref_view);
  static nsArrayPtr<const nsInternal::RenderDataCacheEntry> GetCachedRenderData(const nsView& view, const nsGameObjectHandle& hOwner, nsUInt16 uiComponentVersion);

  static void AddViewToRender(const nsViewHandle& hView);

  static void ExtractMainViews();

  static void Render(nsRenderContext* pRenderContext);

  static void BeginFrame();
  static void EndFrame();

  static nsEvent<nsView*, nsMutex> s_ViewCreatedEvent;
  static nsEvent<nsView*, nsMutex> s_ViewDeletedEvent;

  static const nsEvent<const nsRenderWorldExtractionEvent&, nsMutex>& GetExtractionEvent() { return s_ExtractionEvent; }
  static const nsEvent<const nsRenderWorldRenderEvent&, nsMutex>& GetRenderEvent() { return s_RenderEvent; }

  static bool GetUseMultithreadedRendering();

  /// \brief Resets the frame counter to zero. Only for test purposes !
  NS_ALWAYS_INLINE static void ResetFrameCounter() { s_uiFrameCounter = 0; }

  NS_ALWAYS_INLINE static nsUInt64 GetFrameCounter() { return s_uiFrameCounter; }

  NS_FORCE_INLINE static nsUInt32 GetDataIndexForExtraction() { return GetUseMultithreadedRendering() ? (s_uiFrameCounter & 1) : 0; }

  NS_FORCE_INLINE static nsUInt32 GetDataIndexForRendering() { return GetUseMultithreadedRendering() ? ((s_uiFrameCounter + 1) & 1) : 0; }

  static bool IsRenderingThread();

  /// \name Render To Texture
  /// @{
public:
  struct CameraConfig
  {
    nsRenderPipelineResourceHandle m_hRenderPipeline;
  };

  static void BeginModifyCameraConfigs();
  static void EndModifyCameraConfigs();
  static void ClearCameraConfigs();
  static void SetCameraConfig(const char* szName, const CameraConfig& config);
  static const CameraConfig* FindCameraConfig(const char* szName);

  static nsEvent<void*> s_CameraConfigsModifiedEvent;

private:
  static bool s_bModifyingCameraConfigs;
  static nsMap<nsString, CameraConfig> s_CameraConfigs;

  /// @}

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, RenderWorld);
  friend class nsView;
  friend class nsRenderPipeline;

  static void DeleteCachedRenderDataInternal(const nsGameObjectHandle& hOwnerObject);
  static void ClearRenderDataCache();
  static void UpdateRenderDataCache();

  static void AddRenderPipelineToRebuild(nsRenderPipeline* pRenderPipeline, const nsViewHandle& hView);
  static void RebuildPipelines();

  static void OnEngineStartup();
  static void OnEngineShutdown();

  static nsEvent<const nsRenderWorldExtractionEvent&, nsMutex> s_ExtractionEvent;
  static nsEvent<const nsRenderWorldRenderEvent&, nsMutex> s_RenderEvent;
  static nsUInt64 s_uiFrameCounter;
};
