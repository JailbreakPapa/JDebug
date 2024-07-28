#include <RendererCore/RendererCorePCH.h>

#include <Core/World/SpatialSystem_RegularGrid.h>
#include <Core/World/World.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/IO/TypeVersionContext.h>
#include <RendererCore/Debug/DebugRenderer.h>
#include <RendererCore/Pipeline/ExtractedRenderData.h>
#include <RendererCore/Pipeline/Extractor.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
nsCVarBool cvar_SpatialVisBounds("Spatial.VisBounds", false, nsCVarFlags::Default, "Enables debug visualization of object bounds");
nsCVarBool cvar_SpatialVisLocalBBox("Spatial.VisLocalBBox", false, nsCVarFlags::Default, "Enables debug visualization of object local bounding box");
nsCVarBool cvar_SpatialVisData("Spatial.VisData", false, nsCVarFlags::Default, "Enables debug visualization of the spatial data structure");
nsCVarString cvar_SpatialVisDataOnlyCategory("Spatial.VisData.OnlyCategory", "", nsCVarFlags::Default, "When set the debug visualization is only shown for the given spatial data category");
nsCVarBool cvar_SpatialVisDataOnlySelected("Spatial.VisData.OnlySelected", false, nsCVarFlags::Default, "When set the debug visualization is only shown for selected objects");
nsCVarString cvar_SpatialVisDataOnlyObject("Spatial.VisData.OnlyObject", "", nsCVarFlags::Default, "When set the debug visualization is only shown for objects with the given name");

nsCVarBool cvar_SpatialExtractionShowStats("Spatial.Extraction.ShowStats", false, nsCVarFlags::Default, "Display some stats of the render data extraction");
#endif

namespace
{
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  void VisualizeSpatialData(const nsView& view)
  {
    if (cvar_SpatialVisData && cvar_SpatialVisDataOnlyObject.GetValue().IsEmpty() && !cvar_SpatialVisDataOnlySelected)
    {
      const nsSpatialSystem& spatialSystem = *view.GetWorld()->GetSpatialSystem();
      if (auto pSpatialSystemGrid = nsDynamicCast<const nsSpatialSystem_RegularGrid*>(&spatialSystem))
      {
        nsSpatialData::Category filterCategory = nsSpatialData::FindCategory(cvar_SpatialVisDataOnlyCategory.GetValue());

        nsHybridArray<nsBoundingBox, 16> boxes;
        pSpatialSystemGrid->GetAllCellBoxes(boxes, filterCategory);

        for (auto& box : boxes)
        {
          nsDebugRenderer::DrawLineBox(view.GetHandle(), box, nsColor::Cyan);
        }
      }
    }
  }

  void VisualizeObject(const nsView& view, const nsGameObject* pObject)
  {
    if (!cvar_SpatialVisBounds && !cvar_SpatialVisLocalBBox && !cvar_SpatialVisData)
      return;

    if (cvar_SpatialVisLocalBBox)
    {
      const nsBoundingBoxSphere& localBounds = pObject->GetLocalBounds();
      if (localBounds.IsValid())
      {
        nsDebugRenderer::DrawLineBox(view.GetHandle(), localBounds.GetBox(), nsColor::Yellow, pObject->GetGlobalTransform());
      }
    }

    if (cvar_SpatialVisBounds)
    {
      const nsBoundingBoxSphere& globalBounds = pObject->GetGlobalBounds();
      if (globalBounds.IsValid())
      {
        nsDebugRenderer::DrawLineBox(view.GetHandle(), globalBounds.GetBox(), nsColor::Lime);
        nsDebugRenderer::DrawLineSphere(view.GetHandle(), globalBounds.GetSphere(), nsColor::Magenta);
      }
    }

    if (cvar_SpatialVisData && cvar_SpatialVisDataOnlyCategory.GetValue().IsEmpty())
    {
      const nsSpatialSystem& spatialSystem = *view.GetWorld()->GetSpatialSystem();
      if (auto pSpatialSystemGrid = nsDynamicCast<const nsSpatialSystem_RegularGrid*>(&spatialSystem))
      {
        nsBoundingBox box;
        if (pSpatialSystemGrid->GetCellBoxForSpatialData(pObject->GetSpatialData(), box).Succeeded())
        {
          nsDebugRenderer::DrawLineBox(view.GetHandle(), box, nsColor::Cyan);
        }
      }
    }
  }
#endif
} // namespace

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsExtractor, 1, nsRTTINoAllocator)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("Active", m_bActive)->AddAttributes(new nsDefaultValueAttribute(true)),
      NS_ACCESSOR_PROPERTY("Name", GetName, SetName),
    }
    NS_END_PROPERTIES;
    NS_BEGIN_ATTRIBUTES
    {
      new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Red)),
    }
    NS_END_ATTRIBUTES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format om

nsExtractor::nsExtractor(const char* szName)
{
  m_bActive = true;
  m_sName.Assign(szName);

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  m_uiNumCachedRenderData = 0;
  m_uiNumUncachedRenderData = 0;
#endif
}

nsExtractor::~nsExtractor() = default;

void nsExtractor::SetName(const char* szName)
{
  if (!nsStringUtils::IsNullOrEmpty(szName))
  {
    m_sName.Assign(szName);
  }
}

const char* nsExtractor::GetName() const
{
  return m_sName.GetData();
}

bool nsExtractor::FilterByViewTags(const nsView& view, const nsGameObject* pObject) const
{
  if (!view.m_ExcludeTags.IsEmpty() && view.m_ExcludeTags.IsAnySet(pObject->GetTags()))
    return true;

  if (!view.m_IncludeTags.IsEmpty() && !view.m_IncludeTags.IsAnySet(pObject->GetTags()))
    return true;

  return false;
}

void nsExtractor::ExtractRenderData(const nsView& view, const nsGameObject* pObject, nsMsgExtractRenderData& msg, nsExtractedRenderData& extractedRenderData) const
{
  auto AddRenderDataFromMessage = [&](const nsMsgExtractRenderData& msg) {
    if (msg.m_OverrideCategory != nsInvalidRenderDataCategory)
    {
      for (auto& data : msg.m_ExtractedRenderData)
      {
        extractedRenderData.AddRenderData(data.m_pRenderData, msg.m_OverrideCategory);
      }
    }
    else
    {
      for (auto& data : msg.m_ExtractedRenderData)
      {
        extractedRenderData.AddRenderData(data.m_pRenderData, nsRenderData::Category(data.m_uiCategory));
      }
    }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    m_uiNumUncachedRenderData += msg.m_ExtractedRenderData.GetCount();
#endif
  };

  if (pObject->IsStatic())
  {
    nsUInt16 uiComponentVersion = pObject->GetComponentVersion();

    auto cachedRenderData = nsRenderWorld::GetCachedRenderData(view, pObject->GetHandle(), uiComponentVersion);

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    for (nsUInt32 i = 1; i < cachedRenderData.GetCount(); ++i)
    {
      NS_ASSERT_DEBUG(cachedRenderData[i - 1].m_uiComponentIndex <= cachedRenderData[i].m_uiComponentIndex, "Cached render data needs to be sorted");
      if (cachedRenderData[i - 1].m_uiComponentIndex == cachedRenderData[i].m_uiComponentIndex)
      {
        NS_ASSERT_DEBUG(cachedRenderData[i - 1].m_uiPartIndex < cachedRenderData[i].m_uiPartIndex, "Cached render data needs to be sorted");
      }
    }
#endif

    nsUInt32 uiCacheIndex = 0;

    auto components = pObject->GetComponents();
    const nsUInt32 uiNumComponents = components.GetCount();
    for (nsUInt32 uiComponentIndex = 0; uiComponentIndex < uiNumComponents; ++uiComponentIndex)
    {
      bool bCacheFound = false;
      while (uiCacheIndex < cachedRenderData.GetCount() && cachedRenderData[uiCacheIndex].m_uiComponentIndex == uiComponentIndex)
      {
        const nsInternal::RenderDataCacheEntry& cacheEntry = cachedRenderData[uiCacheIndex];
        if (cacheEntry.m_pRenderData != nullptr)
        {
          extractedRenderData.AddRenderData(cacheEntry.m_pRenderData, msg.m_OverrideCategory != nsInvalidRenderDataCategory ? msg.m_OverrideCategory : nsRenderData::Category(cacheEntry.m_uiCategory));

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
          ++m_uiNumCachedRenderData;
#endif
        }
        ++uiCacheIndex;

        bCacheFound = true;
      }

      if (bCacheFound)
      {
        continue;
      }

      const nsComponent* pComponent = components[uiComponentIndex];

      msg.m_ExtractedRenderData.Clear();
      msg.m_uiNumCacheIfStatic = 0;

      if (pComponent->SendMessage(msg))
      {
        // Only cache render data if all parts should be cached otherwise the cache is incomplete and we won't call SendMessage again
        if (msg.m_uiNumCacheIfStatic > 0 && msg.m_ExtractedRenderData.GetCount() == msg.m_uiNumCacheIfStatic)
        {
          nsHybridArray<nsInternal::RenderDataCacheEntry, 16> newCacheEntries(nsFrameAllocator::GetCurrentAllocator());

          for (nsUInt32 uiPartIndex = 0; uiPartIndex < msg.m_ExtractedRenderData.GetCount(); ++uiPartIndex)
          {
            auto& newCacheEntry = newCacheEntries.ExpandAndGetRef();
            newCacheEntry.m_pRenderData = msg.m_ExtractedRenderData[uiPartIndex].m_pRenderData;
            newCacheEntry.m_uiCategory = msg.m_ExtractedRenderData[uiPartIndex].m_uiCategory;
            newCacheEntry.m_uiComponentIndex = static_cast<nsUInt16>(uiComponentIndex);
            newCacheEntry.m_uiPartIndex = static_cast<nsUInt16>(uiPartIndex);
          }

          nsRenderWorld::CacheRenderData(view, pObject->GetHandle(), pComponent->GetHandle(), uiComponentVersion, newCacheEntries);
        }

        AddRenderDataFromMessage(msg);
      }
      else if (pComponent->IsActiveAndInitialized()) // component does not handle extract message at all
      {
        NS_ASSERT_DEV(pComponent->GetDynamicRTTI()->CanHandleMessage<nsMsgExtractRenderData>() == false, "");

        // Create a dummy cache entry so we don't call send message next time
        nsInternal::RenderDataCacheEntry dummyEntry;
        dummyEntry.m_pRenderData = nullptr;
        dummyEntry.m_uiCategory = nsInvalidRenderDataCategory.m_uiValue;
        dummyEntry.m_uiComponentIndex = static_cast<nsUInt16>(uiComponentIndex);

        nsRenderWorld::CacheRenderData(view, pObject->GetHandle(), pComponent->GetHandle(), uiComponentVersion, nsMakeArrayPtr(&dummyEntry, 1));
      }
    }
  }
  else
  {
    msg.m_ExtractedRenderData.Clear();
    pObject->SendMessage(msg);

    AddRenderDataFromMessage(msg);
  }
}

void nsExtractor::Extract(const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData)
{
}

void nsExtractor::PostSortAndBatch(
  const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData)
{
}


nsResult nsExtractor::Serialize(nsStreamWriter& inout_stream) const
{
  inout_stream << m_bActive;
  inout_stream << m_sName;
  return NS_SUCCESS;
}


nsResult nsExtractor::Deserialize(nsStreamReader& inout_stream)
{
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_ASSERT_DEBUG(uiVersion == 1, "Unknown version encountered");

  inout_stream >> m_bActive;
  inout_stream >> m_sName;
  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsVisibleObjectsExtractor, 1, nsRTTIDefaultAllocator<nsVisibleObjectsExtractor>)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsVisibleObjectsExtractor::nsVisibleObjectsExtractor(const char* szName)
  : nsExtractor(szName)
{
}

nsVisibleObjectsExtractor::~nsVisibleObjectsExtractor() = default;

void nsVisibleObjectsExtractor::Extract(
  const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData)
{
  nsMsgExtractRenderData msg;
  msg.m_pView = &view;

  NS_LOCK(view.GetWorld()->GetReadMarker());

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  VisualizeSpatialData(view);

  m_uiNumCachedRenderData = 0;
  m_uiNumUncachedRenderData = 0;
#endif

  for (auto pObject : visibleObjects)
  {
    ExtractRenderData(view, pObject, msg, ref_extractedRenderData);

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    if (cvar_SpatialVisBounds || cvar_SpatialVisLocalBBox || cvar_SpatialVisData)
    {
      if ((cvar_SpatialVisDataOnlyObject.GetValue().IsEmpty() ||
            pObject->GetName().FindSubString_NoCase(cvar_SpatialVisDataOnlyObject.GetValue()) != nullptr) &&
          !cvar_SpatialVisDataOnlySelected)
      {
        VisualizeObject(view, pObject);
      }
    }
#endif
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  const bool bIsMainView = (view.GetCameraUsageHint() == nsCameraUsageHint::MainView || view.GetCameraUsageHint() == nsCameraUsageHint::EditorView);

  if (cvar_SpatialExtractionShowStats && bIsMainView)
  {
    nsViewHandle hView = view.GetHandle();

    nsStringBuilder sb;

    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "ExtractionStats", "Extraction Stats:");

    sb.SetFormat("Num Cached Render Data: {0}", m_uiNumCachedRenderData);
    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "ExtractionStats", sb);

    sb.SetFormat("Num Uncached Render Data: {0}", m_uiNumUncachedRenderData);
    nsDebugRenderer::DrawInfoText(hView, nsDebugTextPlacement::TopLeft, "ExtractionStats", sb);
  }
#endif
}

nsResult nsVisibleObjectsExtractor::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  return NS_SUCCESS;
}

nsResult nsVisibleObjectsExtractor::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSelectedObjectsExtractorBase, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsSelectedObjectsExtractorBase::nsSelectedObjectsExtractorBase(const char* szName)
  : nsExtractor(szName)
  , m_OverrideCategory(nsDefaultRenderDataCategories::Selection)
{
}

nsSelectedObjectsExtractorBase::~nsSelectedObjectsExtractorBase() = default;

void nsSelectedObjectsExtractorBase::Extract(
  const nsView& view, const nsDynamicArray<const nsGameObject*>& visibleObjects, nsExtractedRenderData& ref_extractedRenderData)
{
  const nsDeque<nsGameObjectHandle>* pSelection = GetSelection();
  if (pSelection == nullptr)
    return;

  nsMsgExtractRenderData msg;
  msg.m_pView = &view;
  msg.m_OverrideCategory = m_OverrideCategory;

  NS_LOCK(view.GetWorld()->GetReadMarker());

  for (const auto& hObj : *pSelection)
  {
    const nsGameObject* pObject = nullptr;
    if (!view.GetWorld()->TryGetObject(hObj, pObject))
      continue;

    ExtractRenderData(view, pObject, msg, ref_extractedRenderData);

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    if (cvar_SpatialVisBounds || cvar_SpatialVisLocalBBox || cvar_SpatialVisData)
    {
      if (cvar_SpatialVisDataOnlySelected)
      {
        VisualizeObject(view, pObject);
      }
    }
#endif
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSelectedObjectsContext, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSelectedObjectsExtractor, 1, nsRTTIDefaultAllocator<nsSelectedObjectsExtractor>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("SelectionContext", GetSelectionContext, SetSelectionContext),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsSelectedObjectsContext::nsSelectedObjectsContext() = default;
nsSelectedObjectsContext::~nsSelectedObjectsContext() = default;

void nsSelectedObjectsContext::RemoveDeadObjects(const nsWorld& world)
{
  for (nsUInt32 i = 0; i < m_Objects.GetCount();)
  {
    const nsGameObject* pObj;
    if (world.TryGetObject(m_Objects[i], pObj) == false)
    {
      m_Objects.RemoveAtAndSwap(i);
    }
    else
      ++i;
  }
}

void nsSelectedObjectsContext::AddObjectAndChildren(const nsWorld& world, const nsGameObjectHandle& hObject)
{
  const nsGameObject* pObj;
  if (world.TryGetObject(hObject, pObj))
  {
    m_Objects.PushBack(hObject);

    for (auto it = pObj->GetChildren(); it.IsValid(); ++it)
    {
      AddObjectAndChildren(world, it);
    }
  }
}

void nsSelectedObjectsContext::AddObjectAndChildren(const nsWorld& world, const nsGameObject* pObject)
{
  m_Objects.PushBack(pObject->GetHandle());

  for (auto it = pObject->GetChildren(); it.IsValid(); ++it)
  {
    AddObjectAndChildren(world, it);
  }
}

nsSelectedObjectsExtractor::nsSelectedObjectsExtractor(const char* szName /*= "ExplicitlySelectedObjectsExtractor"*/)
  : nsSelectedObjectsExtractorBase(szName)
{
}

nsSelectedObjectsExtractor::~nsSelectedObjectsExtractor() = default;

const nsDeque<nsGameObjectHandle>* nsSelectedObjectsExtractor::GetSelection()
{
  if (m_pSelectionContext)
    return &m_pSelectionContext->m_Objects;

  return nullptr;
}

nsResult nsSelectedObjectsExtractor::Serialize(nsStreamWriter& inout_stream) const
{
  NS_SUCCEED_OR_RETURN(SUPER::Serialize(inout_stream));
  return NS_SUCCESS;
}

nsResult nsSelectedObjectsExtractor::Deserialize(nsStreamReader& inout_stream)
{
  NS_SUCCEED_OR_RETURN(SUPER::Deserialize(inout_stream));
  const nsUInt32 uiVersion = nsTypeVersionReadContext::GetContext()->GetTypeVersion(GetStaticRTTI());
  NS_IGNORE_UNUSED(uiVersion);
  return NS_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_Extractor);
