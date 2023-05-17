#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Selection/SelectionManager.h>

wdSelectionManager::wdSelectionManager(const wdDocumentObjectManager* pObjectManager)
{
  auto pStorage = WD_DEFAULT_NEW(Storage);
  pStorage->m_pObjectManager = pObjectManager;
  SwapStorage(pStorage);
}

wdSelectionManager::~wdSelectionManager()
{
  m_ObjectStructureUnsubscriber.Unsubscribe();
  m_EventsUnsubscriber.Unsubscribe();
}

void wdSelectionManager::TreeEventHandler(const wdDocumentObjectStructureEvent& e)
{
  switch (e.m_EventType)
  {
    case wdDocumentObjectStructureEvent::Type::BeforeObjectRemoved:
      RemoveObject(e.m_pObject, true);
      break;
    default:
      return;
  }
}

bool wdSelectionManager::RecursiveRemoveFromSelection(const wdDocumentObject* pObject)
{
  auto it = m_pSelectionStorage->m_SelectionSet.Find(pObject->GetGuid());

  bool bRemoved = false;
  if (it.IsValid())
  {
    m_pSelectionStorage->m_SelectionSet.Remove(it);
    m_pSelectionStorage->m_SelectionList.RemoveAndCopy(pObject);
    bRemoved = true;
  }

  for (const wdDocumentObject* pChild : pObject->GetChildren())
  {
    bRemoved = bRemoved || RecursiveRemoveFromSelection(pChild);
  }
  return bRemoved;
}

void wdSelectionManager::Clear()
{
  if (!m_pSelectionStorage->m_SelectionList.IsEmpty() || !m_pSelectionStorage->m_SelectionSet.IsEmpty())
  {
    m_pSelectionStorage->m_SelectionList.Clear();
    m_pSelectionStorage->m_SelectionSet.Clear();

    wdSelectionManagerEvent e;
    e.m_pDocument = GetDocument();
    e.m_pObject = nullptr;
    e.m_Type = wdSelectionManagerEvent::Type::SelectionCleared;

    m_pSelectionStorage->m_Events.Broadcast(e);
  }
}

void wdSelectionManager::AddObject(const wdDocumentObject* pObject)
{
  WD_ASSERT_DEBUG(pObject, "Object must be valid");

  if (IsSelected(pObject))
    return;

  WD_ASSERT_DEV(pObject->GetDocumentObjectManager() == m_pSelectionStorage->m_pObjectManager, "Passed in object does not belong to same object manager.");
  wdStatus res = m_pSelectionStorage->m_pObjectManager->CanSelect(pObject);
  if (res.m_Result.Failed())
  {
    wdLog::Error("{0}", res.m_sMessage);
    return;
  }

  m_pSelectionStorage->m_SelectionList.PushBack(pObject);
  m_pSelectionStorage->m_SelectionSet.Insert(pObject->GetGuid());

  wdSelectionManagerEvent e;
  e.m_pDocument = GetDocument();
  e.m_pObject = pObject;
  e.m_Type = wdSelectionManagerEvent::Type::ObjectAdded;

  m_pSelectionStorage->m_Events.Broadcast(e);
}

void wdSelectionManager::RemoveObject(const wdDocumentObject* pObject, bool bRecurseChildren)
{
  if (bRecurseChildren)
  {
    // We only want one message for the change in selection so we first everything and then fire
    // SelectionSet instead of multiple ObjectRemoved messages.
    if (RecursiveRemoveFromSelection(pObject))
    {
      wdSelectionManagerEvent e;
      e.m_pDocument = GetDocument();
      e.m_pObject = nullptr;
      e.m_Type = wdSelectionManagerEvent::Type::SelectionSet;
      m_pSelectionStorage->m_Events.Broadcast(e);
    }
  }
  else
  {
    auto it = m_pSelectionStorage->m_SelectionSet.Find(pObject->GetGuid());

    if (!it.IsValid())
      return;

    m_pSelectionStorage->m_SelectionSet.Remove(it);
    m_pSelectionStorage->m_SelectionList.RemoveAndCopy(pObject);

    wdSelectionManagerEvent e;
    e.m_pDocument = GetDocument();
    e.m_pObject = pObject;
    e.m_Type = wdSelectionManagerEvent::Type::ObjectRemoved;

    m_pSelectionStorage->m_Events.Broadcast(e);
  }
}

void wdSelectionManager::SetSelection(const wdDocumentObject* pSingleObject)
{
  wdDeque<const wdDocumentObject*> objs;
  objs.PushBack(pSingleObject);
  SetSelection(objs);
}

void wdSelectionManager::SetSelection(const wdDeque<const wdDocumentObject*>& selection)
{
  if (selection.IsEmpty())
  {
    Clear();
    return;
  }

  if (m_pSelectionStorage->m_SelectionList == selection)
    return;

  m_pSelectionStorage->m_SelectionList.Clear();
  m_pSelectionStorage->m_SelectionSet.Clear();

  m_pSelectionStorage->m_SelectionList.Reserve(selection.GetCount());

  for (wdUInt32 i = 0; i < selection.GetCount(); ++i)
  {
    if (selection[i] != nullptr)
    {
      WD_ASSERT_DEV(selection[i]->GetDocumentObjectManager() == m_pSelectionStorage->m_pObjectManager, "Passed in object does not belong to same object manager.");
      wdStatus res = m_pSelectionStorage->m_pObjectManager->CanSelect(selection[i]);
      if (res.m_Result.Failed())
      {
        wdLog::Error("{0}", res.m_sMessage);
        continue;
      }
      // actually == nullptr should never happen, unless we have an error somewhere else
      m_pSelectionStorage->m_SelectionList.PushBack(selection[i]);
      m_pSelectionStorage->m_SelectionSet.Insert(selection[i]->GetGuid());
    }
  }

  {
    // Sync selection model.
    wdSelectionManagerEvent e;
    e.m_pDocument = GetDocument();
    e.m_pObject = nullptr;
    e.m_Type = wdSelectionManagerEvent::Type::SelectionSet;
    m_pSelectionStorage->m_Events.Broadcast(e);
  }
}

void wdSelectionManager::ToggleObject(const wdDocumentObject* pObject)
{
  if (IsSelected(pObject))
    RemoveObject(pObject);
  else
    AddObject(pObject);
}

const wdDocumentObject* wdSelectionManager::GetCurrentObject() const
{
  return m_pSelectionStorage->m_SelectionList.IsEmpty() ? nullptr : m_pSelectionStorage->m_SelectionList[m_pSelectionStorage->m_SelectionList.GetCount() - 1];
}

bool wdSelectionManager::IsSelected(const wdDocumentObject* pObject) const
{
  return m_pSelectionStorage->m_SelectionSet.Find(pObject->GetGuid()).IsValid();
}

bool wdSelectionManager::IsParentSelected(const wdDocumentObject* pObject) const
{
  const wdDocumentObject* pParent = pObject->GetParent();

  while (pParent != nullptr)
  {
    if (m_pSelectionStorage->m_SelectionSet.Find(pParent->GetGuid()).IsValid())
      return true;

    pParent = pParent->GetParent();
  }

  return false;
}

const wdDocument* wdSelectionManager::GetDocument() const
{
  return m_pSelectionStorage->m_pObjectManager->GetDocument();
}

wdSharedPtr<wdSelectionManager::Storage> wdSelectionManager::SwapStorage(wdSharedPtr<wdSelectionManager::Storage> pNewStorage)
{
  WD_ASSERT_ALWAYS(pNewStorage != nullptr, "Need a valid history storage object");

  auto retVal = m_pSelectionStorage;

  m_ObjectStructureUnsubscriber.Unsubscribe();
  m_EventsUnsubscriber.Unsubscribe();

  m_pSelectionStorage = pNewStorage;

  m_pSelectionStorage->m_pObjectManager->m_StructureEvents.AddEventHandler(wdMakeDelegate(&wdSelectionManager::TreeEventHandler, this), m_ObjectStructureUnsubscriber);
  m_pSelectionStorage->m_Events.AddEventHandler([this](const wdSelectionManagerEvent& e) { m_Events.Broadcast(e); }, m_EventsUnsubscriber);

  return retVal;
}

struct wdObjectHierarchyComparor
{
  using Tree = wdHybridArray<const wdDocumentObject*, 4>;
  wdObjectHierarchyComparor(wdDeque<const wdDocumentObject*>& ref_items)
  {
    for (const wdDocumentObject* pObject : ref_items)
    {
      Tree& tree = lookup[pObject];
      while (pObject)
      {
        tree.PushBack(pObject);
        pObject = pObject->GetParent();
      }
      std::reverse(begin(tree), end(tree));
    }
  }

  WD_ALWAYS_INLINE bool Less(const wdDocumentObject* lhs, const wdDocumentObject* rhs) const
  {
    const Tree& A = *lookup.GetValue(lhs);
    const Tree& B = *lookup.GetValue(rhs);

    const wdUInt32 minSize = wdMath::Min(A.GetCount(), B.GetCount());
    for (wdUInt32 i = 0; i < minSize; i++)
    {
      // The first element in the loop should always be the root so there is not risk that there is no common parent.
      if (A[i] != B[i])
      {
        // These elements are the first different ones so they share the same parent.
        // We just assume that the hierarchy is integer-based for now.
        return A[i]->GetPropertyIndex().ConvertTo<wdUInt32>() < B[i]->GetPropertyIndex().ConvertTo<wdUInt32>();
      }
    }

    return A.GetCount() < B.GetCount();
  }

  WD_ALWAYS_INLINE bool Equal(const wdDocumentObject* lhs, const wdDocumentObject* rhs) const { return lhs == rhs; }

  wdMap<const wdDocumentObject*, Tree> lookup;
};

const wdDeque<const wdDocumentObject*> wdSelectionManager::GetTopLevelSelection() const
{
  wdDeque<const wdDocumentObject*> items;

  for (const auto* pObj : m_pSelectionStorage->m_SelectionList)
  {
    if (!IsParentSelected(pObj))
    {
      items.PushBack(pObj);
    }
  }

  wdObjectHierarchyComparor c(items);
  items.Sort(c);

  return items;
}

const wdDeque<const wdDocumentObject*> wdSelectionManager::GetTopLevelSelection(const wdRTTI* pBase) const
{
  wdDeque<const wdDocumentObject*> items;

  for (const auto* pObj : m_pSelectionStorage->m_SelectionList)
  {
    if (!pObj->GetTypeAccessor().GetType()->IsDerivedFrom(pBase))
      continue;

    if (!IsParentSelected(pObj))
    {
      items.PushBack(pObj);
    }
  }

  wdObjectHierarchyComparor c(items);
  items.Sort(c);

  return items;
}
