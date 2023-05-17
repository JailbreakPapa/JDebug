#pragma once

#include <ToolsFoundation/Object/ObjectAccessorBase.h>

class wdDocumentObject;

class WD_TOOLSFOUNDATION_DLL wdObjectProxyAccessor : public wdObjectAccessorBase
{
public:
  wdObjectProxyAccessor(wdObjectAccessorBase* pSource);
  virtual ~wdObjectProxyAccessor();

  /// \name Transaction Operations
  ///@{

  virtual void StartTransaction(const char* szDisplayString) override;
  virtual void CancelTransaction() override;
  virtual void FinishTransaction() override;
  virtual void BeginTemporaryCommands(const char* szDisplayString, bool bFireEventsWhenUndoingTempCommands = false) override;
  virtual void CancelTemporaryCommands() override;
  virtual void FinishTemporaryCommands() override;

  ///@}
  /// \name wdObjectAccessorBase overrides
  ///@{

  virtual const wdDocumentObject* GetObject(const wdUuid& object) override;
  virtual wdStatus GetValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant& out_value, wdVariant index = wdVariant()) override;
  virtual wdStatus SetValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index = wdVariant()) override;
  virtual wdStatus InsertValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index = wdVariant()) override;
  virtual wdStatus RemoveValue(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant()) override;
  virtual wdStatus MoveValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& oldIndex, const wdVariant& newIndex) override;
  virtual wdStatus GetCount(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdInt32& out_iCount) override;

  virtual wdStatus AddObject(const wdDocumentObject* pParent, const wdAbstractProperty* pParentProp, const wdVariant& index, const wdRTTI* pType,
    wdUuid& inout_objectGuid) override;
  virtual wdStatus RemoveObject(const wdDocumentObject* pObject) override;
  virtual wdStatus MoveObject(
    const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const wdAbstractProperty* pParentProp, const wdVariant& index) override;

  virtual wdStatus GetKeys(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDynamicArray<wdVariant>& out_keys) override;
  virtual wdStatus GetValues(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDynamicArray<wdVariant>& out_values) override;

  ///@}

protected:
  wdObjectAccessorBase* m_pSource = nullptr;
};
