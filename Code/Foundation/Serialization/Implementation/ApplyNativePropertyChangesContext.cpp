#include <Foundation/FoundationPCH.h>

#include <Foundation/Serialization/ApplyNativePropertyChangesContext.h>


wdApplyNativePropertyChangesContext::wdApplyNativePropertyChangesContext(wdRttiConverterContext& ref_source, const wdAbstractObjectGraph& originalGraph)
  : m_NativeContext(ref_source)
  , m_OriginalGraph(originalGraph)
{
}

wdUuid wdApplyNativePropertyChangesContext::GenerateObjectGuid(const wdUuid& parentGuid, const wdAbstractProperty* pProp, wdVariant index, void* pObject) const
{
  wdUuid guid;
  if (pProp->GetFlags().IsSet(wdPropertyFlags::PointerOwner))
  {
    // If the object is already known by the native context (a pointer that existed before the native changes)
    // we can just return it. Any other pointer will get a new guid assigned.
    guid = m_NativeContext.GetObjectGUID(pProp->GetSpecificType(), pObject);
    if (guid.IsValid())
      return guid;
  }
  else if (pProp->GetFlags().IsSet(wdPropertyFlags::Class))
  {
    // In case of by-value classes we lookup the guid in the object manager graph by using
    // the index as the identify of the object. If the index is not valid (e.g. the array was expanded by native changes)
    // a new guid is assigned.
    if (const wdAbstractObjectNode* originalNode = m_OriginalGraph.GetNode(parentGuid))
    {
      if (const wdAbstractObjectNode::Property* originalProp = originalNode->FindProperty(pProp->GetPropertyName()))
      {
        switch (pProp->GetCategory())
        {
          case wdPropertyCategory::Member:
          {
            if (originalProp->m_Value.IsA<wdUuid>() && originalProp->m_Value.Get<wdUuid>().IsValid())
              return originalProp->m_Value.Get<wdUuid>();
          }
          break;
          case wdPropertyCategory::Array:
          {
            wdUInt32 uiIndex = index.Get<wdUInt32>();
            if (originalProp->m_Value.IsA<wdVariantArray>())
            {
              const wdVariantArray& values = originalProp->m_Value.Get<wdVariantArray>();
              if (uiIndex < values.GetCount())
              {
                const auto& originalElemValue = values[uiIndex];
                if (originalElemValue.IsA<wdUuid>() && originalElemValue.Get<wdUuid>().IsValid())
                  return originalElemValue.Get<wdUuid>();
              }
            }
          }
          break;
          case wdPropertyCategory::Map:
          {
            const wdString& sIndex = index.Get<wdString>();
            if (originalProp->m_Value.IsA<wdVariantDictionary>())
            {
              const wdVariantDictionary& values = originalProp->m_Value.Get<wdVariantDictionary>();
              if (values.Contains(sIndex))
              {
                const auto& originalElemValue = *values.GetValue(sIndex);
                if (originalElemValue.IsA<wdUuid>() && originalElemValue.Get<wdUuid>().IsValid())
                  return originalElemValue.Get<wdUuid>();
              }
            }
          }
          break;

          default:
            break;
        }
      }
    }
  }
  guid.CreateNewUuid();
  return guid;
}


WD_STATICLINK_FILE(Foundation, Foundation_Serialization_Implementation_ApplyNativePropertyChangesContext);
