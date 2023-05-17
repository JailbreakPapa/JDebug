#pragma once

#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/SharedPtr.h>
#include <ToolsFoundation/Command/Command.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdCommandHistory;

class WD_TOOLSFOUNDATION_DLL wdCommandTransaction : public wdCommand
{
  WD_ADD_DYNAMIC_REFLECTION(wdCommandTransaction, wdCommand);

public:
  wdCommandTransaction();
  ~wdCommandTransaction();

  wdString m_sDisplayString;

private:
  virtual wdStatus DoInternal(bool bRedo) override;
  virtual wdStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;
  wdStatus AddCommandTransaction(wdCommand* command);

private:
  friend class wdCommandHistory;
};

struct wdCommandHistoryEvent
{
  enum class Type
  {
    UndoStarted,
    UndoEnded,
    RedoStarted,
    RedoEnded,
    TransactionStarted,        ///< Emit after initial transaction started.
    BeforeTransactionEnded,    ///< Emit before initial transaction ended.
    BeforeTransactionCanceled, ///< Emit before initial transaction ended.
    TransactionEnded,          ///< Emit after initial transaction ended.
    TransactionCanceled,       ///< Emit after initial transaction canceled.
    HistoryChanged,
  };

  Type m_Type;
  const wdDocument* m_pDocument;
};

/// \brief Stores the undo / redo stacks of transactions done on a document.
class WD_TOOLSFOUNDATION_DLL wdCommandHistory
{
public:
  wdEvent<const wdCommandHistoryEvent&, wdMutex> m_Events;

  // \brief Storage for the command history so it can be swapped when using multiple sub documents.
  class Storage : public wdRefCounted
  {
  public:
    wdHybridArray<wdCommandTransaction*, 4> m_TransactionStack;
    wdHybridArray<wdCommand*, 4> m_ActiveCommandStack;
    wdDeque<wdCommandTransaction*> m_UndoHistory;
    wdDeque<wdCommandTransaction*> m_RedoHistory;
    wdDocument* m_pDocument = nullptr;
    wdEvent<const wdCommandHistoryEvent&, wdMutex> m_Events;
  };

public:
  wdCommandHistory(wdDocument* pDocument);
  ~wdCommandHistory();

  const wdDocument* GetDocument() const { return m_pHistoryStorage->m_pDocument; }

  wdStatus Undo(wdUInt32 uiNumEntries = 1);
  wdStatus Redo(wdUInt32 uiNumEntries = 1);

  bool CanUndo() const;
  bool CanRedo() const;

  const char* GetUndoDisplayString() const;
  const char* GetRedoDisplayString() const;

  void StartTransaction(const wdFormatString& displayString);
  void CancelTransaction() { EndTransaction(true); }
  void FinishTransaction() { EndTransaction(false); }

  /// \brief Returns true, if between StartTransaction / EndTransaction. False during Undo/Redo.
  bool IsInTransaction() const { return !m_pHistoryStorage->m_TransactionStack.IsEmpty(); }
  bool IsInUndoRedo() const { return m_bIsInUndoRedo; }

  /// \brief Call this to start a series of transactions that typically change the same value over and over (e.g. dragging an object to a position).
  /// Every time a new transaction is started, the previous one is undone first. At the end of a series of temporary transactions, only the last
  /// transaction will be stored as a single undo step. Call this first and then start a transaction inside it.
  void BeginTemporaryCommands(const char* szDisplayString, bool bFireEventsWhenUndoingTempCommands = false);
  void CancelTemporaryCommands();
  void FinishTemporaryCommands();

  bool InTemporaryTransaction() const;
  void SuspendTemporaryTransaction();
  void ResumeTemporaryTransaction();

  wdStatus AddCommand(wdCommand& ref_command);

  void ClearUndoHistory();
  void ClearRedoHistory();

  void MergeLastTwoTransactions();

  wdUInt32 GetUndoStackSize() const;
  wdUInt32 GetRedoStackSize() const;
  const wdCommandTransaction* GetUndoStackEntry(wdUInt32 uiIndex) const;
  const wdCommandTransaction* GetRedoStackEntry(wdUInt32 uiIndex) const;

  wdSharedPtr<wdCommandHistory::Storage> SwapStorage(wdSharedPtr<wdCommandHistory::Storage> pNewStorage);
  wdSharedPtr<wdCommandHistory::Storage> GetStorage() { return m_pHistoryStorage; }

private:
  friend class wdCommand;

  wdStatus UndoInternal();
  wdStatus RedoInternal();

  void EndTransaction(bool bCancel);
  void EndTemporaryCommands(bool bCancel);

  wdSharedPtr<wdCommandHistory::Storage> m_pHistoryStorage;

  wdEvent<const wdCommandHistoryEvent&, wdMutex>::Unsubscriber m_EventsUnsubscriber;

  bool m_bFireEventsWhenUndoingTempCommands = false;
  bool m_bTemporaryMode = false;
  wdInt32 m_iTemporaryDepth = -1;
  wdInt32 m_iPreSuspendTemporaryDepth = -1;
  bool m_bIsInUndoRedo = false;
};
