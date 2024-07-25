#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/Action.h>
#include <GuiFoundation/Action/ActionManager.h>

const nsActionDescriptor* nsActionDescriptorHandle::GetDescriptor() const
{
  return nsActionManager::GetActionDescriptor(*this);
}

nsActionDescriptor::nsActionDescriptor(nsActionType::Enum type, nsActionScope::Enum scope, const char* szName, const char* szCategoryPath,
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

nsAction* nsActionDescriptor::CreateAction(const nsActionContext& context) const
{
  NS_ASSERT_DEV(!m_Handle.IsInvalidated(), "Handle invalid!");
  auto pAction = m_CreateAction(context);
  pAction->m_hDescriptorHandle = m_Handle;

  m_CreatedActions.PushBack(pAction);
  return pAction;
}

void nsActionDescriptor::DeleteAction(nsAction* pAction) const
{
  m_CreatedActions.RemoveAndSwap(pAction);

  if (m_DeleteAction == nullptr)
  {
    NS_DEFAULT_DELETE(pAction);
  }
  else
    m_DeleteAction(pAction);
}


void nsActionDescriptor::UpdateExistingActions()
{
  for (auto pAction : m_CreatedActions)
  {
    pAction->TriggerUpdate();
  }
}

void nsAction::TriggerUpdate()
{
  m_StatusUpdateEvent.Broadcast(this);
}

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
