#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCommandHistoryAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsActionDescriptorHandle nsCommandHistoryActions::s_hCommandHistoryCategory;
nsActionDescriptorHandle nsCommandHistoryActions::s_hUndo;
nsActionDescriptorHandle nsCommandHistoryActions::s_hRedo;

void nsCommandHistoryActions::RegisterActions()
{
  s_hCommandHistoryCategory = NS_REGISTER_CATEGORY("CmdHistoryCategory");
  s_hUndo = NS_REGISTER_ACTION_AND_DYNAMIC_MENU_1("Document.Undo", nsActionScope::Document, "Document", "Ctrl+Z", nsCommandHistoryAction, nsCommandHistoryAction::ButtonType::Undo);
  s_hRedo = NS_REGISTER_ACTION_AND_DYNAMIC_MENU_1("Document.Redo", nsActionScope::Document, "Document", "Ctrl+Y", nsCommandHistoryAction, nsCommandHistoryAction::ButtonType::Redo);
}

void nsCommandHistoryActions::UnregisterActions()
{
  nsActionManager::UnregisterAction(s_hCommandHistoryCategory);
  nsActionManager::UnregisterAction(s_hUndo);
  nsActionManager::UnregisterAction(s_hRedo);
}

void nsCommandHistoryActions::MapActions(nsStringView sMapping, nsStringView sTargetMenu)
{
  nsActionMap* pMap = nsActionMapManager::GetActionMap(sMapping);
  NS_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the actions failed!", sMapping);

  pMap->MapAction(s_hCommandHistoryCategory, sTargetMenu, 3.0f);
  pMap->MapAction(s_hUndo, sTargetMenu, "CmdHistoryCategory", 1.0f);
  pMap->MapAction(s_hRedo, sTargetMenu, "CmdHistoryCategory", 2.0f);
}

nsCommandHistoryAction::nsCommandHistoryAction(const nsActionContext& context, const char* szName, ButtonType button)
  : nsDynamicActionAndMenuAction(context, szName, "")
{
  m_ButtonType = button;

  switch (m_ButtonType)
  {
    case nsCommandHistoryAction::ButtonType::Undo:
      SetIconPath(":/GuiFoundation/Icons/Undo.svg");
      break;
    case nsCommandHistoryAction::ButtonType::Redo:
      SetIconPath(":/GuiFoundation/Icons/Redo.svg");
      break;
  }

  m_Context.m_pDocument->GetCommandHistory()->m_Events.AddEventHandler(nsMakeDelegate(&nsCommandHistoryAction::CommandHistoryEventHandler, this));

  UpdateState();
}

nsCommandHistoryAction::~nsCommandHistoryAction()
{
  m_Context.m_pDocument->GetCommandHistory()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsCommandHistoryAction::CommandHistoryEventHandler, this));
}

void nsCommandHistoryAction::GetEntries(nsHybridArray<nsDynamicMenuAction::Item, 16>& out_entries)
{
  out_entries.Clear();

  nsCommandHistory* pHistory = m_Context.m_pDocument->GetCommandHistory();

  const nsUInt32 iCount = (m_ButtonType == ButtonType::Undo) ? pHistory->GetUndoStackSize() : pHistory->GetRedoStackSize();
  for (nsUInt32 i = 0; i < iCount; i++)
  {
    const nsCommandTransaction* pTransaction = (m_ButtonType == ButtonType::Undo) ? pHistory->GetUndoStackEntry(i) : pHistory->GetRedoStackEntry(i);
    nsDynamicMenuAction::Item entryItem;
    entryItem.m_sDisplay = pTransaction->m_sDisplayString;
    entryItem.m_UserValue = (nsUInt32)i + 1; // Number of steps to undo / redo.
    out_entries.PushBack(entryItem);
  }
}

void nsCommandHistoryAction::Execute(const nsVariant& value)
{
  nsUInt32 iCount = value.IsValid() ? value.ConvertTo<nsUInt32>() : 1;

  switch (m_ButtonType)
  {
    case ButtonType::Undo:
    {
      NS_ASSERT_DEV(m_Context.m_pDocument->GetCommandHistory()->CanUndo(), "The action should not be active");

      auto stat = m_Context.m_pDocument->GetCommandHistory()->Undo(iCount);
      nsQtUiServices::MessageBoxStatus(stat, "Could not execute the Undo operation");
    }
    break;

    case ButtonType::Redo:
    {
      NS_ASSERT_DEV(m_Context.m_pDocument->GetCommandHistory()->CanRedo(), "The action should not be active");

      auto stat = m_Context.m_pDocument->GetCommandHistory()->Redo(iCount);
      nsQtUiServices::MessageBoxStatus(stat, "Could not execute the Redo operation");
    }
    break;
  }
}

void nsCommandHistoryAction::UpdateState()
{
  switch (m_ButtonType)
  {
    case ButtonType::Undo:
      SetAdditionalDisplayString(m_Context.m_pDocument->GetCommandHistory()->GetUndoDisplayString(), false);
      SetEnabled(m_Context.m_pDocument->GetCommandHistory()->CanUndo());
      break;

    case ButtonType::Redo:
      SetAdditionalDisplayString(m_Context.m_pDocument->GetCommandHistory()->GetRedoDisplayString(), false);
      SetEnabled(m_Context.m_pDocument->GetCommandHistory()->CanRedo());
      break;
  }
}

void nsCommandHistoryAction::CommandHistoryEventHandler(const nsCommandHistoryEvent& e)
{
  UpdateState();
}
