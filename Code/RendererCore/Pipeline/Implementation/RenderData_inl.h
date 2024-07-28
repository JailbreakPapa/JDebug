#include <Core/World/GameObject.h>

NS_ALWAYS_INLINE nsRenderData::Category::Category() = default;

NS_ALWAYS_INLINE nsRenderData::Category::Category(nsUInt16 uiValue)
  : m_uiValue(uiValue)
{
}

NS_ALWAYS_INLINE bool nsRenderData::Category::operator==(const Category& other) const
{
  return m_uiValue == other.m_uiValue;
}

NS_ALWAYS_INLINE bool nsRenderData::Category::operator!=(const Category& other) const
{
  return m_uiValue != other.m_uiValue;
}

//////////////////////////////////////////////////////////////////////////

// static
NS_FORCE_INLINE const nsRenderer* nsRenderData::GetCategoryRenderer(Category category, const nsRTTI* pRenderDataType)
{
  if (s_bRendererInstancesDirty)
  {
    CreateRendererInstances();
  }

  auto& categoryData = s_CategoryData[category.m_uiValue];

  nsUInt32 uiIndex = 0;
  if (categoryData.m_TypeToRendererIndex.TryGetValue(pRenderDataType, uiIndex))
  {
    return s_RendererInstances[uiIndex].Borrow();
  }

  return nullptr;
}

// static
NS_FORCE_INLINE nsHashedString nsRenderData::GetCategoryName(Category category)
{
  if (category.m_uiValue < s_CategoryData.GetCount())
  {
    return s_CategoryData[category.m_uiValue].m_sName;
  }

  return nsHashedString();
}

NS_FORCE_INLINE nsUInt64 nsRenderData::GetCategorySortingKey(Category category, const nsCamera& camera) const
{
  return s_CategoryData[category.m_uiValue].m_sortingKeyFunc(this, camera);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
static T* nsCreateRenderDataForThisFrame(const nsGameObject* pOwner)
{
  NS_CHECK_AT_COMPILETIME(NS_IS_DERIVED_FROM_STATIC(nsRenderData, T));

  T* pRenderData = NS_NEW(nsFrameAllocator::GetCurrentAllocator(), T);

  if (pOwner != nullptr)
  {
    pRenderData->m_hOwner = pOwner->GetHandle();
  }

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  pRenderData->m_pOwner = pOwner;
#endif

  return pRenderData;
}
