#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/Action/BaseActions.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsNamedAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCategoryAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMenuAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDynamicMenuAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDynamicActionAndMenuAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEnumerationMenuAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsButtonAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSliderAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsDynamicActionAndMenuAction::nsDynamicActionAndMenuAction(const nsActionContext& context, const char* szName, const char* szIconPath)
  : nsDynamicMenuAction(context, szName, szIconPath)
{
  m_bEnabled = true;
  m_bVisible = true;
}

nsEnumerationMenuAction::nsEnumerationMenuAction(const nsActionContext& context, const char* szName, const char* szIconPath)
  : nsDynamicMenuAction(context, szName, szIconPath)
{
  m_pEnumerationType = nullptr;
}

void nsEnumerationMenuAction::InitEnumerationType(const nsRTTI* pEnumerationType)
{
  m_pEnumerationType = pEnumerationType;
}

void nsEnumerationMenuAction::GetEntries(nsHybridArray<nsDynamicMenuAction::Item, 16>& out_entries)
{
  out_entries.Clear();
  out_entries.Reserve(m_pEnumerationType->GetProperties().GetCount() - 1);
  nsInt64 iCurrentValue = nsReflectionUtils::MakeEnumerationValid(m_pEnumerationType, GetValue());

  // sort entries by group / category
  // categories appear in the order in which they are used on the reflected properties
  // within each category, items are sorted by 'order'
  // all items that have the same 'order' are sorted alphabetically by display string

  nsStringBuilder sCurGroup;
  float fPrevOrder = -1;
  struct ItemWithOrder
  {
    float m_fOrder = -1;
    nsDynamicMenuAction::Item m_Item;

    bool operator<(const ItemWithOrder& rhs) const
    {
      if (m_fOrder == rhs.m_fOrder)
      {
        return m_Item.m_sDisplay < rhs.m_Item.m_sDisplay;
      }

      return m_fOrder < rhs.m_fOrder;
    }
  };

  nsHybridArray<ItemWithOrder, 16> unsortedItems;

  auto appendToOutput = [&]()
  {
    if (unsortedItems.IsEmpty())
      return;

    unsortedItems.Sort();

    if (!out_entries.IsEmpty())
    {
      // add a separator between groups
      out_entries.ExpandAndGetRef().m_ItemFlags.Add(nsDynamicMenuAction::Item::ItemFlags::Separator);
    }

    for (const auto& sortedItem : unsortedItems)
    {
      out_entries.PushBack(sortedItem.m_Item);
    }

    unsortedItems.Clear();
  };

  for (auto pProp : m_pEnumerationType->GetProperties().GetSubArray(1))
  {
    if (pProp->GetCategory() == nsPropertyCategory::Constant)
    {
      if (const nsGroupAttribute* pGroup = pProp->GetAttributeByType<nsGroupAttribute>())
      {
        if (sCurGroup != pGroup->GetGroup())
        {
          sCurGroup = pGroup->GetGroup();

          appendToOutput();
        }

        fPrevOrder = pGroup->GetOrder();
      }

      ItemWithOrder& newItem = unsortedItems.ExpandAndGetRef();
      newItem.m_fOrder = fPrevOrder;
      auto& item = newItem.m_Item;

      {
        nsInt64 iValue = static_cast<const nsAbstractConstantProperty*>(pProp)->GetConstant().ConvertTo<nsInt64>();

        item.m_sDisplay = nsTranslate(pProp->GetPropertyName());

        item.m_UserValue = iValue;
        if (m_pEnumerationType->IsDerivedFrom<nsEnumBase>())
        {
          item.m_CheckState =
            (iCurrentValue == iValue) ? nsDynamicMenuAction::Item::CheckMark::Checked : nsDynamicMenuAction::Item::CheckMark::Unchecked;
        }
        else if (m_pEnumerationType->IsDerivedFrom<nsBitflagsBase>())
        {
          item.m_CheckState =
            ((iCurrentValue & iValue) != 0) ? nsDynamicMenuAction::Item::CheckMark::Checked : nsDynamicMenuAction::Item::CheckMark::Unchecked;
        }
      }
    }
  }

  appendToOutput();
}

nsButtonAction::nsButtonAction(const nsActionContext& context, const char* szName, bool bCheckable, const char* szIconPath)
  : nsNamedAction(context, szName, szIconPath)
{
  m_bCheckable = false;
  m_bChecked = false;
  m_bEnabled = true;
  m_bVisible = true;
}


nsSliderAction::nsSliderAction(const nsActionContext& context, const char* szName)
  : nsNamedAction(context, szName, nullptr)
{
  m_bEnabled = true;
  m_bVisible = true;
  m_iMinValue = 0;
  m_iMaxValue = 100;
  m_iCurValue = 50;
}

void nsSliderAction::SetRange(nsInt32 iMin, nsInt32 iMax, bool bTriggerUpdate /*= true*/)
{
  NS_ASSERT_DEBUG(iMin < iMax, "Invalid range");

  m_iMinValue = iMin;
  m_iMaxValue = iMax;

  if (bTriggerUpdate)
    TriggerUpdate();
}

void nsSliderAction::SetValue(nsInt32 iVal, bool bTriggerUpdate /*= true*/)
{
  m_iCurValue = nsMath::Clamp(iVal, m_iMinValue, m_iMaxValue);
  if (bTriggerUpdate)
    TriggerUpdate();
}
