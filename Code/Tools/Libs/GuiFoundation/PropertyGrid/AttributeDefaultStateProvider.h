/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <GuiFoundation/PropertyGrid/DefaultState.h>

/// \brief This is the fall back default state provider which handles the default state set via the nsDefaultAttribute on the reflected type.
class NS_GUIFOUNDATION_DLL nsAttributeDefaultStateProvider : public nsDefaultStateProvider
{
public:
  static nsSharedPtr<nsDefaultStateProvider> CreateProvider(nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp);

  virtual nsInt32 GetRootDepth() const override;
  virtual nsColorGammaUB GetBackgroundColor() const override;
  virtual nsString GetStateProviderName() const override { return "Attribute"; }

  virtual nsVariant GetDefaultValue(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index = nsVariant()) override;
  virtual nsStatus CreateRevertContainerDiff(SuperArray superPtr, nsObjectAccessorBase* pAccessor, const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDeque<nsAbstractGraphDiffOperation>& out_diff) override;
};
