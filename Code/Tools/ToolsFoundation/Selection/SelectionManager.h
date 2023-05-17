#pragma once

#include <ToolsFoundation/Object/DocumentObjectBase.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdDocument;
struct wdDocumentObjectStructureEvent;

struct wdSelectionManagerEvent
{
  enum class Type
  {
    SelectionCleared,
    SelectionSet,
    ObjectAdded,
    ObjectRemoved,
  };

  Type m_Type;
  const wdDocument* m_pDocument;
  const wdDocumentObject* m_pObject;
};

/// \brief Selection Manager stores a set of selected document objects.
class WD_TOOLSFOUNDATION_DLL wdSelectionManager
{
public:
  wdCopyOnBroadcastEvent<const wdSelectionManagerEvent&> m_Events;

  // \brief Storage for the selection so it can be swapped when using multiple sub documents.
  class Storage : public wdRefCounted
  {
  public:
    wdDeque<const wdDocumentObject*> m_SelectionList;
    wdSet<wdUuid> m_SelectionSet;
    const wdDocumentObjectManager* m_pObjectManager = nullptr;
    wdCopyOnBroadcastEvent<const wdSelectionManagerEvent&> m_Events;
  };

public:
  wdSelectionManager(const wdDocumentObjectManager* pObjectManager);
  ~wdSelectionManager();

  void Clear();
  void AddObject(const wdDocumentObject* pObject);
  void RemoveObject(const wdDocumentObject* pObject, bool bRecurseChildren = false);
  void SetSelection(const wdDocumentObject* pSingleObject);
  void SetSelection(const wdDeque<const wdDocumentObject*>& selection);
  void ToggleObject(const wdDocumentObject* pObject);

  /// \brief Returns the last selected object in the selection or null if empty.
  const wdDocumentObject* GetCurrentObject() const;

  /// \brief Returns the selection in the same order the objects were added to the list.
  const wdDeque<const wdDocumentObject*>& GetSelection() const { return m_pSelectionStorage->m_SelectionList; }

  bool IsSelectionEmpty() const { return m_pSelectionStorage->m_SelectionList.IsEmpty(); }

  /// \brief Returns the subset of selected items which have no parent selected. I.e. if an object is selected and one of its ancestors is selected, it is culled from the list. Items are returned in the order of appearance in an expanded scene tree.
  const wdDeque<const wdDocumentObject*> GetTopLevelSelection() const;

  /// \brief Same as GetTopLevelSelection() but additionally requires that all objects are derived from type pBase.
  const wdDeque<const wdDocumentObject*> GetTopLevelSelection(const wdRTTI* pBase) const;

  bool IsSelected(const wdDocumentObject* pObject) const;
  bool IsParentSelected(const wdDocumentObject* pObject) const;

  const wdDocument* GetDocument() const;

  wdSharedPtr<wdSelectionManager::Storage> SwapStorage(wdSharedPtr<wdSelectionManager::Storage> pNewStorage);
  wdSharedPtr<wdSelectionManager::Storage> GetStorage() { return m_pSelectionStorage; }

private:
  void TreeEventHandler(const wdDocumentObjectStructureEvent& e);
  bool RecursiveRemoveFromSelection(const wdDocumentObject* pObject);

  friend class wdDocument;

  wdSharedPtr<wdSelectionManager::Storage> m_pSelectionStorage;

  wdCopyOnBroadcastEvent<const wdDocumentObjectStructureEvent&>::Unsubscriber m_ObjectStructureUnsubscriber;
  wdCopyOnBroadcastEvent<const wdSelectionManagerEvent&>::Unsubscriber m_EventsUnsubscriber;
};
