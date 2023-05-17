#pragma once

#include <GuiFoundation/Action/BaseActions.h>
#include <GuiFoundation/GuiFoundationDLL.h>

///
class WD_GUIFOUNDATION_DLL wdDocumentActions
{
public:
  static void RegisterActions();
  static void UnregisterActions();

  static void MapActions(const char* szMapping, const char* szPath, bool bForToolbar);
  static void MapToolsActions(const char* szMapping, const char* szPath);

  static wdActionDescriptorHandle s_hSaveCategory;
  static wdActionDescriptorHandle s_hSave;
  static wdActionDescriptorHandle s_hSaveAs;
  static wdActionDescriptorHandle s_hSaveAll;

  static wdActionDescriptorHandle s_hCloseCategory;
  static wdActionDescriptorHandle s_hClose;
  static wdActionDescriptorHandle s_hOpenContainingFolder;
  static wdActionDescriptorHandle s_hCopyAssetGuid;

  static wdActionDescriptorHandle s_hUpdatePrefabs;
  static wdActionDescriptorHandle s_hDocumentCategory;
};


/// \brief Standard document actions.
class WD_GUIFOUNDATION_DLL wdDocumentAction : public wdButtonAction
{
  WD_ADD_DYNAMIC_REFLECTION(wdDocumentAction, wdButtonAction);

public:
  enum class ButtonType
  {
    Save,
    SaveAs,
    SaveAll,
    Close,
    OpenContainingFolder,
    UpdatePrefabs,
    CopyAssetGuid,
  };
  wdDocumentAction(const wdActionContext& context, const char* szName, ButtonType button);
  ~wdDocumentAction();

  virtual void Execute(const wdVariant& value) override;

private:
  void DocumentEventHandler(const wdDocumentEvent& e);

  ButtonType m_ButtonType;
};
