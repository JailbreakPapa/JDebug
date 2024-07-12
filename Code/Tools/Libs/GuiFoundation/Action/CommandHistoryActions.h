/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */
#pragma once

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/CommandHistory/CommandHistory.h>

///
class NS_GUIFOUNDATION_DLL nsCommandHistoryActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(nsStringView sMapping, nsStringView sTargetMenu = "G.Edit");

  static nsActionDescriptorHandle s_hCommandHistoryCategory;
  static nsActionDescriptorHandle s_hUndo;
  static nsActionDescriptorHandle s_hRedo;
};


///
class NS_GUIFOUNDATION_DLL nsCommandHistoryAction : public nsDynamicActionAndMenuAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsCommandHistoryAction, nsDynamicActionAndMenuAction);

public:
  enum class ButtonType
  {
    Undo,
    Redo,
  };

  nsCommandHistoryAction(const nsActionContext& context, const char* szName, ButtonType button);
  ~nsCommandHistoryAction();

  virtual void Execute(const nsVariant& value) override;
  virtual void GetEntries(nsHybridArray<nsDynamicMenuAction::Item, 16>& out_entries) override;

private:
  void UpdateState();
  void CommandHistoryEventHandler(const nsCommandHistoryEvent& e);

  ButtonType m_ButtonType;
};
