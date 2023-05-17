#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Widgets/InlinedGroupBox.moc.h>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionToolButton>

wdQtInlinedGroupBox::wdQtInlinedGroupBox(QWidget* pParent)
  : wdQtGroupBoxBase(pParent, false)
{
  QHBoxLayout* pRootLayout = new QHBoxLayout(this);
  pRootLayout->setContentsMargins(0, 1, 0, 1);
  pRootLayout->setSpacing(0);

  m_pContent = new QWidget(this);
  QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Preferred);
  sp.setHorizontalStretch(2);
  sp.setVerticalStretch(0);
  m_pContent->setSizePolicy(sp);

  m_pHeader = new QFrame(this);
  QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  m_pHeader->setSizePolicy(sizePolicy1);

  QHBoxLayout* pHeaderLayout = new QHBoxLayout(m_pHeader);
  pHeaderLayout->setSpacing(0);
  pHeaderLayout->setContentsMargins(0, 0, 0, 0);

  pRootLayout->addSpacerItem(new QSpacerItem(0, 0));
  pRootLayout->setStretch(0, 1);
  pRootLayout->addWidget(m_pContent);
  pRootLayout->addWidget(m_pHeader);

  installEventFilter(this);
}

void wdQtInlinedGroupBox::SetTitle(const char* szTitle)
{
  wdQtGroupBoxBase::SetTitle(szTitle);
  update();
}

void wdQtInlinedGroupBox::SetIcon(const QIcon& icon)
{
  wdQtGroupBoxBase::SetIcon(icon);
  update();
}

void wdQtInlinedGroupBox::SetFillColor(const QColor& color)
{
  wdQtGroupBoxBase::SetFillColor(color);
  if (color.isValid())
    layout()->setContentsMargins(0, 1, 0, 1);
  else
    layout()->setContentsMargins(0, 0, 0, 0);
  update();
}

void wdQtInlinedGroupBox::SetCollapseState(bool bCollapsed) {}

bool wdQtInlinedGroupBox::GetCollapseState() const
{
  return false;
}

QWidget* wdQtInlinedGroupBox::GetContent()
{
  return m_pContent;
}

QWidget* wdQtInlinedGroupBox::GetHeader()
{
  return m_pHeader;
}

void wdQtInlinedGroupBox::paintEvent(QPaintEvent* event)
{
  const QPalette& pal = palette();
  QWidget::paintEvent(event);

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  QRect wr = contentsRect();

  if (m_FillColor.isValid())
  {
    QRectF wrAdjusted = wr;
    wrAdjusted.adjust(0.5, 0.5, Rounding, -0.5);
    QPainterPath oPath;
    oPath.addRoundedRect(wrAdjusted, Rounding, Rounding);
    p.fillPath(oPath, pal.alternateBase());
  }

  DrawHeader(p, wr.adjusted(Rounding, 0, 0, 0));
}

bool wdQtInlinedGroupBox::eventFilter(QObject* object, QEvent* event)
{
  switch (event->type())
  {
    case QEvent::Type::MouseButtonPress:
      HeaderMousePress(static_cast<QMouseEvent*>(event));
      return true;
    case QEvent::Type::MouseMove:
      HeaderMouseMove(static_cast<QMouseEvent*>(event));
      return true;
    case QEvent::Type::MouseButtonRelease:
      HeaderMouseRelease(static_cast<QMouseEvent*>(event));
      return true;
    default:
      break;
  }
  return false;
}
