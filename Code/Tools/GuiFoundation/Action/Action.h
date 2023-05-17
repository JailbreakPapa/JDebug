#pragma once

#include <Foundation/Communication/Event.h>
#include <Foundation/Containers/IdTable.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Types/Enum.h>
#include <Foundation/Types/Variant.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <QKeySequence>
#include <ToolsFoundation/Document/DocumentManager.h>

class QWidget;
struct wdActionDescriptor;
class wdAction;
struct wdActionContext;

typedef wdGenericId<24, 8> wdActionId;
typedef wdAction* (*CreateActionFunc)(const wdActionContext& context);
typedef void (*DeleteActionFunc)(wdAction* pAction);

/// \brief Handle for a wdAction.
///
/// wdAction can be invalidated at runtime so don't store them.
class WD_GUIFOUNDATION_DLL wdActionDescriptorHandle
{
public:
  typedef wdUInt32 StorageType;
  WD_DECLARE_HANDLE_TYPE(wdActionDescriptorHandle, wdActionId);
  friend class wdActionManager;

public:
  const wdActionDescriptor* GetDescriptor() const;
};

///
struct wdActionScope
{
  enum Enum
  {
    Global,
    Document,
    Window,
    Default = Global
  };
  typedef wdUInt8 StorageType;
};

///
struct wdActionType
{
  enum Enum
  {
    Action,
    Category,
    Menu,
    ActionAndMenu,
    Default = Action
  };
  typedef wdUInt8 StorageType;
};

///
struct WD_GUIFOUNDATION_DLL wdActionContext
{
  wdActionContext() = default;
  wdActionContext(wdDocument* pDoc) { m_pDocument = pDoc; }

  wdDocument* m_pDocument = nullptr;
  wdString m_sMapping;
  QWidget* m_pWindow = nullptr;
};


///
struct WD_GUIFOUNDATION_DLL wdActionDescriptor
{
  wdActionDescriptor(){};
  wdActionDescriptor(wdActionType::Enum type, wdActionScope::Enum scope, const char* szName, const char* szCategoryPath, const char* szShortcut,
    CreateActionFunc createAction, DeleteActionFunc deleteAction = nullptr);

  wdActionDescriptorHandle m_Handle;
  wdEnum<wdActionType> m_Type;

  wdEnum<wdActionScope> m_Scope;
  wdString m_sActionName;   ///< Unique within category path, shown in key configuration dialog
  wdString m_sCategoryPath; ///< Category in key configuration dialog, e.g. "Tree View" or "File"

  wdString m_sShortcut;
  wdString m_sDefaultShortcut;

  wdAction* CreateAction(const wdActionContext& context) const;
  void DeleteAction(wdAction* pAction) const;

  void UpdateExistingActions();

private:
  CreateActionFunc m_CreateAction;
  DeleteActionFunc m_DeleteAction;

  mutable wdHybridArray<wdAction*, 4> m_CreatedActions;
};



///
class WD_GUIFOUNDATION_DLL wdAction : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdAction, wdReflectedClass);
  WD_DISALLOW_COPY_AND_ASSIGN(wdAction);

public:
  wdAction(const wdActionContext& context) { m_Context = context; }
  virtual void Execute(const wdVariant& value) = 0;

  void TriggerUpdate();
  const wdActionContext& GetContext() const { return m_Context; }
  wdActionDescriptorHandle GetDescriptorHandle() { return m_hDescriptorHandle; }

public:
  wdEvent<wdAction*> m_StatusUpdateEvent; ///< Fire when the state of the action changes (enabled, value etc...)

protected:
  wdActionContext m_Context;

private:
  friend struct wdActionDescriptor;
  wdActionDescriptorHandle m_hDescriptorHandle;
};
