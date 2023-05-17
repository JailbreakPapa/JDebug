#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/IO/FileSystem/DeferredFileWriter.h>
#include <Foundation/IO/FileSystem/FileReader.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/IO/MemoryStream.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Serialization/DdlSerializer.h>
#include <Foundation/Serialization/RttiConverter.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Time/Stopwatch.h>
#include <Foundation/Utilities/Progress.h>
#include <ToolsFoundation/Command/TreeCommands.h>
#include <ToolsFoundation/Document/Document.h>
#include <ToolsFoundation/Document/DocumentManager.h>
#include <ToolsFoundation/Document/DocumentTasks.h>
#include <ToolsFoundation/Document/PrefabCache.h>
#include <ToolsFoundation/Document/PrefabUtils.h>
#include <ToolsFoundation/Object/ObjectCommandAccessor.h>
#include <ToolsFoundation/Serialization/DocumentObjectConverter.h>
#include <ToolsFoundation/Serialization/ToolsSerializationUtils.h>

// clang-format off
WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDocumentObjectMetaData, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    //WD_MEMBER_PROPERTY("MetaHidden", m_bHidden) // remove this property to disable serialization
    WD_MEMBER_PROPERTY("MetaFromPrefab", m_CreateFromPrefab),
    WD_MEMBER_PROPERTY("MetaPrefabSeed", m_PrefabSeedGuid),
    WD_MEMBER_PROPERTY("MetaBasePrefab", m_sBasePrefab),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;

WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDocumentInfo, 1, wdRTTINoAllocator)
{
  WD_BEGIN_PROPERTIES
  {
    WD_MEMBER_PROPERTY("DocumentID", m_DocumentID),
  }
  WD_END_PROPERTIES;
}
WD_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

wdDocumentInfo::wdDocumentInfo()
{
  m_DocumentID.CreateNewUuid();
}


WD_BEGIN_DYNAMIC_REFLECTED_TYPE(wdDocument, 1, wdRTTINoAllocator)
WD_END_DYNAMIC_REFLECTED_TYPE;

wdEvent<const wdDocumentEvent&> wdDocument::s_EventsAny;

wdDocument::wdDocument(const char* szPath, wdDocumentObjectManager* pDocumentObjectManagerImpl)
{
  using ObjectMetaData = wdObjectMetaData<wdUuid, wdDocumentObjectMetaData>;
  m_DocumentObjectMetaData = WD_DEFAULT_NEW(ObjectMetaData);
  m_pDocumentInfo = nullptr;
  m_sDocumentPath = szPath;
  m_pObjectManager = wdUniquePtr<wdDocumentObjectManager>(pDocumentObjectManagerImpl, wdFoundation::GetDefaultAllocator());
  m_pObjectManager->SetDocument(this);
  m_pCommandHistory = WD_DEFAULT_NEW(wdCommandHistory, this);
  m_pSelectionManager = WD_DEFAULT_NEW(wdSelectionManager, m_pObjectManager.Borrow());
  m_pObjectAccessor = WD_DEFAULT_NEW(wdObjectCommandAccessor, m_pCommandHistory.Borrow());

  m_bWindowRequested = false;
  m_bModified = true;
  m_bReadOnly = false;
  m_bAddToRecentFilesList = true;

  m_uiUnknownObjectTypeInstances = 0;

  m_pHostDocument = this;
  m_pActiveSubDocument = this;
}

wdDocument::~wdDocument()
{
  m_pSelectionManager = nullptr;

  m_pObjectManager->DestroyAllObjects();

  m_pCommandHistory->ClearRedoHistory();
  m_pCommandHistory->ClearUndoHistory();

  WD_DEFAULT_DELETE(m_pDocumentInfo);
}

void wdDocument::SetupDocumentInfo(const wdDocumentTypeDescriptor* pTypeDescriptor)
{
  m_pTypeDescriptor = pTypeDescriptor;
  m_pDocumentInfo = CreateDocumentInfo();

  WD_ASSERT_DEV(m_pDocumentInfo != nullptr, "invalid document info");
}

void wdDocument::SetModified(bool b)
{
  if (m_bModified == b)
    return;

  m_bModified = b;

  wdDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = wdDocumentEvent::Type::ModifiedChanged;

  m_EventsOne.Broadcast(e);
  s_EventsAny.Broadcast(e);
}

void wdDocument::SetReadOnly(bool b)
{
  if (m_bReadOnly == b)
    return;

  m_bReadOnly = b;

  wdDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = wdDocumentEvent::Type::ReadOnlyChanged;

  m_EventsOne.Broadcast(e);
  s_EventsAny.Broadcast(e);
}

wdStatus wdDocument::SaveDocument(bool bForce)
{
  if (!IsModified() && !bForce)
    return wdStatus(WD_SUCCESS);

  // In the unlikely event that we manage to edit a doc and call save again while
  // an async save is already in progress we block on the first save to ensure
  // the correct chronological state on disk after both save ops are done.
  if (m_ActiveSaveTask.IsValid())
  {
    wdTaskSystem::WaitForGroup(m_ActiveSaveTask);
    m_ActiveSaveTask.Invalidate();
  }
  wdStatus result;
  m_ActiveSaveTask = InternalSaveDocument([&result](wdDocument* pDoc, wdStatus res) { result = res; });
  wdTaskSystem::WaitForGroup(m_ActiveSaveTask);
  m_ActiveSaveTask.Invalidate();
  return result;
}


wdTaskGroupID wdDocument::SaveDocumentAsync(AfterSaveCallback callback, bool bForce)
{
  if (!IsModified() && !bForce)
    return wdTaskGroupID();

  m_ActiveSaveTask = InternalSaveDocument(callback);
  return m_ActiveSaveTask;
}

void wdDocument::EnsureVisible()
{
  wdDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = wdDocumentEvent::Type::EnsureVisible;

  m_EventsOne.Broadcast(e);
  s_EventsAny.Broadcast(e);
}

wdTaskGroupID wdDocument::InternalSaveDocument(AfterSaveCallback callback)
{
  WD_PROFILE_SCOPE("InternalSaveDocument");
  wdTaskGroupID saveID = wdTaskSystem::CreateTaskGroup(wdTaskPriority::LongRunningHighPriority);
  auto saveTask = WD_DEFAULT_NEW(wdSaveDocumentTask);

  {
    saveTask->m_document = this;
    saveTask->file.SetOutput(m_sDocumentPath);
    wdTaskSystem::AddTaskToGroup(saveID, saveTask);

    {
      wdRttiConverterContext context;
      wdRttiConverterWriter rttiConverter(&saveTask->headerGraph, &context, true, true);
      context.RegisterObject(GetGuid(), m_pDocumentInfo->GetDynamicRTTI(), m_pDocumentInfo);
      rttiConverter.AddObjectToGraph(m_pDocumentInfo, "Header");
    }
    {
      // Do not serialize any temporary properties into the document.
      auto filter = [](const wdDocumentObject*, const wdAbstractProperty* pProp) -> bool
      {
        if (pProp->GetAttributeByType<wdTemporaryAttribute>() != nullptr)
          return false;
        return true;
      };
      wdDocumentObjectConverterWriter objectConverter(&saveTask->objectGraph, GetObjectManager(), filter);
      objectConverter.AddObjectToGraph(GetObjectManager()->GetRootObject(), "ObjectTree");

      AttachMetaDataBeforeSaving(saveTask->objectGraph);
    }
    {
      wdSet<const wdRTTI*> types;
      wdToolsReflectionUtils::GatherObjectTypes(GetObjectManager()->GetRootObject(), types);
      wdToolsSerializationUtils::SerializeTypes(types, saveTask->typesGraph);
    }
  }

  wdTaskGroupID afterSaveID = wdTaskSystem::CreateTaskGroup(wdTaskPriority::SomeFrameMainThread);
  {
    auto afterSaveTask = WD_DEFAULT_NEW(wdAfterSaveDocumentTask);
    afterSaveTask->m_document = this;
    afterSaveTask->m_callback = callback;
    wdTaskSystem::AddTaskToGroup(afterSaveID, afterSaveTask);
  }
  wdTaskSystem::AddTaskGroupDependency(afterSaveID, saveID);
  if (!wdTaskSystem::IsTaskGroupFinished(m_ActiveSaveTask))
  {
    wdTaskSystem::AddTaskGroupDependency(saveID, m_ActiveSaveTask);
  }

  wdTaskSystem::StartTaskGroup(saveID);
  wdTaskSystem::StartTaskGroup(afterSaveID);
  return afterSaveID;
}

wdStatus wdDocument::ReadDocument(const char* szDocumentPath, wdUniquePtr<wdAbstractObjectGraph>& ref_pHeader, wdUniquePtr<wdAbstractObjectGraph>& ref_pObjects,
  wdUniquePtr<wdAbstractObjectGraph>& ref_pTypes)
{
  wdDefaultMemoryStreamStorage storage;
  wdMemoryStreamReader memreader(&storage);

  {
    WD_PROFILE_SCOPE("Read File");
    wdFileReader file;
    if (file.Open(szDocumentPath) == WD_FAILURE)
    {
      return wdStatus("Unable to open file for reading!");
    }

    // range.BeginNextStep("Reading File");
    storage.ReadAll(file);

    // range.BeginNextStep("Parsing Graph");
    {
      WD_PROFILE_SCOPE("parse DDL graph");
      wdStopwatch sw;
      if (wdAbstractGraphDdlSerializer::ReadDocument(memreader, ref_pHeader, ref_pObjects, ref_pTypes, true).Failed())
        return wdStatus("Failed to parse DDL graph");

      wdTime t = sw.GetRunningTotal();
      wdLog::Debug("DDL parsing time: {0} msec", wdArgF(t.GetMilliseconds(), 1));
    }
  }
  return wdStatus(WD_SUCCESS);
}

wdStatus wdDocument::ReadAndRegisterTypes(const wdAbstractObjectGraph& types)
{
  WD_PROFILE_SCOPE("Deserializing Types");
  // range.BeginNextStep("Deserializing Types");

  // Deserialize and register serialized phantom types.
  wdString sDescTypeName = wdGetStaticRTTI<wdReflectedTypeDescriptor>()->GetTypeName();
  wdDynamicArray<wdReflectedTypeDescriptor*> descriptors;
  auto& nodes = types.GetAllNodes();
  descriptors.Reserve(nodes.GetCount()); // Overkill but doesn't matter much as it's just temporary.
  wdRttiConverterContext context;
  wdRttiConverterReader rttiConverter(&types, &context);

  for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value()->GetType() == sDescTypeName)
    {
      wdReflectedTypeDescriptor* pDesc = rttiConverter.CreateObjectFromNode(it.Value()).Cast<wdReflectedTypeDescriptor>();
      if (pDesc->m_Flags.IsSet(wdTypeFlags::Minimal))
      {
        wdGetStaticRTTI<wdReflectedTypeDescriptor>()->GetAllocator()->Deallocate(pDesc);
      }
      else
      {
        descriptors.PushBack(pDesc);
      }
    }
  }
  wdToolsReflectionUtils::DependencySortTypeDescriptorArray(descriptors);
  for (wdReflectedTypeDescriptor* desc : descriptors)
  {
    if (!wdRTTI::FindTypeByName(desc->m_sTypeName))
    {
      wdPhantomRttiManager::RegisterType(*desc);
    }
    wdGetStaticRTTI<wdReflectedTypeDescriptor>()->GetAllocator()->Deallocate(desc);
  }
  return wdStatus(WD_SUCCESS);
}

wdStatus wdDocument::InternalLoadDocument()
{
  WD_PROFILE_SCOPE("InternalLoadDocument");
  // this would currently crash in Qt, due to the processEvents in the QtProgressBar
  // wdProgressRange range("Loading Document", 5, false);

  wdUniquePtr<wdAbstractObjectGraph> header;
  wdUniquePtr<wdAbstractObjectGraph> objects;
  wdUniquePtr<wdAbstractObjectGraph> types;

  wdStatus res = ReadDocument(m_sDocumentPath, header, objects, types);
  if (res.Failed())
    return res;

  res = ReadAndRegisterTypes(*types.Borrow());
  if (res.Failed())
    return res;

  {
    WD_PROFILE_SCOPE("Restoring Header");
    wdRttiConverterContext context;
    wdRttiConverterReader rttiConverter(header.Borrow(), &context);
    auto* pHeaderNode = header->GetNodeByName("Header");
    rttiConverter.ApplyPropertiesToObject(pHeaderNode, m_pDocumentInfo->GetDynamicRTTI(), m_pDocumentInfo);
  }

  {
    WD_PROFILE_SCOPE("Restoring Objects");
    wdDocumentObjectConverterReader objectConverter(
      objects.Borrow(), GetObjectManager(), wdDocumentObjectConverterReader::Mode::CreateAndAddToDocument);
    // range.BeginNextStep("Restoring Objects");
    auto* pRootNode = objects->GetNodeByName("ObjectTree");
    objectConverter.ApplyPropertiesToObject(pRootNode, GetObjectManager()->GetRootObject());

    SetUnknownObjectTypes(objectConverter.GetUnknownObjectTypes(), objectConverter.GetNumUnknownObjectCreations());
  }

  {
    WD_PROFILE_SCOPE("Restoring Meta-Data");
    // range.BeginNextStep("Restoring Meta-Data");
    RestoreMetaDataAfterLoading(*objects.Borrow(), false);
  }

  SetModified(false);
  return wdStatus(WD_SUCCESS);
}

void wdDocument::AttachMetaDataBeforeSaving(wdAbstractObjectGraph& graph) const
{
  m_DocumentObjectMetaData->AttachMetaDataToAbstractGraph(graph);
}

void wdDocument::RestoreMetaDataAfterLoading(const wdAbstractObjectGraph& graph, bool bUndoable)
{
  m_DocumentObjectMetaData->RestoreMetaDataFromAbstractGraph(graph);
}

void wdDocument::BeforeClosing()
{
  // This can't be done in the dtor as the task uses virtual functions on this object.
  if (m_ActiveSaveTask.IsValid())
  {
    wdTaskSystem::WaitForGroup(m_ActiveSaveTask);
    m_ActiveSaveTask.Invalidate();
  }
}

void wdDocument::SetUnknownObjectTypes(const wdSet<wdString>& Types, wdUInt32 uiInstances)
{
  m_UnknownObjectTypes = Types;
  m_uiUnknownObjectTypeInstances = uiInstances;
}


void wdDocument::BroadcastInterDocumentMessage(wdReflectedClass* pMessage, wdDocument* pSender)
{
  for (auto& man : wdDocumentManager::GetAllDocumentManagers())
  {
    for (auto pDoc : man->GetAllOpenDocuments())
    {
      if (pDoc == pSender)
        continue;

      pDoc->OnInterDocumentMessage(pMessage, pSender);
    }
  }
}

void wdDocument::DeleteSelectedObjects() const
{
  auto objects = GetSelectionManager()->GetTopLevelSelection();

  // make sure the whole selection is cleared, otherwise each delete command would reduce the selection one by one
  GetSelectionManager()->Clear();

  auto history = GetCommandHistory();
  history->StartTransaction("Delete Object");

  wdRemoveObjectCommand cmd;

  for (const wdDocumentObject* pObject : objects)
  {
    cmd.m_Object = pObject->GetGuid();

    if (history->AddCommand(cmd).m_Result.Failed())
    {
      history->CancelTransaction();
      return;
    }
  }

  history->FinishTransaction();
}

void wdDocument::ShowDocumentStatus(const wdFormatString& msg) const
{
  wdStringBuilder tmp;

  wdDocumentEvent e;
  e.m_pDocument = this;
  e.m_szStatusMsg = msg.GetText(tmp);
  e.m_Type = wdDocumentEvent::Type::DocumentStatusMsg;

  m_EventsOne.Broadcast(e);
}


wdResult wdDocument::ComputeObjectTransformation(const wdDocumentObject* pObject, wdTransform& out_result) const
{
  out_result.SetIdentity();
  return WD_FAILURE;
}

wdObjectAccessorBase* wdDocument::GetObjectAccessor() const
{
  return m_pObjectAccessor.Borrow();
}
