#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/VisualizerManager.h>
#include <ToolsFoundation/Object/DocumentObjectManager.h>

NS_IMPLEMENT_SINGLETON(nsVisualizerManager);

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(GuiFoundation, VisualizerManager)

  ON_CORESYSTEMS_STARTUP
  {
    NS_DEFAULT_NEW(nsVisualizerManager);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    if (nsVisualizerManager::GetSingleton())
    {
      auto ptr = nsVisualizerManager::GetSingleton();
      NS_DEFAULT_DELETE(ptr);
    }
  }


NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsVisualizerManager::nsVisualizerManager()
  : m_SingletonRegistrar(this)
{
  nsDocumentManager::s_Events.AddEventHandler(nsMakeDelegate(&nsVisualizerManager::DocumentManagerEventHandler, this));
}

nsVisualizerManager::~nsVisualizerManager()
{
  nsDocumentManager::s_Events.RemoveEventHandler(nsMakeDelegate(&nsVisualizerManager::DocumentManagerEventHandler, this));
}

void nsVisualizerManager::SetVisualizersActive(const nsDocument* pDoc, bool bActive)
{
  if (m_DocsSubscribed[pDoc].m_bActivated == bActive)
    return;

  m_DocsSubscribed[pDoc].m_bActivated = bActive;

  SendEventToRecreateVisualizers(pDoc);
}

bool nsVisualizerManager::GetVisualizersActive(const nsDocument* pDoc)
{
  return m_DocsSubscribed[pDoc].m_bActivated;
}

void nsVisualizerManager::SelectionEventHandler(const nsSelectionManagerEvent& event)
{
  if (!m_DocsSubscribed[event.m_pDocument].m_bActivated)
    return;

  SendEventToRecreateVisualizers(event.m_pDocument);
}

void nsVisualizerManager::SendEventToRecreateVisualizers(const nsDocument* pDoc)
{
  if (m_DocsSubscribed[pDoc].m_bActivated)
  {
    const auto& sel = pDoc->GetSelectionManager()->GetSelection();

    nsVisualizerManagerEvent e;
    e.m_pSelection = &sel;
    e.m_pDocument = pDoc;
    m_Events.Broadcast(e);
  }
  else
  {
    nsDeque<const nsDocumentObject*> sel;

    nsVisualizerManagerEvent e;
    e.m_pSelection = &sel;
    e.m_pDocument = pDoc;

    m_Events.Broadcast(e);
  }
}

void nsVisualizerManager::DocumentManagerEventHandler(const nsDocumentManager::Event& e)
{
  if (e.m_Type == nsDocumentManager::Event::Type::DocumentOpened)
  {
    e.m_pDocument->GetSelectionManager()->m_Events.AddEventHandler(nsMakeDelegate(&nsVisualizerManager::SelectionEventHandler, this));
    e.m_pDocument->GetObjectManager()->m_StructureEvents.AddEventHandler(nsMakeDelegate(&nsVisualizerManager::StructureEventHandler, this));
  }

  if (e.m_Type == nsDocumentManager::Event::Type::DocumentClosing)
  {
    e.m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsVisualizerManager::SelectionEventHandler, this));
    e.m_pDocument->GetObjectManager()->m_StructureEvents.RemoveEventHandler(nsMakeDelegate(&nsVisualizerManager::StructureEventHandler, this));

    SetVisualizersActive(e.m_pDocument, false);
  }
}

void nsVisualizerManager::StructureEventHandler(const nsDocumentObjectStructureEvent& event)
{
  if (!m_DocsSubscribed[event.m_pDocument].m_bActivated)
    return;

  if (!event.m_pDocument->GetSelectionManager()->IsSelectionEmpty() &&
      (event.m_EventType == nsDocumentObjectStructureEvent::Type::AfterObjectAdded ||
        event.m_EventType == nsDocumentObjectStructureEvent::Type::AfterObjectRemoved))
  {
    SendEventToRecreateVisualizers(event.m_pDocument);
  }
}
