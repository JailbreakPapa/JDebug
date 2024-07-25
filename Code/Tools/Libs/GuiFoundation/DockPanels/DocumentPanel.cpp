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
  if (pEvent->type() == QEvent::ShortcutOverride || pEvent->type() == QEvent::KeyPress)
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(pEvent);
    if (nsQtProxy::TriggerDocumentAction(m_pDocument, keyEvent, pEvent->type() == QEvent::ShortcutOverride))
      return true;
  }
  return QDockWidget::event(pEvent);
}
