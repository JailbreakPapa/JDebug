#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/ReflectionSerializer.h>
#include <ToolsFoundation/Command/Command.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Document/Document.h>

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCommand, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsCommand::nsCommand() = default;
nsCommand::~nsCommand() = default;

bool nsCommand::HasModifiedDocument() const
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

nsStatus nsCommand::Do(bool bRedo)
{
  nsStatus status = DoInternal(bRedo);
  if (status.m_Result == NS_FAILURE)
  {
    if (bRedo)
    {
      // A command that originally succeeded failed on redo!
      return status;
    }
    else
    {
      for (nsInt32 j = m_ChildActions.GetCount() - 1; j >= 0; --j)
      {
        nsStatus status2 = m_ChildActions[j]->Undo(true);
        NS_ASSERT_DEV(status2.m_Result == NS_SUCCESS, "Failed do could not be recovered! Inconsistent state!");
      }
      return status;
    }
  }
  if (!bRedo)
    return nsStatus(NS_SUCCESS);

  const nsUInt32 uiChildActions = m_ChildActions.GetCount();
  for (nsUInt32 i = 0; i < uiChildActions; ++i)
  {
    status = m_ChildActions[i]->Do(bRedo);
    if (status.m_Result == NS_FAILURE)
    {
      for (nsInt32 j = i - 1; j >= 0; --j)
      {
        nsStatus status2 = m_ChildActions[j]->Undo(true);
        NS_ASSERT_DEV(status2.m_Result == NS_SUCCESS, "Failed redo could not be recovered! Inconsistent state!");
      }
      // A command that originally succeeded failed on redo!
      return status;
    }
  }
  return nsStatus(NS_SUCCESS);
}

nsStatus nsCommand::Undo(bool bFireEvents)
{
  const nsUInt32 uiChildActions = m_ChildActions.GetCount();
  for (nsInt32 i = uiChildActions - 1; i >= 0; --i)
  {
    nsStatus status = m_ChildActions[i]->Undo(bFireEvents);
    if (status.m_Result == NS_FAILURE)
    {
      for (nsUInt32 j = i + 1; j < uiChildActions; ++j)
      {
        nsStatus status2 = m_ChildActions[j]->Do(true);
        NS_ASSERT_DEV(status2.m_Result == NS_SUCCESS, "Failed undo could not be recovered! Inconsistent state!");
      }
      // A command that originally succeeded failed on undo!
      return status;
    }
  }

  nsStatus status = UndoInternal(bFireEvents);
  if (status.m_Result == NS_FAILURE)
  {
    for (nsUInt32 j = 0; j < uiChildActions; ++j)
    {
      nsStatus status2 = m_ChildActions[j]->Do(true);
      NS_ASSERT_DEV(status2.m_Result == NS_SUCCESS, "Failed undo could not be recovered! Inconsistent state!");
    }
    // A command that originally succeeded failed on undo!
    return status;
  }

  return nsStatus(NS_SUCCESS);
}

void nsCommand::Cleanup(CommandState state)
{
  CleanupInternal(state);

  for (nsCommand* pCommand : m_ChildActions)
  {
    pCommand->Cleanup(state);
    pCommand->GetDynamicRTTI()->GetAllocator()->Deallocate(pCommand);
  }

  m_ChildActions.Clear();
}


nsStatus nsCommand::AddSubCommand(nsCommand& command)
{
  nsCommand* pCommand = nsReflectionSerializer::Clone(&command);
  const nsRTTI* pRtti = pCommand->GetDynamicRTTI();

  pCommand->m_pDocument = m_pDocument;

  m_ChildActions.PushBack(pCommand);
  m_pDocument->GetCommandHistory()->GetStorage()->m_ActiveCommandStack.PushBack(pCommand);
  nsStatus ret = pCommand->Do(false);
  m_pDocument->GetCommandHistory()->GetStorage()->m_ActiveCommandStack.PopBack();

  if (ret.m_Result == NS_FAILURE)
  {
    m_ChildActions.PopBack();
    pCommand->GetDynamicRTTI()->GetAllocator()->Deallocate(pCommand);
    return ret;
  }

  if (pCommand->HasReturnValues())
  {
    // Write properties back so any return values get written.
    nsDefaultMemoryStreamStorage storage;
    nsMemoryStreamWriter writer(&storage);
    nsMemoryStreamReader reader(&storage);

    nsReflectionSerializer::WriteObjectToBinary(writer, pCommand->GetDynamicRTTI(), pCommand);
    nsReflectionSerializer::ReadObjectPropertiesFromBinary(reader, *pRtti, &command);
  }

  return nsStatus(NS_SUCCESS);
}
