#include <GuiFoundation/GuiFoundationPCH.h>

#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/CommandHistoryActions.h>
#include <GuiFoundation/UIServices/UIServices.moc.h>

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCommandHistoryAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

wdActionDescriptorHandle wdCommandHistoryActions::s_hCommandHistoryCategory;
wdActionDescriptorHandle wdCommandHistoryActions::s_hUndo;
wdActionDescriptorHandle wdCommandHistoryActions::s_hRedo;

void wdCommandHistoryActions::RegisterActions()
{
  s_hCommandHistoryCategory = WD_REGISTER_CATEGORY("CmdHistoryCategory");
  s_hUndo = WD_REGISTER_ACTION_AND_DYNAMIC_MENU_1(
    "Document.Undo", wdActionScope::Document, "Document", "Ctrl+Z", wdCommandHistoryAction, wdCommandHistoryAction::ButtonType::Undo);
  s_hRedo = WD_REGISTER_ACTION_AND_DYNAMIC_MENU_1(
    "Document.Redo", wdActionScope::Document, "Document", "Ctrl+Y", wdCommandHistoryAction, wdCommandHistoryAction::ButtonType::Redo);
}

void wdCommandHistoryActions::UnregisterActions()
{
  wdActionManager::UnregisterAction(s_hCommandHistoryCategory);
  wdActionManager::UnregisterAction(s_hUndo);
  wdActionManager::UnregisterAction(s_hRedo);
}

void wdCommandHistoryActions::MapActions(const char* szMapping, const char* szPath)
{
  wdActionMap* pMap = wdActionMapManager::GetActionMap(szMapping);
  WD_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the actions failed!", szMapping);

  wdStringBuilder sSubPath(szPath, "/CmdHistoryCategory");

  pMap->MapAction(s_hCommandHistoryCategory, szPath, 3.0f);
  pMap->MapAction(s_hUndo, sSubPath, 1.0f);
  pMap->MapAction(s_hRedo, sSubPath, 2.0f);
}

wdCommandHistoryAction::wdCommandHistoryAction(const wdActionContext& context, const char* szName, ButtonType button)
  : wdDynamicActionAndMenuAction(context, szName, "")
{
  m_ButtonType = button;

  switch (m_ButtonType)
  {
    case wdCommandHistoryAction::ButtonType::Undo:
      SetIconPath(":/GuiFoundation/Icons/Undo16.png");
      break;
    case wdCommandHistoryAction::ButtonType::Redo:
      SetIconPath(":/GuiFoundation/Icons/Redo16.png");
      break;
  }

  m_Context.m_pDocument->GetCommandHistory()->m_Events.AddEventHandler(wdMakeDelegate(&wdCommandHistoryAction::CommandHistoryEventHandler, this));

  UpdateState();
}

wdCommandHistoryAction::~wdCommandHistoryAction()
{
  m_Context.m_pDocument->GetCommandHistory()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdCommandHistoryAction::CommandHistoryEventHandler, this));
}

void wdCommandHistoryAction::GetEntries(wdHybridArray<wdDynamicMenuAction::Item, 16>& out_entries)
{
  out_entries.Clear();

  wdCommandHistory* pHistory = m_Context.m_pDocument->GetCommandHistory();

  const wdUInt32 iCount = (m_ButtonType == ButtonType::Undo) ? pHistory->GetUndoStackSize() : pHistory->GetRedoStackSize();
  for (wdUInt32 i = 0; i < iCount; i++)
  {
    const wdCommandTransaction* pTransaction = (m_ButtonType == ButtonType::Undo) ? pHistory->GetUndoStackEntry(i) : pHistory->GetRedoStackEntry(i);
    wdDynamicMenuAction::Item entryItem;
    entryItem.m_sDisplay = pTransaction->m_sDisplayString;
    entryItem.m_UserValue = (wdUInt32)i + 1; // Number of steps to undo / redo.
    out_entries.PushBack(entryItem);
  }
}

void wdCommandHistoryAction::Execute(const wdVariant& value)
{
  wdUInt32 iCount = value.IsValid() ? value.ConvertTo<wdUInt32>() : 1;

  switch (m_ButtonType)
  {
    case ButtonType::Undo:
    {
      WD_ASSERT_DEV(m_Context.m_pDocument->GetCommandHistory()->CanUndo(), "The action should not be active");

      auto stat = m_Context.m_pDocument->GetCommandHistory()->Undo(iCount);
      wdQtUiServices::MessageBoxStatus(stat, "Could not execute the Undo operation");
    }
    break;

    case ButtonType::Redo:
    {
      WD_ASSERT_DEV(m_Context.m_pDocument->GetCommandHistory()->CanRedo(), "The action should not be active");

      auto stat = m_Context.m_pDocument->GetCommandHistory()->Redo(iCount);
      wdQtUiServices::MessageBoxStatus(stat, "Could not execute the Redo operation");
    }
    break;
  }
}

void wdCommandHistoryAction::UpdateState()
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

void wdCommandHistoryAction::CommandHistoryEventHandler(const wdCommandHistoryEvent& e)
{
  UpdateState();
}
