#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/PropertyGrid/Implementation/ElementGroupButton.moc.h>

wdQtElementGroupButton::wdQtElementGroupButton(QWidget* pParent, wdQtElementGroupButton::ElementAction action, wdQtPropertyWidget* pGroupWidget)
  : QToolButton(pParent)
{
  m_Action = action;
  m_pGroupWidget = pGroupWidget;

  setAutoRaise(true);

  setIconSize(QSize(16, 16));

  switch (action)
  {
    case wdQtElementGroupButton::ElementAction::MoveElementUp:
      setIcon(QIcon(QStringLiteral(":/GuiFoundation/Icons/MoveUp16.png")));
      break;
    case wdQtElementGroupButton::ElementAction::MoveElementDown:
      setIcon(QIcon(QStringLiteral(":/GuiFoundation/Icons/MoveDown16.png")));
      break;
    case wdQtElementGroupButton::ElementAction::DeleteElement:
      setIcon(QIcon(QStringLiteral(":/GuiFoundation/Icons/Delete16.png")));
      break;
    case wdQtElementGroupButton::ElementAction::Help:
      setIcon(QIcon(QStringLiteral(":/GuiFoundation/Icons/Log.png")));
      break;
  }
}
