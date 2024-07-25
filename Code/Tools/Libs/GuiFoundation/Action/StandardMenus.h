#pragma once

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

struct nsStandardMenuTypes
{
  using StorageType = nsUInt32;

  enum Enum
  {
    Project = NS_BIT(0),
    File = NS_BIT(1),
    Edit = NS_BIT(2),
    Panels = NS_BIT(3),
    Scene = NS_BIT(4),
    View = NS_BIT(5),
    Tools = NS_BIT(6),
    Help = NS_BIT(7),

    Default = Project | File | Panels | Tools | Help
  };

  struct Bits
  {
    StorageType Project : 1;
    StorageType File : 1;
    StorageType Edit : 1;
    StorageType Panels : 1;
    StorageType Scene : 1;
    StorageType View : 1;
    StorageType Tools : 1;
    StorageType Help : 1;
  };
};

NS_DECLARE_FLAGS_OPERATORS(nsStandardMenuTypes);

///
class NS_GUIFOUNDATION_DLL nsStandardMenus
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(nsStringView sMapping, const nsBitflags<nsStandardMenuTypes>& menus);

  static nsActionDescriptorHandle s_hMenuProject;
  static nsActionDescriptorHandle s_hMenuFile;
  static nsActionDescriptorHandle s_hMenuEdit;
  static nsActionDescriptorHandle s_hMenuPanels;
  static nsActionDescriptorHandle s_hMenuScene;
  static nsActionDescriptorHandle s_hMenuView;
  static nsActionDescriptorHandle s_hMenuTools;
  static nsActionDescriptorHandle s_hMenuHelp;
  static nsActionDescriptorHandle s_hCheckForUpdates;
  static nsActionDescriptorHandle s_hReportProblem;
};

///
class NS_GUIFOUNDATION_DLL nsApplicationPanelsMenuAction : public nsDynamicMenuAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsApplicationPanelsMenuAction, nsDynamicMenuAction);

public:
  nsApplicationPanelsMenuAction(const nsActionContext& context, const char* szName, const char* szIconPath)
    : nsDynamicMenuAction(context, szName, szIconPath)
  {
  }
  virtual void GetEntries(nsHybridArray<nsDynamicMenuAction::Item, 16>& out_entries) override;
  virtual void Execute(const nsVariant& value) override;
};

//////////////////////////////////////////////////////////////////////////

class NS_GUIFOUNDATION_DLL nsHelpActions : public nsButtonAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsHelpActions, nsButtonAction);

public:
  enum class ButtonType
  {
    CheckForUpdates,
    ReportProblem,
  };

  nsHelpActions(const nsActionContext& context, const char* szName, ButtonType button);
  ~nsHelpActions();

  virtual void Execute(const nsVariant& value) override;

private:
  ButtonType m_ButtonType;
};
