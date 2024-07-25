#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/StandardMenus.h>
#include <GuiFoundation/DockPanels/ApplicationPanel.moc.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>

nsActionDescriptorHandle nsStandardMenus::s_hMenuProject;
nsActionDescriptorHandle nsStandardMenus::s_hMenuFile;
nsActionDescriptorHandle nsStandardMenus::s_hMenuEdit;
nsActionDescriptorHandle nsStandardMenus::s_hMenuPanels;
nsActionDescriptorHandle nsStandardMenus::s_hMenuScene;
nsActionDescriptorHandle nsStandardMenus::s_hMenuView;
nsActionDescriptorHandle nsStandardMenus::s_hMenuTools;
nsActionDescriptorHandle nsStandardMenus::s_hMenuHelp;
nsActionDescriptorHandle nsStandardMenus::s_hCheckForUpdates;
nsActionDescriptorHandle nsStandardMenus::s_hReportProblem;

void nsStandardMenus::RegisterActions()
{
  s_hMenuProject = NS_REGISTER_MENU("G.Project");
  s_hMenuFile = NS_REGISTER_MENU("G.File");
  s_hMenuEdit = NS_REGISTER_MENU("G.Edit");
  s_hMenuPanels = NS_REGISTER_DYNAMIC_MENU("G.Panels", nsApplicationPanelsMenuAction, "");
  s_hMenuScene = NS_REGISTER_MENU("G.Scene");
  s_hMenuView = NS_REGISTER_MENU("G.View");
  s_hMenuTools = NS_REGISTER_MENU("G.Tools");
  s_hMenuHelp = NS_REGISTER_MENU("G.Help");
  s_hCheckForUpdates = NS_REGISTER_ACTION_1("Help.CheckForUpdates", nsActionScope::Global, "Help", "", nsHelpActions, nsHelpActions::ButtonType::CheckForUpdates);
  s_hReportProblem = NS_REGISTER_ACTION_1("Help.ReportProblem", nsActionScope::Global, "Help", "", nsHelpActions, nsHelpActions::ButtonType::ReportProblem);
}

void nsStandardMenus::UnregisterActions()
{
  nsActionManager::UnregisterAction(s_hMenuProject);
  nsActionManager::UnregisterAction(s_hMenuFile);
  nsActionManager::UnregisterAction(s_hMenuEdit);
  nsActionManager::UnregisterAction(s_hMenuPanels);
  nsActionManager::UnregisterAction(s_hMenuScene);
  nsActionManager::UnregisterAction(s_hMenuView);
  nsActionManager::UnregisterAction(s_hMenuTools);
  nsActionManager::UnregisterAction(s_hMenuHelp);
  nsActionManager::UnregisterAction(s_hCheckForUpdates);
  nsActionManager::UnregisterAction(s_hReportProblem);
}

void nsStandardMenus::MapActions(nsStringView sMapping, const nsBitflags<nsStandardMenuTypes>& menus)
{
  nsActionMap* pMap = nsActionMapManager::GetActionMap(sMapping);
  NS_ASSERT_DEV(pMap != nullptr, "'{0}' does not exist", sMapping);

  nsActionMapDescriptor md;

  if (menus.IsAnySet(nsStandardMenuTypes::Project))
    pMap->MapAction(s_hMenuProject, "", -10000.0f);

  if (menus.IsAnySet(nsStandardMenuTypes::File))
    pMap->MapAction(s_hMenuFile, "", 1.0f);

  if (menus.IsAnySet(nsStandardMenuTypes::Edit))
    pMap->MapAction(s_hMenuEdit, "", 2.0f);

  if (menus.IsAnySet(nsStandardMenuTypes::Scene))
    pMap->MapAction(s_hMenuScene, "", 3.0f);

  if (menus.IsAnySet(nsStandardMenuTypes::View))
    pMap->MapAction(s_hMenuView, "", 4.0f);

  if (menus.IsAnySet(nsStandardMenuTypes::Tools))
    pMap->MapAction(s_hMenuTools, "", 5.0f);

  if (menus.IsAnySet(nsStandardMenuTypes::Panels))
    pMap->MapAction(s_hMenuPanels, "", 6.0f);

  if (menus.IsAnySet(nsStandardMenuTypes::Help))
  {
    pMap->MapAction(s_hMenuHelp, "", 7.0f);
    pMap->MapAction(s_hReportProblem, "G.Help", 3.0f);
    pMap->MapAction(s_hCheckForUpdates, "G.Help", 10.0f);
  }
}

////////////////////////////////////////////////////////////////////////
// nsApplicationPanelsMenuAction
////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsApplicationPanelsMenuAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

struct nsComparePanels
{
  /// \brief Returns true if a is less than b
  NS_ALWAYS_INLINE bool Less(const nsDynamicMenuAction::Item& p1, const nsDynamicMenuAction::Item& p2) const { return p1.m_sDisplay < p2.m_sDisplay; }

  /// \brief Returns true if a is equal to b
  NS_ALWAYS_INLINE bool Equal(const nsDynamicMenuAction::Item& p1, const nsDynamicMenuAction::Item& p2) const
  {
    return p1.m_sDisplay == p2.m_sDisplay;
  }
};


void nsApplicationPanelsMenuAction::GetEntries(nsHybridArray<nsDynamicMenuAction::Item, 16>& out_entries)
{
  out_entries.Clear();

  for (auto* pPanel : nsQtApplicationPanel::GetAllApplicationPanels())
  {
    nsDynamicMenuAction::Item item;
    item.m_sDisplay = pPanel->windowTitle().toUtf8().data();
    item.m_UserValue = pPanel;
    item.m_Icon = pPanel->icon();
    item.m_CheckState = pPanel->isClosed() ? nsDynamicMenuAction::Item::CheckMark::Unchecked : nsDynamicMenuAction::Item::CheckMark::Checked;

    out_entries.PushBack(item);
  }

  // make sure the panels appear in alphabetical order in the menu
  nsComparePanels cp;
  out_entries.Sort<nsComparePanels>(cp);
}

void nsApplicationPanelsMenuAction::Execute(const nsVariant& value)
{
  nsQtApplicationPanel* pPanel = static_cast<nsQtApplicationPanel*>(value.ConvertTo<void*>());
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
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsHelpActions, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsHelpActions::nsHelpActions(const nsActionContext& context, const char* szName, ButtonType button)
  : nsButtonAction(context, szName, false, "")
{
  m_ButtonType = button;

  if (button == ButtonType::ReportProblem)
  {
    SetIconPath(":/EditorFramework/Icons/GitHub-Light.svg");
  }
}

nsHelpActions::~nsHelpActions() = default;

void nsHelpActions::Execute(const nsVariant& value)
{
  if (m_ButtonType == ButtonType::ReportProblem)
  {
    QDesktopServices::openUrl(QUrl("https://github.com/nsEngine/nsEngine/issues"));
  }
  if (m_ButtonType == ButtonType::CheckForUpdates)
  {
    nsQtUiServices::CheckForUpdates();
  }
}
