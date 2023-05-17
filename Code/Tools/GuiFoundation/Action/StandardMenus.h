#pragma once

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

struct wdStandardMenuTypes
{
  typedef wdUInt32 StorageType;

  enum Enum
  {
    File = WD_BIT(0),
    Edit = WD_BIT(1),
    Panels = WD_BIT(2),
    Project = WD_BIT(3),
    Scene = WD_BIT(4),
    View = WD_BIT(5),
    Help = WD_BIT(6),

    Default = 0
  };

  struct Bits
  {
    StorageType File : 1;
    StorageType Edit : 1;
    StorageType Panels : 1;
    StorageType Project : 1;
    StorageType Scene : 1;
    StorageType View : 1;
    StorageType Help : 1;
  };
};

WD_DECLARE_FLAGS_OPERATORS(wdStandardMenuTypes);

///
class WD_GUIFOUNDATION_DLL wdStandardMenus
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(const char* szMapping, const wdBitflags<wdStandardMenuTypes>& menus);

  static wdActionDescriptorHandle s_hMenuFile;
  static wdActionDescriptorHandle s_hMenuEdit;
  static wdActionDescriptorHandle s_hMenuPanels;
  static wdActionDescriptorHandle s_hMenuProject;
  static wdActionDescriptorHandle s_hMenuScene;
  static wdActionDescriptorHandle s_hMenuView;
  static wdActionDescriptorHandle s_hMenuHelp;
  static wdActionDescriptorHandle s_hCheckForUpdates;
  static wdActionDescriptorHandle s_hReportProblem;
};

///
class WD_GUIFOUNDATION_DLL wdApplicationPanelsMenuAction : public wdDynamicMenuAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdApplicationPanelsMenuAction, wdDynamicMenuAction);

public:
  wdApplicationPanelsMenuAction(const wdActionContext& context, const char* szName, const char* szIconPath)
    : wdDynamicMenuAction(context, szName, szIconPath)
  {
  }
  virtual void GetEntries(wdHybridArray<wdDynamicMenuAction::Item, 16>& out_entries) override;
  virtual void Execute(const wdVariant& value) override;
};

//////////////////////////////////////////////////////////////////////////

class WD_GUIFOUNDATION_DLL wdHelpActions : public wdButtonAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdHelpActions, wdButtonAction);

public:
  enum class ButtonType
  {
    CheckForUpdates,
    ReportProblem,
  };

  wdHelpActions(const wdActionContext& context, const char* szName, ButtonType button);
  ~wdHelpActions();

  virtual void Execute(const wdVariant& value) override;

private:
  ButtonType m_ButtonType;
};
