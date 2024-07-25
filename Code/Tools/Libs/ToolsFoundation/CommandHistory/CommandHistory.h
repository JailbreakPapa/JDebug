#pragma once

#include <Foundation/Types/RefCounted.h>
#include <Foundation/Types/SharedPtr.h>
#include <ToolsFoundation/Command/Command.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsCommandHistory;

class NS_TOOLSFOUNDATION_DLL nsCommandTransaction : public nsCommand
{
  NS_ADD_DYNAMIC_REFLECTION(nsCommandTransaction, nsCommand);

public:
  nsCommandTransaction();
  ~nsCommandTransaction();

  nsString m_sDisplayString;

private:
  virtual nsStatus DoInternal(bool bRedo) override;
  virtual nsStatus UndoInternal(bool bFireEvents) override;
  virtual void CleanupInternal(CommandState state) override;
  nsStatus AddCommandTransaction(nsCommand* command);

private:
  friend class nsCommandHistory;
};

struct nsCommandHistoryEvent
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
  const nsDocument* m_pDocument;
};

/// \brief Stores the undo / redo stacks of transactions done on a document.
class NS_TOOLSFOUNDATION_DLL nsCommandHistory
{
public:
  nsEvent<const nsCommandHistoryEvent&, nsMutex> m_Events;

  // \brief Storage for the command history so it can be swapped when using multiple sub documents.
  class Storage : public nsRefCounted
  {
  public:
    nsHybridArray<nsCommandTransaction*, 4> m_TransactionStack;
    nsHybridArray<nsCommand*, 4> m_ActiveCommandStack;
    nsDeque<nsCommandTransaction*> m_UndoHistory;
    nsDeque<nsCommandTransaction*> m_RedoHistory;
    nsDocument* m_pDocument = nullptr;
    nsEvent<const nsCommandHistoryEvent&, nsMutex> m_Events;
  };

public:
  nsCommandHistory(nsDocument* pDocument);
  ~nsCommandHistory();

  const nsDocument* GetDocument() const { return m_pHistoryStorage->m_pDocument; }

  nsStatus Undo(nsUInt32 uiNumEntries = 1);
  nsStatus Redo(nsUInt32 uiNumEntries = 1);

  bool CanUndo() const;
  bool CanRedo() const;

  nsStringView GetUndoDisplayString() const;
  nsStringView GetRedoDisplayString() const;

  void StartTransaction(const nsFormatString& displayString);
  void CancelTransaction() { EndTransaction(true); }
  void FinishTransaction() { EndTransaction(false); }

  /// \brief Returns true, if between StartTransaction / EndTransaction. False during Undo/Redo.
  bool IsInTransaction() const { return !m_pHistoryStorage->m_TransactionStack.IsEmpty(); }
  bool IsInUndoRedo() const { return m_bIsInUndoRedo; }

  /// \brief Call this to start a series of transactions that typically change the same value over and over (e.g. dragging an object to a position).
  /// Every time a new transaction is started, the previous one is undone first. At the end of a series of temporary transactions, only the last
  /// transaction will be stored as a single undo step. Call this first and then start a transaction inside it.
  void BeginTemporaryCommands(nsStringView sDisplayString, bool bFireEventsWhenUndoingTempCommands = false);
  void CancelTemporaryCommands();
  void FinishTemporaryCommands();

  bool InTemporaryTransaction() const;
  void SuspendTemporaryTransaction();
  void ResumeTemporaryTransaction();

  nsStatus AddCommand(nsCommand& ref_command);

  void ClearUndoHistory();
  void ClearRedoHistory();

  void MergeLastTwoTransactions();

  nsUInt32 GetUndoStackSize() const;
  nsUInt32 GetRedoStackSize() const;
  const nsCommandTransaction* GetUndoStackEntry(nsUInt32 uiIndex) const;
  const nsCommandTransaction* GetRedoStackEntry(nsUInt32 uiIndex) const;

  nsSharedPtr<nsCommandHistory::Storage> SwapStorage(nsSharedPtr<nsCommandHistory::Storage> pNewStorage);
  nsSharedPtr<nsCommandHistory::Storage> GetStorage() { return m_pHistoryStorage; }

private:
  friend class nsCommand;

  nsStatus UndoInternal();
  nsStatus RedoInternal();

  void EndTransaction(bool bCancel);
  void EndTemporaryCommands(bool bCancel);

  nsSharedPtr<nsCommandHistory::Storage> m_pHistoryStorage;

  nsEvent<const nsCommandHistoryEvent&, nsMutex>::Unsubscriber m_EventsUnsubscriber;

  bool m_bFireEventsWhenUndoingTempCommands = false;
  bool m_bTemporaryMode = false;
  nsInt32 m_iTemporaryDepth = -1;
  nsInt32 m_iPreSuspendTemporaryDepth = -1;
  bool m_bIsInUndoRedo = false;
};
