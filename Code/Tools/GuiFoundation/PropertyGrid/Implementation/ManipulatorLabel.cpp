#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/Implementation/ManipulatorLabel.moc.h>
#include <GuiFoundation/PropertyGrid/ManipulatorManager.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QFont>
#include <qevent.h>

wdQtManipulatorLabel::wdQtManipulatorLabel(QWidget* pParent, Qt::WindowFlags f)
  : QLabel(pParent, f)
  , m_pItems(nullptr)
  , m_pManipulator(nullptr)
  , m_bActive(false)
{
  setCursor(Qt::WhatsThisCursor);
}

wdQtManipulatorLabel::wdQtManipulatorLabel(const QString& sText, QWidget* pParent, Qt::WindowFlags f)
  : QLabel(sText, pParent, f)
  , m_pItems(nullptr)
  , m_pManipulator(nullptr)
  , m_bActive(false)
  , m_bIsDefault(true)
{
}

const wdManipulatorAttribute* wdQtManipulatorLabel::GetManipulator() const
{
  return m_pManipulator;
}

void wdQtManipulatorLabel::SetManipulator(const wdManipulatorAttribute* pManipulator)
{
  m_pManipulator = pManipulator;

  if (m_pManipulator)
  {
    setCursor(Qt::PointingHandCursor);
    setForegroundRole(QPalette::ColorRole::Link);
  }
}

bool wdQtManipulatorLabel::GetManipulatorActive() const
{
  return m_bActive;
}

void wdQtManipulatorLabel::SetManipulatorActive(bool bActive)
{
  m_bActive = bActive;

  if (m_pManipulator)
  {
    setForegroundRole(m_bActive ? QPalette::ColorRole::LinkVisited : QPalette::ColorRole::Link);
  }
}

void wdQtManipulatorLabel::SetSelection(const wdHybridArray<wdPropertySelection, 8>& items)
{
  m_pItems = &items;
}


void wdQtManipulatorLabel::SetIsDefault(bool bIsDefault)
{
  if (m_bIsDefault != bIsDefault)
  {
    m_bIsDefault = bIsDefault;
    m_Font.setBold(!m_bIsDefault);
    setFont(m_Font);
  }
}


void wdQtManipulatorLabel::contextMenuEvent(QContextMenuEvent* ev)
{
  Q_EMIT customContextMenuRequested(ev->globalPos());
}

void wdQtManipulatorLabel::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set font.
  setFont(m_Font);
  QLabel::showEvent(event);
}

void wdQtManipulatorLabel::mousePressEvent(QMouseEvent* ev)
{
  if (ev->button() != Qt::LeftButton)
    return;

  if (m_pManipulator == nullptr)
    return;

  const wdDocument* pDoc = (*m_pItems)[0].m_pObject->GetDocumentObjectManager()->GetDocument()->GetMainDocument();

  if (m_bActive)
    wdManipulatorManager::GetSingleton()->ClearActiveManipulator(pDoc);
  else
    wdManipulatorManager::GetSingleton()->SetActiveManipulator(pDoc, m_pManipulator, *m_pItems);
}

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
void wdQtManipulatorLabel::enterEvent(QEnterEvent* ev)
#else
void wdQtManipulatorLabel::enterEvent(QEvent* ev)
#endif
{
  if (m_pManipulator)
  {
    m_Font.setUnderline(true);
    setFont(m_Font);
  }

  QLabel::enterEvent(ev);
}

void wdQtManipulatorLabel::leaveEvent(QEvent* ev)
{
  if (m_pManipulator)
  {
    m_Font.setUnderline(false);
    setFont(m_Font);
  }

  QLabel::leaveEvent(ev);
}
