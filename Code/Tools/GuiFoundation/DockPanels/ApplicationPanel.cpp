#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>
#include <GuiFoundation/ContainerWindow/ContainerWindow.moc.h>
#include <GuiFoundation/DockPanels/ApplicationPanel.moc.h>

#include <ads/DockAreaWidget.h>
#include <ads/DockContainerWidget.h>
#include <ads/DockWidgetTab.h>

WD_BEGIN_STATIC_REFLECTED_TYPE(wdQtApplicationPanel, wdNoBase, 1, wdRTTINoAllocator)
WD_END_STATIC_REFLECTED_TYPE;

wdDynamicArray<wdQtApplicationPanel*> wdQtApplicationPanel::s_AllApplicationPanels;

wdQtApplicationPanel::wdQtApplicationPanel(const char* szPanelName)
  : ads::CDockWidget(szPanelName, wdQtContainerWindow::GetContainerWindow())
{
  wdStringBuilder sPanel("AppPanel_", szPanelName);

  setObjectName(QString::fromUtf8(sPanel.GetData()));
  setWindowTitle(QString::fromUtf8(wdTranslate(szPanelName)));

  s_AllApplicationPanels.PushBack(this);

  m_pContainerWindow = nullptr;

  wdQtContainerWindow::GetContainerWindow()->AddApplicationPanel(this);

  wdToolsProject::s_Events.AddEventHandler(wdMakeDelegate(&wdQtApplicationPanel::ToolsProjectEventHandler, this));
}

wdQtApplicationPanel::~wdQtApplicationPanel()
{
  wdToolsProject::s_Events.RemoveEventHandler(wdMakeDelegate(&wdQtApplicationPanel::ToolsProjectEventHandler, this));

  s_AllApplicationPanels.RemoveAndSwap(this);
}

void wdQtApplicationPanel::EnsureVisible()
{
  m_pContainerWindow->EnsureVisible(this).IgnoreResult();

  QWidget* pThis = this;

  if (dockAreaWidget())
  {
    dockAreaWidget()->setCurrentDockWidget(this);
  }

  while (pThis)
  {
    pThis->raise();
    pThis = qobject_cast<QWidget*>(pThis->parent());
  }
}


void wdQtApplicationPanel::ToolsProjectEventHandler(const wdToolsProjectEvent& e)
{
  switch (e.m_Type)
  {
    case wdToolsProjectEvent::Type::ProjectClosing:
      setEnabled(false);
      break;
    case wdToolsProjectEvent::Type::ProjectOpened:
      setEnabled(true);
      break;

    default:
      break;
  }
}

bool wdQtApplicationPanel::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride)
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (wdQtProxy::TriggerDocumentAction(nullptr, keyEvent))
      return true;
  }
  return ads::CDockWidget::event(event);
}
