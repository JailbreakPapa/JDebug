#pragma once

#include <Foundation/Configuration/Singleton.h>
#include <Foundation/Configuration/Startup.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <ToolsFoundation/Document/DocumentManager.h>

class wdManipulatorAttribute;
struct wdPhantomRttiManagerEvent;
struct wdSelectionManagerEvent;

struct WD_GUIFOUNDATION_DLL wdManipulatorManagerEvent
{
  const wdDocument* m_pDocument;
  const wdManipulatorAttribute* m_pManipulator;
  const wdHybridArray<wdPropertySelection, 8>* m_pSelection;
  bool m_bHideManipulators;
};

class WD_GUIFOUNDATION_DLL wdManipulatorManager
{
  WD_DECLARE_SINGLETON(wdManipulatorManager);

public:
  wdManipulatorManager();
  ~wdManipulatorManager();

  const wdManipulatorAttribute* GetActiveManipulator(const wdDocument* pDoc, const wdHybridArray<wdPropertySelection, 8>*& out_pSelection) const;

  void SetActiveManipulator(
    const wdDocument* pDoc, const wdManipulatorAttribute* pManipulator, const wdHybridArray<wdPropertySelection, 8>& selection);

  void ClearActiveManipulator(const wdDocument* pDoc);

  wdCopyOnBroadcastEvent<const wdManipulatorManagerEvent&> m_Events;

  void HideActiveManipulator(const wdDocument* pDoc, bool bHide);
  void ToggleHideActiveManipulator(const wdDocument* pDoc);

private:
  struct Data
  {
    Data()
    {
      m_pAttribute = nullptr;
      m_bHideManipulators = false;
    }

    const wdManipulatorAttribute* m_pAttribute;
    wdHybridArray<wdPropertySelection, 8> m_Selection;
    bool m_bHideManipulators;
  };

  void InternalSetActiveManipulator(
    const wdDocument* pDoc, const wdManipulatorAttribute* pManipulator, const wdHybridArray<wdPropertySelection, 8>& selection, bool bUnhide);

  void StructureEventHandler(const wdDocumentObjectStructureEvent& e);
  void SelectionEventHandler(const wdSelectionManagerEvent& e);

  void TransferToCurrentSelection(const wdDocument* pDoc);

  void PhantomTypeManagerEventHandler(const wdPhantomRttiManagerEvent& e);
  void DocumentManagerEventHandler(const wdDocumentManager::Event& e);

  wdMap<const wdDocument*, Data> m_ActiveManipulator;
};
