#pragma once

#include <Foundation/Serialization/RttiConverter.h>

/// \brief The wdApplyNativePropertyChangesContext takes care of generating guids for native pointers that match those of the wdAbstractObjectGraph that was passed in. This allows native changes to be tracked and applied to the object graph at a later point.
/// \sa wdAbstractObjectGraph::ModifyNodeViaNativeCounterpart
class WD_FOUNDATION_DLL wdApplyNativePropertyChangesContext : public wdRttiConverterContext
{
public:
  wdApplyNativePropertyChangesContext(wdRttiConverterContext& ref_source, const wdAbstractObjectGraph& originalGraph);

  virtual wdUuid GenerateObjectGuid(const wdUuid& parentGuid, const wdAbstractProperty* pProp, wdVariant index, void* pObject) const override;

private:
  wdRttiConverterContext& m_NativeContext;
  const wdAbstractObjectGraph& m_OriginalGraph;
};
