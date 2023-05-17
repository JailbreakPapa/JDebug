#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/Action.h>
#include <GuiFoundation/Action/ActionManager.h>

const wdActionDescriptor* wdActionDescriptorHandle::GetDescriptor() const
{
  return wdActionManager::GetActionDescriptor(*this);
}

wdActionDescriptor::wdActionDescriptor(wdActionType::Enum type, wdActionScope::Enum scope, const char* szName, const char* szCategoryPath,
  const char* szShortcut, CreateActionFunc createAction, DeleteActionFunc deleteAction)
  : m_Type(type)
  , m_Scope(scope)
  , m_sActionName(szName)
  , m_sCategoryPath(szCategoryPath)
  , m_sShortcut(szShortcut)
  , m_sDefaultShortcut(szShortcut)
  , m_CreateAction(createAction)
  , m_DeleteAction(deleteAction)
{
}

wdAction* wdActionDescriptor::CreateAction(const wdActionContext& context) const
{
  WD_ASSERT_DEV(!m_Handle.IsInvalidated(), "Handle invalid!");
  auto pAction = m_CreateAction(context);
  pAction->m_hDescriptorHandle = m_Handle;

  m_CreatedActions.PushBack(pAction);
  return pAction;
}

void wdActionDescriptor::DeleteAction(wdAction* pAction) const
{
  m_CreatedActions.RemoveAndSwap(pAction);

  if (m_DeleteAction == nullptr)
  {
    WD_DEFAULT_DELETE(pAction);
  }
  else
    m_DeleteAction(pAction);
}


void wdActionDescriptor::UpdateExistingActions()
{
  for (auto pAction : m_CreatedActions)
  {
    pAction->TriggerUpdate();
  }
}

void wdAction::TriggerUpdate()
{
  m_StatusUpdateEvent.Broadcast(this);
}

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;
