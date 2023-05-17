#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <ToolsFoundation/CommandHistory/CommandHistory.h>
#include <ToolsFoundation/Document/Document.h>

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdCommandTransaction, 1, wdRTTIDefaultAllocator<wdCommandTransaction>)
WD_END_DYNAMIC_REFLECTED_TYPE;

////////////////////////////////////////////////////////////////////////
// wdCommandTransaction
////////////////////////////////////////////////////////////////////////

wdCommandTransaction::wdCommandTransaction()
{
  // doesn't do anything on its own
  m_bModifiedDocument = false;
}

wdCommandTransaction::~wdCommandTransaction()
{
  WD_ASSERT_DEV(m_ChildActions.IsEmpty(), "The list should be cleared in 'Cleanup'");
}

wdStatus wdCommandTransaction::DoInternal(bool bRedo)
{
  WD_ASSERT_DEV(bRedo == true, "Implementation error");
  return wdStatus(WD_SUCCESS);
}

wdStatus wdCommandTransaction::UndoInternal(bool bFireEvents)
{
  return wdStatus(WD_SUCCESS);
}

void wdCommandTransaction::CleanupInternal(CommandState state) {}

wdStatus wdCommandTransaction::AddCommandTransaction(wdCommand* pCommand)
{
  pCommand->m_pDocument = m_pDocument;
  m_ChildActions.PushBack(pCommand);
  return wdStatus(WD_SUCCESS);
}

////////////////////////////////////////////////////////////////////////
// wdCommandHistory
////////////////////////////////////////////////////////////////////////

wdCommandHistory::wdCommandHistory(wdDocument* pDocument)
{
  auto pStorage = WD_DEFAULT_NEW(Storage);
  pStorage->m_pDocument = pDocument;
  SwapStorage(pStorage);

  m_bTemporaryMode = false;
  m_bIsInUndoRedo = false;
}

wdCommandHistory::~wdCommandHistory()
{
  if (m_pHistoryStorage->GetRefCount() == 1)
  {
    WD_ASSERT_ALWAYS(m_pHistoryStorage->m_UndoHistory.IsEmpty(), "Must clear history before destructor as object manager will be dead already");
    WD_ASSERT_ALWAYS(m_pHistoryStorage->m_RedoHistory.IsEmpty(), "Must clear history before destructor as object manager will be dead already");
  }
}

void wdCommandHistory::BeginTemporaryCommands(const char* szDisplayString, bool bFireEventsWhenUndoingTempCommands)
{
  WD_ASSERT_DEV(!m_bTemporaryMode, "Temporary Mode cannot be nested");
  StartTransaction(szDisplayString);
  StartTransaction("[Temporary]");

  m_bFireEventsWhenUndoingTempCommands = bFireEventsWhenUndoingTempCommands;
  m_bTemporaryMode = true;
  m_iTemporaryDepth = (wdInt32)m_pHistoryStorage->m_TransactionStack.GetCount();
}

void wdCommandHistory::CancelTemporaryCommands()
{
  EndTemporaryCommands(true);
  EndTransaction(true);
}

void wdCommandHistory::FinishTemporaryCommands()
{
  EndTemporaryCommands(false);
  EndTransaction(false);
}

bool wdCommandHistory::InTemporaryTransaction() const
{
  return m_bTemporaryMode;
}


void wdCommandHistory::SuspendTemporaryTransaction()
{
  m_iPreSuspendTemporaryDepth = (wdInt32)m_pHistoryStorage->m_TransactionStack.GetCount();
  WD_ASSERT_DEV(m_bTemporaryMode, "No temporary transaction active.");
  while (m_iTemporaryDepth < (wdInt32)m_pHistoryStorage->m_TransactionStack.GetCount())
  {
    EndTransaction(true);
  }
  EndTemporaryCommands(true);
}

void wdCommandHistory::ResumeTemporaryTransaction()
{
  WD_ASSERT_DEV(m_iTemporaryDepth == (wdInt32)m_pHistoryStorage->m_TransactionStack.GetCount() + 1, "Can't resume temporary, not before temporary depth.");
  while (m_iPreSuspendTemporaryDepth > (wdInt32)m_pHistoryStorage->m_TransactionStack.GetCount())
  {
    StartTransaction("[Temporary]");
  }
  m_bTemporaryMode = true;
  WD_ASSERT_DEV(m_iPreSuspendTemporaryDepth == (wdInt32)m_pHistoryStorage->m_TransactionStack.GetCount(), "");
}

void wdCommandHistory::EndTemporaryCommands(bool bCancel)
{
  WD_ASSERT_DEV(m_bTemporaryMode, "Temporary Mode was not enabled");
  WD_ASSERT_DEV(m_iTemporaryDepth == (wdInt32)m_pHistoryStorage->m_TransactionStack.GetCount(), "Transaction stack is at depth {0} but temporary is at {1}",
    m_pHistoryStorage->m_TransactionStack.GetCount(), m_iTemporaryDepth);
  m_bTemporaryMode = false;

  EndTransaction(bCancel);
}

wdStatus wdCommandHistory::UndoInternal()
{
  WD_ASSERT_DEV(!m_bIsInUndoRedo, "invalidly nested undo/redo");
  WD_ASSERT_DEV(m_pHistoryStorage->m_TransactionStack.IsEmpty(), "Can't undo with active transaction!");
  WD_ASSERT_DEV(!m_pHistoryStorage->m_UndoHistory.IsEmpty(), "Can't undo with empty undo queue!");

  m_bIsInUndoRedo = true;
  {
    wdCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = wdCommandHistoryEvent::Type::UndoStarted;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }

  wdCommandTransaction* pTransaction = m_pHistoryStorage->m_UndoHistory.PeekBack();

  wdStatus status = pTransaction->Undo(true);
  if (status.m_Result == WD_SUCCESS)
  {
    m_pHistoryStorage->m_UndoHistory.PopBack();
    m_pHistoryStorage->m_RedoHistory.PushBack(pTransaction);

    m_pHistoryStorage->m_pDocument->SetModified(true);

    status = wdStatus(WD_SUCCESS);
  }

  m_bIsInUndoRedo = false;
  {
    wdCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = wdCommandHistoryEvent::Type::UndoEnded;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }
  return status;
}

wdStatus wdCommandHistory::Undo(wdUInt32 uiNumEntries)
{
  for (wdUInt32 i = 0; i < uiNumEntries; i++)
  {
    WD_SUCCEED_OR_RETURN(UndoInternal());
  }

  return wdStatus(WD_SUCCESS);
}

wdStatus wdCommandHistory::RedoInternal()
{
  WD_ASSERT_DEV(!m_bIsInUndoRedo, "invalidly nested undo/redo");
  WD_ASSERT_DEV(m_pHistoryStorage->m_TransactionStack.IsEmpty(), "Can't redo with active transaction!");
  WD_ASSERT_DEV(!m_pHistoryStorage->m_RedoHistory.IsEmpty(), "Can't redo with empty undo queue!");

  m_bIsInUndoRedo = true;
  {
    wdCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = wdCommandHistoryEvent::Type::RedoStarted;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }

  wdCommandTransaction* pTransaction = m_pHistoryStorage->m_RedoHistory.PeekBack();

  wdStatus status(WD_FAILURE);
  if (pTransaction->Do(true).m_Result == WD_SUCCESS)
  {
    m_pHistoryStorage->m_RedoHistory.PopBack();
    m_pHistoryStorage->m_UndoHistory.PushBack(pTransaction);

    m_pHistoryStorage->m_pDocument->SetModified(true);

    status = wdStatus(WD_SUCCESS);
  }

  m_bIsInUndoRedo = false;
  {
    wdCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = wdCommandHistoryEvent::Type::RedoEnded;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }
  return status;
}

wdStatus wdCommandHistory::Redo(wdUInt32 uiNumEntries)
{
  for (wdUInt32 i = 0; i < uiNumEntries; i++)
  {
    WD_SUCCEED_OR_RETURN(RedoInternal());
  }

  return wdStatus(WD_SUCCESS);
}

bool wdCommandHistory::CanUndo() const
{
  if (!m_pHistoryStorage->m_TransactionStack.IsEmpty())
    return false;

  return !m_pHistoryStorage->m_UndoHistory.IsEmpty();
}

bool wdCommandHistory::CanRedo() const
{
  if (!m_pHistoryStorage->m_TransactionStack.IsEmpty())
    return false;

  return !m_pHistoryStorage->m_RedoHistory.IsEmpty();
}


const char* wdCommandHistory::GetUndoDisplayString() const
{
  if (m_pHistoryStorage->m_UndoHistory.IsEmpty())
    return "";

  return m_pHistoryStorage->m_UndoHistory.PeekBack()->m_sDisplayString;
}


const char* wdCommandHistory::GetRedoDisplayString() const
{
  if (m_pHistoryStorage->m_RedoHistory.IsEmpty())
    return "";

  return m_pHistoryStorage->m_RedoHistory.PeekBack()->m_sDisplayString;
}

void wdCommandHistory::StartTransaction(const wdFormatString& displayString)
{
  WD_ASSERT_DEV(!m_bIsInUndoRedo, "Cannot start new transaction while redoing/undoing.");

  /// \todo Allow to have a limited transaction history and clean up transactions after a while

  wdCommandTransaction* pTransaction;

  if (m_bTemporaryMode && !m_pHistoryStorage->m_TransactionStack.IsEmpty())
  {
    pTransaction = m_pHistoryStorage->m_TransactionStack.PeekBack();
    pTransaction->Undo(m_bFireEventsWhenUndoingTempCommands);
    pTransaction->Cleanup(wdCommand::CommandState::WasUndone);
    m_pHistoryStorage->m_TransactionStack.PushBack(pTransaction);
    m_pHistoryStorage->m_ActiveCommandStack.PushBack(pTransaction);
    return;
  }

  wdStringBuilder tmp;

  pTransaction = wdGetStaticRTTI<wdCommandTransaction>()->GetAllocator()->Allocate<wdCommandTransaction>();
  pTransaction->m_pDocument = m_pHistoryStorage->m_pDocument;
  pTransaction->m_sDisplayString = displayString.GetText(tmp);

  if (!m_pHistoryStorage->m_TransactionStack.IsEmpty())
  {
    // Stacked transaction
    m_pHistoryStorage->m_TransactionStack.PeekBack()->AddCommandTransaction(pTransaction);
    m_pHistoryStorage->m_TransactionStack.PushBack(pTransaction);
    m_pHistoryStorage->m_ActiveCommandStack.PushBack(pTransaction);
  }
  else
  {
    // Initial transaction
    m_pHistoryStorage->m_TransactionStack.PushBack(pTransaction);
    m_pHistoryStorage->m_ActiveCommandStack.PushBack(pTransaction);
    {
      wdCommandHistoryEvent e;
      e.m_pDocument = m_pHistoryStorage->m_pDocument;
      e.m_Type = wdCommandHistoryEvent::Type::TransactionStarted;
      m_pHistoryStorage->m_Events.Broadcast(e);
    }
  }
  return;
}

void wdCommandHistory::EndTransaction(bool bCancel)
{
  WD_ASSERT_DEV(!m_pHistoryStorage->m_TransactionStack.IsEmpty(), "Trying to end transaction without starting one!");

  if (m_pHistoryStorage->m_TransactionStack.GetCount() == 1)
  {
    /// Empty transactions are always canceled, so that they do not create an unnecessary undo action and clear the redo stack

    const bool bDidAnything = m_pHistoryStorage->m_TransactionStack.PeekBack()->HasChildActions();
    if (!bDidAnything)
      bCancel = true;

    wdCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = bCancel ? wdCommandHistoryEvent::Type::BeforeTransactionCanceled : wdCommandHistoryEvent::Type::BeforeTransactionEnded;
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
    wdCommandTransaction* pTransaction = m_pHistoryStorage->m_TransactionStack.PeekBack();

    pTransaction->Undo(true);
    m_pHistoryStorage->m_TransactionStack.PopBack();
    m_pHistoryStorage->m_ActiveCommandStack.PopBack();

    if (m_pHistoryStorage->m_TransactionStack.IsEmpty())
    {
      pTransaction->Cleanup(wdCommand::CommandState::WasUndone);
      pTransaction->GetDynamicRTTI()->GetAllocator()->Deallocate(pTransaction);
    }
  }

  if (m_pHistoryStorage->m_TransactionStack.IsEmpty())
  {
    // All transactions done
    wdCommandHistoryEvent e;
    e.m_pDocument = m_pHistoryStorage->m_pDocument;
    e.m_Type = bCancel ? wdCommandHistoryEvent::Type::TransactionCanceled : wdCommandHistoryEvent::Type::TransactionEnded;
    m_pHistoryStorage->m_Events.Broadcast(e);
  }
}

wdStatus wdCommandHistory::AddCommand(wdCommand& ref_command)
{
  WD_ASSERT_DEV(!m_pHistoryStorage->m_TransactionStack.IsEmpty(), "Cannot add command while no transaction is started");
  WD_ASSERT_DEV(!m_pHistoryStorage->m_ActiveCommandStack.IsEmpty(), "Transaction stack is not synced anymore with m_ActiveCommandStack");

  auto res = m_pHistoryStorage->m_ActiveCommandStack.PeekBack()->AddSubCommand(ref_command);

  // Error handling should be on the caller side.
  // if (res.Failed() && !res.m_sMessage.IsEmpty())
  //{
  //  wdLog::Error("Command failed: '{0}'", res.m_sMessage);
  //}

  return res;
}

void wdCommandHistory::ClearUndoHistory()
{
  WD_ASSERT_DEV(!m_bIsInUndoRedo, "Cannot clear undo/redo history while redoing/undoing.");
  while (!m_pHistoryStorage->m_UndoHistory.IsEmpty())
  {
    wdCommandTransaction* pTransaction = m_pHistoryStorage->m_UndoHistory.PeekBack();

    pTransaction->Cleanup(wdCommand::CommandState::WasDone);
    pTransaction->GetDynamicRTTI()->GetAllocator()->Deallocate(pTransaction);

    m_pHistoryStorage->m_UndoHistory.PopBack();
  }
}

void wdCommandHistory::ClearRedoHistory()
{
  WD_ASSERT_DEV(!m_bIsInUndoRedo, "Cannot clear undo/redo history while redoing/undoing.");
  while (!m_pHistoryStorage->m_RedoHistory.IsEmpty())
  {
    wdCommandTransaction* pTransaction = m_pHistoryStorage->m_RedoHistory.PeekBack();

    pTransaction->Cleanup(wdCommand::CommandState::WasUndone);
    pTransaction->GetDynamicRTTI()->GetAllocator()->Deallocate(pTransaction);

    m_pHistoryStorage->m_RedoHistory.PopBack();
  }
}

void wdCommandHistory::MergeLastTwoTransactions()
{
  /// \todo This would not be necessary, if hierarchical transactions would not crash

  WD_ASSERT_DEV(m_pHistoryStorage->m_RedoHistory.IsEmpty(), "This can only be called directly after EndTransaction, when the redo history is empty");
  WD_ASSERT_DEV(m_pHistoryStorage->m_UndoHistory.GetCount() >= 2, "Can only do this when at least two transcations are in the queue");

  wdCommandTransaction* pLast = m_pHistoryStorage->m_UndoHistory.PeekBack();
  m_pHistoryStorage->m_UndoHistory.PopBack();

  wdCommandTransaction* pNowLast = m_pHistoryStorage->m_UndoHistory.PeekBack();
  pNowLast->m_ChildActions.PushBackRange(pLast->m_ChildActions);

  pLast->m_ChildActions.Clear();

  pLast->GetDynamicRTTI()->GetAllocator()->Deallocate(pLast);
}

wdUInt32 wdCommandHistory::GetUndoStackSize() const
{
  return m_pHistoryStorage->m_UndoHistory.GetCount();
}

wdUInt32 wdCommandHistory::GetRedoStackSize() const
{
  return m_pHistoryStorage->m_RedoHistory.GetCount();
}

const wdCommandTransaction* wdCommandHistory::GetUndoStackEntry(wdUInt32 uiIndex) const
{
  return m_pHistoryStorage->m_UndoHistory[GetUndoStackSize() - 1 - uiIndex];
}

const wdCommandTransaction* wdCommandHistory::GetRedoStackEntry(wdUInt32 uiIndex) const
{
  return m_pHistoryStorage->m_RedoHistory[GetRedoStackSize() - 1 - uiIndex];
}

wdSharedPtr<wdCommandHistory::Storage> wdCommandHistory::SwapStorage(wdSharedPtr<wdCommandHistory::Storage> pNewStorage)
{
  WD_ASSERT_ALWAYS(pNewStorage != nullptr, "Need a valid history storage object");

  WD_ASSERT_DEV(!m_bIsInUndoRedo, "Can't be in Undo/Redo when swapping storage.");

  auto retVal = m_pHistoryStorage;

  m_EventsUnsubscriber.Unsubscribe();

  m_pHistoryStorage = pNewStorage;

  m_pHistoryStorage->m_Events.AddEventHandler([this](const wdCommandHistoryEvent& e) { m_Events.Broadcast(e); }, m_EventsUnsubscriber);

  return retVal;
}
