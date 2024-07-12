/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/ActionViews/QtProxy.moc.h>
#include <GuiFoundation/DockPanels/DocumentPanel.moc.h>
#include <QCloseEvent>

nsDynamicArray<nsQtDocumentPanel*> nsQtDocumentPanel::s_AllDocumentPanels;

nsQtDocumentPanel::nsQtDocumentPanel(QWidget* pParent, nsDocument* pDocument)
  : QDockWidget(pParent)
{
  m_pDocument = pDocument;
  s_AllDocumentPanels.PushBack(this);

  setBackgroundRole(QPalette::ColorRole::Highlight);

  setFeatures(DockWidgetFeature::DockWidgetFloatable | DockWidgetFeature::DockWidgetMovable);
}

nsQtDocumentPanel::~nsQtDocumentPanel()
{
  s_AllDocumentPanels.RemoveAndSwap(this);
}

void nsQtDocumentPanel::closeEvent(QCloseEvent* e)
{
  e->ignore();
}

bool nsQtDocumentPanel::event(QEvent* pEvent)
{
  if (pEvent->type() == QEvent::ShortcutOverride)
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(pEvent);
    if (nsQtProxy::TriggerDocumentAction(m_pDocument, keyEvent))
      return true;
  }
  return QDockWidget::event(pEvent);
}
