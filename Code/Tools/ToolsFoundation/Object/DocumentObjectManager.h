#pragma once

#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/SharedPtr.h>
#include <Foundation/Types/Status.h>
#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/Reflection/ReflectedType.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdDocumentObjectManager;
class wdDocument;

// Prevent conflicts with windows.h
#ifdef GetObject
#  undef GetObject
#endif

/// \brief Standard root object for most documents.
/// m_RootObjects stores what is in the document and m_TempObjects stores transient data used during editing which is not part of the document.
class WD_TOOLSFOUNDATION_DLL wdDocumentRoot : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdDocumentRoot, wdReflectedClass);

  wdHybridArray<wdReflectedClass*, 1> m_RootObjects;
  wdHybridArray<wdReflectedClass*, 1> m_TempObjects;
};

/// \brief Implementation detail of wdDocumentObjectManager.
class wdDocumentRootObject : public wdDocumentStorageObject
{
public:
  wdDocumentRootObject(const wdRTTI* pRootType)
    : wdDocumentStorageObject(pRootType)
  {
    m_Guid = wdUuid::StableUuidForString("DocumentRoot");
  }

public:
  virtual void InsertSubObject(wdDocumentObject* pObject, const char* szProperty, const wdVariant& index) override;
  virtual void RemoveSubObject(wdDocumentObject* pObject) override;
};

/// \brief Used by wdDocumentObjectManager::m_StructureEvents.
struct wdDocumentObjectStructureEvent
{
  wdDocumentObjectStructureEvent()
    : m_pObject(nullptr)
    , m_pPreviousParent(nullptr)
    , m_pNewParent(nullptr)
  {
  }

  const wdAbstractProperty* GetProperty() const;
  wdVariant getInsertIndex() const;
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
  const wdDocument* m_pDocument = nullptr;
  const wdDocumentObject* m_pObject = nullptr;
  const wdDocumentObject* m_pPreviousParent = nullptr;
  const wdDocumentObject* m_pNewParent = nullptr;
  wdString m_sParentProperty;
  wdVariant m_OldPropertyIndex;
  wdVariant m_NewPropertyIndex;
};

/// \brief Used by wdDocumentObjectManager::m_PropertyEvents.
struct wdDocumentObjectPropertyEvent
{
  wdDocumentObjectPropertyEvent() { m_pObject = nullptr; }
  wdVariant getInsertIndex() const;

  enum class Type
  {
    PropertySet,
    PropertyInserted,
    PropertyRemoved,
    PropertyMoved,
  };

  Type m_EventType;
  const wdDocumentObject* m_pObject;
  wdVariant m_OldValue;
  wdVariant m_NewValue;
  wdString m_sProperty;
  wdVariant m_OldIndex;
  wdVariant m_NewIndex;
};

/// \brief Used by wdDocumentObjectManager::m_ObjectEvents.
struct wdDocumentObjectEvent
{
  wdDocumentObjectEvent() { m_pObject = nullptr; }

  enum class Type
  {
    BeforeObjectDestroyed,
    AfterObjectCreated,
  };

  Type m_EventType;
  const wdDocumentObject* m_pObject;
};

/// \brief Represents to content of a document. Every document has exactly one root object under which all objects need to be parented. The default root object is wdDocumentRoot.
class WD_TOOLSFOUNDATION_DLL wdDocumentObjectManager
{
public:
  // \brief Storage for the object manager so it can be swapped when using multiple sub documents.
  class Storage : public wdRefCounted
  {
  public:
    Storage(const wdRTTI* pRootType);

    wdDocument* m_pDocument = nullptr;
    wdDocumentRootObject m_RootObject;

    wdHashTable<wdUuid, const wdDocumentObject*> m_GuidToObject;

    mutable wdCopyOnBroadcastEvent<const wdDocumentObjectStructureEvent&> m_StructureEvents;
    mutable wdCopyOnBroadcastEvent<const wdDocumentObjectPropertyEvent&> m_PropertyEvents;
    wdEvent<const wdDocumentObjectEvent&> m_ObjectEvents;
  };

public:
  mutable wdCopyOnBroadcastEvent<const wdDocumentObjectStructureEvent&> m_StructureEvents;
  mutable wdCopyOnBroadcastEvent<const wdDocumentObjectPropertyEvent&> m_PropertyEvents;
  wdEvent<const wdDocumentObjectEvent&> m_ObjectEvents;

  wdDocumentObjectManager(const wdRTTI* pRootType = wdDocumentRoot::GetStaticRTTI());
  virtual ~wdDocumentObjectManager();
  void SetDocument(wdDocument* pDocument) { m_pObjectStorage->m_pDocument = pDocument; }

  // Object Construction / Destruction
  // holds object data
  wdDocumentObject* CreateObject(const wdRTTI* pRtti, wdUuid guid = wdUuid());

  void DestroyObject(wdDocumentObject* pObject);
  virtual void DestroyAllObjects();
  virtual void GetCreateableTypes(wdHybridArray<const wdRTTI*, 32>& ref_types) const {};

  /// \brief Allows to annotate types with a category (group), such that things like creator menus can use this to present the types in a more user
  /// friendly way
  virtual const char* GetTypeCategory(const wdRTTI* pRtti) const { return nullptr; }
  void PatchEmbeddedClassObjects(const wdDocumentObject* pObject) const;

  const wdDocumentObject* GetRootObject() const { return &m_pObjectStorage->m_RootObject; }
  wdDocumentObject* GetRootObject() { return &m_pObjectStorage->m_RootObject; }
  const wdDocumentObject* GetObject(const wdUuid& guid) const;
  wdDocumentObject* GetObject(const wdUuid& guid);
  const wdDocument* GetDocument() const { return m_pObjectStorage->m_pDocument; }
  wdDocument* GetDocument() { return m_pObjectStorage->m_pDocument; }

  // Property Change
  wdStatus SetValue(wdDocumentObject* pObject, const char* szProperty, const wdVariant& newValue, wdVariant index = wdVariant());
  wdStatus InsertValue(wdDocumentObject* pObject, const char* szProperty, const wdVariant& newValue, wdVariant index = wdVariant());
  wdStatus RemoveValue(wdDocumentObject* pObject, const char* szProperty, wdVariant index = wdVariant());
  wdStatus MoveValue(wdDocumentObject* pObject, const char* szProperty, const wdVariant& oldIndex, const wdVariant& newIndex);

  // Structure Change
  void AddObject(wdDocumentObject* pObject, wdDocumentObject* pParent, const char* szParentProperty, wdVariant index);
  void RemoveObject(wdDocumentObject* pObject);
  void MoveObject(wdDocumentObject* pObject, wdDocumentObject* pNewParent, const char* szParentProperty, wdVariant index);

  // Structure Change Test
  wdStatus CanAdd(const wdRTTI* pRtti, const wdDocumentObject* pParent, const char* szParentProperty, const wdVariant& index) const;
  wdStatus CanRemove(const wdDocumentObject* pObject) const;
  wdStatus CanMove(const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const char* szParentProperty, const wdVariant& index) const;
  wdStatus CanSelect(const wdDocumentObject* pObject) const;

  bool IsUnderRootProperty(const char* szRootProperty, const wdDocumentObject* pObject) const;
  bool IsUnderRootProperty(const char* szRootProperty, const wdDocumentObject* pParent, const char* szParentProperty) const;

  wdSharedPtr<wdDocumentObjectManager::Storage> SwapStorage(wdSharedPtr<wdDocumentObjectManager::Storage> pNewStorage);
  wdSharedPtr<wdDocumentObjectManager::Storage> GetStorage() { return m_pObjectStorage; }

private:
  virtual wdDocumentObject* InternalCreateObject(const wdRTTI* pRtti) { return WD_DEFAULT_NEW(wdDocumentStorageObject, pRtti); }
  virtual void InternalDestroyObject(wdDocumentObject* pObject) { WD_DEFAULT_DELETE(pObject); }

  void InternalAddObject(wdDocumentObject* pObject, wdDocumentObject* pParent, const char* szParentProperty, wdVariant index);
  void InternalRemoveObject(wdDocumentObject* pObject);
  void InternalMoveObject(wdDocumentObject* pNewParent, wdDocumentObject* pObject, const char* szParentProperty, wdVariant index);

  virtual wdStatus InternalCanAdd(const wdRTTI* pRtti, const wdDocumentObject* pParent, const char* szParentProperty, const wdVariant& index) const
  {
    return wdStatus(WD_SUCCESS);
  };
  virtual wdStatus InternalCanRemove(const wdDocumentObject* pObject) const { return wdStatus(WD_SUCCESS); };
  virtual wdStatus InternalCanMove(
    const wdDocumentObject* pObject, const wdDocumentObject* pNewParent, const char* szParentProperty, const wdVariant& index) const
  {
    return wdStatus(WD_SUCCESS);
  };
  virtual wdStatus InternalCanSelect(const wdDocumentObject* pObject) const { return wdStatus(WD_SUCCESS); };

  void RecursiveAddGuids(wdDocumentObject* pObject);
  void RecursiveRemoveGuids(wdDocumentObject* pObject);
  void PatchEmbeddedClassObjectsInternal(wdDocumentObject* pObject, const wdRTTI* pType, bool addToDoc);

private:
  friend class wdObjectAccessorBase;

  wdSharedPtr<wdDocumentObjectManager::Storage> m_pObjectStorage;

  wdCopyOnBroadcastEvent<const wdDocumentObjectStructureEvent&>::Unsubscriber m_StructureEventsUnsubscriber;
  wdCopyOnBroadcastEvent<const wdDocumentObjectPropertyEvent&>::Unsubscriber m_PropertyEventsUnsubscriber;
  wdEvent<const wdDocumentObjectEvent&>::Unsubscriber m_ObjectEventsUnsubscriber;
};
