#pragma once

#include <ToolsFoundation/Object/DocumentObjectManager.h>

class wdDocumentObject;

class WD_TOOLSFOUNDATION_DLL wdObjectAccessorBase
{
public:
  virtual ~wdObjectAccessorBase();
  const wdDocumentObjectManager* GetObjectManager() const;

  /// \name Transaction Operations
  ///@{

  virtual void StartTransaction(const char* szDisplayString);
  virtual void CancelTransaction();
  virtual void FinishTransaction();
  virtual void BeginTemporaryCommands(const char* szDisplayString, bool bFireEventsWhenUndoingTempCommands = false);
  virtual void CancelTemporaryCommands();
  virtual void FinishTemporaryCommands();

  ///@}
  /// \name Object Access Interface
  ///@{

  virtual const wdDocumentObject* GetObject(const wdUuid& object) = 0;
  virtual wdStatus GetValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant& out_value, wdVariant index = wdVariant()) = 0;
  virtual wdStatus SetValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index = wdVariant()) = 0;
  virtual wdStatus InsertValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& newValue, wdVariant index = wdVariant()) = 0;
  virtual wdStatus RemoveValue(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant()) = 0;
  virtual wdStatus MoveValue(
    const wdDocumentObject* pObject, const wdAbstractProperty* pProp, const wdVariant& oldIndex, const wdVariant& newIndex) = 0;
  virtual wdStatus GetCount(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdInt32& out_iCount) = 0;

  virtual wdStatus AddObject(const wdDocumentObject* pParent, const wdAbstractProperty* pParentProp, const wdVariant& index, const wdRTTI* pType,
    wdUuid& inout_objectGuid) = 0;
  virtual wdStatus RemoveObject(const wdDocumentObject* pObject) = 0;
  virtual wdStatus MoveObject(
    const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const wdAbstractProperty* pParentProp, const wdVariant& index) = 0;

  virtual wdStatus GetKeys(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDynamicArray<wdVariant>& out_keys) = 0;
  virtual wdStatus GetValues(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdDynamicArray<wdVariant>& out_values) = 0;

  ///@}
  /// \name Object Access Convenience Functions
  ///@{

  wdStatus GetValue(const wdDocumentObject* pObject, const char* szProp, wdVariant& out_value, wdVariant index = wdVariant());
  wdStatus SetValue(const wdDocumentObject* pObject, const char* szProp, const wdVariant& newValue, wdVariant index = wdVariant());
  wdStatus InsertValue(const wdDocumentObject* pObject, const char* szProp, const wdVariant& newValue, wdVariant index = wdVariant());
  wdStatus RemoveValue(const wdDocumentObject* pObject, const char* szProp, wdVariant index = wdVariant());
  wdStatus MoveValue(const wdDocumentObject* pObject, const char* szProp, const wdVariant& oldIndex, const wdVariant& newIndex);
  wdStatus GetCount(const wdDocumentObject* pObject, const char* szProp, wdInt32& out_iCount);

  wdStatus AddObject(
    const wdDocumentObject* pParent, const char* szParentProp, const wdVariant& index, const wdRTTI* pType, wdUuid& inout_objectGuid);
  wdStatus MoveObject(const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const char* szParentProp, const wdVariant& index);

  wdStatus GetKeys(const wdDocumentObject* pObject, const char* szProp, wdDynamicArray<wdVariant>& out_keys);
  wdStatus GetValues(const wdDocumentObject* pObject, const char* szProp, wdDynamicArray<wdVariant>& out_values);
  const wdDocumentObject* GetChildObject(const wdDocumentObject* pObject, const char* szProp, wdVariant index);

  wdStatus Clear(const wdDocumentObject* pObject, const char* szProp);

  template <typename T>
  T Get(const wdDocumentObject* pObject, const wdAbstractProperty* pProp, wdVariant index = wdVariant());
  template <typename T>
  T Get(const wdDocumentObject* pObject, const char* szProp, wdVariant index = wdVariant());
  wdInt32 GetCount(const wdDocumentObject* pObject, const wdAbstractProperty* pProp);
  wdInt32 GetCount(const wdDocumentObject* pObject, const char* szProp);

  ///@}

protected:
  wdObjectAccessorBase(const wdDocumentObjectManager* pManager);
  void FireDocumentObjectStructureEvent(const wdDocumentObjectStructureEvent& e);
  void FireDocumentObjectPropertyEvent(const wdDocumentObjectPropertyEvent& e);

protected:
  const wdDocumentObjectManager* m_pConstManager;
};

#include <ToolsFoundation/Object/Implementation/ObjectAccessorBase_inl.h>
