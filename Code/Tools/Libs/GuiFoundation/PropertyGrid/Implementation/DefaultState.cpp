#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Configuration/SubSystem.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <GuiFoundation/PropertyGrid/AttributeDefaultStateProvider.h>
#include <GuiFoundation/PropertyGrid/DefaultState.h>
#include <GuiFoundation/PropertyGrid/PrefabDefaultStateProvider.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, DefaultState)
  ON_CORESYSTEMS_STARTUP
  {
    nsDefaultState::RegisterDefaultStateProvider(nsAttributeDefaultStateProvider::CreateProvider);
    nsDefaultState::RegisterDefaultStateProvider(nsPrefabDefaultStateProvider::CreateProvider);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsDefaultState::UnregisterDefaultStateProvider(nsAttributeDefaultStateProvider::CreateProvider);
    nsDefaultState::UnregisterDefaultStateProvider(nsPrefabDefaultStateProvider::CreateProvider);
  }
NS_END_SUBSYSTEM_DECLARATION;
// clang-format on


nsDynamicArray<nsDefaultState::CreateStateProviderFunc> nsDefaultState::s_Factories;

void nsDefaultState::RegisterDefaultStateProvider(CreateStateProviderFunc func)
{
  s_Factories.PushBack(func);
}

void nsDefaultState::UnregisterDefaultStateProvider(CreateStateProviderFunc func)
{
  s_Factories.RemoveAndCopy(func);
}

//////////////////////////////////////////////////////////////////////////

nsDefaultObjectState::nsDefaultObjectState(nsObjectAccessorBase* pAccessor, const nsArrayPtr<nsPropertySelection> selection)
{
  m_pAccessor = pAccessor;
  m_Selection = selection;
  m_Providers.Reserve(m_Selection.GetCount());
  for (const nsPropertySelection& sel : m_Selection)
  {
    auto& pProviders = m_Providers.ExpandAndGetRef();
    for (auto& func : nsDefaultState::s_Factories)
    {
      nsSharedPtr<nsDefaultStateProvider> pProvider = func(pAccessor, sel.m_pObject, nullptr);
      if (pProvider != nullptr)
      {
        pProviders.PushBack(std::move(pProvider));
      }
      pProviders.Sort([](const nsSharedPtr<nsDefaultStateProvider>& pA, const nsSharedPtr<nsDefaultStateProvider>& pB) -> bool
        { return pA->GetRootDepth() > pB->GetRootDepth(); });
    }
  }
}

nsColorGammaUB nsDefaultObjectState::GetBackgroundColor() const
{
  return m_Providers[0][0]->GetBackgroundColor();
}

nsString nsDefaultObjectState::GetStateProviderName() const
{
  return m_Providers[0][0]->GetStateProviderName();
}

bool nsDefaultObjectState::IsDefaultValue(const char* szProperty) const
{
  const nsAbstractProperty* pProp = m_Selection[0].m_pObject->GetTypeAccessor().GetType()->FindPropertyByName(szProperty);
  return IsDefaultValue(pProp);
}

bool nsDefaultObjectState::IsDefaultValue(const nsAbstractProperty* pProp) const
{
  const nsUInt32 uiObjects = m_Providers.GetCount();
  for (nsUInt32 i = 0; i < uiObjects; i++)
  {
    nsDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    const bool bNewDefault = m_Providers[i][0]->IsDefaultValue(super, m_pAccessor, m_Selection[i].m_pObject, pProp);
    if (!bNewDefault)
      return false;
  }
  return true;
}

nsStatus nsDefaultObjectState::RevertProperty(const char* szProperty)
{
  const nsAbstractProperty* pProp = m_Selection[0].m_pObject->GetTypeAccessor().GetType()->FindPropertyByName(szProperty);
  return RevertProperty(pProp);
}

nsStatus nsDefaultObjectState::RevertProperty(const nsAbstractProperty* pProp)
{
  const nsUInt32 uiObjects = m_Providers.GetCount();
  for (nsUInt32 i = 0; i < uiObjects; i++)
  {
    nsDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    nsStatus res = m_Providers[i][0]->RevertProperty(super, m_pAccessor, m_Selection[i].m_pObject, pProp);
    if (res.Failed())
      return res;
  }
  return nsStatus(NS_SUCCESS);
}

nsStatus nsDefaultObjectState::RevertObject()
{
  const nsUInt32 uiObjects = m_Providers.GetCount();
  for (nsUInt32 i = 0; i < uiObjects; i++)
  {
    nsDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);

    nsHybridArray<const nsAbstractProperty*, 32> properties;
    m_Selection[i].m_pObject->GetType()->GetAllProperties(properties);
    for (auto pProp : properties)
    {
      if (pProp->GetFlags().IsAnySet(nsPropertyFlags::Hidden | nsPropertyFlags::ReadOnly))
        continue;
      nsStatus res = m_Providers[i][0]->RevertProperty(super, m_pAccessor, m_Selection[i].m_pObject, pProp);
      if (res.Failed())
        return res;
    }
  }
  return nsStatus(NS_SUCCESS);
}

nsVariant nsDefaultObjectState::GetDefaultValue(const char* szProperty, nsUInt32 uiSelectionIndex) const
{
  const nsAbstractProperty* pProp = m_Selection[0].m_pObject->GetTypeAccessor().GetType()->FindPropertyByName(szProperty);
  return GetDefaultValue(pProp, uiSelectionIndex);
}

nsVariant nsDefaultObjectState::GetDefaultValue(const nsAbstractProperty* pProp, nsUInt32 uiSelectionIndex) const
{
  NS_ASSERT_DEBUG(uiSelectionIndex < m_Selection.GetCount(), "Selection index is out of bounds.");
  nsDefaultStateProvider::SuperArray super = m_Providers[uiSelectionIndex].GetArrayPtr().GetSubArray(1);
  return m_Providers[uiSelectionIndex][0]->GetDefaultValue(super, m_pAccessor, m_Selection[uiSelectionIndex].m_pObject, pProp);
}

//////////////////////////////////////////////////////////////////////////

nsDefaultContainerState::nsDefaultContainerState(nsObjectAccessorBase* pAccessor, const nsArrayPtr<nsPropertySelection> selection, const char* szProperty)
{
  m_pAccessor = pAccessor;
  m_Selection = selection;
  // We assume selections can only contain objects of the same (base) type.
  m_pProp = szProperty ? selection[0].m_pObject->GetTypeAccessor().GetType()->FindPropertyByName(szProperty) : nullptr;
  m_Providers.Reserve(m_Selection.GetCount());
  for (const nsPropertySelection& sel : m_Selection)
  {
    auto& pProviders = m_Providers.ExpandAndGetRef();
    for (auto& func : nsDefaultState::s_Factories)
    {
      nsSharedPtr<nsDefaultStateProvider> pProvider = func(pAccessor, sel.m_pObject, m_pProp);
      if (pProvider != nullptr)
      {
        pProviders.PushBack(std::move(pProvider));
      }
      pProviders.Sort([](const nsSharedPtr<nsDefaultStateProvider>& pA, const nsSharedPtr<nsDefaultStateProvider>& pB) -> bool
        { return pA->GetRootDepth() > pB->GetRootDepth(); });
    }
  }
}

nsColorGammaUB nsDefaultContainerState::GetBackgroundColor() const
{
  return m_Providers[0][0]->GetBackgroundColor();
}

nsString nsDefaultContainerState::GetStateProviderName() const
{
  return m_Providers[0][0]->GetStateProviderName();
}

bool nsDefaultContainerState::IsDefaultElement(nsVariant index) const
{
  const nsUInt32 uiObjects = m_Providers.GetCount();
  for (nsUInt32 i = 0; i < uiObjects; i++)
  {
    nsDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    NS_ASSERT_DEBUG(index.IsValid() || m_Selection[i].m_Index.IsValid(), "If nsDefaultContainerState is constructed without giving an indices in the selection, one must be provided on the IsDefaultElement call.");
    const bool bNewDefault = m_Providers[i][0]->IsDefaultValue(super, m_pAccessor, m_Selection[i].m_pObject, m_pProp, index.IsValid() ? index : m_Selection[i].m_Index);
    if (!bNewDefault)
      return false;
  }
  return true;
}

bool nsDefaultContainerState::IsDefaultContainer() const
{
  const nsUInt32 uiObjects = m_Providers.GetCount();
  for (nsUInt32 i = 0; i < uiObjects; i++)
  {
    nsDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    const bool bNewDefault = m_Providers[i][0]->IsDefaultValue(super, m_pAccessor, m_Selection[i].m_pObject, m_pProp);
    if (!bNewDefault)
      return false;
  }
  return true;
}

nsStatus nsDefaultContainerState::RevertElement(nsVariant index)
{
  const nsUInt32 uiObjects = m_Providers.GetCount();
  for (nsUInt32 i = 0; i < uiObjects; i++)
  {
    nsDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    NS_ASSERT_DEBUG(index.IsValid() || m_Selection[i].m_Index.IsValid(), "If nsDefaultContainerState is constructed without giving an indices in the selection, one must be provided on the RevertElement call.");
    nsStatus res = m_Providers[i][0]->RevertProperty(super, m_pAccessor, m_Selection[i].m_pObject, m_pProp, index.IsValid() ? index : m_Selection[i].m_Index);
    if (res.Failed())
      return res;
  }
  return nsStatus(NS_SUCCESS);
}

nsStatus nsDefaultContainerState::RevertContainer()
{
  const nsUInt32 uiObjects = m_Providers.GetCount();
  for (nsUInt32 i = 0; i < uiObjects; i++)
  {
    nsDefaultStateProvider::SuperArray super = m_Providers[i].GetArrayPtr().GetSubArray(1);
    nsStatus res = m_Providers[i][0]->RevertProperty(super, m_pAccessor, m_Selection[i].m_pObject, m_pProp);
    if (res.Failed())
      return res;
  }
  return nsStatus(NS_SUCCESS);
}

nsVariant nsDefaultContainerState::GetDefaultElement(nsVariant index, nsUInt32 uiSelectionIndex) const
{
  NS_ASSERT_DEBUG(uiSelectionIndex < m_Selection.GetCount(), "Selection index is out of bounds.");
  nsDefaultStateProvider::SuperArray super = m_Providers[uiSelectionIndex].GetArrayPtr().GetSubArray(1);
  return m_Providers[uiSelectionIndex][0]->GetDefaultValue(super, m_pAccessor, m_Selection[uiSelectionIndex].m_pObject, m_pProp, index);
}

nsVariant nsDefaultContainerState::GetDefaultContainer(nsUInt32 uiSelectionIndex) const
{
  NS_ASSERT_DEBUG(uiSelectionIndex < m_Selection.GetCount(), "Selection index is out of bounds.");
  nsDefaultStateProvider::SuperArray super = m_Providers[uiSelectionIndex].GetArrayPtr().GetSubArray(1);
  return m_Providers[uiSelectionIndex][0]->GetDefaultValue(super, m_pAccessor, m_Selection[uiSelectionIndex].m_pObject, m_pProp);
}

//////////////////////////////////////////////////////////////////////////


bool nsDefaultStateProvider::IsDefaultValue(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index)
{
  const nsVariant def = GetDefaultValue(superPtr, pAccessor, pObject, pProp, index);
  nsVariant value;
  pAccessor->GetValue(pObject, pProp, value, index).LogFailure();

  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp) || pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags);
  if (index.IsValid() && !bIsValueType)
  {
    // #TODO we do not support reverting entire objects just yet.
    return true;
  }

  return def == value;
}

nsStatus nsDefaultStateProvider::RevertProperty(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index)
{
  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp) || pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags);
  if (!bIsValueType)
  {
    NS_ASSERT_DEBUG(!index.IsValid(), "Reverting non-value type container elements is not supported yet. IsDefaultValue should have returned true to prevent this call from being allowed.");

    return RevertObjectContainer(superPtr, pAccessor, pObject, pProp);
  }

  nsDeque<nsAbstractGraphDiffOperation> diff;
  auto& op = diff.ExpandAndGetRef();
  op.m_Node = pObject->GetGuid();
  op.m_Operation = nsAbstractGraphDiffOperation::Op::PropertyChanged;
  op.m_sProperty = pProp->GetPropertyName();
  op.m_uiTypeVersion = 0;
  if (index.IsValid())
  {
    nsVariant def = GetDefaultValue(superPtr, pAccessor, pObject, pProp, index);
    switch (pProp->GetCategory())
    {
      case nsPropertyCategory::Array:
      case nsPropertyCategory::Set:
      {
        NS_ASSERT_DEBUG(index.CanConvertTo<nsInt32>(), "Array / Set indices must be integers.");
        NS_SUCCEED_OR_RETURN(pAccessor->GetValue(pObject, pProp, op.m_Value));
        NS_ASSERT_DEBUG(op.m_Value.IsA<nsVariantArray>(), "");

        nsVariantArray& currentValue2 = op.m_Value.GetWritable<nsVariantArray>();
        currentValue2[index.ConvertTo<nsUInt32>()] = def;
      }
      break;
      case nsPropertyCategory::Map:
      {
        NS_ASSERT_DEBUG(index.IsString(), "Map indices must be strings.");
        NS_SUCCEED_OR_RETURN(pAccessor->GetValue(pObject, pProp, op.m_Value));
        NS_ASSERT_DEBUG(op.m_Value.IsA<nsVariantDictionary>(), "");

        nsVariantDictionary& currentValue2 = op.m_Value.GetWritable<nsVariantDictionary>();
        currentValue2[index.ConvertTo<nsString>()] = def;
      }
      break;
      default:
        break;
    }
  }
  else
  {
    nsVariant def = GetDefaultValue(superPtr, pAccessor, pObject, pProp, index);
    op.m_Value = def;
  }

  nsDocumentObjectConverterReader::ApplyDiffToObject(pAccessor, pObject, diff);
  return nsStatus(NS_SUCCESS);
}

nsStatus nsDefaultStateProvider::RevertObjectContainer(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp)
{
  nsDeque<nsAbstractGraphDiffOperation> diff;
  nsStatus res = CreateRevertContainerDiff(superPtr, pAccessor, pObject, pProp, diff);
  if (res.Succeeded())
  {
    nsDocumentObjectConverterReader::ApplyDiffToObject(pAccessor, pObject, diff);
  }
  return res;
}

bool nsDefaultStateProvider::DoesVariantMatchProperty(const nsVariant& value, const nsAbstractProperty* pProp, nsVariant index)
{
  const bool bIsValueType = nsReflectionUtils::IsValueType(pProp) || pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags);

  if (pProp->GetSpecificType() == nsGetStaticRTTI<nsVariant>())
    return true;

  auto MatchesElementType = [&](const nsVariant& value2) -> bool
  {
    if (pProp->GetFlags().IsAnySet(nsPropertyFlags::IsEnum | nsPropertyFlags::Bitflags))
    {
      return value2.IsNumber() && !value2.IsFloatingPoint();
    }
    else if (pProp->GetFlags().IsAnySet(nsPropertyFlags::StandardType))
    {
      return value2.CanConvertTo(pProp->GetSpecificType()->GetVariantType());
    }
    else if (bIsValueType)
    {
      return value2.GetReflectedType() == pProp->GetSpecificType();
    }
    else
    {
      return value2.IsA<nsUuid>();
    }
  };

  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      return MatchesElementType(value);
    }
    break;
    case nsPropertyCategory::Array:
    case nsPropertyCategory::Set:
    {
      if (index.IsValid())
      {
        return MatchesElementType(value);
      }
      else
      {
        if (value.IsA<nsVariantArray>())
        {
          const nsVariantArray& valueArray = value.Get<nsVariantArray>();
          return std::all_of(cbegin(valueArray), cend(valueArray), MatchesElementType);
        }
      }
    }
    break;
    case nsPropertyCategory::Map:
    {
      if (index.IsValid())
      {
        return MatchesElementType(value);
      }
      else
      {
        if (value.IsA<nsVariantDictionary>())
        {
          const nsVariantDictionary& valueDict = value.Get<nsVariantDictionary>();
          return std::all_of(cbegin(valueDict), cend(valueDict), [&](const auto& it)
            { return MatchesElementType(it.Value()); });
        }
      }
    }
    break;
    default:
      break;
  }
  return false;
}
