/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/Document/DocumentManager.h>

struct nsSelectionManagerEvent;
class nsDocumentObject;
class nsVisualizerAttribute;

struct NS_GUIFOUNDATION_DLL nsVisualizerManagerEvent
{
  const nsDocument* m_pDocument;
  const nsDeque<const nsDocumentObject*>* m_pSelection;
};

class NS_GUIFOUNDATION_DLL nsVisualizerManager
{
  NS_DECLARE_SINGLETON(nsVisualizerManager);

public:
  nsVisualizerManager();
  ~nsVisualizerManager();

  void SetVisualizersActive(const nsDocument* pDoc, bool bActive);
  bool GetVisualizersActive(const nsDocument* pDoc);

  nsEvent<const nsVisualizerManagerEvent&> m_Events;

private:
  void SelectionEventHandler(const nsSelectionManagerEvent& e);
  void DocumentManagerEventHandler(const nsDocumentManager::Event& e);
  void StructureEventHandler(const nsDocumentObjectStructureEvent& e);
  void SendEventToRecreateVisualizers(const nsDocument* pDoc);

  struct DocData
  {
    bool m_bActivated;

    DocData() { m_bActivated = true; }
  };

  nsMap<const nsDocument*, DocData> m_DocsSubscribed;
};
