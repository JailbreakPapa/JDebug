#pragma once

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/Selection/SelectionManager.h>
///
class WD_GUIFOUNDATION_DLL wdEditActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(const char* szMapping, const char* szPath, bool bDeleteAction, bool bAdvancedPasteActions);
  static void MapContextMenuActions(const char* szMapping, const char* szPath);
  static void MapViewContextMenuActions(const char* szMapping, const char* szPath);

  static wdActionDescriptorHandle s_hEditCategory;
  static wdActionDescriptorHandle s_hCopy;
  static wdActionDescriptorHandle s_hPaste;
  static wdActionDescriptorHandle s_hPasteAsChild;
  static wdActionDescriptorHandle s_hPasteAtOriginalLocation;
  static wdActionDescriptorHandle s_hDelete;
};


///
class WD_GUIFOUNDATION_DLL wdEditAction : public wdButtonAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdEditAction, wdButtonAction);

public:
  enum class ButtonType
  {
    Copy,
    Paste,
    PasteAsChild,
    PasteAtOriginalLocation,
    Delete,
  };
  wdEditAction(const wdActionContext& context, const char* szName, ButtonType button);
  ~wdEditAction();

  virtual void Execute(const wdVariant& value) override;

private:
  void SelectionEventHandler(const wdSelectionManagerEvent& e);

  ButtonType m_ButtonType;
};
