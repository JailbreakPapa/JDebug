#pragma once

#include <ToolsFoundation/Object/ObjectDirectAccessor.h>

class wdDocumentObject;
class wdCommandHistory;

class WD_TOOLSFOUNDATION_DLL wdObjectCommandAccessor : public wdObjectDirectAccessor
{
public:
  wdObjectCommandAccessor(wdCommandHistory* pHistory);

  virtual void StartTransaction(const char* szDisplayString) override;
  virtual void CancelTransaction() override;
  virtual void FinishTransaction() override;
  virtual void BeginTemporaryCommands(const char* szDisplayString, bool bFireEventsWhenUndoingTempCommands = false) override;
  virtual void CancelTemporaryCommands() override;
  virtual void FinishTemporaryCommands() override;

  virtual wdStatus SetValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index = wdVariant()) override;
  virtual wdStatus InsertValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index = wdVariant()) override;
  virtual wdStatus RemoveValue(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant()) override;
  virtual wdStatus MoveValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& oldIndex, const wdVariant& newIndex) override;

  virtual wdStatus AddObject(const wdDocumentObject* pParent, const wdAbstractProperty* pParentProp, const wdVariant& index, const wdRTTI* pType,
    wdUuid& inout_objectGuid) override;
  virtual wdStatus RemoveObject(const wdDocumentObject* pObject) override;
  virtual wdStatus MoveObject(
    const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const wdAbstractProperty* pParentProp, const wdVariant& index) override;

private:
  wdCommandHistory* m_pHistory;
};
