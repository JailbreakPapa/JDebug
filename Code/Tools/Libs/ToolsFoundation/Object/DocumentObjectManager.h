#pragma once

#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/Status.h>
#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsDocumentObjectManager;
class nsDocument;

// Prevent conflicts with windows.h
#ifdef GetObject
#  undef GetObject
#endif

/// \brief Standard root object for most documents.
/// m_RootObjects stores what is in the document and m_TempObjects stores transient data used during editing which is not part of the document.
class NS_TOOLSFOUNDATION_DLL nsDocumentRoot : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsDocumentRoot, nsReflectedClass);

  nsHybridArray<nsReflectedClass*, 1> m_RootObjects;
  nsHybridArray<nsReflectedClass*, 1> m_TempObjects;
};

/// \brief Implementation detail of nsDocumentObjectManager.
class nsDocumentRootObject : public nsDocumentStorageObject
{
public:
  nsDocumentRootObject(const nsRTTI* pRootType)
    : nsDocumentStorageObject(pRootType)
  {
    m_Guid = nsUuid::MakeStableUuidFromString("DocumentRoot");
  }

public:
  virtual void InsertSubObject(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& index) override;
  virtual void RemoveSubObject(nsDocumentObject* pObject) override;
};

/// \brief Used by nsDocumentObjectManager::m_StructureEvents.
struct nsDocumentObjectStructureEvent
{
  nsDocumentObjectStructureEvent()

    = default;

  const nsAbstractProperty* GetProperty() const;
  nsVariant getInsertIndex() const;
  enum class Type
  {
    BeforeReset,
    AfterReset,
    BeforeObjectAdded,
    AfterObjectAdded,
    BeforeObjectRemoved,
    AfterObjectRemoved,
    BeforeObjectMoved,
    AfterObjectMoved,
    AfterObjectMoved2,
  };

  Type m_EventType;
  const nsDocument* m_pDocument = nullptr;
  const nsDocumentObject* m_pObject = nullptr;
  const nsDocumentObject* m_pPreviousParent = nullptr;
  const nsDocumentObject* m_pNewParent = nullptr;
  nsString m_sParentProperty;
  nsVariant m_OldPropertyIndex;
  nsVariant m_NewPropertyIndex;
};

/// \brief Used by nsDocumentObjectManager::m_PropertyEvents.
struct nsDocumentObjectPropertyEvent
{
  nsDocumentObjectPropertyEvent() { m_pObject = nullptr; }
  nsVariant getInsertIndex() const;

  enum class Type
  {
    PropertySet,
    PropertyInserted,
    PropertyRemoved,
    PropertyMoved,
  };

  Type m_EventType;
  const nsDocumentObject* m_pObject;
  nsVariant m_OldValue;
  nsVariant m_NewValue;
  nsString m_sProperty;
  nsVariant m_OldIndex;
  nsVariant m_NewIndex;
};

/// \brief Used by nsDocumentObjectManager::m_ObjectEvents.
struct nsDocumentObjectEvent
{
  nsDocumentObjectEvent() { m_pObject = nullptr; }

  enum class Type
  {
    BeforeObjectDestroyed,
    AfterObjectCreated,
    Invalid
  };

  Type m_EventType = Type::Invalid;
  const nsDocumentObject* m_pObject;
};

/// \brief Represents to content of a document. Every document has exactly one root object under which all objects need to be parented. The default root object is nsDocumentRoot.
class NS_TOOLSFOUNDATION_DLL nsDocumentObjectManager
{
public:
  // \brief Storage for the object manager so it can be swapped when using multiple sub documents.
  class Storage : public nsRefCounted
  {
  public:
    Storage(const nsRTTI* pRootType);

    nsDocument* m_pDocument = nullptr;
    nsDocumentRootObject m_RootObject;

    nsHashTable<nsUuid, const nsDocumentObject*> m_GuidToObject;

    mutable nsCopyOnBroadcastEvent<const nsDocumentObjectStructureEvent&> m_StructureEvents;
    mutable nsCopyOnBroadcastEvent<const nsDocumentObjectPropertyEvent&> m_PropertyEvents;
    nsEvent<const nsDocumentObjectEvent&> m_ObjectEvents;
  };

public:
  mutable nsCopyOnBroadcastEvent<const nsDocumentObjectStructureEvent&> m_StructureEvents;
  mutable nsCopyOnBroadcastEvent<const nsDocumentObjectPropertyEvent&> m_PropertyEvents;
  nsEvent<const nsDocumentObjectEvent&> m_ObjectEvents;

  nsDocumentObjectManager(const nsRTTI* pRootType = nsDocumentRoot::GetStaticRTTI());
  virtual ~nsDocumentObjectManager();
  void SetDocument(nsDocument* pDocument) { m_pObjectStorage->m_pDocument = pDocument; }

  // Object Construction / Destruction
  // holds object data
  nsDocumentObject* CreateObject(const nsRTTI* pRtti, nsUuid guid = nsUuid());

  void DestroyObject(nsDocumentObject* pObject);
  virtual void DestroyAllObjects();
  virtual void GetCreateableTypes(nsHybridArray<const nsRTTI*, 32>& ref_types) const {};

  void PatchEmbeddedClassObjects(const nsDocumentObject* pObject) const;

  const nsDocumentObject* GetRootObject() const { return &m_pObjectStorage->m_RootObject; }
  nsDocumentObject* GetRootObject() { return &m_pObjectStorage->m_RootObject; }
  const nsDocumentObject* GetObject(const nsUuid& guid) const;
  nsDocumentObject* GetObject(const nsUuid& guid);
  const nsDocument* GetDocument() const { return m_pObjectStorage->m_pDocument; }
  nsDocument* GetDocument() { return m_pObjectStorage->m_pDocument; }

  // Property Change
  nsStatus SetValue(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& newValue, nsVariant index = nsVariant());
  nsStatus InsertValue(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& newValue, nsVariant index = nsVariant());
  nsStatus RemoveValue(nsDocumentObject* pObject, nsStringView sProperty, nsVariant index = nsVariant());
  nsStatus MoveValue(nsDocumentObject* pObject, nsStringView sProperty, const nsVariant& oldIndex, const nsVariant& newIndex);

  // Structure Change
  void AddObject(nsDocumentObject* pObject, nsDocumentObject* pParent, nsStringView sParentProperty, nsVariant index);
  void RemoveObject(nsDocumentObject* pObject);
  void MoveObject(nsDocumentObject* pObject, nsDocumentObject* pNewParent, nsStringView sParentProperty, nsVariant index);

  // Structure Change Test
  nsStatus CanAdd(const nsRTTI* pRtti, const nsDocumentObject* pParent, nsStringView sParentProperty, const nsVariant& index) const;
  nsStatus CanRemove(const nsDocumentObject* pObject) const;
  nsStatus CanMove(const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, nsStringView sParentProperty, const nsVariant& index) const;
  nsStatus CanSelect(const nsDocumentObject* pObject) const;

  bool IsUnderRootProperty(nsStringView sRootProperty, const nsDocumentObject* pObject) const;
  bool IsUnderRootProperty(nsStringView sRootProperty, const nsDocumentObject* pParent, nsStringView sParentProperty) const;
  bool IsTemporary(const nsDocumentObject* pObject) const;
  bool IsTemporary(const nsDocumentObject* pParent, nsStringView sParentProperty) const;

  nsSharedPtr<nsDocumentObjectManager::Storage> SwapStorage(nsSharedPtr<nsDocumentObjectManager::Storage> pNewStorage);
  nsSharedPtr<nsDocumentObjectManager::Storage> GetStorage() { return m_pObjectStorage; }

private:
  virtual nsDocumentObject* InternalCreateObject(const nsRTTI* pRtti) { return NS_DEFAULT_NEW(nsDocumentStorageObject, pRtti); }
  virtual void InternalDestroyObject(nsDocumentObject* pObject) { NS_DEFAULT_DELETE(pObject); }

  void InternalAddObject(nsDocumentObject* pObject, nsDocumentObject* pParent, nsStringView sParentProperty, nsVariant index);
  void InternalRemoveObject(nsDocumentObject* pObject);
  void InternalMoveObject(nsDocumentObject* pNewParent, nsDocumentObject* pObject, nsStringView sParentProperty, nsVariant index);

  virtual nsStatus InternalCanAdd(const nsRTTI* pRtti, const nsDocumentObject* pParent, nsStringView sParentProperty, const nsVariant& index) const
  {
    return nsStatus(NS_SUCCESS);
  };
  virtual nsStatus InternalCanRemove(const nsDocumentObject* pObject) const { return nsStatus(NS_SUCCESS); };
  virtual nsStatus InternalCanMove(
    const nsDocumentObject* pObject, const nsDocumentObject* pNewParent, nsStringView sParentProperty, const nsVariant& index) const
  {
    return nsStatus(NS_SUCCESS);
  };
  virtual nsStatus InternalCanSelect(const nsDocumentObject* pObject) const { return nsStatus(NS_SUCCESS); };

  void RecursiveAddGuids(nsDocumentObject* pObject);
  void RecursiveRemoveGuids(nsDocumentObject* pObject);
  void PatchEmbeddedClassObjectsInternal(nsDocumentObject* pObject, const nsRTTI* pType, bool addToDoc);

private:
  friend class nsObjectAccessorBase;

  nsSharedPtr<nsDocumentObjectManager::Storage> m_pObjectStorage;

  nsCopyOnBroadcastEvent<const nsDocumentObjectStructureEvent&>::Unsubscriber m_StructureEventsUnsubscriber;
  nsCopyOnBroadcastEvent<const nsDocumentObjectPropertyEvent&>::Unsubscriber m_PropertyEventsUnsubscriber;
  nsEvent<const nsDocumentObjectEvent&>::Unsubscriber m_ObjectEventsUnsubscriber;
};
