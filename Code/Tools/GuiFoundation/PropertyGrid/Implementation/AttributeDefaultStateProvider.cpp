#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/AttributeDefaultStateProvider.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

static wdSharedPtr<wdDefaultStateProvider> g_pAttributeDefaultStateProvider = WD_DEFAULT_NEW(wdAttributeDefaultStateProvider);
wdSharedPtr<wdDefaultStateProvider> wdAttributeDefaultStateProvider::CreateProvider(wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp)
{
  // One global instance handles all. No need to create a new instance per request as no state need to be tracked.
  return g_pAttributeDefaultStateProvider;
}

wdInt32 wdAttributeDefaultStateProvider::GetRootDepth() const
{
  return -1;
}

wdColorGammaUB wdAttributeDefaultStateProvider::GetBackgroundColor() const
{
  // Set alpha to 0 -> color will be ignored.
  return wdColorGammaUB(0, 0, 0, 0);
}

wdVariant wdAttributeDefaultStateProvider::GetDefaultValue(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index)
{
  if (!pProp->GetFlags().IsSet(wdPropertyFlags::Pointer) && pProp->GetFlags().IsSet(wdPropertyFlags::Class) && pProp->GetCategory() == wdPropertyCategory::Member && !wdReflectionUtils::IsValueType(pProp))
  {
    // An embedded class that is not a value type can never change its value.
    wdVariant value;
    pAccessor->GetValue(pObject, pProp, value).LogFailure();
    return value;
  }
  return wdReflectionUtils::GetDefaultValue(pProp, index);
}

wdStatus wdAttributeDefaultStateProvider::CreateRevertContainerDiff(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDeque<wdAbstractGraphDiffOperation>& out_diff)
{
  auto RemoveObject = [&](const wdUuid& object) {
    auto& op = out_diff.ExpandAndGetRef();
    op.m_Node = object;
    op.m_Operation = wdAbstractGraphDiffOperation::Op::NodeRemoved;
    op.m_uiTypeVersion = 0;
    op.m_sProperty = pAccessor->GetObject(object)->GetType()->GetTypeName();
  };

  auto SetProperty = [&](const wdVariant& newValue) {
    auto& op = out_diff.ExpandAndGetRef();
    op.m_Node = pObject->GetGuid();
    op.m_Operation = wdAbstractGraphDiffOperation::Op::PropertyChanged;
    op.m_uiTypeVersion = 0;
    op.m_sProperty = pProp->GetPropertyName();
    op.m_Value = newValue;
  };

  wdVariant currentValue;
  pAccessor->GetValue(pObject, pProp, currentValue).LogFailure();
  switch (pProp->GetCategory())
  {
    case wdPropertyCategory::Member:
    {
      const auto& objectGuid = currentValue.Get<wdUuid>();
      if (objectGuid.IsValid())
      {
        RemoveObject(objectGuid);
        SetProperty(wdUuid());
      }
    }
    break;
    case wdPropertyCategory::Array:
    case wdPropertyCategory::Set:
    {
      const auto& currentArray = currentValue.Get<wdVariantArray>();
      for (wdInt32 i = (wdInt32)currentArray.GetCount() - 1; i >= 0; i--)
      {
        const auto& objectGuid = currentArray[i].Get<wdUuid>();
        if (objectGuid.IsValid())
        {
          RemoveObject(objectGuid);
        }
      }
      SetProperty(wdVariantArray());
    }
    break;
    case wdPropertyCategory::Map:
    {
      const auto& currentArray = currentValue.Get<wdVariantDictionary>();
      for (auto val : currentArray)
      {
        const auto& objectGuid = val.Value().Get<wdUuid>();
        if (objectGuid.IsValid())
        {
          RemoveObject(objectGuid);
        }
      }
      SetProperty(wdVariantDictionary());
    }
    break;
    default:
      WD_REPORT_FAILURE("Unreachable code");
      break;
  }
  return wdStatus(WD_SUCCESS);
}
