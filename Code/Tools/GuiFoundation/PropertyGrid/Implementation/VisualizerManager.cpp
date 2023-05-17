#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/VisualizerManager.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

WD_IMPLEMENT_SINGLETON(wdVisualizerManager);

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, VisualizerManager)

  ON_CORESYSTEMS_STARTUP
  {
    WD_DEFAULT_NEW(wdVisualizerManager);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    if (wdVisualizerManager::GetSingleton())
    {
      auto ptr = wdVisualizerManager::GetSingleton();
      WD_DEFAULT_DELETE(ptr);
    }
  }


WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdVisualizerManager::wdVisualizerManager()
  : m_SingletonRegistrar(this)
{
  wdDocumentManager::s_Events.AddEventHandler(wdMakeDelegate(&wdVisualizerManager::DocumentManagerEventHandler, this));
}

wdVisualizerManager::~wdVisualizerManager()
{
  wdDocumentManager::s_Events.RemoveEventHandler(wdMakeDelegate(&wdVisualizerManager::DocumentManagerEventHandler, this));
}

void wdVisualizerManager::SetVisualizersActive(const wdDocument* pDoc, bool bActive)
{
  if (m_DocsSubscribed[pDoc].m_bActivated == bActive)
    return;

  m_DocsSubscribed[pDoc].m_bActivated = bActive;

  SendEventToRecreateVisualizers(pDoc);
}

bool wdVisualizerManager::GetVisualizersActive(const wdDocument* pDoc)
{
  return m_DocsSubscribed[pDoc].m_bActivated;
}

void wdVisualizerManager::SelectionEventHandler(const wdSelectionManagerEvent& event)
{
  if (!m_DocsSubscribed[event.m_pDocument].m_bActivated)
    return;

  SendEventToRecreateVisualizers(event.m_pDocument);
}

void wdVisualizerManager::SendEventToRecreateVisualizers(const wdDocument* pDoc)
{
  if (m_DocsSubscribed[pDoc].m_bActivated)
  {
    const auto& sel = pDoc->GetSelectionManager()->GetSelection();

    wdVisualizerManagerEvent e;
    e.m_pSelection = &sel;
    e.m_pDocument = pDoc;
    m_Events.Broadcast(e);
  }
  else
  {
    wdDeque<const wdDocumentObject*> sel;

    wdVisualizerManagerEvent e;
    e.m_pSelection = &sel;
    e.m_pDocument = pDoc;

    m_Events.Broadcast(e);
  }
}

void wdVisualizerManager::DocumentManagerEventHandler(const wdDocumentManager::Event& e)
{
  if (e.m_Type == wdDocumentManager::Event::Type::DocumentOpened)
  {
    e.m_pDocument->GetSelectionManager()->m_Events.AddEventHandler(wdMakeDelegate(&wdVisualizerManager::SelectionEventHandler, this));
    e.m_pDocument->GetObjectManager()->m_StructureEvents.AddEventHandler(wdMakeDelegate(&wdVisualizerManager::StructureEventHandler, this));
  }

  if (e.m_Type == wdDocumentManager::Event::Type::DocumentClosing)
  {
    e.m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdVisualizerManager::SelectionEventHandler, this));
    e.m_pDocument->GetObjectManager()->m_StructureEvents.RemoveEventHandler(wdMakeDelegate(&wdVisualizerManager::StructureEventHandler, this));

    SetVisualizersActive(e.m_pDocument, false);
  }
}

void wdVisualizerManager::StructureEventHandler(const wdDocumentObjectStructureEvent& event)
{
  if (!m_DocsSubscribed[event.m_pDocument].m_bActivated)
    return;

  if (!event.m_pDocument->GetSelectionManager()->IsSelectionEmpty() &&
      (event.m_EventType == wdDocumentObjectStructureEvent::Type::AfterObjectAdded ||
        event.m_EventType == wdDocumentObjectStructureEvent::Type::AfterObjectRemoved))
  {
    SendEventToRecreateVisualizers(event.m_pDocument);
  }
}
