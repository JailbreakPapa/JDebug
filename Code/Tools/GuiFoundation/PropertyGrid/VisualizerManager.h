#pragma once

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/Document/DocumentManager.h>

struct wdSelectionManagerEvent;
class wdDocumentObject;
class wdVisualizerAttribute;

struct WD_GUIFOUNDATION_DLL wdVisualizerManagerEvent
{
  const wdDocument* m_pDocument;
  const wdDeque<const wdDocumentObject*>* m_pSelection;
};

class WD_GUIFOUNDATION_DLL wdVisualizerManager
{
  WD_DECLARE_SINGLETON(wdVisualizerManager);

public:
  wdVisualizerManager();
  ~wdVisualizerManager();

  void SetVisualizersActive(const wdDocument* pDoc, bool bActive);
  bool GetVisualizersActive(const wdDocument* pDoc);

  wdEvent<const wdVisualizerManagerEvent&> m_Events;

private:
  void SelectionEventHandler(const wdSelectionManagerEvent& e);
  void DocumentManagerEventHandler(const wdDocumentManager::Event& e);
  void StructureEventHandler(const wdDocumentObjectStructureEvent& e);
  void SendEventToRecreateVisualizers(const wdDocument* pDoc);

  struct DocData
  {
    bool m_bActivated;

    DocData() { m_bActivated = true; }
  };

  wdMap<const wdDocument*, DocData> m_DocsSubscribed;
};
