#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>

#include <GuiFoundation/PropertyGrid/Implementation/PropertyWidget.moc.h>

class QHBoxLayout;
class QPushButton;
class QMenu;
class QCheckBox;

class NS_GUIFOUNDATION_DLL nsQtPropertyEditorTagSetWidget : public nsQtPropertyWidget
{
  Q_OBJECT

public:
  nsQtPropertyEditorTagSetWidget();
  virtual ~nsQtPropertyEditorTagSetWidget();

  virtual void SetSelection(const nsHybridArray<nsPropertySelection, 8>& items) override;
  virtual bool HasLabel() const override { return true; }

protected:
  virtual void DoPrepareToDie() override {}

private Q_SLOTS:
  void on_Menu_aboutToShow();
  void onCheckBoxClicked(bool bChecked);

private:
  virtual void OnInit() override;
  void InternalUpdateValue();

private:
  nsDynamicArray<QCheckBox*> m_Tags;
  QHBoxLayout* m_pLayout;
  QPushButton* m_pWidget;
  QMenu* m_pMenu;
};
