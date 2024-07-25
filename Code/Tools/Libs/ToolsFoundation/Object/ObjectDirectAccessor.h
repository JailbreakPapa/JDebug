#pragma once

#include <ToolsFoundation/Object/ObjectAccessorBase.h>

class nsDocumentObjectManager;

class NS_TOOLSFOUNDATION_DLL nsObjectDirectAccessor : public nsObjectAccessorBase
{
public:
  nsObjectDirectAccessor(nsDocumentObjectManager* pManager);

  virtual const nsDocumentObject* GetObject(const nsUuid& object) override;
  virtual nsStatus GetValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant& out_value, nsVariant index = nsVariant()) override;
  virtual nsStatus SetValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index = nsVariant()) override;
  virtual nsStatus InsertValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index = nsVariant()) override;
  virtual nsStatus RemoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index = nsVariant()) override;
  virtual nsStatus MoveValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& oldIndex, const nsVariant& newIndex) override;
  virtual nsStatus GetCount(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsInt32& out_iCount) override;

  virtual nsStatus AddObject(const nsDocumentObject* pParent, const nsAbstractProperty* pParentProp, const nsVariant& index, const nsRTTI* pType,
    nsUuid& inout_objectGuid) override;
  virtual nsStatus RemoveObject(const nsDocumentObject* pObject) override;
  virtual nsStatus MoveObject(
    const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, const nsAbstractProperty* pParentProp, const nsVariant& index) override;

  virtual nsStatus GetKeys(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDynamicArray<nsVariant>& out_keys) override;
  virtual nsStatus GetValues(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDynamicArray<nsVariant>& out_values) override;

protected:
  nsDocumentObjectManager* m_pManager;
};
