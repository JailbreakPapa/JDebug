#pragma once

#include <Foundation/Basics.h>
#include <GuiFoundation/Action/Action.h>

/// \brief Registers an nsAction whose constructor takes no arguments.
#define NS_REGISTER_ACTION_0(ActionName, Scope, CategoryName, ShortCut, ActionClass)                                  \
  nsActionManager::RegisterAction(nsActionDescriptor(nsActionType::Action, Scope, ActionName, CategoryName, ShortCut, \
    [](const nsActionContext& context) -> nsAction* { return NS_DEFAULT_NEW(ActionClass, context, ActionName); }));

/// \brief Registers an nsAction whose constructor takes one argument.
#define NS_REGISTER_ACTION_1(ActionName, Scope, CategoryName, ShortCut, ActionClass, Param1)                          \
  nsActionManager::RegisterAction(nsActionDescriptor(nsActionType::Action, Scope, ActionName, CategoryName, ShortCut, \
    [](const nsActionContext& context) -> nsAction* { return NS_DEFAULT_NEW(ActionClass, context, ActionName, Param1); }));

/// \brief Registers an nsAction whose constructor takes two arguments.
#define NS_REGISTER_ACTION_2(ActionName, Scope, CategoryName, ShortCut, ActionClass, Param1, Param2)                  \
  nsActionManager::RegisterAction(nsActionDescriptor(nsActionType::Action, Scope, ActionName, CategoryName, ShortCut, \
    [](const nsActionContext& context) -> nsAction* { return NS_DEFAULT_NEW(ActionClass, context, ActionName, Param1, Param2); }));

/// \brief Registers an nsDynamicMenuAction
#define NS_REGISTER_DYNAMIC_MENU(ActionName, ActionClass, IconPath)                                                  \
  nsActionManager::RegisterAction(nsActionDescriptor(nsActionType::Menu, nsActionScope::Default, ActionName, "", "", \
    [](const nsActionContext& context) -> nsAction* { return NS_DEFAULT_NEW(ActionClass, context, ActionName, IconPath); }));

/// \brief Registers an nsDynamicActionAndMenuAction.
#define NS_REGISTER_ACTION_AND_DYNAMIC_MENU_1(ActionName, Scope, CategoryName, ShortCut, ActionClass, Param1)                \
  nsActionManager::RegisterAction(nsActionDescriptor(nsActionType::ActionAndMenu, Scope, ActionName, CategoryName, ShortCut, \
    [](const nsActionContext& context) -> nsAction* { return NS_DEFAULT_NEW(ActionClass, context, ActionName, Param1); }));

/// \brief Registers a category that should be treated as a sub-menu.
#define NS_REGISTER_MENU(ActionName)                                                                                 \
  nsActionManager::RegisterAction(nsActionDescriptor(nsActionType::Menu, nsActionScope::Default, ActionName, "", "", \
    [](const nsActionContext& context) -> nsAction* { return NS_DEFAULT_NEW(nsMenuAction, context, ActionName, ""); }));

/// \brief Registers a category that should be treated as a sub-menu and specifies a custom QIcon path.
#define NS_REGISTER_MENU_WITH_ICON(ActionName, IconPath)                                                             \
  nsActionManager::RegisterAction(nsActionDescriptor(nsActionType::Menu, nsActionScope::Default, ActionName, "", "", \
    [](const nsActionContext& context) -> nsAction* { return NS_DEFAULT_NEW(nsMenuAction, context, ActionName, IconPath); }));

/// \brief Registers a category that should just be a grouped area in a menu, but no dedicated sub-menu.
#define NS_REGISTER_CATEGORY(CategoryName)                                                                                 \
  nsActionManager::RegisterAction(nsActionDescriptor(nsActionType::Category, nsActionScope::Default, CategoryName, "", "", \
    [](const nsActionContext& context) -> nsAction* { return NS_DEFAULT_NEW(nsCategoryAction, context); }));

/// \brief Stores 'actions' (things that can be triggered from UI).
///
/// Actions are usually represented by a button in a toolbar, or a menu entry.
/// Actions are unique across the entire application. Each action is registered exactly once,
/// but it may be referenced by many different nsActionMap instances, which defines how an action shows up in a window.
///
/// Through RegisterAction() / UnregisterAction() an action is added or removed.
/// These functions are usually not called directly, but rather the macros at the top of this file are used (see NS_REGISTER_CATEGORY, NS_REGISTER_MENU, NS_REGISTER_ACTION_X, ...).
///
/// Unit tests can call ExecuteAction() to directly invoke an action.
/// Widgets use nsActionMap to organize which actions are available in a window, and how they are structured.
/// For instance, the same action can appear in a menu, in a toolbar and a context menu. In each case their location may be different (top-level, in a sub-menu, etc).
/// See nsActionMap for details.
class NS_GUIFOUNDATION_DLL nsActionManager
{
public:
  static nsActionDescriptorHandle RegisterAction(const nsActionDescriptor& desc);
  static bool UnregisterAction(nsActionDescriptorHandle& ref_hAction);
  static const nsActionDescriptor* GetActionDescriptor(nsActionDescriptorHandle hAction);
  static nsActionDescriptorHandle GetActionHandle(const char* szCategory, const char* szActionName);

  /// \brief Searches all action categories for the given action name. Returns the category name in which the action name was found, or an empty
  /// string.
  static nsString FindActionCategory(const char* szActionName);

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
  /// \param context The context in which to execute the action. Depending on the nsActionScope of the target action,
  ///        some members are optional. E.g. for document actions, only the m_pDocument member must be specified.
  /// \param value Optional value passed through to the nsAction::Execute() call. Some actions use it, most don't.
  /// \return Returns failure in case the action could not be found.
  static nsResult ExecuteAction(const char* szCategory, const char* szActionName, const nsActionContext& context, const nsVariant& value = nsVariant());

  static void SaveShortcutAssignment();
  static void LoadShortcutAssignment();

  static const nsIdTable<nsActionId, nsActionDescriptor*>::ConstIterator GetActionIterator();

  struct Event
  {
    enum class Type
    {
      ActionAdded,
      ActionRemoved
    };

    Type m_Type;
    const nsActionDescriptor* m_pDesc;
    nsActionDescriptorHandle m_Handle;
  };

  static nsEvent<const Event&> s_Events;

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(GuiFoundation, ActionManager);

  static void Startup();
  static void Shutdown();
  static nsActionDescriptor* CreateActionDesc(const nsActionDescriptor& desc);
  static void DeleteActionDesc(nsActionDescriptor* pDesc);

  struct CategoryData
  {
    nsSet<nsActionDescriptorHandle> m_Actions;
    nsHashTable<const char*, nsActionDescriptorHandle> m_ActionNameToHandle;
  };

private:
  static nsIdTable<nsActionId, nsActionDescriptor*> s_ActionTable;
  static nsMap<nsString, CategoryData> s_CategoryPathToActions;
  static nsMap<nsString, nsString> s_ShortcutOverride;
};
