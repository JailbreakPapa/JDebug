#pragma once

#include <Foundation/Basics.h>
#include <GuiFoundation/Action/Action.h>

#define WD_REGISTER_ACTION_0(ActionName, Scope, CategoryName, ShortCut, ActionClass)                                                                 \
  wdActionManager::RegisterAction(wdActionDescriptor(wdActionType::Action, Scope, ActionName, CategoryName, ShortCut,                                \
    [](const wdActionContext& context) -> wdAction* { return WD_DEFAULT_NEW(ActionClass, context, ActionName); }));

#define WD_REGISTER_ACTION_1(ActionName, Scope, CategoryName, ShortCut, ActionClass, Param1)                                                         \
  wdActionManager::RegisterAction(wdActionDescriptor(wdActionType::Action, Scope, ActionName, CategoryName, ShortCut,                                \
    [](const wdActionContext& context) -> wdAction* { return WD_DEFAULT_NEW(ActionClass, context, ActionName, Param1); }));

#define WD_REGISTER_ACTION_2(ActionName, Scope, CategoryName, ShortCut, ActionClass, Param1, Param2)                                                 \
  wdActionManager::RegisterAction(wdActionDescriptor(wdActionType::Action, Scope, ActionName, CategoryName, ShortCut,                                \
    [](const wdActionContext& context) -> wdAction* { return WD_DEFAULT_NEW(ActionClass, context, ActionName, Param1, Param2); }));

#define WD_REGISTER_DYNAMIC_MENU(ActionName, ActionClass, IconPath)                                                                                  \
  wdActionManager::RegisterAction(wdActionDescriptor(wdActionType::Menu, wdActionScope::Default, ActionName, "", "",                                 \
    [](const wdActionContext& context) -> wdAction* { return WD_DEFAULT_NEW(ActionClass, context, ActionName, IconPath); }));

#define WD_REGISTER_ACTION_AND_DYNAMIC_MENU_1(ActionName, Scope, CategoryName, ShortCut, ActionClass, Param1)                                        \
  wdActionManager::RegisterAction(wdActionDescriptor(wdActionType::ActionAndMenu, Scope, ActionName, CategoryName, ShortCut,                         \
    [](const wdActionContext& context) -> wdAction* { return WD_DEFAULT_NEW(ActionClass, context, ActionName, Param1); }));

#define WD_REGISTER_MENU(ActionName)                                                                                                                 \
  wdActionManager::RegisterAction(wdActionDescriptor(wdActionType::Menu, wdActionScope::Default, ActionName, "", "",                                 \
    [](const wdActionContext& context) -> wdAction* { return WD_DEFAULT_NEW(wdMenuAction, context, ActionName, ""); }));

#define WD_REGISTER_MENU_WITH_ICON(ActionName, IconPath)                                                                                             \
  wdActionManager::RegisterAction(wdActionDescriptor(wdActionType::Menu, wdActionScope::Default, ActionName, "", "",                                 \
    [](const wdActionContext& context) -> wdAction* { return WD_DEFAULT_NEW(wdMenuAction, context, ActionName, IconPath); }));

#define WD_REGISTER_CATEGORY(CategoryName)                                                                                                           \
  wdActionManager::RegisterAction(wdActionDescriptor(wdActionType::Category, wdActionScope::Default, CategoryName, "", "",                           \
    [](const wdActionContext& context) -> wdAction* { return WD_DEFAULT_NEW(wdCategoryAction, context); }));

///
class WD_GUIFOUNDATION_DLL wdActionManager
{
public:
  static wdActionDescriptorHandle RegisterAction(const wdActionDescriptor& desc);
  static bool UnregisterAction(wdActionDescriptorHandle& ref_hAction);
  static const wdActionDescriptor* GetActionDescriptor(wdActionDescriptorHandle hAction);
  static wdActionDescriptorHandle GetActionHandle(const char* szCategory, const char* szActionName);

  /// \brief Searches all action categories for the given action name. Returns the category name in which the action name was found, or an empty
  /// string.
  static wdString FindActionCategory(const char* szActionName);

  /// \brief Quick way to execute an action from code
  ///
  /// The use case is mostly for unit tests, which need to execute actions directly and without a link dependency on
  /// the code that registered the action.
  ///
  /// \param szCategory The category of the action, ie. under which name the action appears in the Shortcut binding dialog.
  ///        For example "Scene", "Scene - Cameras", "Scene - Selection", "Assets" etc.
  ///        This parameter may be nullptr in which case FindActionCategory(szActionName) is used to try to detect the category automatically.
  /// \param szActionName The name (not mapped path) under which the action was registered.
  ///        For example "Selection.Copy", "Prefabs.ConvertToEngine", "Scene.Camera.SnapObjectToCamera"
  /// \param context The context in which to execute the action. Depending on the wdActionScope of the target action,
  ///        some members are optional. E.g. for document actions, only the m_pDocument member must be specified.
  /// \param value Optional value passed through to the wdAction::Execute() call. Some actions use it, most don't.
  /// \return Returns failure in case the action could not be found.
  static wdResult ExecuteAction(
    const char* szCategory, const char* szActionName, const wdActionContext& context, const wdVariant& value = wdVariant());

  static void SaveShortcutAssignment();
  static void LoadShortcutAssignment();

  static const wdIdTable<wdActionId, wdActionDescriptor*>::ConstIterator GetActionIterator();

  struct Event
  {
    enum class Type
    {
      ActionAdded,
      ActionRemoved
    };

    Type m_Type;
    const wdActionDescriptor* m_pDesc;
    wdActionDescriptorHandle m_Handle;
  };

  static wdEvent<const Event&> s_Events;

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(GuiFoundation, ActionManager);

  static void Startup();
  static void Shutdown();
  static wdActionDescriptor* CreateActionDesc(const wdActionDescriptor& desc);
  static void DeleteActionDesc(wdActionDescriptor* pDesc);

  struct CategoryData
  {
    wdSet<wdActionDescriptorHandle> m_Actions;
    wdHashTable<const char*, wdActionDescriptorHandle> m_ActionNameToHandle;
  };

private:
  static wdIdTable<wdActionId, wdActionDescriptor*> s_ActionTable;
  static wdMap<wdString, CategoryData> s_CategoryPathToActions;
  static wdMap<wdString, wdString> s_ShortcutOverride;
};
