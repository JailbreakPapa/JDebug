#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/Pipeline/RenderData.h>
#include <RendererCore/Pipeline/Renderer.h>
#include <RendererCore/Pipeline/SortingFunctions.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, RenderData)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    nsRenderData::UpdateRendererTypes();

    nsPlugin::Events().AddEventHandler(nsRenderData::PluginEventHandler);
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    nsPlugin::Events().RemoveEventHandler(nsRenderData::PluginEventHandler);

    nsRenderData::ClearRendererInstances();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRenderData, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRenderer, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgExtractRenderData);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgExtractRenderData, 1, nsRTTIDefaultAllocator<nsMsgExtractRenderData>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_IMPLEMENT_MESSAGE_TYPE(nsMsgExtractOccluderData);
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMsgExtractOccluderData, 1, nsRTTIDefaultAllocator<nsMsgExtractOccluderData>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsExcludeFromScript()
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsHybridArray<nsRenderData::CategoryData, 32> nsRenderData::s_CategoryData;

nsHybridArray<const nsRTTI*, 16> nsRenderData::s_RendererTypes;
nsDynamicArray<nsUniquePtr<nsRenderer>> nsRenderData::s_RendererInstances;
bool nsRenderData::s_bRendererInstancesDirty = false;

// static
nsRenderData::Category nsRenderData::RegisterCategory(const char* szCategoryName, SortingKeyFunc sortingKeyFunc)
{
  nsHashedString sCategoryName;
  sCategoryName.Assign(szCategoryName);

  Category oldCategory = FindCategory(sCategoryName);
  if (oldCategory != nsInvalidRenderDataCategory)
    return oldCategory;

  Category newCategory = Category(static_cast<nsUInt16>(s_CategoryData.GetCount()));

  auto& data = s_CategoryData.ExpandAndGetRef();
  data.m_sName = sCategoryName;
  data.m_sortingKeyFunc = sortingKeyFunc;

  return newCategory;
}

// static
nsRenderData::Category nsRenderData::FindCategory(nsTempHashedString sCategoryName)
{
  for (nsUInt32 uiCategoryIndex = 0; uiCategoryIndex < s_CategoryData.GetCount(); ++uiCategoryIndex)
  {
    if (s_CategoryData[uiCategoryIndex].m_sName == sCategoryName)
      return Category(static_cast<nsUInt16>(uiCategoryIndex));
  }

  return nsInvalidRenderDataCategory;
}

// static
void nsRenderData::GetAllCategoryNames(nsDynamicArray<nsHashedString>& out_categoryNames)
{
  out_categoryNames.Clear();

  for (auto& data : s_CategoryData)
  {
    out_categoryNames.PushBack(data.m_sName);
  }
}

// static
void nsRenderData::PluginEventHandler(const nsPluginEvent& e)
{
  switch (e.m_EventType)
  {
    case nsPluginEvent::AfterPluginChanges:
      UpdateRendererTypes();
      break;

    default:
      break;
  }
}

// static
void nsRenderData::UpdateRendererTypes()
{
  s_RendererTypes.Clear();

  nsRTTI::ForEachDerivedType<nsRenderer>([](const nsRTTI* pRtti)
    { s_RendererTypes.PushBack(pRtti); },
    nsRTTI::ForEachOptions::ExcludeNonAllocatable);

  s_bRendererInstancesDirty = true;
}

// static
void nsRenderData::CreateRendererInstances()
{
  ClearRendererInstances();

  for (auto pRendererType : s_RendererTypes)
  {
    NS_ASSERT_DEV(pRendererType->IsDerivedFrom(nsGetStaticRTTI<nsRenderer>()), "Renderer type '{}' must be derived from nsRenderer",
      pRendererType->GetTypeName());

    auto pRenderer = pRendererType->GetAllocator()->Allocate<nsRenderer>();

    nsUInt32 uiIndex = s_RendererInstances.GetCount();
    s_RendererInstances.PushBack(pRenderer);

    nsHybridArray<Category, 8> supportedCategories;
    pRenderer->GetSupportedRenderDataCategories(supportedCategories);

    nsHybridArray<const nsRTTI*, 8> supportedTypes;
    pRenderer->GetSupportedRenderDataTypes(supportedTypes);

    for (Category category : supportedCategories)
    {
      auto& categoryData = s_CategoryData[category.m_uiValue];

      for (nsUInt32 i = 0; i < supportedTypes.GetCount(); ++i)
      {
        categoryData.m_TypeToRendererIndex.Insert(supportedTypes[i], uiIndex);
      }
    }
  }

  s_bRendererInstancesDirty = false;
}

// static
void nsRenderData::ClearRendererInstances()
{
  s_RendererInstances.Clear();

  for (auto& categoryData : s_CategoryData)
  {
    categoryData.m_TypeToRendererIndex.Clear();
  }
}

//////////////////////////////////////////////////////////////////////////

nsRenderData::Category nsDefaultRenderDataCategories::Light = nsRenderData::RegisterCategory("Light", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::Decal = nsRenderData::RegisterCategory("Decal", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::ReflectionProbe = nsRenderData::RegisterCategory("ReflectionProbe", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::Sky = nsRenderData::RegisterCategory("Sky", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::LitOpaque = nsRenderData::RegisterCategory("LitOpaque", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::LitMasked = nsRenderData::RegisterCategory("LitMasked", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::LitTransparent = nsRenderData::RegisterCategory("LitTransparent", &nsRenderSortingFunctions::BackToFrontThenByRenderData);
nsRenderData::Category nsDefaultRenderDataCategories::LitForeground = nsRenderData::RegisterCategory("LitForeground", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::LitScreenFX = nsRenderData::RegisterCategory("LitScreenFX", &nsRenderSortingFunctions::BackToFrontThenByRenderData);
nsRenderData::Category nsDefaultRenderDataCategories::SimpleOpaque = nsRenderData::RegisterCategory("SimpleOpaque", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::SimpleTransparent = nsRenderData::RegisterCategory("SimpleTransparent", &nsRenderSortingFunctions::BackToFrontThenByRenderData);
nsRenderData::Category nsDefaultRenderDataCategories::SimpleForeground = nsRenderData::RegisterCategory("SimpleForeground", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::Selection = nsRenderData::RegisterCategory("Selection", &nsRenderSortingFunctions::ByRenderDataThenFrontToBack);
nsRenderData::Category nsDefaultRenderDataCategories::GUI = nsRenderData::RegisterCategory("GUI", &nsRenderSortingFunctions::BackToFrontThenByRenderData);

//////////////////////////////////////////////////////////////////////////

void nsMsgExtractRenderData::AddRenderData(
  const nsRenderData* pRenderData, nsRenderData::Category category, nsRenderData::Caching::Enum cachingBehavior)
{
  auto& cached = m_ExtractedRenderData.ExpandAndGetRef();
  cached.m_pRenderData = pRenderData;
  cached.m_uiCategory = category.m_uiValue;

  if (cachingBehavior == nsRenderData::Caching::IfStatic)
  {
    ++m_uiNumCacheIfStatic;
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_Pipeline_Implementation_RenderData);
