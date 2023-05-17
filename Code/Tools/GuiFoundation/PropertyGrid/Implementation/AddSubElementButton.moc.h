#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>

class QHBoxLayout;
class QPushButton;
class QMenu;
class wdQtSearchableMenu;

class WD_GUIFOUNDATION_DLL wdQtAddSubElementButton : public wdQtPropertyWidget
{
  Q_OBJECT

public:
  wdQtAddSubElementButton();

  static bool s_bShowInDevelopmentFeatures;

protected:
  virtual void DoPrepareToDie() override {}

private Q_SLOTS:
  void onMenuAboutToShow();
  void on_Button_clicked();
  void OnMenuAction();

private:
  virtual void OnInit() override;
  void OnAction(const wdRTTI* pRtti);

  QMenu* CreateCategoryMenu(const char* szCategory, wdMap<wdString, QMenu*>& existingMenus);

  QHBoxLayout* m_pLayout;
  QPushButton* m_pButton;

  wdSet<const wdRTTI*> m_SupportedTypes;

  bool m_bNoMoreElementsAllowed = false;
  QMenu* m_pMenu = nullptr;
  wdQtSearchableMenu* m_pSearchableMenu = nullptr;
  wdUInt32 m_uiMaxElements = 0; // 0 means unlimited
  bool m_bPreventDuplicates = false;
  const wdConstrainPointerAttribute* m_pConstraint = nullptr;

  // used to remember the last search term entered into the searchable menu
  // this should probably be per 'distinguishable menu', but currently it is just global
  static wdString s_sLastMenuSearch;
};

