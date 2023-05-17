#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/Action/BaseActions.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdNamedAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCategoryAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdMenuAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDynamicMenuAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDynamicActionAndMenuAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdEnumerationMenuAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdButtonAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdSliderAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdDynamicActionAndMenuAction::wdDynamicActionAndMenuAction(const wdActionContext& context, const char* szName, const char* szIconPath)
  : wdDynamicMenuAction(context, szName, szIconPath)
{
  m_bEnabled = true;
  m_bVisible = true;
}

wdEnumerationMenuAction::wdEnumerationMenuAction(const wdActionContext& context, const char* szName, const char* szIconPath)
  : wdDynamicMenuAction(context, szName, szIconPath)
{
  m_pEnumerationType = nullptr;
}

void wdEnumerationMenuAction::InitEnumerationType(const wdRTTI* pEnumerationType)
{
  m_pEnumerationType = pEnumerationType;
}

void wdEnumerationMenuAction::GetEntries(wdHybridArray<wdDynamicMenuAction::Item, 16>& out_entries)
{
  out_entries.Clear();
  out_entries.Reserve(m_pEnumerationType->GetProperties().GetCount() - 1);
  wdInt64 iCurrentValue = wdReflectionUtils::MakeEnumerationValid(m_pEnumerationType, GetValue());

  // sort entries by group / category
  // categories appear in the order in which they are used on the reflected properties
  // within each category, items are sorted by 'order'
  // all items that have the same 'order' are sorted alphabetically by display string

  wdStringBuilder sCurGroup;
  float fPrevOrder = -1;
  struct ItemWithOrder
  {
    float m_fOrder = -1;
    wdDynamicMenuAction::Item m_Item;

    bool operator<(const ItemWithOrder& rhs) const
    {
      if (m_fOrder == rhs.m_fOrder)
      {
        return m_Item.m_sDisplay < rhs.m_Item.m_sDisplay;
      }

      return m_fOrder < rhs.m_fOrder;
    }
  };

  wdHybridArray<ItemWithOrder, 16> unsortedItems;

  auto appendToOutput = [&]() {
    if (unsortedItems.IsEmpty())
      return;

    unsortedItems.Sort();

    if (!out_entries.IsEmpty())
    {
      // add a separator between groups
      out_entries.ExpandAndGetRef().m_ItemFlags.Add(wdDynamicMenuAction::Item::ItemFlags::Separator);
    }

    for (const auto& sortedItem : unsortedItems)
    {
      out_entries.PushBack(sortedItem.m_Item);
    }

    unsortedItems.Clear();
  };

  for (auto pProp : m_pEnumerationType->GetProperties().GetSubArray(1))
  {
    if (pProp->GetCategory() == wdPropertyCategory::Constant)
    {
      if (const wdGroupAttribute* pGroup = pProp->GetAttributeByType<wdGroupAttribute>())
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
        wdInt64 iValue = static_cast<const wdAbstractConstantProperty*>(pProp)->GetConstant().ConvertTo<wdInt64>();

        item.m_sDisplay = wdTranslate(pProp->GetPropertyName());

        item.m_UserValue = iValue;
        if (m_pEnumerationType->IsDerivedFrom<wdEnumBase>())
        {
          item.m_CheckState =
            (iCurrentValue == iValue) ? wdDynamicMenuAction::Item::CheckMark::Checked : wdDynamicMenuAction::Item::CheckMark::Unchecked;
        }
        else if (m_pEnumerationType->IsDerivedFrom<wdBitflagsBase>())
        {
          item.m_CheckState =
            ((iCurrentValue & iValue) != 0) ? wdDynamicMenuAction::Item::CheckMark::Checked : wdDynamicMenuAction::Item::CheckMark::Unchecked;
        }
      }
    }
  }

  appendToOutput();
}

wdButtonAction::wdButtonAction(const wdActionContext& context, const char* szName, bool bCheckable, const char* szIconPath)
  : wdNamedAction(context, szName, szIconPath)
{
  m_bCheckable = false;
  m_bChecked = false;
  m_bEnabled = true;
  m_bVisible = true;
}


wdSliderAction::wdSliderAction(const wdActionContext& context, const char* szName)
  : wdNamedAction(context, szName, nullptr)
{
  m_bEnabled = true;
  m_bVisible = true;
  m_iMinValue = 0;
  m_iMaxValue = 100;
  m_iCurValue = 50;
}

void wdSliderAction::SetRange(wdInt32 iMin, wdInt32 iMax, bool bTriggerUpdate /*= true*/)
{
  WD_ASSERT_DEBUG(iMin < iMax, "Invalid range");

  m_iMinValue = iMin;
  m_iMaxValue = iMax;

  if (bTriggerUpdate)
    TriggerUpdate();
}

void wdSliderAction::SetValue(wdInt32 iVal, bool bTriggerUpdate /*= true*/)
{
  m_iCurValue = wdMath::Clamp(iVal, m_iMinValue, m_iMaxValue);
  if (bTriggerUpdate)
    TriggerUpdate();
}
