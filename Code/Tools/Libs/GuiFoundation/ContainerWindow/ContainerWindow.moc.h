#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QMainWindow>
#include <QSet>
#include <ToolsFoundation/Project/ToolsProject.h>

class nsDocumentManager;
class nsDocument;
class nsQtApplicationPanel;
struct nsDocumentTypeDescriptor;
class QLabel;

namespace ads
{
  class CDockManager;
  class CFloatingDockContainer;
  class CDockWidget;
} // namespace ads

/// \brief Container window that hosts documents and applications panels.
class NS_GUIFOUNDATION_DLL nsQtContainerWindow : public QMainWindow
{
  Q_OBJECT

public:
  /// \brief Constructor.
  nsQtContainerWindow();
  ~nsQtContainerWindow();

  static nsQtContainerWindow* GetContainerWindow() { return s_pContainerWindow; }

  void AddDocumentWindow(nsQtDocumentWindow* pDocWindow);
  void DocumentWindowRenamed(nsQtDocumentWindow* pDocWindow);
  void AddApplicationPanel(nsQtApplicationPanel* pPanel);

  ads::CDockManager* GetDockManager() { return m_pDockManager; }

  static nsResult EnsureVisibleAnyContainer(nsDocument* pDocument);

  void GetDocumentWindows(nsHybridArray<nsQtDocumentWindow*, 16>& ref_windows);

  void SaveWindowLayout();
  void SaveDocumentLayouts();
  void RestoreWindowLayout();

  void ScheduleRestoreWindowLayout();

protected:
  virtual bool eventFilter(QObject* obj, QEvent* e) override;

private:
  friend class nsQtDocumentWindow;
  friend class nsQtApplicationPanel;

  nsResult EnsureVisible(nsQtDocumentWindow* pDocWindow);
  nsResult EnsureVisible(nsDocument* pDocument);
  nsResult EnsureVisible(nsQtApplicationPanel* pPanel);

  bool m_bWindowLayoutRestored;
  nsInt32 m_iWindowLayoutRestoreScheduled;

private Q_SLOTS:
  void SlotDocumentTabCloseRequested();
  void SlotRestoreLayout();
  void SlotTabsContextMenuRequested(const QPoint& pos);
  void SlotUpdateWindowDecoration(void* pDocWindow);
  void SlotFloatingWidgetOpened(ads::CFloatingDockContainer* FloatingWidget);
  void SlotDockWidgetFloatingChanged(bool bFloating);

private:
  void UpdateWindowTitle();

  void RemoveDocumentWindow(nsQtDocumentWindow* pDocWindow);
  void RemoveApplicationPanel(nsQtApplicationPanel* pPanel);

  void UpdateWindowDecoration(nsQtDocumentWindow* pDocWindow);

  void DocumentWindowEventHandler(const nsQtDocumentWindowEvent& e);
  void ProjectEventHandler(const nsToolsProjectEvent& e);
  void UIServicesEventHandler(const nsQtUiServices::Event& e);

  virtual void closeEvent(QCloseEvent* e) override;

private:
  ads::CDockManager* m_pDockManager = nullptr;
  QLabel* m_pStatusBarLabel;
  nsDynamicArray<nsQtDocumentWindow*> m_DocumentWindows;
  nsDynamicArray<ads::CDockWidget*> m_DocumentDocks;

  nsDynamicArray<nsQtApplicationPanel*> m_ApplicationPanels;
  QSet<QString> m_DockNames;

  static nsQtContainerWindow* s_pContainerWindow;
  static bool s_bForceClose;
};
