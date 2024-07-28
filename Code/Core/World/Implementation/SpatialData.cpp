#include <Core/CorePCH.h>

#include <Core/World/SpatialData.h>

nsHybridArray<nsSpatialData::CategoryData, 32>& nsSpatialData::GetCategoryData()
{
  static nsHybridArray<nsSpatialData::CategoryData, 32> CategoryData;
  return CategoryData;
}

// static
nsSpatialData::Category nsSpatialData::RegisterCategory(nsStringView sCategoryName, const nsBitflags<Flags>& flags)
{
  if (sCategoryName.IsEmpty())
    return nsInvalidSpatialDataCategory;

  Category oldCategory = FindCategory(sCategoryName);
  if (oldCategory != nsInvalidSpatialDataCategory)
  {
    NS_ASSERT_DEV(GetCategoryFlags(oldCategory) == flags, "Category registered with different flags");
    return oldCategory;
  }

  if (GetCategoryData().GetCount() == 32)
  {
    NS_REPORT_FAILURE("Too many spatial data categories");
    return nsInvalidSpatialDataCategory;
  }

  Category newCategory = Category(static_cast<nsUInt16>(GetCategoryData().GetCount()));

  auto& data = GetCategoryData().ExpandAndGetRef();
  data.m_sName.Assign(sCategoryName);
  data.m_Flags = flags;

  return newCategory;
}

// static
nsSpatialData::Category nsSpatialData::FindCategory(nsStringView sCategoryName)
{
  nsTempHashedString categoryName(sCategoryName);

  for (nsUInt32 uiCategoryIndex = 0; uiCategoryIndex < GetCategoryData().GetCount(); ++uiCategoryIndex)
  {
    if (GetCategoryData()[uiCategoryIndex].m_sName == categoryName)
      return Category(static_cast<nsUInt16>(uiCategoryIndex));
  }

  return nsInvalidSpatialDataCategory;
}

// static
const nsHashedString& nsSpatialData::GetCategoryName(Category category)
{
  if (category.m_uiValue < GetCategoryData().GetCount())
  {
    return GetCategoryData()[category.m_uiValue].m_sName;
  }

  static nsHashedString sInvalidSpatialDataCategoryName;
  return sInvalidSpatialDataCategoryName;
}

// static
const nsBitflags<nsSpatialData::Flags>& nsSpatialData::GetCategoryFlags(Category category)
{
  return GetCategoryData()[category.m_uiValue].m_Flags;
}

//////////////////////////////////////////////////////////////////////////

nsSpatialData::Category nsDefaultSpatialDataCategories::RenderStatic = nsSpatialData::RegisterCategory("RenderStatic", nsSpatialData::Flags::None);
nsSpatialData::Category nsDefaultSpatialDataCategories::RenderDynamic = nsSpatialData::RegisterCategory("RenderDynamic", nsSpatialData::Flags::FrequentChanges);
nsSpatialData::Category nsDefaultSpatialDataCategories::OcclusionStatic = nsSpatialData::RegisterCategory("OcclusionStatic", nsSpatialData::Flags::None);
nsSpatialData::Category nsDefaultSpatialDataCategories::OcclusionDynamic = nsSpatialData::RegisterCategory("OcclusionDynamic", nsSpatialData::Flags::FrequentChanges);
