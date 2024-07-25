#pragma once

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <ToolsFoundation/Document/DocumentManager.h>

class nsManipulatorAttribute;
struct nsPhantomRttiManagerEvent;
struct nsSelectionManagerEvent;

struct NS_GUIFOUNDATION_DLL nsManipulatorManagerEvent
{
  const nsDocument* m_pDocument;
  const nsManipulatorAttribute* m_pManipulator;
  const nsHybridArray<nsPropertySelection, 8>* m_pSelection;
  bool m_bHideManipulators;
};

class NS_GUIFOUNDATION_DLL nsManipulatorManager
{
  NS_DECLARE_SINGLETON(nsManipulatorManager);

public:
  nsManipulatorManager();
  ~nsManipulatorManager();

  const nsManipulatorAttribute* GetActiveManipulator(const nsDocument* pDoc, const nsHybridArray<nsPropertySelection, 8>*& out_pSelection) const;

  void SetActiveManipulator(
    const nsDocument* pDoc, const nsManipulatorAttribute* pManipulator, const nsHybridArray<nsPropertySelection, 8>& selection);

  void ClearActiveManipulator(const nsDocument* pDoc);

  nsCopyOnBroadcastEvent<const nsManipulatorManagerEvent&> m_Events;

  void HideActiveManipulator(const nsDocument* pDoc, bool bHide);
  void ToggleHideActiveManipulator(const nsDocument* pDoc);

private:
  struct Data
  {
    Data()
    {
      m_pAttribute = nullptr;
      m_bHideManipulators = false;
    }

    const nsManipulatorAttribute* m_pAttribute;
    nsHybridArray<nsPropertySelection, 8> m_Selection;
    bool m_bHideManipulators;
  };

  void InternalSetActiveManipulator(
    const nsDocument* pDoc, const nsManipulatorAttribute* pManipulator, const nsHybridArray<nsPropertySelection, 8>& selection, bool bUnhide);

  void StructureEventHandler(const nsDocumentObjectStructureEvent& e);
  void SelectionEventHandler(const nsSelectionManagerEvent& e);

  void TransferToCurrentSelection(const nsDocument* pDoc);

  void PhantomTypeManagerEventHandler(const nsPhantomRttiManagerEvent& e);
  void DocumentManagerEventHandler(const nsDocumentManager::Event& e);

  nsMap<const nsDocument*, Data> m_ActiveManipulator;
};
