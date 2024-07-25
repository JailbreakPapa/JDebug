#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/Strings/TranslationLookup.h>

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsPixmapItem>
#include <QPainter>

#include "CommentNode.h"

nsQtCommentNode::nsQtCommentNode()
{
  auto palette = QApplication::palette();

  setFlag(QGraphicsItem::ItemIsMovable);
  setFlag(QGraphicsItem::ItemIsSelectable);
  setFlag(QGraphicsItem::ItemSendsGeometryChanges);
  setBrush(palette.window());

  {
    QFont font = QApplication::font();
    font.setBold(true);

    m_pTitleLabel = new QGraphicsTextItem(this);
    m_pTitleLabel->setFont(font);
  }

  {
    m_pIcon = new QGraphicsPixmapItem(this);
  }
}

nsQtCommentNode::~nsQtCommentNode()
{
  EnableDropShadow(false);
}

void nsQtCommentNode::InitComment(const nsDocumentNodeManager* pManager, const nsDocumentObject* pObject)
{
  m_pManager = pManager;
  m_pObject = pObject;
  CreateComment();
  UpdateCommentRect();

  if (const nsColorAttribute* pColorAttr = pObject->GetType()->GetAttributeByType<nsColorAttribute>())
  {
    m_HeaderColor = nsToQtColor(pColorAttr->GetColor());
  }

  m_DirtyFlags.Add(nsNodeFlags::UpdateTitle);
}

void nsQtCommentNode::CreateComment()
{
  /// Rundown on how comments will work logically:
  /*
   * 1: User Holds down left mouse button and drags to a point.
   * 2: User releases left mouse button, and nsQtNodeScene Sends a signal to us, with the contained elements.
   * 3: nsQtNodeScene shows "Create Comment Group" Action, and sends us: the contained elements, and the position & size of the comment (should be based upon the rect that the user has created upon selecting said nodes).
   */
}

void nsQtCommentNode::UpdateCommentRect()
{
}

void nsQtCommentNode::SetCommentMainText(const nsString& text)
{
}

void nsQtCommentNode::SetCommentSubText(const nsString& text)
{
}

void nsQtCommentNode::EnableDropShadow(bool bEnable)
{
  /// Same Behavior as in Node.cpp
  if (bEnable && m_pShadow == nullptr)
  {
    auto palette = QApplication::palette();

    m_pShadow = new QGraphicsDropShadowEffect();
    m_pShadow->setOffset(3, 3);
    m_pShadow->setColor(palette.color(QPalette::Shadow));
    m_pShadow->setBlurRadius(10);
    setGraphicsEffect(m_pShadow);
  }

  if (!bEnable && m_pShadow != nullptr)
  {
    delete m_pShadow;
    m_pShadow = nullptr;
  }
}

void nsQtCommentNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
}

nsBitflags<nsNodeFlags> nsQtCommentNode::GetFlags() const
{
  return m_DirtyFlags;
}

QVariant nsQtCommentNode::itemChange(GraphicsItemChange change, const QVariant& value)
{
  if (change == QGraphicsItem::ItemPositionChange)
  {
    m_DirtyFlags.Add(nsNodeFlags::UpdateTitle);
  }

  return QGraphicsItem::itemChange(change, value);
}
