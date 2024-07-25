#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <QToolButton>

class NS_GUIFOUNDATION_DLL nsQtElementGroupButton : public QToolButton
{
  Q_OBJECT
public:
  enum class ElementAction
  {
    MoveElementUp,
    MoveElementDown,
    DeleteElement,
    Help,
  };

  explicit nsQtElementGroupButton(QWidget* pParent, ElementAction action, nsQtPropertyWidget* pGroupWidget);
  ElementAction GetAction() const { return m_Action; }
  nsQtPropertyWidget* GetGroupWidget() const { return m_pGroupWidget; }

private:
  ElementAction m_Action;
  nsQtPropertyWidget* m_pGroupWidget;
};
