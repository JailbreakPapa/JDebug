#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <GuiFoundation/PropertyGrid/DefaultState.h>

/// \brief Default state provider that reflects the default state defined in the prefab template.
class NS_GUIFOUNDATION_DLL nsPrefabDefaultStateProvider : public nsDefaultStateProvider
{
public:
  static nsSharedPtr<nsDefaultStateProvider> CreateProvider(nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp);

  nsPrefabDefaultStateProvider(const nsUuid& rootObjectGuid, const nsUuid& createFromPrefab, const nsUuid& prefabSeedGuid, nsInt32 iRootDepth);
  virtual nsInt32 GetRootDepth() const override;
  virtual nsColorGammaUB GetBackgroundColor() const override;
  virtual nsString GetStateProviderName() const override { return "Prefab"; }

  virtual nsVariant GetDefaultValue(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index = nsVariant()) override;
  virtual nsStatus CreateRevertContainerDiff(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDeque<nsAbstractGraphDiffOperation>& out_diff) override;

private:
  const nsUuid m_RootObjectGuid;
  const nsUuid m_CreateFromPrefab;
  const nsUuid m_PrefabSeedGuid;
  nsInt32 m_iRootDepth = 0;
};
