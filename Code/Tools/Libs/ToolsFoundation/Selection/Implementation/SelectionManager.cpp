#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>
#include <ToolsFoundation/Selection/SelectionManager.h>

nsSelectionManager::nsSelectionManager(const nsDocumentObjectManager* pObjectManager)
{
  auto pStorage = NS_DEFAULT_NEW(Storage);
  pStorage->m_pObjectManager = pObjectManager;
  SwapStorage(pStorage);
}

nsSelectionManager::~nsSelectionManager()
{
  m_ObjectStructureUnsubscriber.Unsubscribe();
  m_EventsUnsubscriber.Unsubscribe();
}

void nsSelectionManager::TreeEventHandler(const nsDocumentObjectStructureEvent& e)
{
  switch (e.m_EventType)
  {
    case nsDocumentObjectStructureEvent::Type::BeforeObjectRemoved:
      RemoveObject(e.m_pObject, true);
      break;
    default:
      return;
  }
}

bool nsSelectionManager::RecursiveRemoveFromSelection(const nsDocumentObject* pObject)
{
  auto it = m_pSelectionStorage->m_SelectionSet.Find(pObject->GetGuid());

  bool bRemoved = false;
  if (it.IsValid())
  {
    m_pSelectionStorage->m_SelectionSet.Remove(it);
    m_pSelectionStorage->m_SelectionList.RemoveAndCopy(pObject);
    bRemoved = true;
  }

  for (const nsDocumentObject* pChild : pObject->GetChildren())
  {
    bRemoved = bRemoved || RecursiveRemoveFromSelection(pChild);
  }
  return bRemoved;
}

void nsSelectionManager::Clear()
{
  if (!m_pSelectionStorage->m_SelectionList.IsEmpty() || !m_pSelectionStorage->m_SelectionSet.IsEmpty())
  {
    m_pSelectionStorage->m_SelectionList.Clear();
    m_pSelectionStorage->m_SelectionSet.Clear();

    nsSelectionManagerEvent e;
    e.m_pDocument = GetDocument();
    e.m_pObject = nullptr;
    e.m_Type = nsSelectionManagerEvent::Type::SelectionCleared;

    m_pSelectionStorage->m_Events.Broadcast(e);
  }
}

void nsSelectionManager::AddObject(const nsDocumentObject* pObject)
{
  NS_ASSERT_DEBUG(pObject, "Object must be valid");

  if (IsSelected(pObject))
    return;

  NS_ASSERT_DEV(pObject->GetDocumentObjectManager() == m_pSelectionStorage->m_pObjectManager, "Passed in object does not belong to same object manager.");
  nsStatus res = m_pSelectionStorage->m_pObjectManager->CanSelect(pObject);
  if (res.m_Result.Failed())
  {
    nsLog::Error("{0}", res.m_sMessage);
    return;
  }

  m_pSelectionStorage->m_SelectionList.PushBack(pObject);
  m_pSelectionStorage->m_SelectionSet.Insert(pObject->GetGuid());

  nsSelectionManagerEvent e;
  e.m_pDocument = GetDocument();
  e.m_pObject = pObject;
  e.m_Type = nsSelectionManagerEvent::Type::ObjectAdded;

  m_pSelectionStorage->m_Events.Broadcast(e);
}

void nsSelectionManager::RemoveObject(const nsDocumentObject* pObject, bool bRecurseChildren)
{
  if (bRecurseChildren)
  {
    // We only want one message for the change in selection so we first everything and then fire
    // SelectionSet instead of multiple ObjectRemoved messages.
    if (RecursiveRemoveFromSelection(pObject))
    {
      nsSelectionManagerEvent e;
      e.m_pDocument = GetDocument();
      e.m_pObject = nullptr;
      e.m_Type = nsSelectionManagerEvent::Type::SelectionSet;
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

    nsSelectionManagerEvent e;
    e.m_pDocument = GetDocument();
    e.m_pObject = pObject;
    e.m_Type = nsSelectionManagerEvent::Type::ObjectRemoved;

    m_pSelectionStorage->m_Events.Broadcast(e);
  }
}

void nsSelectionManager::SetSelection(const nsDocumentObject* pSingleObject)
{
  nsDeque<const nsDocumentObject*> objs;
  objs.PushBack(pSingleObject);
  SetSelection(objs);
}

void nsSelectionManager::SetSelection(const nsDeque<const nsDocumentObject*>& selection)
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

  for (nsUInt32 i = 0; i < selection.GetCount(); ++i)
  {
    if (selection[i] != nullptr)
    {
      NS_ASSERT_DEV(selection[i]->GetDocumentObjectManager() == m_pSelectionStorage->m_pObjectManager, "Passed in object does not belong to same object manager.");
      nsStatus res = m_pSelectionStorage->m_pObjectManager->CanSelect(selection[i]);
      if (res.m_Result.Failed())
      {
        nsLog::Error("{0}", res.m_sMessage);
        continue;
      }
      // actually == nullptr should never happen, unless we have an error somewhere else
      m_pSelectionStorage->m_SelectionList.PushBack(selection[i]);
      m_pSelectionStorage->m_SelectionSet.Insert(selection[i]->GetGuid());
    }
  }

  {
    // Sync selection model.
    nsSelectionManagerEvent e;
    e.m_pDocument = GetDocument();
    e.m_pObject = nullptr;
    e.m_Type = nsSelectionManagerEvent::Type::SelectionSet;
    m_pSelectionStorage->m_Events.Broadcast(e);
  }
}

void nsSelectionManager::ToggleObject(const nsDocumentObject* pObject)
{
  if (IsSelected(pObject))
    RemoveObject(pObject);
  else
    AddObject(pObject);
}

const nsDocumentObject* nsSelectionManager::GetCurrentObject() const
{
  return m_pSelectionStorage->m_SelectionList.IsEmpty() ? nullptr : m_pSelectionStorage->m_SelectionList.PeekBack();
}

bool nsSelectionManager::IsSelected(const nsDocumentObject* pObject) const
{
  return m_pSelectionStorage->m_SelectionSet.Find(pObject->GetGuid()).IsValid();
}

bool nsSelectionManager::IsParentSelected(const nsDocumentObject* pObject) const
{
  const nsDocumentObject* pParent = pObject->GetParent();

  while (pParent != nullptr)
  {
    if (m_pSelectionStorage->m_SelectionSet.Find(pParent->GetGuid()).IsValid())
      return true;

    pParent = pParent->GetParent();
  }

  return false;
}

const nsDocument* nsSelectionManager::GetDocument() const
{
  return m_pSelectionStorage->m_pObjectManager->GetDocument();
}

nsSharedPtr<nsSelectionManager::Storage> nsSelectionManager::SwapStorage(nsSharedPtr<nsSelectionManager::Storage> pNewStorage)
{
  NS_ASSERT_ALWAYS(pNewStorage != nullptr, "Need a valid history storage object");

  auto retVal = m_pSelectionStorage;

  m_ObjectStructureUnsubscriber.Unsubscribe();
  m_EventsUnsubscriber.Unsubscribe();

  m_pSelectionStorage = pNewStorage;

  m_pSelectionStorage->m_pObjectManager->m_StructureEvents.AddEventHandler(nsMakeDelegate(&nsSelectionManager::TreeEventHandler, this), m_ObjectStructureUnsubscriber);
  m_pSelectionStorage->m_Events.AddEventHandler([this](const nsSelectionManagerEvent& e)
    { m_Events.Broadcast(e); },
    m_EventsUnsubscriber);

  return retVal;
}

struct nsObjectHierarchyComparor
{
  using Tree = nsHybridArray<const nsDocumentObject*, 4>;

  nsObjectHierarchyComparor(nsArrayPtr<nsSelectionEntry> items)
  {
    for (const nsSelectionEntry& e : items)
    {
      const nsDocumentObject* pObject = e.m_pObject;

      Tree& tree = lookup[pObject];
      while (pObject)
      {
        tree.PushBack(pObject);
        pObject = pObject->GetParent();
      }
      std::reverse(begin(tree), end(tree));
    }
  }

  NS_ALWAYS_INLINE bool Less(const nsSelectionEntry& lhs, const nsSelectionEntry& rhs) const
  {
    const Tree& A = *lookup.GetValue(lhs.m_pObject);
    const Tree& B = *lookup.GetValue(rhs.m_pObject);

    const nsUInt32 minSize = nsMath::Min(A.GetCount(), B.GetCount());
    for (nsUInt32 i = 0; i < minSize; i++)
    {
      // The first element in the loop should always be the root so there is not risk that there is no common parent.
      if (A[i] != B[i])
      {
        // These elements are the first different ones so they share the same parent.
        // We just assume that the hierarchy is integer-based for now.
        return A[i]->GetPropertyIndex().ConvertTo<nsUInt32>() < B[i]->GetPropertyIndex().ConvertTo<nsUInt32>();
      }
    }

    return A.GetCount() < B.GetCount();
  }

  NS_ALWAYS_INLINE bool Equal(const nsSelectionEntry& lhs, const nsSelectionEntry& rhs) const { return lhs.m_pObject == rhs.m_pObject; }

  nsMap<const nsDocumentObject*, Tree> lookup;
};

void nsSelectionManager::GetTopLevelSelection(nsDynamicArray<nsSelectionEntry>& out_entries) const
{
  out_entries.Clear();
  out_entries.Reserve(m_pSelectionStorage->m_SelectionList.GetCount());

  nsUInt32 order = 0;

  for (const auto* pObj : m_pSelectionStorage->m_SelectionList)
  {
    if (!IsParentSelected(pObj))
    {
      auto& e = out_entries.ExpandAndGetRef();
      e.m_pObject = pObj;
      e.m_uiSelectionOrder = order++;
    }
  }

  nsObjectHierarchyComparor c(out_entries);
  out_entries.Sort(c);
}

void nsSelectionManager::GetTopLevelSelectionOfType(const nsRTTI* pBase, nsDynamicArray<nsSelectionEntry>& out_entries) const
{
  out_entries.Clear();
  out_entries.Reserve(m_pSelectionStorage->m_SelectionList.GetCount());

  nsUInt32 order = 0;

  for (const auto* pObj : m_pSelectionStorage->m_SelectionList)
  {
    if (!pObj->GetTypeAccessor().GetType()->IsDerivedFrom(pBase))
      continue;

    if (!IsParentSelected(pObj))
    {
      auto& e = out_entries.ExpandAndGetRef();
      e.m_pObject = pObj;
      e.m_uiSelectionOrder = order++;
    }
  }

  nsObjectHierarchyComparor c(out_entries);
  out_entries.Sort(c);
}
