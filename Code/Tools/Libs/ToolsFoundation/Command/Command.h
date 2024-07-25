#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Status.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class nsDocument;
class nsCommandTransaction;

/// \brief Interface for a command
///
/// Commands are the only objects that have non-const access to any data structures (contexts, documents etc.).
/// Thus, any modification must go through a command and the nsCommandHistory is the only class capable of executing commands.
class NS_TOOLSFOUNDATION_DLL nsCommand : public nsReflectedClass
{
  NS_ADD_DYNAMIC_REFLECTION(nsCommand, nsReflectedClass);

public:
  nsCommand();
  ~nsCommand();

  bool IsUndoable() const { return m_bUndoable; };
  bool HasChildActions() const { return !m_ChildActions.IsEmpty(); }
  bool HasModifiedDocument() const;

  enum class CommandState
  {
    WasDone,
    WasUndone
  };

protected:
  nsStatus Do(bool bRedo);
  nsStatus Undo(bool bFireEvents);
  void Cleanup(CommandState state);

  nsStatus AddSubCommand(nsCommand& command);
  nsDocument* GetDocument() { return m_pDocument; };

private:
  virtual bool HasReturnValues() const { return false; }
  virtual nsStatus DoInternal(bool bRedo) = 0;
  virtual nsStatus UndoInternal(bool bFireEvents) = 0;
  virtual void CleanupInternal(CommandState state) = 0;

protected:
  friend class nsCommandHistory;
  friend class nsCommandTransaction;

  nsString m_sDescription;
  bool m_bUndoable = true;
  bool m_bModifiedDocument = true;
  nsHybridArray<nsCommand*, 8> m_ChildActions;
  nsDocument* m_pDocument = nullptr;
};
