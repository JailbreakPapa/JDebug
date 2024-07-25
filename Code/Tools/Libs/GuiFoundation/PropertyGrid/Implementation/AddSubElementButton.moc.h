#pragma once

#include <GuiFoundation/GuiFoundationDLL.h>
#include <GuiFoundation/PropertyGrid/PropertyBaseWidget.moc.h>

class QHBoxLayout;
class QPushButton;
class QMenu;
class nsQtSearchableMenu;

class NS_GUIFOUNDATION_DLL nsQtAddSubElementButton : public nsQtPropertyWidget
{
  Q_OBJECT

public:
  nsQtAddSubElementButton();

  static bool s_bShowInDevelopmentFeatures;

protected:
  virtual void DoPrepareToDie() override {}

private Q_SLOTS:
  void onMenuAboutToShow();
  void on_Button_clicked();
  void OnMenuAction();

private:
  virtual void OnInit() override;
  void OnAction(const nsRTTI* pRtti);

  QMenu* CreateCategoryMenu(const char* szCategory, nsMap<nsString, QMenu*>& existingMenus);

  QHBoxLayout* m_pLayout;
  QPushButton* m_pButton;

  nsSet<const nsRTTI*> m_SupportedTypes;

  bool m_bNoMoreElementsAllowed = false;
  QMenu* m_pMenu = nullptr;
  nsQtSearchableMenu* m_pSearchableMenu = nullptr;
  nsUInt32 m_uiMaxElements = 0; // 0 means unlimited
  bool m_bPreventDuplicates = false;

  // used to remember the last search term entered into the searchable menu
  // this should probably be per 'distinguishable menu', but currently it is just global
  static nsString s_sLastMenuSearch;
};
