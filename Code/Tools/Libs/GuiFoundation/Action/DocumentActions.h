#pragma once

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

///
class NS_GUIFOUNDATION_DLL nsDocumentActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapMenuActions(nsStringView sMapping, nsStringView sTargetMenu = "G.File.Common");
  static void MapToolbarActions(nsStringView sMapping);
  static void MapToolsActions(nsStringView sMapping);

  static nsActionDescriptorHandle s_hSaveCategory;
  static nsActionDescriptorHandle s_hSave;
  static nsActionDescriptorHandle s_hSaveAs;
  static nsActionDescriptorHandle s_hSaveAll;

  static nsActionDescriptorHandle s_hClose;
  static nsActionDescriptorHandle s_hCloseAll;
  static nsActionDescriptorHandle s_hCloseAllButThis;

  static nsActionDescriptorHandle s_hOpenContainingFolder;
  static nsActionDescriptorHandle s_hCopyAssetGuid;

  static nsActionDescriptorHandle s_hUpdatePrefabs;
};


/// \brief Standard document actions.
class NS_GUIFOUNDATION_DLL nsDocumentAction : public nsButtonAction
{
  NS_ADD_DYNAMIC_REFLECTION(nsDocumentAction, nsButtonAction);

public:
  enum class ButtonType
  {
    Save,
    SaveAs,
    SaveAll,
    Close,
    CloseAll,
    CloseAllButThis,
    OpenContainingFolder,
    UpdatePrefabs,
    CopyAssetGuid,
  };
  nsDocumentAction(const nsActionContext& context, const char* szName, ButtonType button);
  ~nsDocumentAction();

  virtual void Execute(const nsVariant& value) override;

private:
  void DocumentEventHandler(const nsDocumentEvent& e);

  ButtonType m_ButtonType;
};
