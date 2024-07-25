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

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsEditAction, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

////////////////////////////////////////////////////////////////////////
// nsEditActions
////////////////////////////////////////////////////////////////////////

nsActionDescriptorHandle nsEditActions::s_hEditCategory;
nsActionDescriptorHandle nsEditActions::s_hCopy;
nsActionDescriptorHandle nsEditActions::s_hPaste;
nsActionDescriptorHandle nsEditActions::s_hPasteAsChild;
nsActionDescriptorHandle nsEditActions::s_hPasteAtOriginalLocation;
nsActionDescriptorHandle nsEditActions::s_hDelete;

void nsEditActions::RegisterActions()
{
  s_hEditCategory = NS_REGISTER_CATEGORY("EditCategory");
  s_hCopy = NS_REGISTER_ACTION_1("Selection.Copy", nsActionScope::Document, "Document", "Ctrl+C", nsEditAction, nsEditAction::ButtonType::Copy);
  s_hPaste = NS_REGISTER_ACTION_1("Selection.Paste", nsActionScope::Document, "Document", "Ctrl+V", nsEditAction, nsEditAction::ButtonType::Paste);
  s_hPasteAsChild = NS_REGISTER_ACTION_1("Selection.PasteAsChild", nsActionScope::Document, "Document", "", nsEditAction, nsEditAction::ButtonType::PasteAsChild);
  s_hPasteAtOriginalLocation = NS_REGISTER_ACTION_1("Selection.PasteAtOriginalLocation", nsActionScope::Document, "Document", "", nsEditAction, nsEditAction::ButtonType::PasteAtOriginalLocation);
  s_hDelete = NS_REGISTER_ACTION_1("Selection.Delete", nsActionScope::Document, "Document", "", nsEditAction, nsEditAction::ButtonType::Delete);
}

void nsEditActions::UnregisterActions()
{
  nsActionManager::UnregisterAction(s_hEditCategory);
  nsActionManager::UnregisterAction(s_hCopy);
  nsActionManager::UnregisterAction(s_hPaste);
  nsActionManager::UnregisterAction(s_hPasteAsChild);
  nsActionManager::UnregisterAction(s_hPasteAtOriginalLocation);
  nsActionManager::UnregisterAction(s_hDelete);
}

void nsEditActions::MapActions(nsStringView sMapping, bool bDeleteAction, bool bAdvancedPasteActions)
{
  nsActionMap* pMap = nsActionMapManager::GetActionMap(sMapping);
  NS_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the edit actions failed!", sMapping);

  pMap->MapAction(s_hEditCategory, "G.Edit", 3.5f);

  pMap->MapAction(s_hCopy, "G.Edit", "EditCategory", 1.0f);
  pMap->MapAction(s_hPaste, "G.Edit", "EditCategory", 2.0f);

  if (bAdvancedPasteActions)
  {
    pMap->MapAction(s_hPasteAsChild, "G.Edit", "EditCategory", 2.5f);
    pMap->MapAction(s_hPasteAtOriginalLocation, "G.Edit", "EditCategory", 2.7f);
  }

  if (bDeleteAction)
    pMap->MapAction(s_hDelete, "G.Edit", "EditCategory", 3.0f);
}


void nsEditActions::MapContextMenuActions(nsStringView sMapping)
{
  nsActionMap* pMap = nsActionMapManager::GetActionMap(sMapping);
  NS_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the edit actions failed!", sMapping);

  pMap->MapAction(s_hEditCategory, "", 10.0f);

  pMap->MapAction(s_hCopy, "EditCategory", 1.0f);
  pMap->MapAction(s_hPasteAsChild, "EditCategory", 2.0f);
  pMap->MapAction(s_hDelete, "EditCategory", 3.0f);
}


void nsEditActions::MapViewContextMenuActions(nsStringView sMapping)
{
  nsActionMap* pMap = nsActionMapManager::GetActionMap(sMapping);
  NS_ASSERT_DEV(pMap != nullptr, "The given mapping ('{0}') does not exist, mapping the edit actions failed!", sMapping);

  pMap->MapAction(s_hEditCategory, "", 10.0f);

  pMap->MapAction(s_hCopy, "EditCategory", 1.0f);
  pMap->MapAction(s_hPasteAsChild, "EditCategory", 2.0f);
  pMap->MapAction(s_hPasteAtOriginalLocation, "EditCategory", 2.5f);
  pMap->MapAction(s_hDelete, "EditCategory", 3.0f);
}

////////////////////////////////////////////////////////////////////////
// nsEditAction
////////////////////////////////////////////////////////////////////////

nsEditAction::nsEditAction(const nsActionContext& context, const char* szName, ButtonType button)
  : nsButtonAction(context, szName, false, "")
{
  m_ButtonType = button;

  switch (m_ButtonType)
  {
    case nsEditAction::ButtonType::Copy:
      SetIconPath(":/GuiFoundation/Icons/Copy.svg");
      break;
    case nsEditAction::ButtonType::Paste:
      SetIconPath(":/GuiFoundation/Icons/Paste.svg");
      break;
    case nsEditAction::ButtonType::PasteAsChild:
      SetIconPath(":/GuiFoundation/Icons/Paste.svg"); /// \todo Icon
      break;
    case nsEditAction::ButtonType::PasteAtOriginalLocation:
      SetIconPath(":/GuiFoundation/Icons/Paste.svg");
      break;
    case nsEditAction::ButtonType::Delete:
      SetIconPath(":/GuiFoundation/Icons/Delete.svg");
      break;
  }

  m_Context.m_pDocument->GetSelectionManager()->m_Events.AddEventHandler(nsMakeDelegate(&nsEditAction::SelectionEventHandler, this));

  if (m_ButtonType == ButtonType::Copy || m_ButtonType == ButtonType::Delete)
  {
    SetEnabled(!m_Context.m_pDocument->GetSelectionManager()->IsSelectionEmpty());
  }
}

nsEditAction::~nsEditAction()
{
  if (m_Context.m_pDocument)
  {
    m_Context.m_pDocument->GetSelectionManager()->m_Events.RemoveEventHandler(nsMakeDelegate(&nsEditAction::SelectionEventHandler, this));
  }
}

void nsEditAction::Execute(const nsVariant& value)
{
  switch (m_ButtonType)
  {
    case nsEditAction::ButtonType::Copy:
    {
      nsStringBuilder sMimeType;

      nsAbstractObjectGraph graph;
      if (!m_Context.m_pDocument->CopySelectedObjects(graph, sMimeType))
        break;

      // Serialize to string
      nsContiguousMemoryStreamStorage streamStorage;
      nsMemoryStreamWriter memoryWriter(&streamStorage);
      nsAbstractGraphDdlSerializer::Write(memoryWriter, &graph, nullptr, false);
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

    case nsEditAction::ButtonType::Paste:
    case nsEditAction::ButtonType::PasteAsChild:
    case nsEditAction::ButtonType::PasteAtOriginalLocation:
    {
      // Check for clipboard data of the correct type.
      QClipboard* clipboard = QApplication::clipboard();
      auto mimedata = clipboard->mimeData();

      nsHybridArray<nsString, 4> MimeTypes;
      m_Context.m_pDocument->GetSupportedMimeTypesForPasting(MimeTypes);

      nsInt32 iFormat = -1;
      {
        for (nsUInt32 i = 0; i < MimeTypes.GetCount(); ++i)
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
      nsPasteObjectsCommand cmd;
      cmd.m_sMimeType = MimeTypes[iFormat];

      QByteArray ba = mimedata->data(MimeTypes[iFormat].GetData());
      cmd.m_sGraphTextFormat = ba.data();

      const nsDocumentObject* pNewParent = m_Context.m_pDocument->GetSelectionManager()->GetCurrentObject();
      if (pNewParent && m_ButtonType != ButtonType::PasteAsChild)
      {
        // default behavior copied from Unity: paste as a sibling of the currently selected item
        // this way if you just select and object and copy/paste it, the new object has the same parent (the clone becomes a sibling of the original)
        // but you can also select any other object as the reference, and clone as a sibling to that one
        pNewParent = pNewParent->GetParent();
      }

      if (pNewParent)
      {
        cmd.m_Parent = pNewParent->GetGuid();
      }

      if (m_ButtonType == ButtonType::PasteAtOriginalLocation)
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

    case nsEditAction::ButtonType::Delete:
    {
      m_Context.m_pDocument->DeleteSelectedObjects();
    }
    break;
  }
}

void nsEditAction::SelectionEventHandler(const nsSelectionManagerEvent& e)
{
  if (m_ButtonType == ButtonType::Copy || m_ButtonType == ButtonType::Delete)
  {
    SetEnabled(!m_Context.m_pDocument->GetSelectionManager()->IsSelectionEmpty());
  }
}
