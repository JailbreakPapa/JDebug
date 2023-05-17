#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Configuration/SubSystem.h>
#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Document/DocumentUtils.h>
#include <ToolsFoundation/Project/ToolsProject.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDocumentManager, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, DocumentManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdPlugin::Events().AddEventHandler(wdDocumentManager::OnPluginEvent);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdPlugin::Events().RemoveEventHandler(wdDocumentManager::OnPluginEvent);
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdSet<const wdRTTI*> wdDocumentManager::s_KnownManagers;
wdHybridArray<wdDocumentManager*, 16> wdDocumentManager::s_AllDocumentManagers;
wdMap<wdString, const wdDocumentTypeDescriptor*> wdDocumentManager::s_AllDocumentDescriptors; // maps from "sDocumentTypeName" to descriptor
wdCopyOnBroadcastEvent<const wdDocumentManager::Event&> wdDocumentManager::s_Events;
wdEvent<wdDocumentManager::Request&> wdDocumentManager::s_Requests;
wdMap<wdString, wdDocumentManager::CustomAction> wdDocumentManager::s_CustomActions;

void wdDocumentManager::OnPluginEvent(const wdPluginEvent& e)
{
  switch (e.m_EventType)
  {
    case wdPluginEvent::BeforeUnloading:
      UpdateBeforeUnloadingPlugins(e);
      break;
    case wdPluginEvent::AfterPluginChanges:
      UpdatedAfterLoadingPlugins();
      break;

    default:
      break;
  }
}

void wdDocumentManager::UpdateBeforeUnloadingPlugins(const wdPluginEvent& e)
{
  bool bChanges = false;

  // triggers a reevaluation next time
  s_AllDocumentDescriptors.Clear();

  // remove all document managers that belong to this plugin
  for (wdUInt32 i = 0; i < s_AllDocumentManagers.GetCount();)
  {
    const wdRTTI* pRtti = s_AllDocumentManagers[i]->GetDynamicRTTI();

    if (wdStringUtils::IsEqual(pRtti->GetPluginName(), e.m_szPluginBinary))
    {
      s_KnownManagers.Remove(pRtti);

      pRtti->GetAllocator()->Deallocate(s_AllDocumentManagers[i]);
      s_AllDocumentManagers.RemoveAtAndSwap(i);

      bChanges = true;
    }
    else
      ++i;
  }

  if (bChanges)
  {
    Event e2;
    e2.m_Type = Event::Type::DocumentTypesRemoved;
    s_Events.Broadcast(e2);
  }
}

void wdDocumentManager::UpdatedAfterLoadingPlugins()
{
  bool bChanges = false;

  wdRTTI* pRtti = wdRTTI::GetFirstInstance();

  while (pRtti)
  {
    // find all types derived from wdDocumentManager
    if (pRtti->IsDerivedFrom<wdDocumentManager>())
    {
      // add the ones that we don't know yet
      if (!s_KnownManagers.Find(pRtti).IsValid())
      {
        // add it as 'known' even if we cannot allocate it
        s_KnownManagers.Insert(pRtti);

        if (pRtti->GetAllocator()->CanAllocate())
        {
          // create one instance of each manager type
          wdDocumentManager* pManager = pRtti->GetAllocator()->Allocate<wdDocumentManager>();
          s_AllDocumentManagers.PushBack(pManager);

          bChanges = true;
        }
      }
    }

    pRtti = pRtti->GetNextInstance();
  }

  // triggers a reevaluation next time
  s_AllDocumentDescriptors.Clear();
  GetAllDocumentDescriptors();


  if (bChanges)
  {
    Event e;
    e.m_Type = Event::Type::DocumentTypesAdded;
    s_Events.Broadcast(e);
  }
}

void wdDocumentManager::GetSupportedDocumentTypes(wdDynamicArray<const wdDocumentTypeDescriptor*>& inout_documentTypes) const
{
  InternalGetSupportedDocumentTypes(inout_documentTypes);

  for (auto& dt : inout_documentTypes)
  {
    WD_ASSERT_DEBUG(dt->m_pDocumentType != nullptr, "No document type is set");
    WD_ASSERT_DEBUG(!dt->m_sFileExtension.IsEmpty(), "File extension must be valid");
    WD_ASSERT_DEBUG(dt->m_pManager != nullptr, "Document manager must be set");
  }
}

wdStatus wdDocumentManager::CanOpenDocument(const char* szFilePath) const
{
  wdHybridArray<const wdDocumentTypeDescriptor*, 4> DocumentTypes;
  GetSupportedDocumentTypes(DocumentTypes);

  wdStringBuilder sPath = szFilePath;
  wdStringBuilder sExt = sPath.GetFileExtension();

  // check whether the file extension is in the list of possible extensions
  // if not, we can definitely not open this file
  for (wdUInt32 i = 0; i < DocumentTypes.GetCount(); ++i)
  {
    if (DocumentTypes[i]->m_sFileExtension.IsEqual_NoCase(sExt))
    {
      return wdStatus(WD_SUCCESS);
    }
  }

  return wdStatus("File extension is not handled by any registered type");
}

void wdDocumentManager::EnsureWindowRequested(wdDocument* pDocument, const wdDocumentObject* pOpenContext /*= nullptr*/)
{
  if (pDocument->m_bWindowRequested)
    return;

  WD_PROFILE_SCOPE("EnsureWindowRequested");
  pDocument->m_bWindowRequested = true;

  Event e;
  e.m_pDocument = pDocument;
  e.m_Type = Event::Type::DocumentWindowRequested;
  e.m_pOpenContext = pOpenContext;
  s_Events.Broadcast(e);

  e.m_pDocument = pDocument;
  e.m_Type = Event::Type::AfterDocumentWindowRequested;
  e.m_pOpenContext = pOpenContext;
  s_Events.Broadcast(e);
}

wdStatus wdDocumentManager::CreateOrOpenDocument(bool bCreate, const char* szDocumentTypeName, const char* szPath, wdDocument*& out_pDocument,
  wdBitflags<wdDocumentFlags> flags, const wdDocumentObject* pOpenContext /*= nullptr*/)
{
  wdFileStats fs;
  wdStringBuilder sPath = szPath;
  sPath.MakeCleanPath();
  if (!bCreate && wdOSFile::GetFileStats(sPath, fs).Failed())
  {
    return wdStatus("The file does not exist.");
  }

  Request r;
  r.m_Type = Request::Type::DocumentAllowedToOpen;
  r.m_RequestStatus.m_Result = WD_SUCCESS;
  r.m_sDocumentType = szDocumentTypeName;
  r.m_sDocumentPath = sPath;
  s_Requests.Broadcast(r);

  // if for example no project is open, or not the correct one, then a document cannot be opened
  if (r.m_RequestStatus.m_Result.Failed())
    return r.m_RequestStatus;

  out_pDocument = nullptr;

  wdStatus status;

  wdHybridArray<const wdDocumentTypeDescriptor*, 4> DocumentTypes;
  GetSupportedDocumentTypes(DocumentTypes);

  for (wdUInt32 i = 0; i < DocumentTypes.GetCount(); ++i)
  {
    if (DocumentTypes[i]->m_sDocumentTypeName == szDocumentTypeName)
    {
      // See if there is a default asset document registered for the type, if so clone
      // it and use that as the new document instead of creating one from scratch.
      if (bCreate && !flags.IsSet(wdDocumentFlags::EmptyDocument))
      {
        wdStringBuilder sTemplateDoc = "Editor/DocumentTemplates/Default";
        sTemplateDoc.ChangeFileExtension(sPath.GetFileExtension());

        if (wdFileSystem::ExistsFile(sTemplateDoc))
        {
          wdUuid CloneUuid;
          if (CloneDocument(sTemplateDoc, sPath, CloneUuid).Succeeded())
          {
            if (OpenDocument(szDocumentTypeName, sPath, out_pDocument, flags, pOpenContext).Succeeded())
            {
              return wdStatus(WD_SUCCESS);
            }
          }

          wdLog::Warning("Failed to create document from template '{}'", sTemplateDoc);
        }
      }

      WD_ASSERT_DEV(DocumentTypes[i]->m_bCanCreate, "This document manager cannot create the document type '{0}'", szDocumentTypeName);

      {
        WD_PROFILE_SCOPE(szDocumentTypeName);
        status = wdStatus(WD_SUCCESS);
        InternalCreateDocument(szDocumentTypeName, sPath, bCreate, out_pDocument, pOpenContext);
      }
      out_pDocument->SetAddToResetFilesList(flags.IsSet(wdDocumentFlags::AddToRecentFilesList));

      if (status.m_Result.Succeeded())
      {
        out_pDocument->SetupDocumentInfo(DocumentTypes[i]);

        out_pDocument->m_pDocumentManager = this;
        m_AllOpenDocuments.PushBack(out_pDocument);

        if (!bCreate)
        {
          status = out_pDocument->LoadDocument();
        }

        {
          WD_PROFILE_SCOPE("InitializeAfterLoading");
          out_pDocument->InitializeAfterLoading(bCreate);
        }

        if (bCreate)
        {
          out_pDocument->SetModified(true);
          if (flags.IsSet(wdDocumentFlags::AsyncSave))
          {
            out_pDocument->SaveDocumentAsync({});
            status = wdStatus(WD_SUCCESS);
          }
          else
          {
            status = out_pDocument->SaveDocument();
          }
        }

        {
          WD_PROFILE_SCOPE("InitializeAfterLoadingAndSaving");
          out_pDocument->InitializeAfterLoadingAndSaving();
        }

        Event e;
        e.m_pDocument = out_pDocument;
        e.m_Type = Event::Type::DocumentOpened;

        s_Events.Broadcast(e);

        if (flags.IsSet(wdDocumentFlags::RequestWindow))
          EnsureWindowRequested(out_pDocument, pOpenContext);
      }

      return status;
    }
  }

  WD_REPORT_FAILURE("This document manager does not support the document type '{0}'", szDocumentTypeName);
  return status;
}

wdStatus wdDocumentManager::CreateDocument(
  const char* szDocumentTypeName, const char* szPath, wdDocument*& out_pDocument, wdBitflags<wdDocumentFlags> flags, const wdDocumentObject* pOpenContext)
{
  return CreateOrOpenDocument(true, szDocumentTypeName, szPath, out_pDocument, flags, pOpenContext);
}

wdStatus wdDocumentManager::OpenDocument(const char* szDocumentTypeName, const char* szPath, wdDocument*& out_pDocument,
  wdBitflags<wdDocumentFlags> flags, const wdDocumentObject* pOpenContext)
{
  return CreateOrOpenDocument(false, szDocumentTypeName, szPath, out_pDocument, flags, pOpenContext);
}


wdStatus wdDocumentManager::CloneDocument(const char* szPath, const char* szClonePath, wdUuid& inout_cloneGuid)
{
  const wdDocumentTypeDescriptor* pTypeDesc = nullptr;
  wdStatus res = wdDocumentUtils::IsValidSaveLocationForDocument(szClonePath, &pTypeDesc);
  if (res.Failed())
    return res;

  wdUniquePtr<wdAbstractObjectGraph> header;
  wdUniquePtr<wdAbstractObjectGraph> objects;
  wdUniquePtr<wdAbstractObjectGraph> types;

  res = wdDocument::ReadDocument(szPath, header, objects, types);
  if (res.Failed())
    return res;

  wdUuid documentId;
  wdAbstractObjectNode::Property* documentIdProp = nullptr;
  {
    auto* pHeaderNode = header->GetNodeByName("Header");
    WD_ASSERT_DEV(pHeaderNode, "No header found, document '{0}' is corrupted.", szPath);
    documentIdProp = pHeaderNode->FindProperty("DocumentID");
    WD_ASSERT_DEV(documentIdProp, "No document ID property found in header, document document '{0}' is corrupted.", szPath);
    documentId = documentIdProp->m_Value.Get<wdUuid>();
  }

  wdUuid seedGuid;
  if (inout_cloneGuid.IsValid())
  {
    seedGuid = inout_cloneGuid;
    seedGuid.RevertCombinationWithSeed(documentId);

    wdUuid test = documentId;
    test.CombineWithSeed(seedGuid);
    WD_ASSERT_DEV(test == inout_cloneGuid, "");
  }
  else
  {
    seedGuid.CreateNewUuid();
    inout_cloneGuid = documentId;
    inout_cloneGuid.CombineWithSeed(seedGuid);
  }

  InternalCloneDocument(szPath, szClonePath, documentId, seedGuid, inout_cloneGuid, header.Borrow(), objects.Borrow(), types.Borrow());

  {
    wdDeferredFileWriter file;
    file.SetOutput(szClonePath);
    wdAbstractGraphDdlSerializer::WriteDocument(file, header.Borrow(), objects.Borrow(), types.Borrow(), false);
    if (file.Close() == WD_FAILURE)
    {
      return wdStatus(wdFmt("Unable to open file '{0}' for writing!", szClonePath));
    }
  }
  return wdStatus(WD_SUCCESS);
}

void wdDocumentManager::InternalCloneDocument(const char* szPath, const char* szClonePath, const wdUuid& documentId, const wdUuid& seedGuid, const wdUuid& cloneGuid, wdAbstractObjectGraph* header, wdAbstractObjectGraph* objects, wdAbstractObjectGraph* types)
{
  // Remap
  header->ReMapNodeGuids(seedGuid);
  objects->ReMapNodeGuids(seedGuid);

  auto* pHeaderNode = header->GetNodeByName("Header");
  auto* documentIdProp = pHeaderNode->FindProperty("DocumentID");
  documentIdProp->m_Value = cloneGuid;

  // Fix cloning of docs containing prefabs.
  // TODO: generalize this for other doc features?
  auto& AllNodes = objects->GetAllNodes();
  for (auto it = AllNodes.GetIterator(); it.IsValid(); ++it)
  {
    auto* pNode = it.Value();
    wdAbstractObjectNode::Property* pProp = pNode->FindProperty("MetaPrefabSeed");
    if (pProp && pProp->m_Value.IsA<wdUuid>())
    {
      wdUuid prefabSeed = pProp->m_Value.Get<wdUuid>();
      prefabSeed.CombineWithSeed(seedGuid);
      pProp->m_Value = prefabSeed;
    }
  }
}

void wdDocumentManager::CloseDocument(wdDocument* pDocument)
{
  WD_ASSERT_DEV(pDocument != nullptr, "Invalid document pointer");

  if (!m_AllOpenDocuments.RemoveAndCopy(pDocument))
    return;

  Event e;
  e.m_pDocument = pDocument;

  e.m_Type = Event::Type::DocumentClosing;
  s_Events.Broadcast(e);

  e.m_Type = Event::Type::DocumentClosing2;
  s_Events.Broadcast(e);

  pDocument->BeforeClosing();
  delete pDocument; // the pointer in e.m_pDocument won't be valid anymore at broadcast time, it is only sent for comparison purposes, not to be dereferenced

  e.m_Type = Event::Type::DocumentClosed;
  s_Events.Broadcast(e);
}

void wdDocumentManager::CloseAllDocumentsOfManager()
{
  while (!m_AllOpenDocuments.IsEmpty())
  {
    CloseDocument(m_AllOpenDocuments[0]);
  }
}

void wdDocumentManager::CloseAllDocuments()
{
  for (wdDocumentManager* pMan : s_AllDocumentManagers)
  {
    pMan->CloseAllDocumentsOfManager();
  }
}

wdDocument* wdDocumentManager::GetDocumentByPath(const char* szPath) const
{
  wdStringBuilder sPath = szPath;
  sPath.MakeCleanPath();

  for (wdDocument* pDoc : m_AllOpenDocuments)
  {
    if (sPath.IsEqual_NoCase(pDoc->GetDocumentPath()))
      return pDoc;
  }

  return nullptr;
}


wdDocument* wdDocumentManager::GetDocumentByGuid(const wdUuid& guid)
{
  for (auto man : s_AllDocumentManagers)
  {
    for (auto doc : man->m_AllOpenDocuments)
    {
      if (doc->GetGuid() == guid)
        return doc;
    }
  }

  return nullptr;
}


bool wdDocumentManager::EnsureDocumentIsClosedInAllManagers(const char* szPath)
{
  bool bClosedAny = false;
  for (auto man : s_AllDocumentManagers)
  {
    if (man->EnsureDocumentIsClosed(szPath))
      bClosedAny = true;
  }

  return bClosedAny;
}

bool wdDocumentManager::EnsureDocumentIsClosed(const char* szPath)
{
  auto pDoc = GetDocumentByPath(szPath);

  if (pDoc == nullptr)
    return false;

  CloseDocument(pDoc);

  return true;
}

wdResult wdDocumentManager::FindDocumentTypeFromPath(const char* szPath, bool bForCreation, const wdDocumentTypeDescriptor*& out_pTypeDesc)
{
  const wdString sFileExt = wdPathUtils::GetFileExtension(szPath);

  const auto& allDesc = GetAllDocumentDescriptors();

  for (auto it : allDesc)
  {
    const auto* desc = it.Value();

    if (bForCreation && !desc->m_bCanCreate)
      continue;

    if (desc->m_sFileExtension.IsEqual_NoCase(sFileExt))
    {
      out_pTypeDesc = desc;
      return WD_SUCCESS;
    }
  }

  return WD_FAILURE;
}

const wdMap<wdString, const wdDocumentTypeDescriptor*>& wdDocumentManager::GetAllDocumentDescriptors()
{
  if (s_AllDocumentDescriptors.IsEmpty())
  {
    for (wdDocumentManager* pMan : wdDocumentManager::GetAllDocumentManagers())
    {
      wdHybridArray<const wdDocumentTypeDescriptor*, 4> descriptors;
      pMan->GetSupportedDocumentTypes(descriptors);

      for (auto pDesc : descriptors)
      {
        s_AllDocumentDescriptors[pDesc->m_sDocumentTypeName] = pDesc;
      }
    }
  }

  return s_AllDocumentDescriptors;
}

const wdDocumentTypeDescriptor* wdDocumentManager::GetDescriptorForDocumentType(const char* szDocumentType)
{
  return GetAllDocumentDescriptors().GetValueOrDefault(szDocumentType, nullptr);
}

/// \todo on close doc: remove from m_AllDocuments
