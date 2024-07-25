#pragma once

#include <ToolsFoundation/Object/ObjectDirectAccessor.h>

class nsDocumentObject;
class nsCommandHistory;

class NS_TOOLSFOUNDATION_DLL nsObjectCommandAccessor : public nsObjectDirectAccessor
{
public:
  nsObjectCommandAccessor(nsCommandHistory* pHistory);

  virtual void StartTransaction(nsStringView sDisplayString) override;
  virtual void CancelTransaction() override;
  virtual void FinishTransaction() override;
  virtual void BeginTemporaryCommands(nsStringView sDisplayString, bool bFireEventsWhenUndoingTempCommands = false) override;
  virtual void CancelTemporaryCommands() override;
  virtual void FinishTemporaryCommands() override;

  virtual nsStatus SetValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index = nsVariant()) override;
  virtual nsStatus InsertValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index = nsVariant()) override;
  virtual nsStatus RemoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index = nsVariant()) override;
  virtual nsStatus MoveValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& oldIndex, const nsVariant& newIndex) override;

  virtual nsStatus AddObject(const nsDocumentObject* pParent, const nsAbstractProperty* pParentProp, const nsVariant& index, const nsRTTI* pType,
    nsUuid& inout_objectGuid) override;
  virtual nsStatus RemoveObject(const nsDocumentObject* pObject) override;
  virtual nsStatus MoveObject(
    const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, const nsAbstractProperty* pParentProp, const nsVariant& index) override;

protected:
  nsCommandHistory* m_pHistory;
};
