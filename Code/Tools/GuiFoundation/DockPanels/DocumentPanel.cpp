#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/ActionViews/QtProxy.moc.h>
#include <GuiFoundation/DockPanels/DocumentPanel.moc.h>
#include <QCloseEvent>

wdDynamicArray<wdQtDocumentPanel*> wdQtDocumentPanel::s_AllDocumentPanels;

wdQtDocumentPanel::wdQtDocumentPanel(QWidget* pParent, wdDocument* pDocument)
  : QDockWidget(pParent)
{
  m_pDocument = pDocument;
  s_AllDocumentPanels.PushBack(this);

  setBackgroundRole(QPalette::ColorRole::Highlight);

  setFeatures(DockWidgetFeature::DockWidgetFloatable | DockWidgetFeature::DockWidgetMovable);
}

wdQtDocumentPanel::~wdQtDocumentPanel()
{
  s_AllDocumentPanels.RemoveAndSwap(this);
}

void wdQtDocumentPanel::closeEvent(QCloseEvent* e)
{
  e->ignore();
}

bool wdQtDocumentPanel::event(QEvent* pEvent)
{
  if (pEvent->type() == QEvent::ShortcutOverride)
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(pEvent);
    if (wdQtProxy::TriggerDocumentAction(m_pDocument, keyEvent))
      return true;
  }
  return QDockWidget::event(pEvent);
}
