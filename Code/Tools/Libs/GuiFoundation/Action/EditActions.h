#pragma once

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>
#include <ToolsFoundation/Selection/SelectionManager.h>
///
class NS_GUIFOUNDATION_DLL nsEditActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(nsStringView sMapping, bool bDeleteAction, bool bAdvancedPasteActions);
  static void MapContextMenuActions(nsStringView sMapping);
  static void MapViewContextMenuActions(nsStringView sMapping);

  static nsActionDescriptorHandle s_hEditCategory;
  static nsActionDescriptorHandle s_hCopy;
  static nsActionDescriptorHandle s_hPaste;
  static nsActionDescriptorHandle s_hPasteAsChild;
  static nsActionDescriptorHandle s_hPasteAtOriginalLocation;
  static nsActionDescriptorHandle s_hDelete;
};


///
class NS_GUIFOUNDATION_DLL nsEditAction : public nsButtonAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsEditAction, nsButtonAction);

public:
  enum class ButtonType
  {
    Copy,
    Paste,
    PasteAsChild,
    PasteAtOriginalLocation,
    Delete,
  };
  nsEditAction(const nsActionContext& context, const char* szName, ButtonType button);
  ~nsEditAction();

  virtual void Execute(const nsVariant& value) override;

private:
  void SelectionEventHandler(const nsSelectionManagerEvent& e);

  ButtonType m_ButtonType;
};
