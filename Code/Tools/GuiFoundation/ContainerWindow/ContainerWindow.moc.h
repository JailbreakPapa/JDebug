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

class wdDocumentManager;
class wdDocument;
class wdQtApplicationPanel;
struct wdDocumentTypeDescriptor;
class QLabel;

namespace ads
{
  class CDockManager;
  class CFloatingDockContainer;
  class CDockWidget;
} // namespace ads

/// \brief Container window that hosts documents and applications panels.
class WD_GUIFOUNDATION_DLL wdQtContainerWindow : public QMainWindow
{
  Q_OBJECT

public:
  /// \brief Constructor.
  wdQtContainerWindow();
  ~wdQtContainerWindow();

  static wdQtContainerWindow* GetContainerWindow() { return s_pContainerWindow; }

  void AddDocumentWindow(wdQtDocumentWindow* pDocWindow);
  void AddApplicationPanel(wdQtApplicationPanel* pPanel);

  ads::CDockManager* GetDockManager() { return m_pDockManager; }

  static wdResult EnsureVisibleAnyContainer(wdDocument* pDocument);

  void GetDocumentWindows(wdHybridArray<wdQtDocumentWindow*, 16>& ref_windows);

  void SaveWindowLayout();
  void SaveDocumentLayouts();
  void RestoreWindowLayout();

  void ScheduleRestoreWindowLayout();

protected:
  virtual bool eventFilter(QObject* obj, QEvent* e) override;

private:
  friend class wdQtDocumentWindow;
  friend class wdQtApplicationPanel;

  wdResult EnsureVisible(wdQtDocumentWindow* pDocWindow);
  wdResult EnsureVisible(wdDocument* pDocument);
  wdResult EnsureVisible(wdQtApplicationPanel* pPanel);

  bool m_bWindowLayoutRestored;
  wdInt32 m_iWindowLayoutRestoreScheduled;

private Q_SLOTS:
  void SlotDocumentTabCloseRequested();
  void SlotRestoreLayout();
  void SlotTabsContextMenuRequested(const QPoint& pos);
  void SlotUpdateWindowDecoration(void* pDocWindow);
  void SlotFloatingWidgetOpened(ads::CFloatingDockContainer* FloatingWidget);
  void SlotDockWidgetFloatingChanged(bool bFloating);

private:
  void UpdateWindowTitle();

  void RemoveDocumentWindow(wdQtDocumentWindow* pDocWindow);
  void RemoveApplicationPanel(wdQtApplicationPanel* pPanel);

  void UpdateWindowDecoration(wdQtDocumentWindow* pDocWindow);

  void DocumentWindowEventHandler(const wdQtDocumentWindowEvent& e);
  void ProjectEventHandler(const wdToolsProjectEvent& e);
  void UIServicesEventHandler(const wdQtUiServices::Event& e);

  virtual void closeEvent(QCloseEvent* e) override;

private:
  ads::CDockManager* m_pDockManager = nullptr;
  QLabel* m_pStatusBarLabel;
  wdDynamicArray<wdQtDocumentWindow*> m_DocumentWindows;
  wdDynamicArray<ads::CDockWidget*> m_DocumentDocks;

  wdDynamicArray<wdQtApplicationPanel*> m_ApplicationPanels;
  QSet<QString> m_DockNames;

  static wdQtContainerWindow* s_pContainerWindow;
  static bool s_bForceClose;
};

