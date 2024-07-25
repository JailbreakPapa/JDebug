#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/ManipulatorManager.h>
#include <ToolsFoundation/Document/Document.h>

NS_IMPLEMENT_SINGLETON(nsManipulatorManager);

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, ManipulatorManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ReflectedTypeManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    NS_DEFAULT_NEW(nsManipulatorManager);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    if (nsManipulatorManager::GetSingleton())
    {
      auto ptr = nsManipulatorManager::GetSingleton();
      NS_DEFAULT_DELETE(ptr);
    }
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsManipulatorManager::nsManipulatorManager()
  : m_SingletonRegistrar(this)
{
  nsPhantomRttiManager::s_Events.AddEventHandler(nsMakeDelegate(&nsManipulatorManager::PhantomTypeManagerEventHandler, this));
  nsDocumentManager::s_Events.AddEventHandler(nsMakeDelegate(&nsManipulatorManager::DocumentManagerEventHandler, this));
}

nsManipulatorManager::~nsManipulatorManager()
{
  nsPhantomRttiManager::s_Events.RemoveEventHandler(nsMakeDelegate(&nsManipulatorManager::PhantomTypeManagerEventHandler, this));
  nsDocumentManager::s_Events.RemoveEventHandler(nsMakeDelegate(&nsManipulatorManager::DocumentManagerEventHandler, this));
}

const nsManipulatorAttribute* nsManipulatorManager::GetActiveManipulator(
  const nsDocument* pDoc, const nsHybridArray<nsPropertySelection, 8>*& out_pSelection) const
{
  out_pSelection = nullptr;
  auto it = m_ActiveManipulator.Find(pDoc);

  if (it.IsValid())
  {
    out_pSelection = &(it.Value().m_Selection);

    return it.Value().m_pAttribute;
  }

  return nullptr;
}

void nsManipulatorManager::InternalSetActiveManipulator(
  const nsDocument* pDoc, const nsManipulatorAttribute* pManipulator, const nsHybridArray<nsPropertySelection, 8>& selection, bool bUnhide)
{
  bool existed = false;
  auto it = m_ActiveManipulator.FindOrAdd(pDoc, &existed);

  it.Value().m_pAttribute = pManipulator;
  it.Value().m_Selection = selection;

  if (!existed)
  {
    pDoc->GetObjectManager()->m_StructureEvents.AddEventHandler(nsMakeDelegate(&nsManipulatorManager::StructureEventHandler, this));
    pDoc->GetSelectionManager()->m_Events.AddEventHandler(nsMakeDelegate(&nsManipulatorManager::SelectionEventHandler, this));
  }

  auto& data = m_ActiveManipulator[pDoc];

  if (bUnhide)
  {
    data.m_bHideManipulators = false;
  }

  nsManipulatorManagerEvent e;
  e.m_bHideManipulators = data.m_bHideManipulators;
  e.m_pDocument = pDoc;
  e.m_pManipulator = pManipulator;
  e.m_pSelection = &data.m_Selection;

  m_Events.Broadcast(e);
}


void nsManipulatorManager::SetActiveManipulator(
  const nsDocument* pDoc, const nsManipulatorAttribute* pManipulator, const nsHybridArray<nsPropertySelection, 8>& selection)
{
  InternalSetActiveManipulator(pDoc, pManipulator, selection, true);
}

void nsManipulatorManager::ClearActiveManipulator(const nsDocument* pDoc)
{
  nsHybridArray<nsPropertySelection, 8> clearSel;

  InternalSetActiveManipulator(pDoc, nullptr, clearSel, false);
}

void nsManipulatorManager::HideActiveManipulator(const nsDocument* pDoc, bool bHide)
{
  auto it = m_ActiveManipulator.Find(pDoc);

  if (it.IsValid() && it.Value().m_bHideManipulators != bHide)
  {
    it.Value().m_bHideManipulators = bHide;

    if (bHide)
    {
      nsHybridArray<nsPropertySelection, 8> clearSel;
      InternalSetActiveManipulator(pDoc, it.Value().m_pAttribute, clearSel, false);
    }
    else
    {
      TransferToCurrentSelection(pDoc);
    }
  }
}

void nsManipulatorManager::ToggleHideActiveManipulator(const nsDocument* pDoc)
{
  auto it = m_ActiveManipulator.Find(pDoc);

  if (it.IsValid())
  {
    it.Value().m_bHideManipulators = !it.Value().m_bHideManipulators;

    if (it.Value().m_bHideManipulators)
    {
      nsHybridArray<nsPropertySelection, 8> clearSel;
      InternalSetActiveManipulator(pDoc, it.Value().m_pAttribute, clearSel, false);
    }
    else
    {
      TransferToCurrentSelection(pDoc);
    }
  }
}

void nsManipulatorManager::StructureEventHandler(const nsDocumentObjectStructureEvent& e)
{
  if (e.m_EventType == nsDocumentObjectStructureEvent::Type::BeforeObjectRemoved)
  {
    auto pDoc = e.m_pObject->GetDocumentObjectManager()->GetDocument();
    auto it = m_ActiveManipulator.Find(pDoc);

    if (it.IsValid())
    {
      for (auto& sel : it.Value().m_Selection)
      {
        if (sel.m_pObject == e.m_pObject)
        {
          it.Value().m_Selection.RemoveAndCopy(sel);
          InternalSetActiveManipulator(pDoc, it.Value().m_pAttribute, it.Value().m_Selection, false);
          return;
        }
      }
    }
  }

  if (e.m_EventType == nsDocumentObjectStructureEvent::Type::BeforeReset)
  {
    auto pDoc = e.m_pDocument;
    auto it = m_ActiveManipulator.Find(pDoc);

    if (it.IsValid())
    {
      for (auto& sel : it.Value().m_Selection)
      {
        it.Value().m_Selection.RemoveAndCopy(sel);
        InternalSetActiveManipulator(pDoc, it.Value().m_pAttribute, it.Value().m_Selection, false);
      }
    }
  }
}

void nsManipulatorManager::SelectionEventHandler(const nsSelectionManagerEvent& e)
{
  TransferToCurrentSelection(e.m_pDocument->GetMainDocument());
}

void nsManipulatorManager::TransferToCurrentSelection(const nsDocument* pDoc)
{
  auto& data = m_ActiveManipulator[pDoc];
  auto pAttribute = data.m_pAttribute;

  if (pAttribute == nullptr)
    return;

  if (data.m_bHideManipulators)
    return;

  nsHybridArray<nsPropertySelection, 8> newSelection;

  const auto& selection = pDoc->GetSelectionManager()->GetSelection();

  NS_ASSERT_DEV(pDoc->GetManipulatorSearchStrategy() != nsManipulatorSearchStrategy::None, "The document type '{}' has to override the function 'GetManipulatorSearchStrategy()'", pDoc->GetDynamicRTTI()->GetTypeName());

  if (pDoc->GetManipulatorSearchStrategy() == nsManipulatorSearchStrategy::SelectedObject)
  {
    for (nsUInt32 i = 0; i < selection.GetCount(); ++i)
    {
      const auto& OtherAttributes = selection[i]->GetTypeAccessor().GetType()->GetAttributes();

      for (const auto pOtherAttr : OtherAttributes)
      {
        if (pOtherAttr->IsInstanceOf(pAttribute->GetDynamicRTTI()))
        {
          auto pOtherManip = static_cast<const nsManipulatorAttribute*>(pOtherAttr);

          if (pOtherManip->m_sProperty1 == pAttribute->m_sProperty1 && pOtherManip->m_sProperty2 == pAttribute->m_sProperty2 &&
              pOtherManip->m_sProperty3 == pAttribute->m_sProperty3 && pOtherManip->m_sProperty4 == pAttribute->m_sProperty4 &&
              pOtherManip->m_sProperty5 == pAttribute->m_sProperty5 && pOtherManip->m_sProperty6 == pAttribute->m_sProperty6)
          {
            auto& newItem = newSelection.ExpandAndGetRef();
            newItem.m_pObject = selection[i];
          }
        }
      }
    }
  }

  if (pDoc->GetManipulatorSearchStrategy() == nsManipulatorSearchStrategy::ChildrenOfSelectedObject)
  {
    for (nsUInt32 i = 0; i < selection.GetCount(); ++i)
    {
      const auto& children = selection[i]->GetChildren();

      for (const auto& child : children)
      {
        const auto& OtherAttributes = child->GetTypeAccessor().GetType()->GetAttributes();

        for (const auto pOtherAttr : OtherAttributes)
        {
          if (pOtherAttr->IsInstanceOf(pAttribute->GetDynamicRTTI()))
          {
            auto pOtherManip = static_cast<const nsManipulatorAttribute*>(pOtherAttr);

            if (pOtherManip->m_sProperty1 == pAttribute->m_sProperty1 && pOtherManip->m_sProperty2 == pAttribute->m_sProperty2 &&
                pOtherManip->m_sProperty3 == pAttribute->m_sProperty3 && pOtherManip->m_sProperty4 == pAttribute->m_sProperty4 &&
                pOtherManip->m_sProperty5 == pAttribute->m_sProperty5 && pOtherManip->m_sProperty6 == pAttribute->m_sProperty6)
            {
              auto& newItem = newSelection.ExpandAndGetRef();
              newItem.m_pObject = child;
            }
          }
        }
      }
    }
  }

  InternalSetActiveManipulator(pDoc, pAttribute, newSelection, false);
}

void nsManipulatorManager::PhantomTypeManagerEventHandler(const nsPhantomRttiManagerEvent& e)
{
  if (e.m_Type == nsPhantomRttiManagerEvent::Type::TypeChanged || e.m_Type == nsPhantomRttiManagerEvent::Type::TypeRemoved)
  {
    for (auto it = m_ActiveManipulator.GetIterator(); it.IsValid(); ++it)
    {
      ClearActiveManipulator(it.Key());
    }
  }
}

void nsManipulatorManager::DocumentManagerEventHandler(const nsDocumentManager::Event& e)
{
  if (e.m_Type == nsDocumentManager::Event::Type::DocumentClosing)
  {
    ClearActiveManipulator(e.m_pDocument);

    e.m_pDocument->GetObjectManager()->m_StructureEvents.RemoveEventHandler(nsMakeDelegate(&nsManipulatorManager::StructureEventHandler, this));
    e.m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsManipulatorManager::SelectionEventHandler, this));

    m_ActiveManipulator.Remove(e.m_pDocument);
  }
}
