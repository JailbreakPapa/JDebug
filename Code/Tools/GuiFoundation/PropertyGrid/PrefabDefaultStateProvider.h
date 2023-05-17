#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <GuiFoundation/PropertyGrid/DefaultState.h>

/// \brief Default state provider that reflects the default state defined in the prefab template.
class WD_GUIFOUNDATION_DLL wdPrefabDefaultStateProvider : public wdDefaultStateProvider
{
public:
  static wdSharedPtr<wdDefaultStateProvider> CreateProvider(wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp);

  wdPrefabDefaultStateProvider(const wdUuid& rootObjectGuid, const wdUuid& createFromPrefab, const wdUuid& prefabSeedGuid, wdInt32 iRootDepth);
  virtual wdInt32 GetRootDepth() const override;
  virtual wdColorGammaUB GetBackgroundColor() const override;
  virtual wdString GetStateProviderName() const override { return "Prefab"; }

  virtual wdVariant GetDefaultValue(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant()) override;
  virtual wdStatus CreateRevertContainerDiff(SuperArray superPtr, wdObjectAccessorBase* pAccessor, const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDeque<wdAbstractGraphDiffOperation>& out_diff) override;

private:
  const wdUuid m_RootObjectGuid;
  const wdUuid m_CreateFromPrefab;
  const wdUuid m_PrefabSeedGuid;
  wdInt32 m_iRootDepth = 0;
};
