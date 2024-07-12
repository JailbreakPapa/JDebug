#include <Foundation/FoundationPCH.h>

#include <Foundation/Serialization/ApplyNativePropertyChangesContext.h>


nsApplyNativePropertyChangesContext::nsApplyNativePropertyChangesContext(nsRttiConverterContext& ref_source, const nsAbstractObjectGraph& originalGraph)
  : m_NativeContext(ref_source)
  , m_OriginalGraph(originalGraph)
{
}

nsUuid nsApplyNativePropertyChangesContext::GenerateObjectGuid(const nsUuid& parentGuid, const nsAbstractProperty* pProp, nsVariant index, void* pObject) const
{
  if (pProp->GetFlags().IsSet(nsPropertyFlags::PointerOwner))
  {
    // If the object is already known by the native context (a pointer that existed before the native changes)
    // we can just return it. Any other pointer will get a new guid assigned.
    nsUuid guid = m_NativeContext.GetObjectGUID(pProp->GetSpecificType(), pObject);
    if (guid.IsValid())
      return guid;
  }
  else if (pProp->GetFlags().IsSet(nsPropertyFlags::Class))
  {
    // In case of by-value classes we lookup the guid in the object manager graph by using
    // the index as the identify of the object. If the index is not valid (e.g. the array was expanded by native changes)
    // a new guid is assigned.
    if (const nsAbstractObjectNode* originalNode = m_OriginalGraph.GetNode(parentGuid))
    {
      if (const nsAbstractObjectNode::Property* originalProp = originalNode->FindProperty(pProp->GetPropertyName()))
      {
        switch (pProp->GetCategory())
        {
          case nsPropertyCategory::Member:
          {
            if (originalProp->m_Value.IsA<nsUuid>() && originalProp->m_Value.Get<nsUuid>().IsValid())
              return originalProp->m_Value.Get<nsUuid>();
          }
          break;
          case nsPropertyCategory::Array:
          {
            nsUInt32 uiIndex = index.Get<nsUInt32>();
            if (originalProp->m_Value.IsA<nsVariantArray>())
            {
              const nsVariantArray& values = originalProp->m_Value.Get<nsVariantArray>();
              if (uiIndex < values.GetCount())
              {
                const auto& originalElemValue = values[uiIndex];
                if (originalElemValue.IsA<nsUuid>() && originalElemValue.Get<nsUuid>().IsValid())
                  return originalElemValue.Get<nsUuid>();
              }
            }
          }
          break;
          case nsPropertyCategory::Map:
          {
            const nsString& sIndex = index.Get<nsString>();
            if (originalProp->m_Value.IsA<nsVariantDictionary>())
            {
              const nsVariantDictionary& values = originalProp->m_Value.Get<nsVariantDictionary>();
              if (values.Contains(sIndex))
              {
                const auto& originalElemValue = *values.GetValue(sIndex);
                if (originalElemValue.IsA<nsUuid>() && originalElemValue.Get<nsUuid>().IsValid())
                  return originalElemValue.Get<nsUuid>();
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

  return nsUuid::MakeUuid();
}
