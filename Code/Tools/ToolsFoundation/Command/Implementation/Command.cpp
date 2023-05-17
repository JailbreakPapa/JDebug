#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <ToolsFoundation/Command/Command.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Document/Document.h>

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCommand, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

wdCommand::wdCommand() = default;
wdCommand::~wdCommand() = default;

bool wdCommand::HasModifiedDocument() const
{
  if (m_bModifiedDocument)
    return true;

  for (const auto& ca : m_ChildActions)
  {
    if (ca->HasModifiedDocument())
      return true;
  }

  return false;
}

wdStatus wdCommand::Do(bool bRedo)
{
  wdStatus status = DoInternal(bRedo);
  if (status.m_Result == WD_FAILURE)
  {
    if (bRedo)
    {
      // A command that originally succeeded failed on redo!
      return status;
    }
    else
    {
      for (wdInt32 j = m_ChildActions.GetCount() - 1; j >= 0; --j)
      {
        wdStatus status2 = m_ChildActions[j]->Undo(true);
        WD_ASSERT_DEV(status2.m_Result == WD_SUCCESS, "Failed do could not be recovered! Inconsistent state!");
      }
      return status;
    }
  }
  if (!bRedo)
    return wdStatus(WD_SUCCESS);

  const wdUInt32 uiChildActions = m_ChildActions.GetCount();
  for (wdUInt32 i = 0; i < uiChildActions; ++i)
  {
    status = m_ChildActions[i]->Do(bRedo);
    if (status.m_Result == WD_FAILURE)
    {
      for (wdInt32 j = i - 1; j >= 0; --j)
      {
        wdStatus status2 = m_ChildActions[j]->Undo(true);
        WD_ASSERT_DEV(status2.m_Result == WD_SUCCESS, "Failed redo could not be recovered! Inconsistent state!");
      }
      // A command that originally succeeded failed on redo!
      return status;
    }
  }
  return wdStatus(WD_SUCCESS);
}

wdStatus wdCommand::Undo(bool bFireEvents)
{
  const wdUInt32 uiChildActions = m_ChildActions.GetCount();
  for (wdInt32 i = uiChildActions - 1; i >= 0; --i)
  {
    wdStatus status = m_ChildActions[i]->Undo(bFireEvents);
    if (status.m_Result == WD_FAILURE)
    {
      for (wdUInt32 j = i + 1; j < uiChildActions; ++j)
      {
        wdStatus status2 = m_ChildActions[j]->Do(true);
        WD_ASSERT_DEV(status2.m_Result == WD_SUCCESS, "Failed undo could not be recovered! Inconsistent state!");
      }
      // A command that originally succeeded failed on undo!
      return status;
    }
  }

  wdStatus status = UndoInternal(bFireEvents);
  if (status.m_Result == WD_FAILURE)
  {
    for (wdUInt32 j = 0; j < uiChildActions; ++j)
    {
      wdStatus status2 = m_ChildActions[j]->Do(true);
      WD_ASSERT_DEV(status2.m_Result == WD_SUCCESS, "Failed undo could not be recovered! Inconsistent state!");
    }
    // A command that originally succeeded failed on undo!
    return status;
  }

  return wdStatus(WD_SUCCESS);
}

void wdCommand::Cleanup(CommandState state)
{
  CleanupInternal(state);

  for (wdCommand* pCommand : m_ChildActions)
  {
    pCommand->Cleanup(state);
    pCommand->GetDynamicRTTI()->GetAllocator()->Deallocate(pCommand);
  }

  m_ChildActions.Clear();
}


wdStatus wdCommand::AddSubCommand(wdCommand& command)
{
  wdCommand* pCommand = wdReflectionSerializer::Clone(&command);
  const wdRTTI* pRtti = pCommand->GetDynamicRTTI();

  pCommand->m_pDocument = m_pDocument;

  m_ChildActions.PushBack(pCommand);
  m_pDocument->GetCommandHistory()->GetStorage()->m_ActiveCommandStack.PushBack(pCommand);
  wdStatus ret = pCommand->Do(false);
  m_pDocument->GetCommandHistory()->GetStorage()->m_ActiveCommandStack.PopBack();

  if (ret.m_Result == WD_FAILURE)
  {
    m_ChildActions.PopBack();
    pCommand->GetDynamicRTTI()->GetAllocator()->Deallocate(pCommand);
    return ret;
  }

  if (pCommand->HasReturnValues())
  {
    // Write properties back so any return values get written.
    wdDefaultMemoryStreamStorage storage;
    wdMemoryStreamWriter writer(&storage);
    wdMemoryStreamReader reader(&storage);

    wdReflectionSerializer::WriteObjectToBinary(writer, pCommand->GetDynamicRTTI(), pCommand);
    wdReflectionSerializer::ReadObjectPropertiesFromBinary(reader, *pRtti, &command);
  }

  return wdStatus(WD_SUCCESS);
}
