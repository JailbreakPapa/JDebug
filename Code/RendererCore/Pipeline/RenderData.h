#pragma once

#include <Foundation/Communication/Message.h>
#include <Foundation/Math/BoundingBoxSphere.h>
#include <Foundation/Math/Transform.h>
#include <Foundation/Memory/FrameAllocator.h>
#include <Foundation/Strings/HashedString.h>
#include <RendererCore/Pipeline/Declarations.h>

class nsRasterizerObject;

/// \brief Base class for all render data. Render data must contain all information that is needed to render the corresponding object.
class NS_RENDERERCORE_DLL nsRenderData : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsRenderData, nsReflectedClass);

public:
  struct Category
  {
    Category();
    explicit Category(nsUInt16 uiValue);

    bool operator==(const Category& other) const;
    bool operator!=(const Category& other) const;
    bool IsValid() const { return m_uiValue != 0xFFFF; }

    nsUInt16 m_uiValue = 0xFFFF;
  };

  struct Caching
  {
    enum Enum
    {
      Never,
      IfStatic
    };
  };

  /// \brief This function generates a 64bit sorting key for the given render data. Data with lower sorting key is rendered first.
  using SortingKeyFunc = nsUInt64 (*)(const nsRenderData*, const nsCamera&);

  static Category RegisterCategory(const char* szCategoryName, SortingKeyFunc sortingKeyFunc);
  static Category FindCategory(nsTempHashedString sCategoryName);

  static void GetAllCategoryNames(nsDynamicArray<nsHashedString>& out_categoryNames);

  static const nsRenderer* GetCategoryRenderer(Category category, const nsRTTI* pRenderDataType);

  static nsHashedString GetCategoryName(Category category);

  nsUInt64 GetCategorySortingKey(Category category, const nsCamera& camera) const;

  nsTransform m_GlobalTransform = nsTransform::MakeIdentity();
  nsBoundingBoxSphere m_GlobalBounds;

  nsUInt32 m_uiBatchId = 0; ///< BatchId is used to group render data in batches.
  nsUInt32 m_uiSortingKey = 0;
  float m_fSortingDepthOffset = 0.0f;

  nsGameObjectHandle m_hOwner;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  const nsGameObject* m_pOwner = nullptr; ///< Debugging only. It is not allowed to access the game object during rendering.
#endif

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(RendererCore, RenderData);

  static void PluginEventHandler(const nsPluginEvent& e);
  static void UpdateRendererTypes();

  static void CreateRendererInstances();
  static void ClearRendererInstances();

  struct CategoryData
  {
    nsHashedString m_sName;
    SortingKeyFunc m_sortingKeyFunc;

    nsHashTable<const nsRTTI*, nsUInt32> m_TypeToRendererIndex;
  };

  static nsHybridArray<CategoryData, 32> s_CategoryData;

  static nsHybridArray<const nsRTTI*, 16> s_RendererTypes;
  static nsDynamicArray<nsUniquePtr<nsRenderer>> s_RendererInstances;
  static bool s_bRendererInstancesDirty;
};

/// \brief Creates render data that is only valid for this frame. The data is automatically deleted after the frame has been rendered.
template <typename T>
static T* nsCreateRenderDataForThisFrame(const nsGameObject* pOwner);

struct NS_RENDERERCORE_DLL nsDefaultRenderDataCategories
{
  static nsRenderData::Category Light;
  static nsRenderData::Category Decal;
  static nsRenderData::Category ReflectionProbe;
  static nsRenderData::Category Sky;
  static nsRenderData::Category LitOpaque;
  static nsRenderData::Category LitMasked;
  static nsRenderData::Category LitTransparent;
  static nsRenderData::Category LitForeground;
  static nsRenderData::Category LitScreenFX;
  static nsRenderData::Category SimpleOpaque;
  static nsRenderData::Category SimpleTransparent;
  static nsRenderData::Category SimpleForeground;
  static nsRenderData::Category Selection;
  static nsRenderData::Category GUI;
};

#define nsInvalidRenderDataCategory nsRenderData::Category()

struct NS_RENDERERCORE_DLL nsMsgExtractRenderData : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgExtractRenderData, nsMessage);

  const nsView* m_pView = nullptr;
  nsRenderData::Category m_OverrideCategory = nsInvalidRenderDataCategory;

  /// \brief Adds render data for the current view. This data can be cached depending on the specified caching behavior.
  /// Non-cached data is only valid for this frame. Cached data must be manually deleted using the nsRenderWorld::DeleteCachedRenderData
  /// function.
  void AddRenderData(const nsRenderData* pRenderData, nsRenderData::Category category, nsRenderData::Caching::Enum cachingBehavior);

private:
  friend class nsExtractor;

  struct Data
  {
    const nsRenderData* m_pRenderData = nullptr;
    nsUInt16 m_uiCategory = 0;
  };

  nsHybridArray<Data, 16> m_ExtractedRenderData;
  nsUInt32 m_uiNumCacheIfStatic = 0;
};

struct NS_RENDERERCORE_DLL nsMsgExtractOccluderData : public nsMessage
{
  NS_DECLARE_MESSAGE_TYPE(nsMsgExtractOccluderData, nsMessage);

  void AddOccluder(const nsRasterizerObject* pObject, const nsTransform& transform)
  {
    auto& d = m_ExtractedOccluderData.ExpandAndGetRef();
    d.m_pObject = pObject;
    d.m_Transform = transform;
  }

private:
  friend class nsRenderPipeline;

  struct Data
  {
    const nsRasterizerObject* m_pObject = nullptr;
    nsTransform m_Transform;
  };

  nsHybridArray<Data, 16> m_ExtractedOccluderData;
};

#include <RendererCore/Pipeline/Implementation/RenderData_inl.h>
