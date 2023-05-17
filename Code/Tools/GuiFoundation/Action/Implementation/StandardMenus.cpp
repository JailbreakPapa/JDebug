#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/StandardMenus.h>
#include <GuiFoundation/DockPanels/ApplicationPanel.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>

wdActionDescriptorHandle wdStandardMenus::s_hMenuFile;
wdActionDescriptorHandle wdStandardMenus::s_hMenuEdit;
wdActionDescriptorHandle wdStandardMenus::s_hMenuPanels;
wdActionDescriptorHandle wdStandardMenus::s_hMenuProject;
wdActionDescriptorHandle wdStandardMenus::s_hMenuScene;
wdActionDescriptorHandle wdStandardMenus::s_hMenuView;
wdActionDescriptorHandle wdStandardMenus::s_hMenuHelp;
wdActionDescriptorHandle wdStandardMenus::s_hCheckForUpdates;
wdActionDescriptorHandle wdStandardMenus::s_hReportProblem;

void wdStandardMenus::RegisterActions()
{
  s_hMenuFile = WD_REGISTER_MENU("Menu.File");
  s_hMenuEdit = WD_REGISTER_MENU("Menu.Edit");
  s_hMenuPanels = WD_REGISTER_DYNAMIC_MENU("Menu.Panels", wdApplicationPanelsMenuAction, "");
  s_hMenuProject = WD_REGISTER_MENU("Menu.Project");
  s_hMenuScene = WD_REGISTER_MENU("Menu.Scene");
  s_hMenuView = WD_REGISTER_MENU("Menu.View");
  s_hMenuHelp = WD_REGISTER_MENU("Menu.Help");
  s_hCheckForUpdates =
    WD_REGISTER_ACTION_1("Help.CheckForUpdates", wdActionScope::Global, "Help", "", wdHelpActions, wdHelpActions::ButtonType::CheckForUpdates);
  s_hReportProblem =
    WD_REGISTER_ACTION_1("Help.ReportProblem", wdActionScope::Global, "Help", "", wdHelpActions, wdHelpActions::ButtonType::ReportProblem);
}

void wdStandardMenus::UnregisterActions()
{
  wdActionManager::UnregisterAction(s_hMenuFile);
  wdActionManager::UnregisterAction(s_hMenuEdit);
  wdActionManager::UnregisterAction(s_hMenuPanels);
  wdActionManager::UnregisterAction(s_hMenuProject);
  wdActionManager::UnregisterAction(s_hMenuScene);
  wdActionManager::UnregisterAction(s_hMenuView);
  wdActionManager::UnregisterAction(s_hMenuHelp);
  wdActionManager::UnregisterAction(s_hCheckForUpdates);
  wdActionManager::UnregisterAction(s_hReportProblem);
}

void wdStandardMenus::MapActions(const char* szMapping, const wdBitflags<wdStandardMenuTypes>& menus)
{
  wdActionMap* pMap = wdActionMapManager::GetActionMap(szMapping);
  WD_ASSERT_DEV(pMap != nullptr, "'{0}' does not exist", szMapping);

  wdActionMapDescriptor md;

  if (menus.IsAnySet(wdStandardMenuTypes::File))
    pMap->MapAction(s_hMenuFile, "", 1.0f);

  if (menus.IsAnySet(wdStandardMenuTypes::Edit))
    pMap->MapAction(s_hMenuEdit, "", 2.0f);

  if (menus.IsAnySet(wdStandardMenuTypes::Project))
    pMap->MapAction(s_hMenuProject, "", 3.0f);

  if (menus.IsAnySet(wdStandardMenuTypes::Scene))
    pMap->MapAction(s_hMenuScene, "", 4.0f);

  if (menus.IsAnySet(wdStandardMenuTypes::View))
    pMap->MapAction(s_hMenuView, "", 5.0f);

  if (menus.IsAnySet(wdStandardMenuTypes::Panels))
    pMap->MapAction(s_hMenuPanels, "", 6.0f);

  if (menus.IsAnySet(wdStandardMenuTypes::Help))
  {
    pMap->MapAction(s_hMenuHelp, "", 7.0f);
    pMap->MapAction(s_hReportProblem, "Menu.Help", 3.0f);
    pMap->MapAction(s_hCheckForUpdates, "Menu.Help", 10.0f);
  }
}

////////////////////////////////////////////////////////////////////////
// wdApplicationPanelsMenuAction
////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdApplicationPanelsMenuAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

struct wdComparePanels
{
  /// \brief Returns true if a is less than b
  WD_ALWAYS_INLINE bool Less(const wdDynamicMenuAction::Item& p1, const wdDynamicMenuAction::Item& p2) const { return p1.m_sDisplay < p2.m_sDisplay; }

  /// \brief Returns true if a is equal to b
  WD_ALWAYS_INLINE bool Equal(const wdDynamicMenuAction::Item& p1, const wdDynamicMenuAction::Item& p2) const
  {
    return p1.m_sDisplay == p2.m_sDisplay;
  }
};


void wdApplicationPanelsMenuAction::GetEntries(wdHybridArray<wdDynamicMenuAction::Item, 16>& out_entries)
{
  out_entries.Clear();

  for (auto* pPanel : wdQtApplicationPanel::GetAllApplicationPanels())
  {
    wdDynamicMenuAction::Item item;
    item.m_sDisplay = pPanel->windowTitle().toUtf8().data();
    item.m_UserValue = pPanel;
    item.m_Icon = pPanel->icon();
    item.m_CheckState = pPanel->isClosed() ? wdDynamicMenuAction::Item::CheckMark::Unchecked : wdDynamicMenuAction::Item::CheckMark::Checked;

    out_entries.PushBack(item);
  }

  // make sure the panels appear in alphabetical order in the menu
  wdComparePanels cp;
  out_entries.Sort<wdComparePanels>(cp);
}

void wdApplicationPanelsMenuAction::Execute(const wdVariant& value)
{
  wdQtApplicationPanel* pPanel = static_cast<wdQtApplicationPanel*>(value.ConvertTo<void*>());
  if (pPanel->isClosed())
  {
    pPanel->toggleView(true);
    pPanel->EnsureVisible();
  }
  else
  {
    pPanel->toggleView(false);
  }
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdHelpActions, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdHelpActions::wdHelpActions(const wdActionContext& context, const char* szName, ButtonType button)
  : wdButtonAction(context, szName, false, "")
{
  m_ButtonType = button;

  if (button == ButtonType::ReportProblem)
  {
    SetIconPath(":/EditorFramework/Icons/GitHub-Light.png");
  }
}

wdHelpActions::~wdHelpActions() = default;

void wdHelpActions::Execute(const wdVariant& value)
{
  if (m_ButtonType == ButtonType::ReportProblem)
  {
    QDesktopServices::openUrl(QUrl("https://github.com/wdEngine/wdEngine/issues"));
  }
  if (m_ButtonType == ButtonType::CheckForUpdates)
  {
    wdQtUiServices::CheckForUpdates();
  }
}
