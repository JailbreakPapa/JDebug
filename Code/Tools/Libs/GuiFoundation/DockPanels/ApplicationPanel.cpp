/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/ActionViews/QtProxy.moc.h>
#include <GuiFoundation/ContainerWindow/ContainerWindow.moc.h>
#include <GuiFoundation/DockPanels/ApplicationPanel.moc.h>

#include <ads/DockAreaWidget.h>
#include <ads/DockContainerWidget.h>
#include <ads/DockWidgetTab.h>

NS_BEGIN_STATIC_REFLECTED_TYPE(nsQtApplicationPanel, nsNoBase, 1, nsRTTINoAllocator)
NS_END_STATIC_REFLECTED_TYPE;

nsDynamicArray<nsQtApplicationPanel*> nsQtApplicationPanel::s_AllApplicationPanels;

nsQtApplicationPanel::nsQtApplicationPanel(const char* szPanelName)
  : ads::CDockWidget(szPanelName, nsQtContainerWindow::GetContainerWindow())
{
  nsStringBuilder sPanel("AppPanel_", szPanelName);

  setObjectName(QString::fromUtf8(sPanel.GetData()));
  setWindowTitle(QString::fromUtf8(nsTranslate(szPanelName)));

  s_AllApplicationPanels.PushBack(this);

  m_pContainerWindow = nullptr;

  nsQtContainerWindow::GetContainerWindow()->AddApplicationPanel(this);

  nsToolsProject::s_Events.AddEventHandler(nsMakeDelegate(&nsQtApplicationPanel::ToolsProjectEventHandler, this));
}

nsQtApplicationPanel::~nsQtApplicationPanel()
{
  nsToolsProject::s_Events.RemoveEventHandler(nsMakeDelegate(&nsQtApplicationPanel::ToolsProjectEventHandler, this));

  s_AllApplicationPanels.RemoveAndSwap(this);
}

void nsQtApplicationPanel::EnsureVisible()
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


void nsQtApplicationPanel::ToolsProjectEventHandler(const nsToolsProjectEvent& e)
{
  switch (e.m_Type)
  {
    case nsToolsProjectEvent::Type::ProjectClosing:
      setEnabled(false);
      break;
    case nsToolsProjectEvent::Type::ProjectOpened:
      setEnabled(true);
      break;

    default:
      break;
  }
}

bool nsQtApplicationPanel::event(QEvent* event)
{
  if (event->type() == QEvent::ShortcutOverride)
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if (nsQtProxy::TriggerDocumentAction(nullptr, keyEvent))
      return true;
  }
  return ads::CDockWidget::event(event);
}
