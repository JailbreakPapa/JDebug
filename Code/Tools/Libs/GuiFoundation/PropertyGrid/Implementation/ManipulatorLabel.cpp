#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/Implementation/ManipulatorLabel.moc.h>
#include <GuiFoundation/PropertyGrid/ManipulatorManager.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <QFont>
#include <qevent.h>

nsQtManipulatorLabel::nsQtManipulatorLabel(QWidget* pParent, Qt::WindowFlags f)
  : QLabel(pParent, f)
  , m_pItems(nullptr)
  , m_pManipulator(nullptr)
  , m_bActive(false)
{
  setCursor(Qt::WhatsThisCursor);
}

nsQtManipulatorLabel::nsQtManipulatorLabel(const QString& sText, QWidget* pParent, Qt::WindowFlags f)
  : QLabel(sText, pParent, f)
  , m_pItems(nullptr)
  , m_pManipulator(nullptr)
  , m_bActive(false)
  , m_bIsDefault(true)
{
}

const nsManipulatorAttribute* nsQtManipulatorLabel::GetManipulator() const
{
  return m_pManipulator;
}

void nsQtManipulatorLabel::SetManipulator(const nsManipulatorAttribute* pManipulator)
{
  m_pManipulator = pManipulator;

  if (m_pManipulator)
  {
    setCursor(Qt::PointingHandCursor);
    setForegroundRole(QPalette::ColorRole::Link);
  }
}

bool nsQtManipulatorLabel::GetManipulatorActive() const
{
  return m_bActive;
}

void nsQtManipulatorLabel::SetManipulatorActive(bool bActive)
{
  m_bActive = bActive;

  if (m_pManipulator)
  {
    setForegroundRole(m_bActive ? QPalette::ColorRole::LinkVisited : QPalette::ColorRole::Link);
  }
}

void nsQtManipulatorLabel::SetSelection(const nsHybridArray<nsPropertySelection, 8>& items)
{
  m_pItems = &items;
}


void nsQtManipulatorLabel::SetIsDefault(bool bIsDefault)
{
  if (m_bIsDefault != bIsDefault)
  {
    m_bIsDefault = bIsDefault;
    m_Font.setBold(!m_bIsDefault);
    setFont(m_Font);
  }
}


void nsQtManipulatorLabel::contextMenuEvent(QContextMenuEvent* ev)
{
  Q_EMIT customContextMenuRequested(ev->globalPos());
}

void nsQtManipulatorLabel::showEvent(QShowEvent* event)
{
  // Use of style sheets (ADS) breaks previously set font.
  setFont(m_Font);
  QLabel::showEvent(event);
}

void nsQtManipulatorLabel::mousePressEvent(QMouseEvent* ev)
{
  if (ev->button() != Qt::LeftButton)
    return;

  if (m_pManipulator == nullptr)
    return;

  const nsDocument* pDoc = (*m_pItems)[0].m_pObject->GetDocumentObjectManager()->GetDocument()->GetMainDocument();

  if (m_bActive)
    nsManipulatorManager::GetSingleton()->ClearActiveManipulator(pDoc);
  else
    nsManipulatorManager::GetSingleton()->SetActiveManipulator(pDoc, m_pManipulator, *m_pItems);
}

#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
void nsQtManipulatorLabel::enterEvent(QEnterEvent* ev)
#else
void nsQtManipulatorLabel::enterEvent(QEvent* ev)
#endif
{
  if (m_pManipulator)
  {
    m_Font.setUnderline(true);
    setFont(m_Font);
  }

  QLabel::enterEvent(ev);
}

void nsQtManipulatorLabel::leaveEvent(QEvent* ev)
{
  if (m_pManipulator)
  {
    m_Font.setUnderline(false);
    setFont(m_Font);
  }

  QLabel::leaveEvent(ev);
}
