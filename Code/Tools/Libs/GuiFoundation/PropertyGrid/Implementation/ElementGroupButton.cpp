#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/Implementation/ElementGroupButton.moc.h>

nsQtElementGroupButton::nsQtElementGroupButton(QWidget* pParent, nsQtElementGroupButton::ElementAction action, nsQtPropertyWidget* pGroupWidget)
  : QToolButton(pParent)
{
  m_Action = action;
  m_pGroupWidget = pGroupWidget;

  setAutoRaise(true);

  setIconSize(QSize(16, 16));

  switch (action)
  {
    case nsQtElementGroupButton::ElementAction::MoveElementUp:
      setIcon(QIcon(QStringLiteral(":/GuiFoundation/Icons/MoveUp.svg")));
      break;
    case nsQtElementGroupButton::ElementAction::MoveElementDown:
      setIcon(QIcon(QStringLiteral(":/GuiFoundation/Icons/MoveDown.svg")));
      break;
    case nsQtElementGroupButton::ElementAction::DeleteElement:
      setIcon(QIcon(QStringLiteral(":/GuiFoundation/Icons/Delete.svg")));
      break;
    case nsQtElementGroupButton::ElementAction::Help:
      setIcon(QIcon(QStringLiteral(":/GuiFoundation/Icons/Log.svg")));
      break;
  }
}
