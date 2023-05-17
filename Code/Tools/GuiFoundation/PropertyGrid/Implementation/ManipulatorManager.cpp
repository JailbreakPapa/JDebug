#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/ManipulatorManager.h>
#include <ToolsFoundation/Document/Document.h>

WD_IMPLEMENT_SINGLETON(wdManipulatorManager);

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, ManipulatorManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ReflectedTypeManager"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    WD_DEFAULT_NEW(wdManipulatorManager);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    if (wdManipulatorManager::GetSingleton())
    {
      auto ptr = wdManipulatorManager::GetSingleton();
      WD_DEFAULT_DELETE(ptr);
    }
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdManipulatorManager::wdManipulatorManager()
  : m_SingletonRegistrar(this)
{
  wdPhantomRttiManager::s_Events.AddEventHandler(wdMakeDelegate(&wdManipulatorManager::PhantomTypeManagerEventHandler, this));
  wdDocumentManager::s_Events.AddEventHandler(wdMakeDelegate(&wdManipulatorManager::DocumentManagerEventHandler, this));
}

wdManipulatorManager::~wdManipulatorManager()
{
  wdPhantomRttiManager::s_Events.RemoveEventHandler(wdMakeDelegate(&wdManipulatorManager::PhantomTypeManagerEventHandler, this));
  wdDocumentManager::s_Events.RemoveEventHandler(wdMakeDelegate(&wdManipulatorManager::DocumentManagerEventHandler, this));
}

const wdManipulatorAttribute* wdManipulatorManager::GetActiveManipulator(
  const wdDocument* pDoc, const wdHybridArray<wdPropertySelection, 8>*& out_pSelection) const
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

void wdManipulatorManager::InternalSetActiveManipulator(
  const wdDocument* pDoc, const wdManipulatorAttribute* pManipulator, const wdHybridArray<wdPropertySelection, 8>& selection, bool bUnhide)
{
  bool existed = false;
  auto it = m_ActiveManipulator.FindOrAdd(pDoc, &existed);

  it.Value().m_pAttribute = pManipulator;
  it.Value().m_Selection = selection;

  if (!existed)
  {
    pDoc->GetObjectManager()->m_StructureEvents.AddEventHandler(wdMakeDelegate(&wdManipulatorManager::StructureEventHandler, this));
    pDoc->GetSelectionManager()->m_Events.AddEventHandler(wdMakeDelegate(&wdManipulatorManager::SelectionEventHandler, this));
  }

  auto& data = m_ActiveManipulator[pDoc];

  if (bUnhide)
  {
    data.m_bHideManipulators = false;
  }

  wdManipulatorManagerEvent e;
  e.m_bHideManipulators = data.m_bHideManipulators;
  e.m_pDocument = pDoc;
  e.m_pManipulator = pManipulator;
  e.m_pSelection = &data.m_Selection;

  m_Events.Broadcast(e);
}


void wdManipulatorManager::SetActiveManipulator(
  const wdDocument* pDoc, const wdManipulatorAttribute* pManipulator, const wdHybridArray<wdPropertySelection, 8>& selection)
{
  InternalSetActiveManipulator(pDoc, pManipulator, selection, true);
}

void wdManipulatorManager::ClearActiveManipulator(const wdDocument* pDoc)
{
  wdHybridArray<wdPropertySelection, 8> clearSel;

  InternalSetActiveManipulator(pDoc, nullptr, clearSel, false);
}

void wdManipulatorManager::HideActiveManipulator(const wdDocument* pDoc, bool bHide)
{
  auto it = m_ActiveManipulator.Find(pDoc);

  if (it.IsValid() && it.Value().m_bHideManipulators != bHide)
  {
    it.Value().m_bHideManipulators = bHide;

    if (bHide)
    {
      wdHybridArray<wdPropertySelection, 8> clearSel;
      InternalSetActiveManipulator(pDoc, it.Value().m_pAttribute, clearSel, false);
    }
    else
    {
      TransferToCurrentSelection(pDoc);
    }
  }
}

void wdManipulatorManager::ToggleHideActiveManipulator(const wdDocument* pDoc)
{
  auto it = m_ActiveManipulator.Find(pDoc);

  if (it.IsValid())
  {
    it.Value().m_bHideManipulators = !it.Value().m_bHideManipulators;

    if (it.Value().m_bHideManipulators)
    {
      wdHybridArray<wdPropertySelection, 8> clearSel;
      InternalSetActiveManipulator(pDoc, it.Value().m_pAttribute, clearSel, false);
    }
    else
    {
      TransferToCurrentSelection(pDoc);
    }
  }
}

void wdManipulatorManager::StructureEventHandler(const wdDocumentObjectStructureEvent& e)
{
  if (e.m_EventType == wdDocumentObjectStructureEvent::Type::BeforeObjectRemoved)
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

  if (e.m_EventType == wdDocumentObjectStructureEvent::Type::BeforeReset)
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

void wdManipulatorManager::SelectionEventHandler(const wdSelectionManagerEvent& e)
{
  TransferToCurrentSelection(e.m_pDocument->GetMainDocument());
}

void wdManipulatorManager::TransferToCurrentSelection(const wdDocument* pDoc)
{
  auto& data = m_ActiveManipulator[pDoc];
  auto pAttribute = data.m_pAttribute;

  if (pAttribute == nullptr)
    return;

  if (data.m_bHideManipulators)
    return;

  wdHybridArray<wdPropertySelection, 8> newSelection;

  const auto& selection = pDoc->GetSelectionManager()->GetSelection();

  WD_ASSERT_DEV(pDoc->GetManipulatorSearchStrategy() != wdManipulatorSearchStrategy::None, "The document type '{}' has to override the function 'GetManipulatorSearchStrategy()'", pDoc->GetDynamicRTTI()->GetTypeName());

  if (pDoc->GetManipulatorSearchStrategy() == wdManipulatorSearchStrategy::SelectedObject)
  {
    for (wdUInt32 i = 0; i < selection.GetCount(); ++i)
    {
      const auto& OtherAttributes = selection[i]->GetTypeAccessor().GetType()->GetAttributes();

      for (const auto pOtherAttr : OtherAttributes)
      {
        if (pOtherAttr->IsInstanceOf(pAttribute->GetDynamicRTTI()))
        {
          wdManipulatorAttribute* pOtherManip = static_cast<wdManipulatorAttribute*>(pOtherAttr);

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

  if (pDoc->GetManipulatorSearchStrategy() == wdManipulatorSearchStrategy::ChildrenOfSelectedObject)
  {
    for (wdUInt32 i = 0; i < selection.GetCount(); ++i)
    {
      const auto& children = selection[i]->GetChildren();

      for (const auto& child : children)
      {
        const auto& OtherAttributes = child->GetTypeAccessor().GetType()->GetAttributes();

        for (const auto pOtherAttr : OtherAttributes)
        {
          if (pOtherAttr->IsInstanceOf(pAttribute->GetDynamicRTTI()))
          {
            wdManipulatorAttribute* pOtherManip = static_cast<wdManipulatorAttribute*>(pOtherAttr);

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

void wdManipulatorManager::PhantomTypeManagerEventHandler(const wdPhantomRttiManagerEvent& e)
{
  if (e.m_Type == wdPhantomRttiManagerEvent::Type::TypeChanged || e.m_Type == wdPhantomRttiManagerEvent::Type::TypeRemoved)
  {
    for (auto it = m_ActiveManipulator.GetIterator(); it.IsValid(); ++it)
    {
      ClearActiveManipulator(it.Key());
    }
  }
}

void wdManipulatorManager::DocumentManagerEventHandler(const wdDocumentManager::Event& e)
{
  if (e.m_Type == wdDocumentManager::Event::Type::DocumentClosing)
  {
    ClearActiveManipulator(e.m_pDocument);

    e.m_pDocument->GetObjectManager()->m_StructureEvents.RemoveEventHandler(wdMakeDelegate(&wdManipulatorManager::StructureEventHandler, this));
    e.m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdManipulatorManager::SelectionEventHandler, this));

    m_ActiveManipulator.Remove(e.m_pDocument);
  }
}
