#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/CollapsibleGroupBox.moc.h>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollArea>

wdQtCollapsibleGroupBox::wdQtCollapsibleGroupBox(QWidget* pParent)
  : wdQtGroupBoxBase(pParent, true)
  , m_bCollapsed(false)
{
  setupUi(this);

  Header->installEventFilter(this);
}

void wdQtCollapsibleGroupBox::SetTitle(const char* szTitle)
{
  wdQtGroupBoxBase::SetTitle(szTitle);
  update();
}

void wdQtCollapsibleGroupBox::SetIcon(const QIcon& icon)
{
  wdQtGroupBoxBase::SetIcon(icon);
  update();
}

void wdQtCollapsibleGroupBox::SetFillColor(const QColor& color)
{
  wdQtGroupBoxBase::SetFillColor(color);
  update();
}

void wdQtCollapsibleGroupBox::SetCollapseState(bool bCollapsed)
{
  if (bCollapsed == m_bCollapsed)
    return;

  wdQtScopedUpdatesDisabled sud(this);

  m_bCollapsed = bCollapsed;
  Content->setVisible(!bCollapsed);

  // Force re-layout of parent hierarchy to prevent flicker.
  QWidget* pCur = this;
  while (pCur != nullptr && qobject_cast<QScrollArea*>(pCur) == nullptr)
  {
    pCur->updateGeometry();
    pCur = pCur->parentWidget();
  }

  Q_EMIT CollapseStateChanged(bCollapsed);
}

bool wdQtCollapsibleGroupBox::GetCollapseState() const
{
  return m_bCollapsed;
}

QWidget* wdQtCollapsibleGroupBox::GetContent()
{
  return Content;
}

QWidget* wdQtCollapsibleGroupBox::GetHeader()
{
  return Header;
}

bool wdQtCollapsibleGroupBox::eventFilter(QObject* object, QEvent* event)
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

void wdQtCollapsibleGroupBox::paintEvent(QPaintEvent* event)
{
  const QPalette& pal = palette();
  QWidget::paintEvent(event);

  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);
  QRect wr = contentsRect();
  QRect hr = Header->contentsRect();
  hr.moveTopLeft(Header->pos());

  QRect cr = wr;
  cr.setTop(hr.height());
  cr.adjust(Rounding / 2, 0, 0, -Rounding / 2);

  if (m_FillColor.isValid())
  {
    QRectF wrAdjusted = wr;
    wrAdjusted.adjust(0.5, 0.5, Rounding, -0.5);
    QPainterPath oPath;
    oPath.addRoundedRect(wrAdjusted, Rounding, Rounding);
    p.fillPath(oPath, pal.alternateBase());

    QRectF crAdjusted = cr;
    crAdjusted.adjust(0.5, 0.5, Rounding, -0.5);
    QPainterPath path;
    path.addRoundedRect(crAdjusted, Rounding, Rounding);
    p.fillPath(path, m_FillColor);
  }

  if (!Header->isHidden())
  {
    DrawHeader(p, hr);
  }
}
