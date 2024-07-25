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
struct nsActionDescriptor;
class nsAction;
struct nsActionContext;

using nsActionId = nsGenericId<24, 8>;
using CreateActionFunc = nsAction* (*)(const nsActionContext&);
using DeleteActionFunc = void (*)(nsAction*);

/// \brief Handle for a nsAction.
///
/// nsAction can be invalidated at runtime so don't store them.
class NS_GUIFOUNDATION_DLL nsActionDescriptorHandle
{
public:
  using StorageType = nsUInt32;

  NS_DECLARE_HANDLE_TYPE(nsActionDescriptorHandle, nsActionId);
  friend class nsActionManager;

public:
  const nsActionDescriptor* GetDescriptor() const;
};

///
struct nsActionScope
{
  enum Enum
  {
    Global,
    Document,
    Window,
    Default = Global
  };
  using StorageType = nsUInt8;
};

///
struct nsActionType
{
  enum Enum
  {
    Action,
    Category,
    Menu,
    ActionAndMenu,
    Default = Action
  };
  using StorageType = nsUInt8;
};

///
struct NS_GUIFOUNDATION_DLL nsActionContext
{
  nsActionContext() = default;
  nsActionContext(nsDocument* pDoc) { m_pDocument = pDoc; }

  nsDocument* m_pDocument = nullptr;
  nsString m_sMapping;
  QWidget* m_pWindow = nullptr;
};


///
struct NS_GUIFOUNDATION_DLL nsActionDescriptor
{
  nsActionDescriptor() = default;
  ;
  nsActionDescriptor(nsActionType::Enum type, nsActionScope::Enum scope, const char* szName, const char* szCategoryPath, const char* szShortcut,
    CreateActionFunc createAction, DeleteActionFunc deleteAction = nullptr);

  nsActionDescriptorHandle m_Handle;
  nsEnum<nsActionType> m_Type;

  nsEnum<nsActionScope> m_Scope;
  nsString m_sActionName;   ///< Unique within category path, shown in key configuration dialog
  nsString m_sCategoryPath; ///< Category in key configuration dialog, e.g. "Tree View" or "File"

  nsString m_sShortcut;
  nsString m_sDefaultShortcut;

  nsAction* CreateAction(const nsActionContext& context) const;
  void DeleteAction(nsAction* pAction) const;

  void UpdateExistingActions();

private:
  CreateActionFunc m_CreateAction;
  DeleteActionFunc m_DeleteAction;

  mutable nsHybridArray<nsAction*, 4> m_CreatedActions;
};



///
class NS_GUIFOUNDATION_DLL nsAction : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsAction, nsReflectedClass);
  NS_DISALLOW_COPY_AND_ASSIGN(nsAction);

public:
  nsAction(const nsActionContext& context) { m_Context = context; }
  virtual void Execute(const nsVariant& value) = 0;

  void TriggerUpdate();
  const nsActionContext& GetContext() const { return m_Context; }
  nsActionDescriptorHandle GetDescriptorHandle() { return m_hDescriptorHandle; }

public:
  nsEvent<nsAction*> m_StatusUpdateEvent; ///< Fire when the state of the action changes (enabled, value etc...)

protected:
  nsActionContext m_Context;

private:
  friend struct nsActionDescriptor;
  nsActionDescriptorHandle m_hDescriptorHandle;
};
