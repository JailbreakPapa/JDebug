#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Document/Document.h>

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCommandTransaction, 1, nsRTTIDefaultAllocator<nsCommandTransaction>)
NS_END_DYNAMIC_REFLECTED_TYPE;

////////////////////////////////////////////////////////////////////////
// nsCommandTransaction
////////////////////////////////////////////////////////////////////////

nsCommandTransaction::nsCommandTransaction()
{
  // doesn't do anything on its own
  m_bModifiedDocument = false;
}

nsCommandTransaction::~nsCommandTransaction()
{
  NS_ASSERT_DEV(m_ChildActions.IsEmpty(), "The list should be cleared in 'Cleanup'");
}

nsStatus nsCommandTransaction::DoInternal(bool bRedo)
{
  NS_ASSERT_DEV(bRedo == true, "Implementation error");
  return nsStatus(NS_SUCCESS);
}

nsStatus nsCommandTransaction::UndoInternal(bool bFireEvents)
{
  return nsStatus(NS_SUCCESS);
}

void nsCommandTransaction::CleanupInternal(CommandState state) {}

nsStatus nsCommandTransaction::AddCommandTransaction(nsCommand* pCommand)
{
  pCommand->m_pDocument = m_pDocument;
  m_ChildActions.PushBack(pCommand);
  return nsStatus(NS_SUCCESS);
}

////////////////////////////////////////////////////////////////////////
// nsCommandHistory
////////////////////////////////////////////////////////////////////////

nsCommandHistory::nsCommandHistory(nsDocument* pDocument)
{
  auto pStorage = NS_DEFAULT_NEW(Storage);
  pStorage->m_pDocument = pDocument;
  SwapStorage(pStorage);

  m_bTemporaryMode = false;
  m_bIsInUndoRedo = false;
}

nsCommandHistory::~nsCommandHistory()
{
  if (m_pHistoryStorage->GetRefCount() == 1)
  {
    NS_ASSERT_ALWAYS(m_pHistoryStorage->m_UndoHistory.IsEmpty(), "Must clear history before destructor as object manager will be dead already");
    NS_ASSERT_ALWAYS(m_pHistoryStorage->m_RedoHistory.IsEmpty(), "Must clear history before destructor as object manager will be dead already");
  }
}

void nsCommandHistory::BeginTemporaryCommands(nsStringView sDisplayString, bool bFireEventsWhenUndoingTempCommands)
{
  NS_ASSERT_DEV(!m_bTemporaryMode, "Temporary Mode cannot be nested");
  StartTransaction(sDisplayString);
  StartTransaction("[Temporary]");

  m_bFireEventsWhenUndoingTempCommands = bFireEventsWhenUndoingTempCommands;
  m_bTemporaryMode = true;
  m_iTemporaryDepth = (nsInt32)m_pHistoryStorage->m_TransactionStack.GetCount();
}

void nsCommandHistory::CancelTemporaryCommands()
{
  EndTemporaryCommands(true);
  EndTransaction(true);
}

void nsCommandHistory::FinishTemporaryCommands()
{
  EndTemporaryCommands(false);
  EndTransaction(false);
}

bool nsCommandHistory::InTemporaryTransaction() const
{
  return m_bTemporaryMode;
}


void nsCommandHistory::SuspendTemporaryTransaction()
{
  m_iPreSuspendTemporaryDepth = (nsInt32)m_pHistoryStorage->m_TransactionStack.GetCount();
  NS_ASSERT_DEV(m_bTemporaryMode, "No temporary transaction active.");
  while (m_iTemporaryDepth < (nsInt32)m_pHistoryStorage->m_TransactionStack.GetCount())
  {
    EndTransaction(true);
  }
  EndTemporaryCommands(true);
}

void nsCommandHistory::ResumeTemporaryTransaction()
{
  NS_ASSERT_DEV(m_iTemporaryDepth == (nsInt32)m_pHistoryStorage->m_TransactionStack.GetCount() + 1, "Can't resume temporary, not before temporary depth.");
  while (m_iPreSuspendTemporaryDepth > (nsInt32)m_pHistoryStorage->m_TransactionStack.GetCount())
  {
    StartTransaction("[Temporary]");
  }
  m_bTemporaryMode = true;
  NS_ASSERT_DEV(m_iPreSuspendTemporaryDepth == (nsInt32)m_pHistoryStorage->m_TransactionStack.GetCount(), "");
}

void nsCommandHistory::EndTemporaryCommands(bool bCancel)
{
  NS_ASSERT_DEV(m_bTemporaryMode, "Temporary Mode was not enabled");
  NS_ASSERT_DEV(m_iTemporaryDepth == (nsInt32)m_pHistoryStorage->m_TransactionStack.GetCount(), "Transaction stack is at depth {0} but temporary is at {1}",
    m_pHistoryStorage->m_TransactionStack.GetCount(), m_iTemporaryDepth);
  m_bTemporaryMode = false;

  EndTransaction(bCancel);
}

nsStatus nsCommandHistory::UndoInternal()
{
  NS_ASSERT_DEV(!m_bIsInUndoRedo, "invalidly nested undo/redo");
  NS_ASSERT_DEV(m_pHistoryStorage->m_TransactionStack.IsEmpty(), "Can't undo with active transaction!");
  NS_ASSERT_DEV(!m_pHistoryStorage->m_UndoHistory.IsEmpty(), "Can't undo with empty undo queue!");

  m_bIsInUndoRedo = true;
  {
    nsCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = nsCommandHistoryEvent::Type::UndoStarted;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }

  nsCommandTransaction* pTransaction = m_pHistoryStorage->m_UndoHistory.PeekBack();

  nsStatus status = pTransaction->Undo(true);
  if (status.m_Result == NS_SUCCESS)
  {
    m_pHistoryStorage->m_UndoHistory.PopBack();
    m_pHistoryStorage->m_RedoHistory.PushBack(pTransaction);

    m_pHistoryStorage->m_pDocument->SetModified(true);

    status = nsStatus(NS_SUCCESS);
  }

  m_bIsInUndoRedo = false;
  {
    nsCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = nsCommandHistoryEvent::Type::UndoEnded;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }
  return status;
}

nsStatus nsCommandHistory::Undo(nsUInt32 uiNumEntries)
{
  for (nsUInt32 i = 0; i < uiNumEntries; i++)
  {
    NS_SUCCEED_OR_RETURN(UndoInternal());
  }

  return nsStatus(NS_SUCCESS);
}

nsStatus nsCommandHistory::RedoInternal()
{
  NS_ASSERT_DEV(!m_bIsInUndoRedo, "invalidly nested undo/redo");
  NS_ASSERT_DEV(m_pHistoryStorage->m_TransactionStack.IsEmpty(), "Can't redo with active transaction!");
  NS_ASSERT_DEV(!m_pHistoryStorage->m_RedoHistory.IsEmpty(), "Can't redo with empty undo queue!");

  m_bIsInUndoRedo = true;
  {
    nsCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = nsCommandHistoryEvent::Type::RedoStarted;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }

  nsCommandTransaction* pTransaction = m_pHistoryStorage->m_RedoHistory.PeekBack();

  nsStatus status(NS_FAILURE);
  if (pTransaction->Do(true).m_Result == NS_SUCCESS)
  {
    m_pHistoryStorage->m_RedoHistory.PopBack();
    m_pHistoryStorage->m_UndoHistory.PushBack(pTransaction);

    m_pHistoryStorage->m_pDocument->SetModified(true);

    status = nsStatus(NS_SUCCESS);
  }

  m_bIsInUndoRedo = false;
  {
    nsCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = nsCommandHistoryEvent::Type::RedoEnded;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }
  return status;
}

nsStatus nsCommandHistory::Redo(nsUInt32 uiNumEntries)
{
  for (nsUInt32 i = 0; i < uiNumEntries; i++)
  {
    NS_SUCCEED_OR_RETURN(RedoInternal());
  }

  return nsStatus(NS_SUCCESS);
}

bool nsCommandHistory::CanUndo() const
{
  if (!m_pHistoryStorage->m_TransactionStack.IsEmpty())
    return false;

  return !m_pHistoryStorage->m_UndoHistory.IsEmpty();
}

bool nsCommandHistory::CanRedo() const
{
  if (!m_pHistoryStorage->m_TransactionStack.IsEmpty())
    return false;

  return !m_pHistoryStorage->m_RedoHistory.IsEmpty();
}


nsStringView nsCommandHistory::GetUndoDisplayString() const
{
  if (m_pHistoryStorage->m_UndoHistory.IsEmpty())
    return "";

  return m_pHistoryStorage->m_UndoHistory.PeekBack()->m_sDisplayString;
}


nsStringView nsCommandHistory::GetRedoDisplayString() const
{
  if (m_pHistoryStorage->m_RedoHistory.IsEmpty())
    return "";

  return m_pHistoryStorage->m_RedoHistory.PeekBack()->m_sDisplayString;
}

void nsCommandHistory::StartTransaction(const nsFormatString& displayString)
{
  NS_ASSERT_DEV(!m_bIsInUndoRedo, "Cannot start new transaction while redoing/undoing.");

  /// \todo Allow to have a limited transaction history and clean up transactions after a while

  nsCommandTransaction* pTransaction;

  if (m_bTemporaryMode && !m_pHistoryStorage->m_TransactionStack.IsEmpty())
  {
    pTransaction = m_pHistoryStorage->m_TransactionStack.PeekBack();
    pTransaction->Undo(m_bFireEventsWhenUndoingTempCommands).IgnoreResult();
    pTransaction->Cleanup(nsCommand::CommandState::WasUndone);
    m_pHistoryStorage->m_TransactionStack.PushBack(pTransaction);
    m_pHistoryStorage->m_ActiveCommandStack.PushBack(pTransaction);
    return;
  }

  nsStringBuilder tmp;

  pTransaction = nsGetStaticRTTI<nsCommandTransaction>()->GetAllocator()->Allocate<nsCommandTransaction>();
  pTransaction->m_pDocument = m_pHistoryStorage->m_pDocument;
  pTransaction->m_sDisplayString = displayString.GetText(tmp);

  if (!m_pHistoryStorage->m_TransactionStack.IsEmpty())
  {
    // Stacked transaction
    m_pHistoryStorage->m_TransactionStack.PeekBack()->AddCommandTransaction(pTransaction).AssertSuccess();
    m_pHistoryStorage->m_TransactionStack.PushBack(pTransaction);
    m_pHistoryStorage->m_ActiveCommandStack.PushBack(pTransaction);
  }
  else
  {
    // Initial transaction
    m_pHistoryStorage->m_TransactionStack.PushBack(pTransaction);
    m_pHistoryStorage->m_ActiveCommandStack.PushBack(pTransaction);
    {
      nsCommandHistoryEvent e;
      e.m_pDocument = m_pHistoryStorage->m_pDocument;
      e.m_Type = nsCommandHistoryEvent::Type::TransactionStarted;
      m_pHistoryStorage->m_Events.Broadcast(e);
    }
  }
  return;
}

void nsCommandHistory::EndTransaction(bool bCancel)
{
  NS_ASSERT_DEV(!m_pHistoryStorage->m_TransactionStack.IsEmpty(), "Trying to end transaction without starting one!");

  if (m_pHistoryStorage->m_TransactionStack.GetCount() == 1)
  {
    /// Empty transactions are always canceled, so that they do not create an unnecessary undo action and clear the redo stack

    const bool bDidAnything = m_pHistoryStorage->m_TransactionStack.PeekBack()->HasChildActions();
    if (!bDidAnything)
      bCancel = true;

    nsCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = bCancel ? nsCommandHistoryEvent::Type::BeforeTransactionCanceled : nsCommandHistoryEvent::Type::BeforeTransactionEnded;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }

  if (!bCancel)
  {
    if (m_pHistoryStorage->m_TransactionStack.GetCount() > 1)
    {
      m_pHistoryStorage->m_TransactionStack.PopBack();
      m_pHistoryStorage->m_ActiveCommandStack.PopBack();
    }
    else
    {
      const bool bDidModifyDoc = m_pHistoryStorage->m_TransactionStack.PeekBack()->HasModifiedDocument();
      m_pHistoryStorage->m_UndoHistory.PushBack(m_pHistoryStorage->m_TransactionStack.PeekBack());
      m_pHistoryStorage->m_TransactionStack.PopBack();
      m_pHistoryStorage->m_ActiveCommandStack.PopBack();
      ClearRedoHistory();

      if (bDidModifyDoc)
      {
        m_pHistoryStorage->m_pDocument->SetModified(true);
      }
    }
  }
  else
  {
    nsCommandTransaction* pTransaction = m_pHistoryStorage->m_TransactionStack.PeekBack();

    pTransaction->Undo(true).AssertSuccess();
    m_pHistoryStorage->m_TransactionStack.PopBack();
    m_pHistoryStorage->m_ActiveCommandStack.PopBack();

    if (m_pHistoryStorage->m_TransactionStack.IsEmpty())
    {
      pTransaction->Cleanup(nsCommand::CommandState::WasUndone);
      pTransaction->GetDynamicRTTI()->GetAllocator()->Deallocate(pTransaction);
    }
  }

  if (m_pHistoryStorage->m_TransactionStack.IsEmpty())
  {
    // All transactions done
    nsCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = bCancel ? nsCommandHistoryEvent::Type::TransactionCanceled : nsCommandHistoryEvent::Type::TransactionEnded;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }
}

nsStatus nsCommandHistory::AddCommand(nsCommand& ref_command)
{
  NS_ASSERT_DEV(!m_pHistoryStorage->m_TransactionStack.IsEmpty(), "Cannot add command while no transaction is started");
  NS_ASSERT_DEV(!m_pHistoryStorage->m_ActiveCommandStack.IsEmpty(), "Transaction stack is not synced anymore with m_ActiveCommandStack");

  auto res = m_pHistoryStorage->m_ActiveCommandStack.PeekBack()->AddSubCommand(ref_command);

  // Error handling should be on the caller side.
  // if (res.Failed() && !res.m_sMessage.IsEmpty())
  //{
  //  nsLog::Error("Command failed: '{0}'", res.m_sMessage);
  //}

  return res;
}

void nsCommandHistory::ClearUndoHistory()
{
  NS_ASSERT_DEV(!m_bIsInUndoRedo, "Cannot clear undo/redo history while redoing/undoing.");
  while (!m_pHistoryStorage->m_UndoHistory.IsEmpty())
  {
    nsCommandTransaction* pTransaction = m_pHistoryStorage->m_UndoHistory.PeekBack();

    pTransaction->Cleanup(nsCommand::CommandState::WasDone);
    pTransaction->GetDynamicRTTI()->GetAllocator()->Deallocate(pTransaction);

    m_pHistoryStorage->m_UndoHistory.PopBack();
  }
}

void nsCommandHistory::ClearRedoHistory()
{
  NS_ASSERT_DEV(!m_bIsInUndoRedo, "Cannot clear undo/redo history while redoing/undoing.");
  while (!m_pHistoryStorage->m_RedoHistory.IsEmpty())
  {
    nsCommandTransaction* pTransaction = m_pHistoryStorage->m_RedoHistory.PeekBack();

    pTransaction->Cleanup(nsCommand::CommandState::WasUndone);
    pTransaction->GetDynamicRTTI()->GetAllocator()->Deallocate(pTransaction);

    m_pHistoryStorage->m_RedoHistory.PopBack();
  }
}

void nsCommandHistory::MergeLastTwoTransactions()
{
  /// \todo This would not be necessary, if hierarchical transactions would not crash

  NS_ASSERT_DEV(m_pHistoryStorage->m_RedoHistory.IsEmpty(), "This can only be called directly after EndTransaction, when the redo history is empty");
  NS_ASSERT_DEV(m_pHistoryStorage->m_UndoHistory.GetCount() >= 2, "Can only do this when at least two transcations are in the queue");

  nsCommandTransaction* pLast = m_pHistoryStorage->m_UndoHistory.PeekBack();
  m_pHistoryStorage->m_UndoHistory.PopBack();

  nsCommandTransaction* pNowLast = m_pHistoryStorage->m_UndoHistory.PeekBack();
  pNowLast->m_ChildActions.PushBackRange(pLast->m_ChildActions);

  pLast->m_ChildActions.Clear();

  pLast->GetDynamicRTTI()->GetAllocator()->Deallocate(pLast);
}

nsUInt32 nsCommandHistory::GetUndoStackSize() const
{
  return m_pHistoryStorage->m_UndoHistory.GetCount();
}

nsUInt32 nsCommandHistory::GetRedoStackSize() const
{
  return m_pHistoryStorage->m_RedoHistory.GetCount();
}

const nsCommandTransaction* nsCommandHistory::GetUndoStackEntry(nsUInt32 uiIndex) const
{
  return m_pHistoryStorage->m_UndoHistory[GetUndoStackSize() - 1 - uiIndex];
}

const nsCommandTransaction* nsCommandHistory::GetRedoStackEntry(nsUInt32 uiIndex) const
{
  return m_pHistoryStorage->m_RedoHistory[GetRedoStackSize() - 1 - uiIndex];
}

nsSharedPtr<nsCommandHistory::Storage> nsCommandHistory::SwapStorage(nsSharedPtr<nsCommandHistory::Storage> pNewStorage)
{
  NS_ASSERT_ALWAYS(pNewStorage != nullptr, "Need a valid history storage object");

  NS_ASSERT_DEV(!m_bIsInUndoRedo, "Can't be in Undo/Redo when swapping storage.");

  auto retVal = m_pHistoryStorage;

  m_EventsUnsubscriber.Unsubscribe();

  m_pHistoryStorage = pNewStorage;

  m_pHistoryStorage->m_Events.AddEventHandler([this](const nsCommandHistoryEvent& e)
    { m_Events.Broadcast(e); },
    m_EventsUnsubscriber);

  return retVal;
}
