#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/OSFile.h>
#include <Foundation/Strings/TranslationLookup.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/DocumentActions.h>
#include <GuiFoundation/ContainerWindow/ContainerWindow.moc.h>
#include <GuiFoundation/DocumentWindow/DocumentWindow.moc.h>
#include <QClipboard>
#include <QFileDialog>
#include <QMimeData>
#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Project/ToolsProject.h>

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDocumentAction, 1, wdRTTINoAllocator)
  ;
WD_END_DYNAMIC_REFLECTED_TYPE;

////////////////////////////////////////////////////////////////////////
// wdDocumentActions
////////////////////////////////////////////////////////////////////////

wdActionDescriptorHandle wdDocumentActions::s_hSaveCategory;
wdActionDescriptorHandle wdDocumentActions::s_hSave;
wdActionDescriptorHandle wdDocumentActions::s_hSaveAs;
wdActionDescriptorHandle wdDocumentActions::s_hSaveAll;
wdActionDescriptorHandle wdDocumentActions::s_hCloseCategory;
wdActionDescriptorHandle wdDocumentActions::s_hClose;
wdActionDescriptorHandle wdDocumentActions::s_hOpenContainingFolder;
wdActionDescriptorHandle wdDocumentActions::s_hCopyAssetGuid;
wdActionDescriptorHandle wdDocumentActions::s_hUpdatePrefabs;
wdActionDescriptorHandle wdDocumentActions::s_hDocumentCategory;

void wdDocumentActions::RegisterActions()
{
  s_hSaveCategory = WD_REGISTER_CATEGORY("SaveCategory");
  s_hSave = WD_REGISTER_ACTION_1("Document.Save", wdActionScope::Document, "Document", "Ctrl+S", wdDocumentAction, wdDocumentAction::ButtonType::Save);
  s_hSaveAll = WD_REGISTER_ACTION_1("Document.SaveAll", wdActionScope::Document, "Document", "Ctrl+Shift+S", wdDocumentAction, wdDocumentAction::ButtonType::SaveAll);
  s_hSaveAs = WD_REGISTER_ACTION_1("Document.SaveAs", wdActionScope::Document, "Document", "", wdDocumentAction, wdDocumentAction::ButtonType::SaveAs);
  s_hCloseCategory = WD_REGISTER_CATEGORY("CloseCategory");
  s_hClose = WD_REGISTER_ACTION_1("Document.Close", wdActionScope::Document, "Document", "Ctrl+W", wdDocumentAction, wdDocumentAction::ButtonType::Close);
  s_hOpenContainingFolder = WD_REGISTER_ACTION_1("Document.OpenContainingFolder", wdActionScope::Document, "Document", "", wdDocumentAction, wdDocumentAction::ButtonType::OpenContainingFolder);
  s_hCopyAssetGuid = WD_REGISTER_ACTION_1("Document.CopyAssetGuid", wdActionScope::Document, "Document", "", wdDocumentAction, wdDocumentAction::ButtonType::CopyAssetGuid);
  s_hDocumentCategory = WD_REGISTER_CATEGORY("Tools.DocumentCategory");
  s_hUpdatePrefabs = WD_REGISTER_ACTION_1("Prefabs.UpdateAll", wdActionScope::Document, "Scene", "Ctrl+Shift+P", wdDocumentAction, wdDocumentAction::ButtonType::UpdatePrefabs);
}

void wdDocumentActions::UnregisterActions()
{
  wdActionManager::UnregisterAction(s_hSaveCategory);
  wdActionManager::UnregisterAction(s_hSave);
  wdActionManager::UnregisterAction(s_hSaveAs);
  wdActionManager::UnregisterAction(s_hSaveAll);
  wdActionManager::UnregisterAction(s_hCloseCategory);
  wdActionManager::UnregisterAction(s_hClose);
  wdActionManager::UnregisterAction(s_hOpenContainingFolder);
  wdActionManager::UnregisterAction(s_hCopyAssetGuid);
  wdActionManager::UnregisterAction(s_hDocumentCategory);
  wdActionManager::UnregisterAction(s_hUpdatePrefabs);
}

void wdDocumentActions::MapActions(const char* szMapping, const char* szPath, bool bForToolbar)
{
  wdActionMap* pMap = wdActionMapManager::GetActionMap(szMapping);
  WD_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the documents actions failed!", szMapping);

  pMap->MapAction(s_hSaveCategory, szPath, 1.0f);
  wdStringBuilder sSubPath(szPath, "/SaveCategory");

  pMap->MapAction(s_hSave, sSubPath, 1.0f);
  pMap->MapAction(s_hSaveAll, sSubPath, 3.0f);

  if (!bForToolbar)
  {
    pMap->MapAction(s_hSaveAs, sSubPath, 2.0f);

    sSubPath.Set(szPath, "/CloseCategory");
    pMap->MapAction(s_hCloseCategory, szPath, 2.0f);
    pMap->MapAction(s_hClose, sSubPath, 1.0f);
    pMap->MapAction(s_hCopyAssetGuid, sSubPath, 2.0f);
    pMap->MapAction(s_hOpenContainingFolder, sSubPath, 3.0f);
  }
}


void wdDocumentActions::MapToolsActions(const char* szMapping, const char* szPath)
{
  wdActionMap* pMap = wdActionMapManager::GetActionMap(szMapping);
  WD_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the documents actions failed!", szMapping);

  pMap->MapAction(s_hDocumentCategory, szPath, 1.0f);
  wdStringBuilder sSubPath(szPath, "/Tools.DocumentCategory");

  pMap->MapAction(s_hUpdatePrefabs, sSubPath, 1.0f);
}

////////////////////////////////////////////////////////////////////////
// wdDocumentAction
////////////////////////////////////////////////////////////////////////

wdDocumentAction::wdDocumentAction(const wdActionContext& context, const char* szName, ButtonType button)
  : wdButtonAction(context, szName, false, "")
{
  m_ButtonType = button;

  switch (m_ButtonType)
  {
    case wdDocumentAction::ButtonType::Save:
      SetIconPath(":/GuiFoundation/Icons/Save16.png");
      break;
    case wdDocumentAction::ButtonType::SaveAs:
      SetIconPath("");
      break;
    case wdDocumentAction::ButtonType::SaveAll:
      SetIconPath(":/GuiFoundation/Icons/SaveAll16.png");
      break;
    case wdDocumentAction::ButtonType::Close:
      SetIconPath("");
      break;
    case wdDocumentAction::ButtonType::OpenContainingFolder:
      SetIconPath(":/GuiFoundation/Icons/OpenFolder16.png");
      break;
    case wdDocumentAction::ButtonType::CopyAssetGuid:
      SetIconPath(":/GuiFoundation/Icons/DocumentGuid16.png");
      break;
    case wdDocumentAction::ButtonType::UpdatePrefabs:
      SetIconPath(":/EditorPluginScene/Icons/PrefabUpdate16.png");
      break;
  }

  if (context.m_pDocument == nullptr)
  {
    if (button == ButtonType::Save || button == ButtonType::SaveAs)
    {
      // for actions that require a document, hide them
      SetVisible(false);
    }
  }
  else
  {
    m_Context.m_pDocument->m_EventsOne.AddEventHandler(wdMakeDelegate(&wdDocumentAction::DocumentEventHandler, this));

    if (m_ButtonType == ButtonType::Save)
    {
      SetVisible(!m_Context.m_pDocument->IsReadOnly());
      SetEnabled(m_Context.m_pDocument->IsModified());
    }
  }
}

wdDocumentAction::~wdDocumentAction()
{
  if (m_Context.m_pDocument)
  {
    m_Context.m_pDocument->m_EventsOne.RemoveEventHandler(wdMakeDelegate(&wdDocumentAction::DocumentEventHandler, this));
  }
}

void wdDocumentAction::DocumentEventHandler(const wdDocumentEvent& e)
{
  switch (e.m_Type)
  {
    case wdDocumentEvent::Type::DocumentSaved:
    case wdDocumentEvent::Type::ModifiedChanged:
    {
      if (m_ButtonType == ButtonType::Save)
      {
        SetEnabled(m_Context.m_pDocument->IsModified());
      }
    }
    break;

    default:
      break;
  }
}

void wdDocumentAction::Execute(const wdVariant& value)
{
  switch (m_ButtonType)
  {
    case wdDocumentAction::ButtonType::Save:
    {
      wdQtDocumentWindow* pWnd = wdQtDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);
      pWnd->SaveDocument();
    }
    break;

    case wdDocumentAction::ButtonType::SaveAs:
    {
      wdQtDocumentWindow* pWnd = wdQtDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);
      if (pWnd->SaveDocument().Succeeded())
      {
        auto* desc = m_Context.m_pDocument->GetDocumentTypeDescriptor();
        wdStringBuilder sAllFilters;
        sAllFilters.Append(desc->m_sDocumentTypeName, " (*.", desc->m_sFileExtension, ")");
        QString sSelectedExt;
        wdString sFile = QFileDialog::getSaveFileName(QApplication::activeWindow(), QLatin1String("Create Document"),
          m_Context.m_pDocument->GetDocumentPath(), QString::fromUtf8(sAllFilters.GetData()), &sSelectedExt, QFileDialog::Option::DontResolveSymlinks)
                           .toUtf8()
                           .data();

        if (!sFile.IsEmpty())
        {
          wdUuid newDoc;
          newDoc.CreateNewUuid();
          wdStatus res = m_Context.m_pDocument->GetDocumentManager()->CloneDocument(m_Context.m_pDocument->GetDocumentPath(), sFile, newDoc);

          if (res.Failed())
          {
            wdStringBuilder s;
            s.Format("Failed to save document: \n'{0}'", sFile);
            wdQtUiServices::MessageBoxStatus(res, s);
          }
          else
          {
            const wdDocumentTypeDescriptor* pTypeDesc = nullptr;
            if (wdDocumentManager::FindDocumentTypeFromPath(sFile, false, pTypeDesc).Succeeded())
            {
              wdDocument* pDocument = nullptr;
              m_Context.m_pDocument->GetDocumentManager()->OpenDocument(pTypeDesc->m_sDocumentTypeName, sFile, pDocument);
            }
          }
        }
      }
    }
    break;

    case wdDocumentAction::ButtonType::SaveAll:
    {
      wdToolsProject::GetSingleton()->BroadcastSaveAll();
    }
    break;

    case wdDocumentAction::ButtonType::Close:
    {
      wdQtDocumentWindow* pWnd = wdQtDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);

      if (!pWnd->CanCloseWindow())
        return;

      pWnd->CloseDocumentWindow();
    }
    break;

    case wdDocumentAction::ButtonType::OpenContainingFolder:
    {
      wdString sPath;

      if (!m_Context.m_pDocument)
      {
        if (wdToolsProject::IsProjectOpen())
          sPath = wdToolsProject::GetSingleton()->GetProjectFile();
        else
          sPath = wdOSFile::GetApplicationDirectory();
      }
      else
        sPath = m_Context.m_pDocument->GetDocumentPath();

      wdQtUiServices::OpenInExplorer(sPath, true);
    }
    break;

    case wdDocumentAction::ButtonType::CopyAssetGuid:
    {
      wdStringBuilder sGuid;
      wdConversionUtils::ToString(m_Context.m_pDocument->GetGuid(), sGuid);

      QClipboard* clipboard = QApplication::clipboard();
      QMimeData* mimeData = new QMimeData();
      mimeData->setText(sGuid.GetData());
      clipboard->setMimeData(mimeData);

      wdQtUiServices::GetSingleton()->ShowAllDocumentsTemporaryStatusBarMessage(wdFmt("Copied asset GUID: {}", sGuid), wdTime::Seconds(5));
    }
    break;

    case wdDocumentAction::ButtonType::UpdatePrefabs:
      // TODO const cast
      const_cast<wdDocument*>(m_Context.m_pDocument)->UpdatePrefabs();
      return;
  }
}
