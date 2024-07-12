/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once
#include <ToolsFoundation/Object/ObjectCommandAccessor.h>

class NS_TOOLSFOUNDATION_DLL nsNodeCommandAccessor : public nsObjectCommandAccessor
{
public:
  nsNodeCommandAccessor(nsCommandHistory* pHistory);
  ~nsNodeCommandAccessor();

  virtual nsStatus SetValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index = nsVariant()) override;

  virtual nsStatus InsertValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index = nsVariant()) override;
  virtual nsStatus RemoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index = nsVariant()) override;
  virtual nsStatus MoveValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& oldIndex, const nsVariant& newIndex) override;

private:
  bool IsDynamicPinProperty(const nsDocumentObject* pObject, const nsAbstractProperty* pProp) const;

  struct ConnectionInfo
  {
    const nsDocumentObject* m_pSource = nullptr;
    const nsDocumentObject* m_pTarget = nullptr;
    nsString m_sSourcePin;
    nsString m_sTargetPin;
  };

  nsStatus DisconnectAllPins(const nsDocumentObject* pObject, nsDynamicArray<ConnectionInfo>& out_oldConnections);
  nsStatus TryReconnectAllPins(const nsDocumentObject* pObject, const nsDynamicArray<ConnectionInfo>& oldConnections);
};
