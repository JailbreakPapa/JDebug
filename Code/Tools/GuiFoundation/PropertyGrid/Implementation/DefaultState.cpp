#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/SubSystem.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <GuiFoundation/PropertyGrid/AttributeDefaultStateProvider.h>
#include <GuiFoundation/PropertyGrid/DefaultState.h>
#include <GuiFoundation/PropertyGrid/PrefabDefaultStateProvider.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, DefaultState)
  ON_CORESYSTEMS_STARTUP
  {
    wdDefaultState::RegisterDefaultStateProvider(wdAttributeDefaultStateProvider::CreateProvider);
    wdDefaultState::RegisterDefaultStateProvider(wdPrefabDefaultStateProvider::CreateProvider);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdDefaultState::UnregisterDefaultStateProvider(wdAttributeDefaultStateProvider::CreateProvider);
    wdDefaultState::UnregisterDefaultStateProvider(wdPrefabDefaultStateProvider::CreateProvider);
  }
WD_END_SUBSYSTEM_DECLARATION;
// clang-format on


wdDynamicArray<wdDefaultState::CreateStateProviderFunc> wdDefaultState::s_Factories;

void wdDefaultState::RegisterDefaultStateProvider(CreateStateProviderFunc func)
{
  s_Factories.PushBack(func);
}

void wdDefaultState::UnregisterDefaultStateProvider(CreateStateProviderFunc func)
{
  s_Factories.RemoveAndCopy(func);
}

//////////////////////////////////////////////////////////////////////////

wdDefaultObjectState::wdDefaultObjectState(wdObjectAccessorBase* pAccessor, const wdArrayPtr<wdPropertySelection> selection)
{
  m_pAccessor = pAccessor;
  m_Selection = selection;
  m_Providers.Reserve(m_Selection.GetCount());
  for (const wdPropertySelection& sel : m_Selection)
  {
    auto& pProviders = m_Providers.ExpandAndGetRef();
    for (auto& func : wdDefaultState::s_Factories)
    {
      wdSharedPtr<wdDefaultStateProvider> pProvider = func(pAccessor, sel.m_pObject, nullptr);
      if (pProvider != nullptr)
      {
        pProviders.PushBack(std::move(pProvider));
      }
      pProviders.Sort([](const wdSharedPtr<wdDefaultStateProvider>& pA, const wdSharedPtr<wdDefaultStateProvider>& pB) -> bool { return pA->GetRootDepth() > pB->GetRootDepth(); });
    }
  }
}

wdColorGammaUB wdDefaultObjectState::GetBackgroundColor() const
{
  return m_Providers[0][0]->GetBackgroundColor();
}

wdString wdDefaultObjectState::GetStateProviderName() const
{
  return m_Providers[0][0]->GetStateProviderName();
}

bool wdDefaultObjectState::IsDefaultValue(const char* szProperty) const
{
  const wdAbstractProperty* pProp = m_Selection[0].m_pObject->GetTypeAccessor().GetType()->FindPropertyByName(szProperty);
  return IsDefaultValue(pProp);
}

bool wdDefaultObjectState::IsDefaultValue(const wdAbstractProperty* pProp) const
{
  const wdUInt32 uiObjects = m_Providers.GetCount();
  for (wdUInt32 i = 0; i < uiObjects; i++)
  {
    wdDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    const bool bNewDefault = m_Providers[i][0]->IsDefaultValue(super, m_pAccessor, m_Selection[i].m_pObject, pProp);
    if (!bNewDefault)
      return false;
  }
  return true;
}

wdStatus wdDefaultObjectState::RevertProperty(const char* szProperty)
{
  const wdAbstractProperty* pProp = m_Selection[0].m_pObject->GetTypeAccessor().GetType()->FindPropertyByName(szProperty);
  return RevertProperty(pProp);
}

wdStatus wdDefaultObjectState::RevertProperty(const wdAbstractProperty* pProp)
{
  const wdUInt32 uiObjects = m_Providers.GetCount();
  for (wdUInt32 i = 0; i < uiObjects; i++)
  {
    wdDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    wdStatus res = m_Providers[i][0]->RevertProperty(super, m_pAccessor, m_Selection[i].m_pObject, pProp);
    if (res.Failed())
      return res;
  }
  return wdStatus(WD_SUCCESS);
}

wdStatus wdDefaultObjectState::RevertObject()
{
  const wdUInt32 uiObjects = m_Providers.GetCount();
  for (wdUInt32 i = 0; i < uiObjects; i++)
  {
    wdDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);

    wdHybridArray<wdAbstractProperty*, 32> properties;
    m_Selection[i].m_pObject->GetType()->GetAllProperties(properties);
    for (wdAbstractProperty* pProp : properties)
    {
      if (pProp->GetFlags().IsAnySet(wdPropertyFlags::Hidden | wdPropertyFlags::ReadOnly))
        continue;
      wdStatus res = m_Providers[i][0]->RevertProperty(super, m_pAccessor, m_Selection[i].m_pObject, pProp);
      if (res.Failed())
        return res;
    }
  }
  return wdStatus(WD_SUCCESS);
}

wdVariant wdDefaultObjectState::GetDefaultValue(const char* szProperty, wdUInt32 uiSelectionIndex) const
{
  const wdAbstractProperty* pProp = m_Selection[0].m_pObject->GetTypeAccessor().GetType()->FindPropertyByName(szProperty);
  return GetDefaultValue(pProp, uiSelectionIndex);
}

wdVariant wdDefaultObjectState::GetDefaultValue(const wdAbstractProperty* pProp, wdUInt32 uiSelectionIndex) const
{
  WD_ASSERT_DEBUG(uiSelectionIndex < m_Selection.GetCount(), "Selection index is out of bounds.");
  wdDefaultStateProvider::SuperArray super = m_Providers[uiSelectionIndex].GetArrayPtr().GetSubArray(1);
  return m_Providers[uiSelectionIndex][0]->GetDefaultValue(super, m_pAccessor, m_Selection[uiSelectionIndex].m_pObject, pProp);
}

//////////////////////////////////////////////////////////////////////////

wdDefaultContainerState::wdDefaultContainerState(wdObjectAccessorBase* pAccessor, const wdArrayPtr<wdPropertySelection> selection, const char* szProperty)
{
  m_pAccessor = pAccessor;
  m_Selection = selection;
  // We assume selections can only contain objects of the same (base) type.
  m_pProp = szProperty ? selection[0].m_pObject->GetTypeAccessor().GetType()->FindPropertyByName(szProperty) : nullptr;
  m_Providers.Reserve(m_Selection.GetCount());
  for (const wdPropertySelection& sel : m_Selection)
  {
    auto& pProviders = m_Providers.ExpandAndGetRef();
    for (auto& func : wdDefaultState::s_Factories)
    {
      wdSharedPtr<wdDefaultStateProvider> pProvider = func(pAccessor, sel.m_pObject, m_pProp);
      if (pProvider != nullptr)
      {
        pProviders.PushBack(std::move(pProvider));
      }
      pProviders.Sort([](const wdSharedPtr<wdDefaultStateProvider>& pA, const wdSharedPtr<wdDefaultStateProvider>& pB) -> bool { return pA->GetRootDepth() > pB->GetRootDepth(); });
    }
  }
}

wdColorGammaUB wdDefaultContainerState::GetBackgroundColor() const
{
  return m_Providers[0][0]->GetBackgroundColor();
}

wdString wdDefaultContainerState::GetStateProviderName() const
{
  return m_Providers[0][0]->GetStateProviderName();
}

bool wdDefaultContainerState::IsDefaultElement(wdVariant index) const
{
  const wdUInt32 uiObjects = m_Providers.GetCount();
  for (wdUInt32 i = 0; i < uiObjects; i++)
  {
    wdDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    WD_ASSERT_DEBUG(index.IsValid() || m_Selection[i].m_Index.IsValid(), "If wdDefaultContainerState is constructed without giving an indices in the selection, one must be provided on the IsDefaultElement call.");
    const bool bNewDefault = m_Providers[i][0]->IsDefaultValue(super, m_pAccessor, m_Selection[i].m_pObject, m_pProp, index.IsValid() ? index : m_Selection[i].m_Index);
    if (!bNewDefault)
      return false;
  }
  return true;
}

bool wdDefaultContainerState::IsDefaultContainer() const
{
  const wdUInt32 uiObjects = m_Providers.GetCount();
  for (wdUInt32 i = 0; i < uiObjects; i++)
  {
    wdDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    const bool bNewDefault = m_Providers[i][0]->IsDefaultValue(super, m_pAccessor, m_Selection[i].m_pObject, m_pProp);
    if (!bNewDefault)
      return false;
  }
  return true;
}

wdStatus wdDefaultContainerState::RevertElement(wdVariant index)
{
  const wdUInt32 uiObjects = m_Providers.GetCount();
  for (wdUInt32 i = 0; i < uiObjects; i++)
  {
    wdDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    WD_ASSERT_DEBUG(index.IsValid() || m_Selection[i].m_Index.IsValid(), "If wdDefaultContainerState is constructed without giving an indices in the selection, one must be provided on the RevertElement call.");
    wdStatus res = m_Providers[i][0]->RevertProperty(super, m_pAccessor, m_Selection[i].m_pObject, m_pProp, index.IsValid() ? index : m_Selection[i].m_Index);
    if (res.Failed())
      return res;
  }
  return wdStatus(WD_SUCCESS);
}

wdStatus wdDefaultContainerState::RevertContainer()
{
  const wdUInt32 uiObjects = m_Providers.GetCount();
  for (wdUInt32 i = 0; i < uiObjects; i++)
  {
    wdDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    wdStatus res = m_Providers[i][0]->RevertProperty(super, m_pAccessor, m_Selection[i].m_pObject, m_pProp);
    if (res.Failed())
      return res;
  }
  return wdStatus(WD_SUCCESS);
}

wdVariant wdDefaultContainerState::GetDefaultElement(wdVariant index, wdUInt32 uiSelectionIndex) const
{
  WD_ASSERT_DEBUG(uiSelectionIndex < m_Selection.GetCount(), "Selection index is out of bounds.");
  wdDefaultStateProvider::SuperArray super = m_Providers[uiSelectionIndex].GetArrayPtr().GetSubArray(1);
  return m_Providers[uiSelectionIndex][0]->GetDefaultValue(super, m_pAccessor, m_Selection[uiSelectionIndex].m_pObject, m_pProp, index);
}

wdVariant wdDefaultContainerState::GetDefaultContainer(wdUInt32 uiSelectionIndex) const
{
  WD_ASSERT_DEBUG(uiSelectionIndex < m_Selection.GetCount(), "Selection index is out of bounds.");
  wdDefaultStateProvider::SuperArray super = m_Providers[uiSelectionIndex].GetArrayPtr().GetSubArray(1);
  return m_Providers[uiSelectionIndex][0]->GetDefaultValue(super, m_pAccessor, m_Selection[uiSelectionIndex].m_pObject, m_pProp);
}

//////////////////////////////////////////////////////////////////////////


bool wdDefaultStateProvider::IsDefaultValue(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index)
{
  const wdVariant def = GetDefaultValue(superPtr, pAccessor, pObject, pProp, index);
  wdVariant value;
  pAccessor->GetValue(pObject, pProp, value, index).LogFailure();

  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp) || pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags);
  if (index.IsValid() && !bIsValueType)
  {
    //#TODO we do not support reverting entire objects just yet.
    return true;
  }

  return def == value;
}

wdStatus wdDefaultStateProvider::RevertProperty(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index)
{
  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp) || pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags);
  if (!bIsValueType)
  {
    WD_ASSERT_DEBUG(!index.IsValid(), "Reverting non-value type container elements is not supported yet. IsDefaultValue should have returned true to prevent this call from being allowed.");

    return RevertObjectContainer(superPtr, pAccessor, pObject, pProp);
  }

  wdDeque<wdAbstractGraphDiffOperation> diff;
  auto& op = diff.ExpandAndGetRef();
  op.m_Node = pObject->GetGuid();
  op.m_Operation = wdAbstractGraphDiffOperation::Op::PropertyChanged;
  op.m_sProperty = pProp->GetPropertyName();
  op.m_uiTypeVersion = 0;
  if (index.IsValid())
  {
    wdVariant def = GetDefaultValue(superPtr, pAccessor, pObject, pProp, index);
    switch (pProp->GetCategory())
    {
      case wdPropertyCategory::Array:
      case wdPropertyCategory::Set:
      {
        WD_ASSERT_DEBUG(index.CanConvertTo<wdInt32>(), "Array / Set indices must be integers.");
        WD_SUCCEED_OR_RETURN(pAccessor->GetValue(pObject, pProp, op.m_Value));
        WD_ASSERT_DEBUG(op.m_Value.IsA<wdVariantArray>(), "");

        wdVariantArray& currentValue2 = op.m_Value.GetWritable<wdVariantArray>();
        currentValue2[index.ConvertTo<wdUInt32>()] = def;
      }
      break;
      case wdPropertyCategory::Map:
      {
        WD_ASSERT_DEBUG(index.IsString(), "Map indices must be strings.");
        WD_SUCCEED_OR_RETURN(pAccessor->GetValue(pObject, pProp, op.m_Value));
        WD_ASSERT_DEBUG(op.m_Value.IsA<wdVariantDictionary>(), "");

        wdVariantDictionary& currentValue2 = op.m_Value.GetWritable<wdVariantDictionary>();
        currentValue2[index.ConvertTo<wdString>()] = def;
      }
      break;
      default:
        break;
    }
  }
  else
  {
    wdVariant def = GetDefaultValue(superPtr, pAccessor, pObject, pProp, index);
    op.m_Value = def;
  }

  wdDocumentObjectConverterReader::ApplyDiffToObject(pAccessor, pObject, diff);
  return wdStatus(WD_SUCCESS);
}

wdStatus wdDefaultStateProvider::RevertObjectContainer(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp)
{
  wdDeque<wdAbstractGraphDiffOperation> diff;
  wdStatus res = CreateRevertContainerDiff(superPtr, pAccessor, pObject, pProp, diff);
  if (res.Succeeded())
  {
    wdDocumentObjectConverterReader::ApplyDiffToObject(pAccessor, pObject, diff);
  }
  return res;
}

bool wdDefaultStateProvider::DoesVariantMatchProperty(const wdVariant& value, const wdAbstractProperty* pProp, wdVariant index)
{
  const bool bIsValueType = wdReflectionUtils::IsValueType(pProp) || pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags);

  if (pProp->GetSpecificType() == wdGetStaticRTTI<wdVariant>())
    return true;

  auto MatchesElementType = [&](const wdVariant& value2) -> bool {
    if (pProp->GetFlags().IsAnySet(wdPropertyFlags::IsEnum | wdPropertyFlags::Bitflags))
    {
      return value2.IsNumber() && !value2.IsFloatingPoint();
    }
    else if (pProp->GetFlags().IsAnySet(wdPropertyFlags::StandardType))
    {
      return value2.CanConvertTo(pProp->GetSpecificType()->GetVariantType());
    }
    else if (bIsValueType)
    {
      return value2.GetReflectedType() == pProp->GetSpecificType();
    }
    else
    {
      return value2.IsA<wdUuid>();
    }
  };

  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      return MatchesElementType(value);
    }
    break;
    case wdPropertyCategory::Array:
    case wdPropertyCategory::Set:
    {
      if (index.IsValid())
      {
        return MatchesElementType(value);
      }
      else
      {
        if (value.IsA<wdVariantArray>())
        {
          const wdVariantArray& valueArray = value.Get<wdVariantArray>();
          return std::all_of(cbegin(valueArray), cend(valueArray), MatchesElementType);
        }
      }
    }
    break;
    case wdPropertyCategory::Map:
    {
      if (index.IsValid())
      {
        return MatchesElementType(value);
      }
      else
      {
        if (value.IsA<wdVariantDictionary>())
        {
          const wdVariantDictionary& valueDict = value.Get<wdVariantDictionary>();
          return std::all_of(cbegin(valueDict), cend(valueDict), [&](const auto& it) { return MatchesElementType(it.Value()); });
        }
      }
    }
    break;
    default:
      break;
  }
  return false;
}
