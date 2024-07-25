#pragma once

#include <ToolsFoundation/Object/DocumentObjectManager.h>

class nsDocumentObject;

class NS_TOOLSFOUNDATION_DLL nsObjectAccessorBase
{
public:
  virtual ~nsObjectAccessorBase();
  const nsDocumentObjectManager* GetObjectManager() const;

  /// \name Transaction Operations
  ///@{

  virtual void StartTransaction(nsStringView sDisplayString);
  virtual void CancelTransaction();
  virtual void FinishTransaction();
  virtual void BeginTemporaryCommands(nsStringView sDisplayString, bool bFireEventsWhenUndoingTempCommands = false);
  virtual void CancelTemporaryCommands();
  virtual void FinishTemporaryCommands();

  ///@}
  /// \name Object Access Interface
  ///@{

  virtual const nsDocumentObject* GetObject(const nsUuid& object) = 0;
  virtual nsStatus GetValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant& out_value, nsVariant index = nsVariant()) = 0;
  virtual nsStatus SetValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index = nsVariant()) = 0;
  virtual nsStatus InsertValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& newValue, nsVariant index = nsVariant()) = 0;
  virtual nsStatus RemoveValue(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index = nsVariant()) = 0;
  virtual nsStatus MoveValue(
    const nsDocumentObject* pObject, const nsAbstractProperty* pProp, const nsVariant& oldIndex, const nsVariant& newIndex) = 0;
  virtual nsStatus GetCount(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsInt32& out_iCount) = 0;

  virtual nsStatus AddObject(const nsDocumentObject* pParent, const nsAbstractProperty* pParentProp, const nsVariant& index, const nsRTTI* pType,
    nsUuid& inout_objectGuid) = 0;
  virtual nsStatus RemoveObject(const nsDocumentObject* pObject) = 0;
  virtual nsStatus MoveObject(
    const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, const nsAbstractProperty* pParentProp, const nsVariant& index) = 0;

  virtual nsStatus GetKeys(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDynamicArray<nsVariant>& out_keys) = 0;
  virtual nsStatus GetValues(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsDynamicArray<nsVariant>& out_values) = 0;

  ///@}
  /// \name Object Access Convenience Functions
  ///@{

  nsStatus GetValue(const nsDocumentObject* pObject, nsStringView sProp, nsVariant& out_value, nsVariant index = nsVariant());
  nsStatus SetValue(const nsDocumentObject* pObject, nsStringView sProp, const nsVariant& newValue, nsVariant index = nsVariant());
  nsStatus InsertValue(const nsDocumentObject* pObject, nsStringView sProp, const nsVariant& newValue, nsVariant index = nsVariant());
  nsStatus RemoveValue(const nsDocumentObject* pObject, nsStringView sProp, nsVariant index = nsVariant());
  nsStatus MoveValue(const nsDocumentObject* pObject, nsStringView sProp, const nsVariant& oldIndex, const nsVariant& newIndex);
  nsStatus GetCount(const nsDocumentObject* pObject, nsStringView sProp, nsInt32& out_iCount);

  nsStatus AddObject(
    const nsDocumentObject* pParent, nsStringView sParentProp, const nsVariant& index, const nsRTTI* pType, nsUuid& inout_objectGuid);
  nsStatus MoveObject(const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, nsStringView sParentProp, const nsVariant& index);

  nsStatus GetKeys(const nsDocumentObject* pObject, nsStringView sProp, nsDynamicArray<nsVariant>& out_keys);
  nsStatus GetValues(const nsDocumentObject* pObject, nsStringView sProp, nsDynamicArray<nsVariant>& out_values);
  const nsDocumentObject* GetChildObject(const nsDocumentObject* pObject, nsStringView sProp, nsVariant index);

  nsStatus Clear(const nsDocumentObject* pObject, nsStringView sProp);

  template <typename T>
  T Get(const nsDocumentObject* pObject, const nsAbstractProperty* pProp, nsVariant index = nsVariant());
  template <typename T>
  T Get(const nsDocumentObject* pObject, nsStringView sProp, nsVariant index = nsVariant());
  nsInt32 GetCount(const nsDocumentObject* pObject, const nsAbstractProperty* pProp);
  nsInt32 GetCount(const nsDocumentObject* pObject, nsStringView sProp);

  ///@}

protected:
  nsObjectAccessorBase(const nsDocumentObjectManager* pManager);
  void FireDocumentObjectStructureEvent(const nsDocumentObjectStructureEvent& e);
  void FireDocumentObjectPropertyEvent(const nsDocumentObjectPropertyEvent& e);

protected:
  const nsDocumentObjectManager* m_pConstManager;
};

#include <ToolsFoundation/Object/Implementation/ObjectAccessorBase_inl.h>
