#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/GroupBoxBase.moc.h>
#include <QBoxLayout>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionToolButton>

nsQtGroupBoxBase::nsQtGroupBoxBase(QWidget* pParent, bool bCollapsible)
  : QWidget(pParent)
{
  m_bCollapsible = bCollapsible;
}

void nsQtGroupBoxBase::SetTitle(nsStringView sTitle)
{
  m_sTitle = nsMakeQString(sTitle);
}

QString nsQtGroupBoxBase::GetTitle() const
{
  return m_sTitle;
}

void nsQtGroupBoxBase::SetBoldTitle(bool bBold)
{
  m_bBoldTitle = bBold;
  update();
}

bool nsQtGroupBoxBase::GetBoldTitle() const
{
  return m_bBoldTitle;
}

void nsQtGroupBoxBase::SetIcon(const QIcon& icon)
{
  m_Icon = icon;
}

QIcon nsQtGroupBoxBase::GetIcon() const
{
  return m_Icon;
}

void nsQtGroupBoxBase::SetFillColor(const QColor& color)
{
  m_FillColor = color;
  update();
}

QColor nsQtGroupBoxBase::GetFillColor() const
{
  return m_FillColor;
}

void nsQtGroupBoxBase::SetDraggable(bool bDraggable)
{
  m_bDraggable = bDraggable;
}

bool nsQtGroupBoxBase::IsDraggable() const
{
  return m_bDraggable;
}

void nsQtGroupBoxBase::DrawHeader(QPainter& p, const QRect& rect)
{
  QRect remainingRect = rect.adjusted(0, 0, 0, 0);

  if (m_bCollapsible)
  {
    QRect iconRect = remainingRect;
    iconRect.setWidth(iconRect.height() / 2);
    bool bCollapsed = GetCollapseState();
    QIcon collapseIcon = bCollapsed ? nsQtUiServices::GetCachedIconResource(":/GuiFoundation/Icons/GroupClosed.svg")
                                    : nsQtUiServices::GetCachedIconResource(":/GuiFoundation/Icons/GroupOpen.svg");
    collapseIcon.paint(&p, iconRect);
    remainingRect.adjust(iconRect.width() + Spacing, 0, 0, 0);
  }

  if (!m_Icon.isNull())
  {
    QRect iconRect = remainingRect;
    iconRect.setWidth(iconRect.height());
    m_Icon.paint(&p, iconRect);
    remainingRect.adjust(iconRect.width() + Spacing, 0, 0, 0);
  }

  QStyle* style = QWidget::style();
  int flags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextExpandTabs | Qt::TextForceLeftToRight;
  QFont fontOld = p.font();
  if (m_bBoldTitle)
  {
    QFont fontBold = fontOld;
    fontBold.setBold(true);
    p.setFont(fontBold);
  }
  style->drawItemText(&p, remainingRect, flags, palette(), isEnabled(), m_sTitle, foregroundRole());
  p.setFont(fontOld);
}

void nsQtGroupBoxBase::HeaderMousePress(QMouseEvent* me)
{
  if (me->button() == Qt::MouseButton::LeftButton)
  {
    m_bDragging = false;
    me->accept();
  }
}

void nsQtGroupBoxBase::HeaderMouseMove(QMouseEvent* me)
{
  if (m_bDraggable)
  {
    me->accept();

    QMimeData* mimeData = new QMimeData;
    Q_EMIT DragStarted(*mimeData);

    QDrag* drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setHotSpot(me->pos());

    {
      QPixmap tempPixmap(QSize(width(), GetHeader()->height()));
      QPainter painter;
      painter.begin(&tempPixmap);
      painter.fillRect(tempPixmap.rect(), palette().alternateBase());
      DrawHeader(painter, GetHeader()->contentsRect());
      painter.end();
      drag->setPixmap(tempPixmap);
    }

    drag->exec(Qt::MoveAction);
  }
}

void nsQtGroupBoxBase::HeaderMouseRelease(QMouseEvent* me)
{
  if (me->button() == Qt::MouseButton::LeftButton)
  {
    if (!m_bDragging && m_bCollapsible)
    {
      SetCollapseState(!GetCollapseState());
    }
    me->accept();
    m_bDragging = false;
  }
}
