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

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDocumentAction, 1, nsRTTINoAllocator)
  ;
NS_END_DYNAMIC_REFLECTED_TYPE;

////////////////////////////////////////////////////////////////////////
// nsDocumentActions
////////////////////////////////////////////////////////////////////////

nsActionDescriptorHandle nsDocumentActions::s_hSaveCategory;
nsActionDescriptorHandle nsDocumentActions::s_hSave;
nsActionDescriptorHandle nsDocumentActions::s_hSaveAs;
nsActionDescriptorHandle nsDocumentActions::s_hSaveAll;
nsActionDescriptorHandle nsDocumentActions::s_hClose;
nsActionDescriptorHandle nsDocumentActions::s_hCloseAll;
nsActionDescriptorHandle nsDocumentActions::s_hCloseAllButThis;
nsActionDescriptorHandle nsDocumentActions::s_hOpenContainingFolder;
nsActionDescriptorHandle nsDocumentActions::s_hCopyAssetGuid;
nsActionDescriptorHandle nsDocumentActions::s_hUpdatePrefabs;

void nsDocumentActions::RegisterActions()
{
  s_hSaveCategory = NS_REGISTER_CATEGORY("SaveCategory");
  s_hSave = NS_REGISTER_ACTION_1("Document.Save", nsActionScope::Document, "Document", "Ctrl+S", nsDocumentAction, nsDocumentAction::ButtonType::Save);
  s_hSaveAll = NS_REGISTER_ACTION_1("Document.SaveAll", nsActionScope::Document, "Document", "Ctrl+Shift+S", nsDocumentAction, nsDocumentAction::ButtonType::SaveAll);
  s_hSaveAs = NS_REGISTER_ACTION_1("Document.SaveAs", nsActionScope::Document, "Document", "", nsDocumentAction, nsDocumentAction::ButtonType::SaveAs);
  s_hClose = NS_REGISTER_ACTION_1("Document.Close", nsActionScope::Document, "Document", "Ctrl+W", nsDocumentAction, nsDocumentAction::ButtonType::Close);
  s_hCloseAll = NS_REGISTER_ACTION_1("Document.CloseAll", nsActionScope::Document, "Document", "Ctrl+Shift+W", nsDocumentAction, nsDocumentAction::ButtonType::CloseAll);
  s_hCloseAllButThis = NS_REGISTER_ACTION_1("Document.CloseAllButThis", nsActionScope::Document, "Document", "Shift+Alt+W", nsDocumentAction, nsDocumentAction::ButtonType::CloseAllButThis);
  s_hOpenContainingFolder = NS_REGISTER_ACTION_1("Document.OpenContainingFolder", nsActionScope::Document, "Document", "", nsDocumentAction, nsDocumentAction::ButtonType::OpenContainingFolder);
  s_hCopyAssetGuid = NS_REGISTER_ACTION_1("Document.CopyAssetGuid", nsActionScope::Document, "Document", "", nsDocumentAction, nsDocumentAction::ButtonType::CopyAssetGuid);
  s_hUpdatePrefabs = NS_REGISTER_ACTION_1("Prefabs.UpdateAll", nsActionScope::Document, "Scene", "Ctrl+Shift+P", nsDocumentAction, nsDocumentAction::ButtonType::UpdatePrefabs);
}

void nsDocumentActions::UnregisterActions()
{
  nsActionManager::UnregisterAction(s_hSaveCategory);
  nsActionManager::UnregisterAction(s_hSave);
  nsActionManager::UnregisterAction(s_hSaveAs);
  nsActionManager::UnregisterAction(s_hSaveAll);
  nsActionManager::UnregisterAction(s_hClose);
  nsActionManager::UnregisterAction(s_hCloseAll);
  nsActionManager::UnregisterAction(s_hCloseAllButThis);
  nsActionManager::UnregisterAction(s_hOpenContainingFolder);
  nsActionManager::UnregisterAction(s_hCopyAssetGuid);
  nsActionManager::UnregisterAction(s_hUpdatePrefabs);
}

void nsDocumentActions::MapMenuActions(nsStringView sMapping, nsStringView sTargetMenu)
{
  nsActionMap* pMap = nsActionMapManager::GetActionMap(sMapping);
  NS_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the documents actions failed!", sMapping);

  pMap->MapAction(s_hSave, sTargetMenu, 5.0f);
  pMap->MapAction(s_hSaveAs, sTargetMenu, 6.0f);
  pMap->MapAction(s_hSaveAll, sTargetMenu, 7.0f);
  pMap->MapAction(s_hClose, sTargetMenu, 8.0f);
  pMap->MapAction(s_hCloseAll, sTargetMenu, 9.0f);
  pMap->MapAction(s_hCloseAllButThis, sTargetMenu, 10.0f);
  pMap->MapAction(s_hOpenContainingFolder, sTargetMenu, 11.0f);

  pMap->MapAction(s_hCopyAssetGuid, sTargetMenu, 12.0f);
}

void nsDocumentActions::MapToolbarActions(nsStringView sMapping)
{
  nsActionMap* pMap = nsActionMapManager::GetActionMap(sMapping);
  NS_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the documents actions failed!", sMapping);

  pMap->MapAction(s_hSaveCategory, "", 1.0f);
  nsStringView sSubPath = "SaveCategory";

  pMap->MapAction(s_hSave, sSubPath, 1.0f);
  pMap->MapAction(s_hSaveAll, sSubPath, 3.0f);
}


void nsDocumentActions::MapToolsActions(nsStringView sMapping)
{
  nsActionMap* pMap = nsActionMapManager::GetActionMap(sMapping);
  NS_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the documents actions failed!", sMapping);

  pMap->MapAction(s_hUpdatePrefabs, "G.Tools.Document", 1.0f);
}

////////////////////////////////////////////////////////////////////////
// nsDocumentAction
////////////////////////////////////////////////////////////////////////

nsDocumentAction::nsDocumentAction(const nsActionContext& context, const char* szName, ButtonType button)
  : nsButtonAction(context, szName, false, "")
{
  m_ButtonType = button;

  switch (m_ButtonType)
  {
    case nsDocumentAction::ButtonType::Save:
      SetIconPath(":/GuiFoundation/Icons/Save.svg");
      break;
    case nsDocumentAction::ButtonType::SaveAs:
      SetIconPath("");
      break;
    case nsDocumentAction::ButtonType::SaveAll:
      SetIconPath(":/GuiFoundation/Icons/SaveAll.svg");
      break;
    case nsDocumentAction::ButtonType::Close:
      SetIconPath("");
      break;
    case nsDocumentAction::ButtonType::CloseAll:
      SetIconPath("");
      break;
    case nsDocumentAction::ButtonType::CloseAllButThis:
      SetIconPath("");
      break;
    case nsDocumentAction::ButtonType::OpenContainingFolder:
      SetIconPath(":/GuiFoundation/Icons/OpenFolder.svg");
      break;
    case nsDocumentAction::ButtonType::CopyAssetGuid:
      SetIconPath(":/GuiFoundation/Icons/Guid.svg");
      break;
    case nsDocumentAction::ButtonType::UpdatePrefabs:
      SetIconPath(":/EditorPluginScene/Icons/PrefabUpdate.svg");
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
    m_Context.m_pDocument->m_EventsOne.AddEventHandler(nsMakeDelegate(&nsDocumentAction::DocumentEventHandler, this));

    if (m_ButtonType == ButtonType::Save)
    {
      SetVisible(!m_Context.m_pDocument->IsReadOnly());
      SetEnabled(m_Context.m_pDocument->IsModified());
    }
  }
}

nsDocumentAction::~nsDocumentAction()
{
  if (m_Context.m_pDocument)
  {
    m_Context.m_pDocument->m_EventsOne.RemoveEventHandler(nsMakeDelegate(&nsDocumentAction::DocumentEventHandler, this));
  }
}

void nsDocumentAction::DocumentEventHandler(const nsDocumentEvent& e)
{
  switch (e.m_Type)
  {
    case nsDocumentEvent::Type::DocumentSaved:
    case nsDocumentEvent::Type::ModifiedChanged:
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

void nsDocumentAction::Execute(const nsVariant& value)
{
  switch (m_ButtonType)
  {
    case nsDocumentAction::ButtonType::Save:
    {
      nsQtDocumentWindow* pWnd = nsQtDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);
      pWnd->SaveDocument().LogFailure();
    }
    break;

    case nsDocumentAction::ButtonType::SaveAs:
    {
      nsQtDocumentWindow* pWnd = nsQtDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);
      if (pWnd->SaveDocument().Succeeded())
      {
        auto* desc = m_Context.m_pDocument->GetDocumentTypeDescriptor();
        nsStringBuilder sAllFilters;
        sAllFilters.Append(desc->m_sDocumentTypeName, " (*.", desc->m_sFileExtension, ")");
        QString sSelectedExt;
        nsString sFile = QFileDialog::getSaveFileName(QApplication::activeWindow(), QLatin1String("Create Document"),
          nsMakeQString(m_Context.m_pDocument->GetDocumentPath()), QString::fromUtf8(sAllFilters.GetData()), &sSelectedExt, QFileDialog::Option::DontResolveSymlinks)
                           .toUtf8()
                           .data();

        if (!sFile.IsEmpty())
        {
          nsUuid newDoc = nsUuid::MakeUuid();
          nsStatus res = m_Context.m_pDocument->GetDocumentManager()->CloneDocument(m_Context.m_pDocument->GetDocumentPath(), sFile, newDoc);

          if (res.Failed())
          {
            nsStringBuilder s;
            s.SetFormat("Failed to save document: \n'{0}'", sFile);
            nsQtUiServices::MessageBoxStatus(res, s);
          }
          else
          {
            const nsDocumentTypeDescriptor* pTypeDesc = nullptr;
            if (nsDocumentManager::FindDocumentTypeFromPath(sFile, false, pTypeDesc).Succeeded())
            {
              nsDocument* pDocument = nullptr;
              m_Context.m_pDocument->GetDocumentManager()->OpenDocument(pTypeDesc->m_sDocumentTypeName, sFile, pDocument).LogFailure();
            }
          }
        }
      }
    }
    break;

    case nsDocumentAction::ButtonType::SaveAll:
    {
      nsToolsProject::GetSingleton()->BroadcastSaveAll();
    }
    break;

    case nsDocumentAction::ButtonType::Close:
    {
      nsQtDocumentWindow* pWindow = nsQtDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);

      if (!pWindow->CanCloseWindow())
        return;

      pWindow->CloseDocumentWindow();
    }
    break;

    case nsDocumentAction::ButtonType::CloseAll:
    {
      auto& documentWindows = nsQtDocumentWindow::GetAllDocumentWindows();
      for (nsQtDocumentWindow* pWindow : documentWindows)
      {
        if (!pWindow->CanCloseWindow())
          continue;

        // Prevent closing the document root window.
        if (nsStringUtils::Compare(pWindow->GetUniqueName(), "Settings") == 0)
          continue;

        pWindow->CloseDocumentWindow();
      }
    }
    break;

    case nsDocumentAction::ButtonType::CloseAllButThis:
    {
      nsQtDocumentWindow* pThisWindow = nsQtDocumentWindow::FindWindowByDocument(m_Context.m_pDocument);

      auto& documentWindows = nsQtDocumentWindow::GetAllDocumentWindows();
      for (nsQtDocumentWindow* pWindow : documentWindows)
      {
        if (!pWindow->CanCloseWindow() || pWindow == pThisWindow)
          continue;

        // Prevent closing the document root window.
        if (nsStringUtils::Compare(pWindow->GetUniqueName(), "Settings") == 0)
          continue;

        pWindow->CloseDocumentWindow();
      }
    }
    break;

    case nsDocumentAction::ButtonType::OpenContainingFolder:
    {
      nsString sPath;

      if (!m_Context.m_pDocument)
      {
        if (nsToolsProject::IsProjectOpen())
          sPath = nsToolsProject::GetSingleton()->GetProjectFile();
        else
          sPath = nsOSFile::GetApplicationDirectory();
      }
      else
        sPath = m_Context.m_pDocument->GetDocumentPath();

      nsQtUiServices::OpenInExplorer(sPath, true);
    }
    break;

    case nsDocumentAction::ButtonType::CopyAssetGuid:
    {
      nsStringBuilder sGuid;
      nsConversionUtils::ToString(m_Context.m_pDocument->GetGuid(), sGuid);

      QClipboard* clipboard = QApplication::clipboard();
      QMimeData* mimeData = new QMimeData();
      mimeData->setText(sGuid.GetData());
      clipboard->setMimeData(mimeData);

      nsQtUiServices::GetSingleton()->ShowAllDocumentsTemporaryStatusBarMessage(nsFmt("Copied asset GUID: {}", sGuid), nsTime::MakeFromSeconds(5));
    }
    break;

    case nsDocumentAction::ButtonType::UpdatePrefabs:
      // TODO const cast
      const_cast<nsDocument*>(m_Context.m_pDocument)->UpdatePrefabs();
      return;
  }
}
