#pragma once

#include <Foundation/Serialization/RttiConverter.h>

/// \brief The nsApplyNativePropertyChangesContext takes care of generating guids for native pointers that match those of the nsAbstractObjectGraph that was passed in. This allows native changes to be tracked and applied to the object graph at a later point.
/// \sa nsAbstractObjectGraph::ModifyNodeViaNativeCounterpart
class NS_FOUNDATION_DLL nsApplyNativePropertyChangesContext : public nsRttiConverterContext
{
public:
  nsApplyNativePropertyChangesContext(nsRttiConverterContext& ref_source, const nsAbstractObjectGraph& originalGraph);

  virtual nsUuid GenerateObjectGuid(const nsUuid& parentGuid, const nsAbstractProperty* pProp, nsVariant index, void* pObject) const override;

private:
  nsRttiConverterContext& m_NativeContext;
  const nsAbstractObjectGraph& m_OriginalGraph;
};
