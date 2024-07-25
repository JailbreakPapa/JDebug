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
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDocumentObjectMetaData, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    //NS_MEMBER_PROPERTY("MetaHidden", m_bHidden) // remove this property to disable serialization
    NS_MEMBER_PROPERTY("MetaFromPrefab", m_CreateFromPrefab),
    NS_MEMBER_PROPERTY("MetaPrefabSeed", m_PrefabSeedGuid),
    NS_MEMBER_PROPERTY("MetaBasePrefab", m_sBasePrefab),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDocumentInfo, 1, nsRTTINoAllocator)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("DocumentID", m_DocumentID),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsDocumentInfo::nsDocumentInfo()
{
  m_DocumentID = nsUuid::MakeUuid();
}


NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsDocument, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;

nsEvent<const nsDocumentEvent&> nsDocument::s_EventsAny;

nsDocument::nsDocument(nsStringView sPath, nsDocumentObjectManager* pDocumentObjectManagerImpl)
{
  using ObjectMetaData = nsObjectMetaData<nsUuid, nsDocumentObjectMetaData>;
  m_DocumentObjectMetaData = NS_DEFAULT_NEW(ObjectMetaData);
  m_pDocumentInfo = nullptr;
  m_sDocumentPath = sPath;
  m_pObjectManager = nsUniquePtr<nsDocumentObjectManager>(pDocumentObjectManagerImpl, nsFoundation::GetDefaultAllocator());
  m_pObjectManager->SetDocument(this);
  m_pCommandHistory = NS_DEFAULT_NEW(nsCommandHistory, this);
  m_pSelectionManager = NS_DEFAULT_NEW(nsSelectionManager, m_pObjectManager.Borrow());

  if (m_pObjectAccessor == nullptr)
  {
    m_pObjectAccessor = NS_DEFAULT_NEW(nsObjectCommandAccessor, m_pCommandHistory.Borrow());
  }

  m_bWindowRequested = false;
  m_bModified = true;
  m_bReadOnly = false;
  m_bAddToRecentFilesList = true;

  m_uiUnknownObjectTypeInstances = 0;

  m_pHostDocument = this;
  m_pActiveSubDocument = this;
}

nsDocument::~nsDocument()
{
  m_pSelectionManager = nullptr;

  m_pObjectManager->DestroyAllObjects();

  m_pCommandHistory->ClearRedoHistory();
  m_pCommandHistory->ClearUndoHistory();

  NS_DEFAULT_DELETE(m_pDocumentInfo);
}

void nsDocument::SetupDocumentInfo(const nsDocumentTypeDescriptor* pTypeDescriptor)
{
  m_pTypeDescriptor = pTypeDescriptor;
  m_pDocumentInfo = CreateDocumentInfo();

  NS_ASSERT_DEV(m_pDocumentInfo != nullptr, "invalid document info");
}

void nsDocument::SetModified(bool b)
{
  if (m_bModified == b)
    return;

  m_bModified = b;

  nsDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = nsDocumentEvent::Type::ModifiedChanged;

  m_EventsOne.Broadcast(e);
  s_EventsAny.Broadcast(e);
}

void nsDocument::SetReadOnly(bool b)
{
  if (m_bReadOnly == b)
    return;

  m_bReadOnly = b;

  nsDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = nsDocumentEvent::Type::ReadOnlyChanged;

  m_EventsOne.Broadcast(e);
  s_EventsAny.Broadcast(e);
}

nsStatus nsDocument::SaveDocument(bool bForce)
{
  if (!IsModified() && !bForce)
    return nsStatus(NS_SUCCESS);

  // In the unlikely event that we manage to edit a doc and call save again while
  // an async save is already in progress we block on the first save to ensure
  // the correct chronological state on disk after both save ops are done.
  if (m_ActiveSaveTask.IsValid())
  {
    nsTaskSystem::WaitForGroup(m_ActiveSaveTask);
    m_ActiveSaveTask.Invalidate();
  }
  nsStatus result;
  m_ActiveSaveTask = InternalSaveDocument([&result](nsDocument* pDoc, nsStatus res)
    { result = res; });
  nsTaskSystem::WaitForGroup(m_ActiveSaveTask);
  m_ActiveSaveTask.Invalidate();
  return result;
}


nsTaskGroupID nsDocument::SaveDocumentAsync(AfterSaveCallback callback, bool bForce)
{
  if (!IsModified() && !bForce)
    return nsTaskGroupID();

  m_ActiveSaveTask = InternalSaveDocument(callback);
  return m_ActiveSaveTask;
}

void nsDocument::DocumentRenamed(nsStringView sNewDocumentPath)
{
  m_sDocumentPath = sNewDocumentPath;

  nsDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = nsDocumentEvent::Type::DocumentRenamed;

  m_EventsOne.Broadcast(e);
  s_EventsAny.Broadcast(e);
}

void nsDocument::EnsureVisible()
{
  nsDocumentEvent e;
  e.m_pDocument = this;
  e.m_Type = nsDocumentEvent::Type::EnsureVisible;

  m_EventsOne.Broadcast(e);
  s_EventsAny.Broadcast(e);
}

nsTaskGroupID nsDocument::InternalSaveDocument(AfterSaveCallback callback)
{
  NS_PROFILE_SCOPE("InternalSaveDocument");
  nsTaskGroupID saveID = nsTaskSystem::CreateTaskGroup(nsTaskPriority::LongRunningHighPriority);
  auto saveTask = NS_DEFAULT_NEW(nsSaveDocumentTask);

  {
    saveTask->m_document = this;
    saveTask->file.SetOutput(m_sDocumentPath);
    nsTaskSystem::AddTaskToGroup(saveID, saveTask);

    {
      nsRttiConverterContext context;
      nsRttiConverterWriter rttiConverter(&saveTask->headerGraph, &context, true, true);
      context.RegisterObject(GetGuid(), m_pDocumentInfo->GetDynamicRTTI(), m_pDocumentInfo);
      rttiConverter.AddObjectToGraph(m_pDocumentInfo, "Header");
    }
    {
      // Do not serialize any temporary properties into the document.
      auto filter = [](const nsDocumentObject*, const nsAbstractProperty* pProp) -> bool
      {
        if (pProp->GetAttributeByType<nsTemporaryAttribute>() != nullptr)
          return false;
        return true;
      };
      nsDocumentObjectConverterWriter objectConverter(&saveTask->objectGraph, GetObjectManager(), filter);
      objectConverter.AddObjectToGraph(GetObjectManager()->GetRootObject(), "ObjectTree");

      AttachMetaDataBeforeSaving(saveTask->objectGraph);
    }
    {
      nsSet<const nsRTTI*> types;
      nsToolsReflectionUtils::GatherObjectTypes(GetObjectManager()->GetRootObject(), types);
      nsToolsSerializationUtils::SerializeTypes(types, saveTask->typesGraph);
    }
  }

  nsTaskGroupID afterSaveID = nsTaskSystem::CreateTaskGroup(nsTaskPriority::SomeFrameMainThread);
  {
    auto afterSaveTask = NS_DEFAULT_NEW(nsAfterSaveDocumentTask);
    afterSaveTask->m_document = this;
    afterSaveTask->m_callback = callback;
    nsTaskSystem::AddTaskToGroup(afterSaveID, afterSaveTask);
  }
  nsTaskSystem::AddTaskGroupDependency(afterSaveID, saveID);
  if (!nsTaskSystem::IsTaskGroupFinished(m_ActiveSaveTask))
  {
    nsTaskSystem::AddTaskGroupDependency(saveID, m_ActiveSaveTask);
  }

  nsTaskSystem::StartTaskGroup(saveID);
  nsTaskSystem::StartTaskGroup(afterSaveID);
  return afterSaveID;
}

nsStatus nsDocument::ReadDocument(nsStringView sDocumentPath, nsUniquePtr<nsAbstractObjectGraph>& ref_pHeader, nsUniquePtr<nsAbstractObjectGraph>& ref_pObjects,
  nsUniquePtr<nsAbstractObjectGraph>& ref_pTypes)
{
  nsDefaultMemoryStreamStorage storage;
  nsMemoryStreamReader memreader(&storage);

  {
    NS_PROFILE_SCOPE("Read File");
    nsFileReader file;
    if (file.Open(sDocumentPath) == NS_FAILURE)
    {
      return nsStatus("Unable to open file for reading!");
    }

    // range.BeginNextStep("Reading File");
    storage.ReadAll(file);

    // range.BeginNextStep("Parsing Graph");
    {
      NS_PROFILE_SCOPE("parse DDL graph");
      nsStopwatch sw;
      if (nsAbstractGraphDdlSerializer::ReadDocument(memreader, ref_pHeader, ref_pObjects, ref_pTypes, true).Failed())
        return nsStatus("Failed to parse DDL graph");

      nsTime t = sw.GetRunningTotal();
      nsLog::Debug("DDL parsing time: {0} msec", nsArgF(t.GetMilliseconds(), 1));
    }
  }
  return nsStatus(NS_SUCCESS);
}

nsStatus nsDocument::ReadAndRegisterTypes(const nsAbstractObjectGraph& types)
{
  NS_PROFILE_SCOPE("Deserializing Types");
  // range.BeginNextStep("Deserializing Types");

  // Deserialize and register serialized phantom types.
  nsString sDescTypeName = nsGetStaticRTTI<nsReflectedTypeDescriptor>()->GetTypeName();
  nsDynamicArray<nsReflectedTypeDescriptor*> descriptors;
  auto& nodes = types.GetAllNodes();
  descriptors.Reserve(nodes.GetCount()); // Overkill but doesn't matter much as it's just temporary.
  nsRttiConverterContext context;
  nsRttiConverterReader rttiConverter(&types, &context);

  for (auto it = nodes.GetIterator(); it.IsValid(); ++it)
  {
    if (it.Value()->GetType() == sDescTypeName)
    {
      nsReflectedTypeDescriptor* pDesc = rttiConverter.CreateObjectFromNode(it.Value()).Cast<nsReflectedTypeDescriptor>();
      if (pDesc->m_Flags.IsSet(nsTypeFlags::Minimal))
      {
        nsGetStaticRTTI<nsReflectedTypeDescriptor>()->GetAllocator()->Deallocate(pDesc);
      }
      else
      {
        descriptors.PushBack(pDesc);
      }
    }
  }
  nsToolsReflectionUtils::DependencySortTypeDescriptorArray(descriptors);
  for (nsReflectedTypeDescriptor* desc : descriptors)
  {
    if (!nsRTTI::FindTypeByName(desc->m_sTypeName))
    {
      nsPhantomRttiManager::RegisterType(*desc);
    }
    nsGetStaticRTTI<nsReflectedTypeDescriptor>()->GetAllocator()->Deallocate(desc);
  }
  return nsStatus(NS_SUCCESS);
}

nsStatus nsDocument::InternalLoadDocument()
{
  NS_PROFILE_SCOPE("InternalLoadDocument");
  // this would currently crash in Qt, due to the processEvents in the QtProgressBar
  // nsProgressRange range("Loading Document", 5, false);

  nsUniquePtr<nsAbstractObjectGraph> header;
  nsUniquePtr<nsAbstractObjectGraph> objects;
  nsUniquePtr<nsAbstractObjectGraph> types;

  nsStatus res = ReadDocument(m_sDocumentPath, header, objects, types);
  if (res.Failed())
    return res;

  res = ReadAndRegisterTypes(*types.Borrow());
  if (res.Failed())
    return res;

  {
    NS_PROFILE_SCOPE("Restoring Header");
    nsRttiConverterContext context;
    nsRttiConverterReader rttiConverter(header.Borrow(), &context);
    auto* pHeaderNode = header->GetNodeByName("Header");
    rttiConverter.ApplyPropertiesToObject(pHeaderNode, m_pDocumentInfo->GetDynamicRTTI(), m_pDocumentInfo);
  }

  {
    NS_PROFILE_SCOPE("Restoring Objects");
    nsDocumentObjectConverterReader objectConverter(
      objects.Borrow(), GetObjectManager(), nsDocumentObjectConverterReader::Mode::CreateAndAddToDocument);
    // range.BeginNextStep("Restoring Objects");
    auto* pRootNode = objects->GetNodeByName("ObjectTree");
    objectConverter.ApplyPropertiesToObject(pRootNode, GetObjectManager()->GetRootObject());

    SetUnknownObjectTypes(objectConverter.GetUnknownObjectTypes(), objectConverter.GetNumUnknownObjectCreations());
  }

  {
    NS_PROFILE_SCOPE("Restoring Meta-Data");
    // range.BeginNextStep("Restoring Meta-Data");
    RestoreMetaDataAfterLoading(*objects.Borrow(), false);
  }

  SetModified(false);
  return nsStatus(NS_SUCCESS);
}

void nsDocument::AttachMetaDataBeforeSaving(nsAbstractObjectGraph& graph) const
{
  m_DocumentObjectMetaData->AttachMetaDataToAbstractGraph(graph);
}

void nsDocument::RestoreMetaDataAfterLoading(const nsAbstractObjectGraph& graph, bool bUndoable)
{
  m_DocumentObjectMetaData->RestoreMetaDataFromAbstractGraph(graph);
}

void nsDocument::BeforeClosing()
{
  // This can't be done in the dtor as the task uses virtual functions on this object.
  if (m_ActiveSaveTask.IsValid())
  {
    nsTaskSystem::WaitForGroup(m_ActiveSaveTask);
    m_ActiveSaveTask.Invalidate();
  }
}

void nsDocument::SetUnknownObjectTypes(const nsSet<nsString>& Types, nsUInt32 uiInstances)
{
  m_UnknownObjectTypes = Types;
  m_uiUnknownObjectTypeInstances = uiInstances;
}


void nsDocument::BroadcastInterDocumentMessage(nsReflectedClass* pMessage, nsDocument* pSender)
{
  for (auto& man : nsDocumentManager::GetAllDocumentManagers())
  {
    for (auto pDoc : man->GetAllOpenDocuments())
    {
      if (pDoc == pSender)
        continue;

      pDoc->OnInterDocumentMessage(pMessage, pSender);
    }
  }
}

void nsDocument::DeleteSelectedObjects() const
{
  nsHybridArray<nsSelectionEntry, 64> objects;
  GetSelectionManager()->GetTopLevelSelection(objects);

  // make sure the whole selection is cleared, otherwise each delete command would reduce the selection one by one
  GetSelectionManager()->Clear();

  auto history = GetCommandHistory();
  history->StartTransaction("Delete Object");

  nsRemoveObjectCommand cmd;

  for (const nsSelectionEntry& entry : objects)
  {
    cmd.m_Object = entry.m_pObject->GetGuid();

    if (history->AddCommand(cmd).m_Result.Failed())
    {
      history->CancelTransaction();
      return;
    }
  }

  history->FinishTransaction();
}

void nsDocument::ShowDocumentStatus(const nsFormatString& msg) const
{
  nsStringBuilder tmp;

  nsDocumentEvent e;
  e.m_pDocument = this;
  e.m_sStatusMsg = msg.GetText(tmp);
  e.m_Type = nsDocumentEvent::Type::DocumentStatusMsg;

  m_EventsOne.Broadcast(e);
}


nsResult nsDocument::ComputeObjectTransformation(const nsDocumentObject* pObject, nsTransform& out_result) const
{
  out_result.SetIdentity();
  return NS_FAILURE;
}

nsObjectAccessorBase* nsDocument::GetObjectAccessor() const
{
  return m_pObjectAccessor.Borrow();
}
