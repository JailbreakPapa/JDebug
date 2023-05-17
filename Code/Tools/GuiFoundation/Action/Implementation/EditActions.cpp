#include <GuiFoundation/GuiFoundationPCH.h>

#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <GuiFoundation/Action/ActionManager.h>
#include <GuiFoundation/Action/ActionMapManager.h>
#include <GuiFoundation/Action/EditActions.h>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdEditAction, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

////////////////////////////////////////////////////////////////////////
// wdEditActions
////////////////////////////////////////////////////////////////////////

wdActionDescriptorHandle wdEditActions::s_hEditCategory;
wdActionDescriptorHandle wdEditActions::s_hCopy;
wdActionDescriptorHandle wdEditActions::s_hPaste;
wdActionDescriptorHandle wdEditActions::s_hPasteAsChild;
wdActionDescriptorHandle wdEditActions::s_hPasteAtOriginalLocation;
wdActionDescriptorHandle wdEditActions::s_hDelete;

void wdEditActions::RegisterActions()
{
  s_hEditCategory = WD_REGISTER_CATEGORY("EditCategory");
  s_hCopy = WD_REGISTER_ACTION_1("Selection.Copy", wdActionScope::Document, "Document", "Ctrl+C", wdEditAction, wdEditAction::ButtonType::Copy);
  s_hPaste = WD_REGISTER_ACTION_1("Selection.Paste", wdActionScope::Document, "Document", "Ctrl+V", wdEditAction, wdEditAction::ButtonType::Paste);
  s_hPasteAsChild = WD_REGISTER_ACTION_1("Selection.PasteAsChild", wdActionScope::Document, "Document", "", wdEditAction, wdEditAction::ButtonType::PasteAsChild);
  s_hPasteAtOriginalLocation = WD_REGISTER_ACTION_1("Selection.PasteAtOriginalLocation", wdActionScope::Document, "Document", "", wdEditAction, wdEditAction::ButtonType::PasteAtOriginalLocation);
  s_hDelete = WD_REGISTER_ACTION_1("Selection.Delete", wdActionScope::Document, "Document", "", wdEditAction, wdEditAction::ButtonType::Delete);
}

void wdEditActions::UnregisterActions()
{
  wdActionManager::UnregisterAction(s_hEditCategory);
  wdActionManager::UnregisterAction(s_hCopy);
  wdActionManager::UnregisterAction(s_hPaste);
  wdActionManager::UnregisterAction(s_hPasteAsChild);
  wdActionManager::UnregisterAction(s_hPasteAtOriginalLocation);
  wdActionManager::UnregisterAction(s_hDelete);
}

void wdEditActions::MapActions(const char* szMapping, const char* szPath, bool bDeleteAction, bool bAdvancedPasteActions)
{
  wdActionMap* pMap = wdActionMapManager::GetActionMap(szMapping);
  WD_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the edit actions failed!", szMapping);

  wdStringBuilder sSubPath(szPath, "/EditCategory");

  pMap->MapAction(s_hEditCategory, szPath, 3.5f);

  pMap->MapAction(s_hCopy, sSubPath, 1.0f);
  pMap->MapAction(s_hPaste, sSubPath, 2.0f);

  if (bAdvancedPasteActions)
  {
    pMap->MapAction(s_hPasteAsChild, sSubPath, 2.5f);
    pMap->MapAction(s_hPasteAtOriginalLocation, sSubPath, 2.7f);
  }

  if (bDeleteAction)
    pMap->MapAction(s_hDelete, sSubPath, 3.0f);
}


void wdEditActions::MapContextMenuActions(const char* szMapping, const char* szPath)
{
  wdActionMap* pMap = wdActionMapManager::GetActionMap(szMapping);
  WD_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the edit actions failed!", szMapping);

  wdStringBuilder sSubPath(szPath, "/EditCategory");

  pMap->MapAction(s_hEditCategory, szPath, 10.0f);

  pMap->MapAction(s_hCopy, sSubPath, 1.0f);
  pMap->MapAction(s_hPasteAsChild, sSubPath, 2.0f);
  pMap->MapAction(s_hDelete, sSubPath, 3.0f);
}


void wdEditActions::MapViewContextMenuActions(const char* szMapping, const char* szPath)
{
  wdActionMap* pMap = wdActionMapManager::GetActionMap(szMapping);
  WD_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the edit actions failed!", szMapping);

  wdStringBuilder sSubPath(szPath, "/EditCategory");

  pMap->MapAction(s_hEditCategory, szPath, 10.0f);

  pMap->MapAction(s_hCopy, sSubPath, 1.0f);
  pMap->MapAction(s_hPasteAsChild, sSubPath, 2.0f);
  pMap->MapAction(s_hPasteAtOriginalLocation, sSubPath, 2.5f);
  pMap->MapAction(s_hDelete, sSubPath, 3.0f);
}

////////////////////////////////////////////////////////////////////////
// wdEditAction
////////////////////////////////////////////////////////////////////////

wdEditAction::wdEditAction(const wdActionContext& context, const char* szName, ButtonType button)
  : wdButtonAction(context, szName, false, "")
{
  m_ButtonType = button;

  switch (m_ButtonType)
  {
    case wdEditAction::ButtonType::Copy:
      SetIconPath(":/GuiFoundation/Icons/Copy16.png");
      break;
    case wdEditAction::ButtonType::Paste:
      SetIconPath(":/GuiFoundation/Icons/Paste16.png");
      break;
    case wdEditAction::ButtonType::PasteAsChild:
      SetIconPath(":/GuiFoundation/Icons/Paste16.png"); /// \todo Icon
      break;
    case wdEditAction::ButtonType::PasteAtOriginalLocation:
      SetIconPath(":/GuiFoundation/Icons/Paste16.png");
      break;
    case wdEditAction::ButtonType::Delete:
      SetIconPath(":/GuiFoundation/Icons/Delete16.png");
      break;
  }

  m_Context.m_pDocument->GetSelectionManager()->m_Events.AddEventHandler(wdMakeDelegate(&wdEditAction::SelectionEventHandler, this));

  if (m_ButtonType == ButtonType::Copy || m_ButtonType == ButtonType::Delete)
  {
    SetEnabled(!m_Context.m_pDocument->GetSelectionManager()->IsSelectionEmpty());
  }
}

wdEditAction::~wdEditAction()
{
  if (m_Context.m_pDocument)
  {
    m_Context.m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(wdMakeDelegate(&wdEditAction::SelectionEventHandler, this));
  }
}

void wdEditAction::Execute(const wdVariant& value)
{
  switch (m_ButtonType)
  {
    case wdEditAction::ButtonType::Copy:
    {
      wdStringBuilder sMimeType;

      wdAbstractObjectGraph graph;
      if (!m_Context.m_pDocument->CopySelectedObjects(graph, sMimeType))
        break;

      // Serialize to string
      wdContiguousMemoryStreamStorage streamStorage;
      wdMemoryStreamWriter memoryWriter(&streamStorage);
      wdAbstractGraphDdlSerializer::Write(memoryWriter, &graph, nullptr, false);
      memoryWriter.WriteBytes("\0", 1).IgnoreResult(); // null terminate

      // Write to clipboard
      QClipboard* clipboard = QApplication::clipboard();
      QMimeData* mimeData = new QMimeData();
      QByteArray encodedData((const char*)streamStorage.GetData(), streamStorage.GetStorageSize32());

      mimeData->setData(sMimeType.GetData(), encodedData);
      mimeData->setText(QString::fromUtf8((const char*)streamStorage.GetData()));
      clipboard->setMimeData(mimeData);
    }
    break;

    case wdEditAction::ButtonType::Paste:
    case wdEditAction::ButtonType::PasteAsChild:
    case wdEditAction::ButtonType::PasteAtOriginalLocation:
    {
      // Check for clipboard data of the correct type.
      QClipboard* clipboard = QApplication::clipboard();
      auto mimedata = clipboard->mimeData();

      wdHybridArray<wdString, 4> MimeTypes;
      m_Context.m_pDocument->GetSupportedMimeTypesForPasting(MimeTypes);

      wdInt32 iFormat = -1;
      {
        for (wdUInt32 i = 0; i < MimeTypes.GetCount(); ++i)
        {
          if (mimedata->hasFormat(MimeTypes[i].GetData()))
          {
            iFormat = i;
            break;
          }
        }

        if (iFormat < 0)
          break;
      }

      // Paste at current selected object.
      wdPasteObjectsCommand cmd;
      cmd.m_sMimeType = MimeTypes[iFormat];

      QByteArray ba = mimedata->data(MimeTypes[iFormat].GetData());
      cmd.m_sGraphTextFormat = ba.data();

      if (m_ButtonType == ButtonType::PasteAsChild)
      {
        if (!m_Context.m_pDocument->GetSelectionManager()->IsSelectionEmpty())
          cmd.m_Parent = m_Context.m_pDocument->GetSelectionManager()->GetSelection().PeekBack()->GetGuid();
      }
      else if (m_ButtonType == ButtonType::PasteAtOriginalLocation)
      {
        cmd.m_bAllowPickedPosition = false;
      }

      auto history = m_Context.m_pDocument->GetCommandHistory();

      history->StartTransaction("Paste");

      if (history->AddCommand(cmd).Failed())
        history->CancelTransaction();
      else
        history->FinishTransaction();
    }
    break;

    case wdEditAction::ButtonType::Delete:
    {
      m_Context.m_pDocument->DeleteSelectedObjects();
    }
    break;
  }
}

void wdEditAction::SelectionEventHandler(const wdSelectionManagerEvent& e)
{
  if (m_ButtonType == ButtonType::Copy || m_ButtonType == ButtonType::Delete)
  {
    SetEnabled(!m_Context.m_pDocument->GetSelectionManager()->IsSelectionEmpty());
  }
}
