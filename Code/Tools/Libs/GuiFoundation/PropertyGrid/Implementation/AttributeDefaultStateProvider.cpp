#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/AttributeDefaultStateProvider.h>
#include <ToolsFoundation/Object/ObjectAccessorBase.h>

static nsSharedPtr<nsDefaultStateProvider> g_pAttributeDefaultStateProvider = NS_DEFAULT_NEW(nsAttributeDefaultStateProvider);
nsSharedPtr<nsDefaultStateProvider> nsAttributeDefaultStateProvider::CreateProvider(nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp)
{
  // One global instance handles all. No need to create a new instance per request as no state need to be tracked.
  return g_pAttributeDefaultStateProvider;
}

nsInt32 nsAttributeDefaultStateProvider::GetRootDepth() const
{
  return -1;
}

nsColorGammaUB nsAttributeDefaultStateProvider::GetBackgroundColor() const
{
  // Set alpha to 0 -> color will be ignored.
  return nsColorGammaUB(0, 0, 0, 0);
}

nsVariant nsAttributeDefaultStateProvider::GetDefaultValue(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index)
{
  if (!pProp->GetFlags().IsSet(nsPropertyFlags::Pointer) && pProp->GetFlags().IsSet(nsPropertyFlags::Class) && pProp->GetCategory() == nsPropertyCategory::Member && !nsReflectionUtils::IsValueType(pProp))
  {
    // An embedded class that is not a value type can never change its value.
    nsVariant value;
    pAccessor->GetValue(pObject, pProp, value).LogFailure();
    return value;
  }
  return nsReflectionUtils::GetDefaultValue(pProp, index);
}

nsStatus nsAttributeDefaultStateProvider::CreateRevertContainerDiff(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDeque<nsAbstractGraphDiffOperation>& out_diff)
{
  auto RemoveObject = [&](const nsUuid& object)
  {
    auto& op = out_diff.ExpandAndGetRef();
    op.m_Node = object;
    op.m_Operation = nsAbstractGraphDiffOperation::Op::NodeRemoved;
    op.m_uiTypeVersion = 0;
    op.m_sProperty = pAccessor->GetObject(object)->GetType()->GetTypeName();
  };

  auto SetProperty = [&](const nsVariant& newValue)
  {
    auto& op = out_diff.ExpandAndGetRef();
    op.m_Node = pObject->GetGuid();
    op.m_Operation = nsAbstractGraphDiffOperation::Op::PropertyChanged;
    op.m_uiTypeVersion = 0;
    op.m_sProperty = pProp->GetPropertyName();
    op.m_Value = newValue;
  };

  nsVariant currentValue;
  pAccessor->GetValue(pObject, pProp, currentValue).LogFailure();
  switch (pProp->GetCategory())
  {
    case nsPropertyCategory::Member:
    {
      const auto& objectGuid = currentValue.Get<nsUuid>();
      if (objectGuid.IsValid())
      {
        RemoveObject(objectGuid);
        SetProperty(nsUuid());
      }
    }
    break;
    case nsPropertyCategory::Array:
    case nsPropertyCategory::Set:
    {
      const auto& currentArray = currentValue.Get<nsVariantArray>();
      for (nsInt32 i = (nsInt32)currentArray.GetCount() - 1; i >= 0; i--)
      {
        const auto& objectGuid = currentArray[i].Get<nsUuid>();
        if (objectGuid.IsValid())
        {
          RemoveObject(objectGuid);
        }
      }
      SetProperty(nsVariantArray());
    }
    break;
    case nsPropertyCategory::Map:
    {
      const auto& currentArray = currentValue.Get<nsVariantDictionary>();
      for (auto val : currentArray)
      {
        const auto& objectGuid = val.Value().Get<nsUuid>();
        if (objectGuid.IsValid())
        {
          RemoveObject(objectGuid);
        }
      }
      SetProperty(nsVariantDictionary());
    }
    break;
    default:
      NS_REPORT_FAILURE("Unreachable code");
      break;
  }
  return nsStatus(NS_SUCCESS);
}
