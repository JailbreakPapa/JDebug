#pragma once

#include <Core/World/Declarations.h>
#include <Foundation/Reflection/Reflection.h>
#include <RendererCore/RendererCoreDLL.h>
#include <RendererFoundation/Descriptors/Descriptors.h>

class nsCamera;
class nsExtractedRenderData;
class nsExtractor;
class nsView;
class nsRenderer;
class nsRenderData;
class nsRenderDataBatch;
class nsRenderPipeline;
class nsRenderPipelinePass;
class nsRenderContext;
class nsDebugRendererContext;

struct nsRenderPipelineNodePin;
struct nsRenderPipelinePassConnection;
struct nsViewData;

namespace nsInternal
{
  struct RenderDataCache;

  struct RenderDataCacheEntry
  {
    NS_DECLARE_POD_TYPE();

    const nsRenderData* m_pRenderData = nullptr;
    nsUInt16 m_uiCategory = 0;
    nsUInt16 m_uiComponentIndex = 0;
    nsUInt16 m_uiPartIndex = 0;

    NS_ALWAYS_INLINE bool operator==(const RenderDataCacheEntry& other) const { return m_pRenderData == other.m_pRenderData && m_uiCategory == other.m_uiCategory && m_uiComponentIndex == other.m_uiComponentIndex && m_uiPartIndex == other.m_uiPartIndex; }

    // Cache entries need to be sorted by component index and then by part index
    NS_ALWAYS_INLINE bool operator<(const RenderDataCacheEntry& other) const
    {
      if (m_uiComponentIndex == other.m_uiComponentIndex)
        return m_uiPartIndex < other.m_uiPartIndex;

      return m_uiComponentIndex < other.m_uiComponentIndex;
    }
  };
} // namespace nsInternal

struct nsRenderViewContext
{
  const nsCamera* m_pCamera;
  const nsCamera* m_pLodCamera;
  const nsViewData* m_pViewData;
  nsRenderContext* m_pRenderContext;

  const nsDebugRendererContext* m_pWorldDebugContext;
  const nsDebugRendererContext* m_pViewDebugContext;
};

using nsViewId = nsGenericId<24, 8>;

class nsViewHandle
{
  NS_DECLARE_HANDLE_TYPE(nsViewHandle, nsViewId);

  friend class nsRenderWorld;
};

/// \brief HashHelper implementation so view handles can be used as key in a hashtable.
template <>
struct nsHashHelper<nsViewHandle>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsViewHandle value) { return value.GetInternalID().m_Data * 2654435761U; }

  NS_ALWAYS_INLINE static bool Equal(nsViewHandle a, nsViewHandle b) { return a == b; }
};

/// \brief Usage hint of a camera/view.
struct NS_RENDERERCORE_DLL nsCameraUsageHint
{
  using StorageType = nsUInt8;

  enum Enum
  {
    None,         ///< No hint, camera may not be used, at all.
    MainView,     ///< The main camera from which the scene gets rendered. There should only be one camera with this hint.
    EditorView,   ///< The editor view shall be rendered from this camera.
    RenderTarget, ///< The camera is used to render to a render target.
    Culling,      ///< This camera should be used for culling only. Usually culling is done from the main view, but with a dedicated culling camera, one can debug the culling system.
    Shadow,       ///< This camera is used for rendering shadow maps.
    Reflection,   ///< This camera is used for rendering reflections.
    Thumbnail,    ///< This camera should be used for rendering a scene thumbnail when exporting from the editor.

    ENUM_COUNT,

    Default = None,
  };
};

NS_DECLARE_REFLECTABLE_TYPE(NS_RENDERERCORE_DLL, nsCameraUsageHint);
