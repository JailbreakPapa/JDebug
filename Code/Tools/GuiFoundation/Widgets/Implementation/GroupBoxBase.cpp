#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/UIServices/UIServices.moc.h>
#include <GuiFoundation/Widgets/GroupBoxBase.moc.h>
#include <QBoxLayout>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionToolButton>

wdQtGroupBoxBase::wdQtGroupBoxBase(QWidget* pParent, bool bCollapsible)
  : QWidget(pParent)
{
  m_bCollapsible = bCollapsible;
}

void wdQtGroupBoxBase::SetTitle(const char* szTitle)
{
  m_sTitle = szTitle;
}

QString wdQtGroupBoxBase::GetTitle() const
{
  return m_sTitle;
}

void wdQtGroupBoxBase::SetBoldTitle(bool bBold)
{
  m_bBoldTitle = bBold;
  update();
}

bool wdQtGroupBoxBase::GetBoldTitle() const
{
  return m_bBoldTitle;
}

void wdQtGroupBoxBase::SetIcon(const QIcon& icon)
{
  m_Icon = icon;
}

QIcon wdQtGroupBoxBase::GetIcon() const
{
  return m_Icon;
}

void wdQtGroupBoxBase::SetFillColor(const QColor& color)
{
  m_FillColor = color;
  update();
}

QColor wdQtGroupBoxBase::GetFillColor() const
{
  return m_FillColor;
}

void wdQtGroupBoxBase::SetDraggable(bool bDraggable)
{
  m_bDraggable = bDraggable;
}

bool wdQtGroupBoxBase::IsDraggable() const
{
  return m_bDraggable;
}

void wdQtGroupBoxBase::DrawHeader(QPainter& p, const QRect& rect)
{
  QRect remainingRect = rect.adjusted(0, 0, 0, 0);

  if (m_bCollapsible)
  {
    QRect iconRect = remainingRect;
    iconRect.setWidth(iconRect.height() / 2);
    bool bCollapsed = GetCollapseState();
    QIcon collapseIcon = bCollapsed ? wdQtUiServices::GetCachedIconResource(":/GuiFoundation/Icons/groupClosed.png")
                                    : wdQtUiServices::GetCachedIconResource(":/GuiFoundation/Icons/groupOpen.png");
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

void wdQtGroupBoxBase::HeaderMousePress(QMouseEvent* me)
{
  if (me->button() == Qt::MouseButton::LeftButton)
  {
    m_bDragging = false;
    me->accept();
  }
}

void wdQtGroupBoxBase::HeaderMouseMove(QMouseEvent* me)
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

void wdQtGroupBoxBase::HeaderMouseRelease(QMouseEvent* me)
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
