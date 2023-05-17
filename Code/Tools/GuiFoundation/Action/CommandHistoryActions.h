#pragma once

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>

///
class WD_GUIFOUNDATION_DLL wdCommandHistoryActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(const char* szMapping, const char* szPath);

  static wdActionDescriptorHandle s_hCommandHistoryCategory;
  static wdActionDescriptorHandle s_hUndo;
  static wdActionDescriptorHandle s_hRedo;
};


///
class WD_GUIFOUNDATION_DLL wdCommandHistoryAction : public wdDynamicActionAndMenuAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdCommandHistoryAction, wdDynamicActionAndMenuAction);

public:
  enum class ButtonType
  {
    Undo,
    Redo,
  };

  wdCommandHistoryAction(const wdActionContext& context, const char* szName, ButtonType button);
  ~wdCommandHistoryAction();

  virtual void Execute(const wdVariant& value) override;
  virtual void GetEntries(wdHybridArray<wdDynamicMenuAction::Item, 16>& out_entries) override;

private:
  void UpdateState();
  void CommandHistoryEventHandler(const wdCommandHistoryEvent& e);

  ButtonType m_ButtonType;
};
