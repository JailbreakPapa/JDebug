#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Types/Status.h>
#include <ToolsFoundation/ToolsFoundationDLL.h>

class wdDocument;
class wdCommandTransaction;

/// \brief Interface for a command
///
/// Commands are the only objects that have non-const access to any data structures (contexts, documents etc.).
/// Thus, any modification must go through a command and the wdCommandHistory is the only class capable of executing commands.
class WD_TOOLSFOUNDATION_DLL wdCommand : public wdReflectedClass
{
  WD_ADD_DYNAMIC_REFLECTION(wdCommand, wdReflectedClass);

public:
  wdCommand();
  ~wdCommand();

  bool IsUndoable() const { return m_bUndoable; };
  bool HasChildActions() const { return !m_ChildActions.IsEmpty(); }
  bool HasModifiedDocument() const;

  enum class CommandState
  {
    WasDone,
    WasUndone
  };

protected:
  wdStatus Do(bool bRedo);
  wdStatus Undo(bool bFireEvents);
  void Cleanup(CommandState state);

  wdStatus AddSubCommand(wdCommand& command);
  wdDocument* GetDocument() { return m_pDocument; };

private:
  virtual bool HasReturnValues() const { return false; }
  virtual wdStatus DoInternal(bool bRedo) = 0;
  virtual wdStatus UndoInternal(bool bFireEvents) = 0;
  virtual void CleanupInternal(CommandState state) = 0;

protected:
  friend class wdCommandHistory;
  friend class wdCommandTransaction;

  wdString m_sDescription;
  bool m_bUndoable = true;
  bool m_bModifiedDocument = true;
  wdHybridArray<wdCommand*, 8> m_ChildActions;
  wdDocument* m_pDocument = nullptr;
};
