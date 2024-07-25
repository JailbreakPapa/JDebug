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
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDocumentManager, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, DocumentManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsPlugin::Events().AddEventHandler(nsDocumentManager::OnPluginEvent);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsPlugin::Events().RemoveEventHandler(nsDocumentManager::OnPluginEvent);
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsSet<const nsRTTI*> nsDocumentManager::s_KnownManagers;
nsHybridArray<nsDocumentManager*, 16> nsDocumentManager::s_AllDocumentManagers;
nsMap<nsString, const nsDocumentTypeDescriptor*> nsDocumentManager::s_AllDocumentDescriptors; // maps from "sDocumentTypeName" to descriptor
nsCopyOnBroadcastEvent<const nsDocumentManager::Event&> nsDocumentManager::s_Events;
nsEvent<nsDocumentManager::Request&> nsDocumentManager::s_Requests;
nsMap<nsString, nsDocumentManager::CustomAction> nsDocumentManager::s_CustomActions;

void nsDocumentManager::OnPluginEvent(const nsPluginEvent& e)
{
  switch (e.m_EventType)
  {
    case nsPluginEvent::BeforeUnloading:
      UpdateBeforeUnloadingPlugins(e);
      break;
    case nsPluginEvent::AfterPluginChanges:
      UpdatedAfterLoadingPlugins();
      break;

    default:
      break;
  }
}

void nsDocumentManager::UpdateBeforeUnloadingPlugins(const nsPluginEvent& e)
{
  bool bChanges = false;

  // triggers a reevaluation next time
  s_AllDocumentDescriptors.Clear();

  // remove all document managers that belong to this plugin
  for (nsUInt32 i = 0; i < s_AllDocumentManagers.GetCount();)
  {
    const nsRTTI* pRtti = s_AllDocumentManagers[i]->GetDynamicRTTI();

    if (pRtti->GetPluginName() == e.m_sPluginBinary)
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

void nsDocumentManager::UpdatedAfterLoadingPlugins()
{
  bool bChanges = false;

  nsRTTI::ForEachDerivedType<nsDocumentManager>(
    [&](const nsRTTI* pRtti)
    {
      // add the ones that we don't know yet
      if (!s_KnownManagers.Find(pRtti).IsValid())
      {
        // add it as 'known' even if we cannot allocate it
        s_KnownManagers.Insert(pRtti);

        if (pRtti->GetAllocator()->CanAllocate())
        {
          // create one instance of each manager type
          nsDocumentManager* pManager = pRtti->GetAllocator()->Allocate<nsDocumentManager>();
          s_AllDocumentManagers.PushBack(pManager);

          bChanges = true;
        }
      }
    });

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

void nsDocumentManager::GetSupportedDocumentTypes(nsDynamicArray<const nsDocumentTypeDescriptor*>& inout_documentTypes) const
{
  InternalGetSupportedDocumentTypes(inout_documentTypes);

  for (auto& dt : inout_documentTypes)
  {
    NS_ASSERT_DEBUG(dt->m_bCanCreate == false || dt->m_pDocumentType != nullptr, "No document type is set");
    NS_ASSERT_DEBUG(!dt->m_sFileExtension.IsEmpty(), "File extension must be valid");
    NS_ASSERT_DEBUG(dt->m_pManager != nullptr, "Document manager must be set");
  }
}

nsStatus nsDocumentManager::CanOpenDocument(nsStringView sFilePath) const
{
  nsHybridArray<const nsDocumentTypeDescriptor*, 4> DocumentTypes;
  GetSupportedDocumentTypes(DocumentTypes);

  nsStringBuilder sPath = sFilePath;
  nsStringBuilder sExt = sPath.GetFileExtension();

  // check whether the file extension is in the list of possible extensions
  // if not, we can definitely not open this file
  for (nsUInt32 i = 0; i < DocumentTypes.GetCount(); ++i)
  {
    if (DocumentTypes[i]->m_sFileExtension.IsEqual_NoCase(sExt))
    {
      return nsStatus(NS_SUCCESS);
    }
  }

  return nsStatus("File extension is not handled by any registered type");
}

void nsDocumentManager::EnsureWindowRequested(nsDocument* pDocument, const nsDocumentObject* pOpenContext /*= nullptr*/)
{
  if (pDocument->m_bWindowRequested)
    return;

  NS_PROFILE_SCOPE("EnsureWindowRequested");
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

nsStatus nsDocumentManager::CreateOrOpenDocument(bool bCreate, nsStringView sDocumentTypeName, nsStringView sPath2, nsDocument*& out_pDocument,
  nsBitflags<nsDocumentFlags> flags, const nsDocumentObject* pOpenContext /*= nullptr*/)
{
  nsFileStats fs;
  nsStringBuilder sPath = sPath2;
  sPath.MakeCleanPath();
  if (!bCreate && nsOSFile::GetFileStats(sPath, fs).Failed())
  {
    return nsStatus("The file does not exist.");
  }

  Request r;
  r.m_Type = Request::Type::DocumentAllowedToOpen;
  r.m_RequestStatus.m_Result = NS_SUCCESS;
  r.m_sDocumentType = sDocumentTypeName;
  r.m_sDocumentPath = sPath;
  s_Requests.Broadcast(r);

  // if for example no project is open, or not the correct one, then a document cannot be opened
  if (r.m_RequestStatus.m_Result.Failed())
    return r.m_RequestStatus;

  out_pDocument = nullptr;

  nsStatus status;

  nsHybridArray<const nsDocumentTypeDescriptor*, 4> DocumentTypes;
  GetSupportedDocumentTypes(DocumentTypes);

  for (nsUInt32 i = 0; i < DocumentTypes.GetCount(); ++i)
  {
    if (DocumentTypes[i]->m_sDocumentTypeName == sDocumentTypeName)
    {
      // See if there is a default asset document registered for the type, if so clone
      // it and use that as the new document instead of creating one from scratch.
      if (bCreate && !flags.IsSet(nsDocumentFlags::EmptyDocument))
      {
        nsStringBuilder sTemplateDoc = "Editor/DocumentTemplates/Default";
        sTemplateDoc.ChangeFileExtension(sPath.GetFileExtension());

        if (nsFileSystem::ExistsFile(sTemplateDoc))
        {
          nsUuid CloneUuid;
          if (CloneDocument(sTemplateDoc, sPath, CloneUuid).Succeeded())
          {
            if (OpenDocument(sDocumentTypeName, sPath, out_pDocument, flags, pOpenContext).Succeeded())
            {
              return nsStatus(NS_SUCCESS);
            }
          }

          nsLog::Warning("Failed to create document from template '{}'", sTemplateDoc);
        }
      }

      NS_ASSERT_DEV(DocumentTypes[i]->m_bCanCreate, "This document manager cannot create the document type '{0}'", sDocumentTypeName);

      {
        NS_PROFILE_SCOPE(sDocumentTypeName);
        status = nsStatus(NS_SUCCESS);
        InternalCreateDocument(sDocumentTypeName, sPath, bCreate, out_pDocument, pOpenContext);
      }
      out_pDocument->SetAddToResetFilesList(flags.IsSet(nsDocumentFlags::AddToRecentFilesList));

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
          NS_PROFILE_SCOPE("InitializeAfterLoading");
          out_pDocument->InitializeAfterLoading(bCreate);
        }

        if (bCreate)
        {
          out_pDocument->SetModified(true);
          if (flags.IsSet(nsDocumentFlags::AsyncSave))
          {
            out_pDocument->SaveDocumentAsync({});
            status = nsStatus(NS_SUCCESS);
          }
          else
          {
            status = out_pDocument->SaveDocument();
          }
        }

        {
          NS_PROFILE_SCOPE("InitializeAfterLoadingAndSaving");
          out_pDocument->InitializeAfterLoadingAndSaving();
        }

        Event e;
        e.m_pDocument = out_pDocument;
        e.m_Type = Event::Type::DocumentOpened;

        s_Events.Broadcast(e);

        if (flags.IsSet(nsDocumentFlags::RequestWindow))
          EnsureWindowRequested(out_pDocument, pOpenContext);
      }

      return status;
    }
  }

  NS_REPORT_FAILURE("This document manager does not support the document type '{0}'", sDocumentTypeName);
  return status;
}

nsStatus nsDocumentManager::CreateDocument(
  nsStringView sDocumentTypeName, nsStringView sPath, nsDocument*& out_pDocument, nsBitflags<nsDocumentFlags> flags, const nsDocumentObject* pOpenContext)
{
  return CreateOrOpenDocument(true, sDocumentTypeName, sPath, out_pDocument, flags, pOpenContext);
}

nsStatus nsDocumentManager::OpenDocument(nsStringView sDocumentTypeName, nsStringView sPath, nsDocument*& out_pDocument,
  nsBitflags<nsDocumentFlags> flags, const nsDocumentObject* pOpenContext)
{
  return CreateOrOpenDocument(false, sDocumentTypeName, sPath, out_pDocument, flags, pOpenContext);
}


nsStatus nsDocumentManager::CloneDocument(nsStringView sPath, nsStringView sClonePath, nsUuid& inout_cloneGuid)
{
  const nsDocumentTypeDescriptor* pTypeDesc = nullptr;
  nsStatus res = nsDocumentUtils::IsValidSaveLocationForDocument(sClonePath, &pTypeDesc);
  if (res.Failed())
    return res;

  nsUniquePtr<nsAbstractObjectGraph> header;
  nsUniquePtr<nsAbstractObjectGraph> objects;
  nsUniquePtr<nsAbstractObjectGraph> types;

  res = nsDocument::ReadDocument(sPath, header, objects, types);
  if (res.Failed())
    return res;

  nsUuid documentId;
  nsAbstractObjectNode::Property* documentIdProp = nullptr;
  {
    auto* pHeaderNode = header->GetNodeByName("Header");
    NS_ASSERT_DEV(pHeaderNode, "No header found, document '{0}' is corrupted.", sPath);
    documentIdProp = pHeaderNode->FindProperty("DocumentID");
    NS_ASSERT_DEV(documentIdProp, "No document ID property found in header, document document '{0}' is corrupted.", sPath);
    documentId = documentIdProp->m_Value.Get<nsUuid>();
  }

  nsUuid seedGuid;
  if (inout_cloneGuid.IsValid())
  {
    seedGuid = inout_cloneGuid;
    seedGuid.RevertCombinationWithSeed(documentId);

    nsUuid test = documentId;
    test.CombineWithSeed(seedGuid);
    NS_ASSERT_DEV(test == inout_cloneGuid, "");
  }
  else
  {
    seedGuid = nsUuid::MakeUuid();
    inout_cloneGuid = documentId;
    inout_cloneGuid.CombineWithSeed(seedGuid);
  }

  InternalCloneDocument(sPath, sClonePath, documentId, seedGuid, inout_cloneGuid, header.Borrow(), objects.Borrow(), types.Borrow());

  {
    nsDeferredFileWriter file;
    file.SetOutput(sClonePath);
    nsAbstractGraphDdlSerializer::WriteDocument(file, header.Borrow(), objects.Borrow(), types.Borrow(), false);
    if (file.Close() == NS_FAILURE)
    {
      return nsStatus(nsFmt("Unable to open file '{0}' for writing!", sClonePath));
    }
  }
  return nsStatus(NS_SUCCESS);
}

void nsDocumentManager::InternalCloneDocument(nsStringView sPath, nsStringView sClonePath, const nsUuid& documentId, const nsUuid& seedGuid, const nsUuid& cloneGuid, nsAbstractObjectGraph* header, nsAbstractObjectGraph* objects, nsAbstractObjectGraph* types)
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
    nsAbstractObjectNode::Property* pProp = pNode->FindProperty("MetaPrefabSeed");
    if (pProp && pProp->m_Value.IsA<nsUuid>())
    {
      nsUuid prefabSeed = pProp->m_Value.Get<nsUuid>();
      prefabSeed.CombineWithSeed(seedGuid);
      pProp->m_Value = prefabSeed;
    }
  }
}

void nsDocumentManager::CloseDocument(nsDocument* pDocument)
{
  NS_ASSERT_DEV(pDocument != nullptr, "Invalid document pointer");

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

void nsDocumentManager::CloseAllDocumentsOfManager()
{
  while (!m_AllOpenDocuments.IsEmpty())
  {
    CloseDocument(m_AllOpenDocuments[0]);
  }
}

void nsDocumentManager::CloseAllDocuments()
{
  for (nsDocumentManager* pMan : s_AllDocumentManagers)
  {
    pMan->CloseAllDocumentsOfManager();
  }
}

nsDocument* nsDocumentManager::GetDocumentByPath(nsStringView sPath) const
{
  nsStringBuilder sPath2 = sPath;
  sPath2.MakeCleanPath();

  for (nsDocument* pDoc : m_AllOpenDocuments)
  {
    if (sPath2.IsEqual_NoCase(pDoc->GetDocumentPath()))
      return pDoc;
  }

  return nullptr;
}


nsDocument* nsDocumentManager::GetDocumentByGuid(const nsUuid& guid)
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


bool nsDocumentManager::EnsureDocumentIsClosedInAllManagers(nsStringView sPath)
{
  bool bClosedAny = false;
  for (auto man : s_AllDocumentManagers)
  {
    if (man->EnsureDocumentIsClosed(sPath))
      bClosedAny = true;
  }

  return bClosedAny;
}

bool nsDocumentManager::EnsureDocumentIsClosed(nsStringView sPath)
{
  auto pDoc = GetDocumentByPath(sPath);

  if (pDoc == nullptr)
    return false;

  CloseDocument(pDoc);

  return true;
}

nsResult nsDocumentManager::FindDocumentTypeFromPath(nsStringView sPath, bool bForCreation, const nsDocumentTypeDescriptor*& out_pTypeDesc)
{
  const nsString sFileExt = nsPathUtils::GetFileExtension(sPath);

  const auto& allDesc = GetAllDocumentDescriptors();

  for (auto it : allDesc)
  {
    const auto* desc = it.Value();

    if (bForCreation && !desc->m_bCanCreate)
      continue;

    if (desc->m_sFileExtension.IsEqual_NoCase(sFileExt))
    {
      out_pTypeDesc = desc;
      return NS_SUCCESS;
    }
  }

  return NS_FAILURE;
}

const nsMap<nsString, const nsDocumentTypeDescriptor*>& nsDocumentManager::GetAllDocumentDescriptors()
{
  if (s_AllDocumentDescriptors.IsEmpty())
  {
    for (nsDocumentManager* pMan : nsDocumentManager::GetAllDocumentManagers())
    {
      nsHybridArray<const nsDocumentTypeDescriptor*, 4> descriptors;
      pMan->GetSupportedDocumentTypes(descriptors);

      for (auto pDesc : descriptors)
      {
        s_AllDocumentDescriptors[pDesc->m_sDocumentTypeName] = pDesc;
      }
    }
  }

  return s_AllDocumentDescriptors;
}

const nsDocumentTypeDescriptor* nsDocumentManager::GetDescriptorForDocumentType(nsStringView sDocumentType)
{
  return GetAllDocumentDescriptors().GetValueOrDefault(sDocumentType, nullptr);
}

/// \todo on close doc: remove from m_AllDocuments
