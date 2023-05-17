#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <GuiFoundation/PropertyGrid/DefaultState.h>

/// \brief This is the fall back default state provider which handles the default state set via the wdDefaultAttribute on the reflected type.
class WD_GUIFOUNDATION_DLL wdAttributeDefaultStateProvider : public wdDefaultStateProvider
{
public:
  static wdSharedPtr<wdDefaultStateProvider> CreateProvider(wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp);

  virtual wdInt32 GetRootDepth() const override;
  virtual wdColorGammaUB GetBackgroundColor() const override;
  virtual wdString GetStateProviderName() const override { return "Attribute"; }

  virtual wdVariant GetDefaultValue(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant()) override;
  virtual wdStatus CreateRevertContainerDiff(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDeque<wdAbstractGraphDiffOperation>& out_diff) override;
};
