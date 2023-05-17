#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>
#include <QToolButton>

class WD_GUIFOUNDATION_DLL wdQtElementGroupButton : public QToolButton
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

  explicit wdQtElementGroupButton(QWidget* pParent, ElementAction action, wdQtPropertyWidget* pGroupWidget);
  ElementAction GetAction() const { return m_Action; }
  wdQtPropertyWidget* GetGroupWidget() const { return m_pGroupWidget; }

private:
  ElementAction m_Action;
  wdQtPropertyWidget* m_pGroupWidget;
};

