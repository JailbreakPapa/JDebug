#include <Core/CorePCH.h>

#include <Core/Messages/DeleteObjectMessage.h>
#include <Core/Messages/HierarchyChangedMessages.h>
#include <Core/World/EventMessageHandlerComponent.h>
#include <Core/World/World.h>
#include <Core/World/WorldModule.h>
#include <Foundation/Memory/FrameAllocator.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Utilities/Stats.h>

nsStaticArray<nsWorld*, nsWorld::GetMaxNumWorlds()> nsWorld::s_Worlds;

static nsGameObjectHandle DefaultGameObjectReferenceResolver(const void* pData, nsComponentHandle hThis, nsStringView sProperty)
{
  const char* szRef = reinterpret_cast<const char*>(pData);

  if (nsStringUtils::IsNullOrEmpty(szRef))
    return nsGameObjectHandle();

  // this is a convention used by nsPrefabReferenceComponent:
  // a string starting with this means a 'global game object reference', ie a reference that is valid within the current world
  // what follows is an integer that is the internal storage of an nsGameObjectHandle
  // thus parsing the int and casting it to an nsGameObjectHandle gives the desired result
  if (nsStringUtils::StartsWith(szRef, "#!GGOR-"))
  {
    nsInt64 id;
    if (nsConversionUtils::StringToInt64(szRef + 7, id).Succeeded())
    {
      return nsGameObjectHandle(nsGameObjectId(reinterpret_cast<nsUInt64&>(id)));
    }
  }

  return nsGameObjectHandle();
}

//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_STATIC_REFLECTED_TYPE(nsWorld, nsNoBase, 1, nsRTTINoAllocator)
{
  NS_BEGIN_FUNCTIONS
  {
    NS_SCRIPT_FUNCTION_PROPERTY(DeleteObjectDelayed, In, "GameObject", In, "DeleteEmptyParents")->AddAttributes(
      new nsFunctionArgumentAttributes(1, new nsDefaultValueAttribute(true))),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_TryGetObjectWithGlobalKey, In, "GlobalKey")->AddFlags(nsPropertyFlags::Const),
    NS_SCRIPT_FUNCTION_PROPERTY(Reflection_GetClock)->AddFlags(nsPropertyFlags::Const),
  }
  NS_END_FUNCTIONS;
}
NS_END_STATIC_REFLECTED_TYPE;
// clang-format on

nsWorld::nsWorld(nsWorldDesc& ref_desc)
  : m_Data(ref_desc)
{
  m_pUpdateTask = NS_DEFAULT_NEW(nsDelegateTask<void>, "WorldUpdate", nsTaskNesting::Never, nsMakeDelegate(&nsWorld::UpdateFromThread, this));
  m_Data.m_pCoordinateSystemProvider->m_pOwnerWorld = this;

  nsStringBuilder sb = ref_desc.m_sName.GetString();
  sb.Append(".Update");
  m_pUpdateTask->ConfigureTask(sb, nsTaskNesting::Maybe);

  m_uiIndex = nsInvalidIndex;

  // find a free world slot
  const nsUInt32 uiWorldCount = s_Worlds.GetCount();
  for (nsUInt32 i = 0; i < uiWorldCount; i++)
  {
    if (s_Worlds[i] == nullptr)
    {
      s_Worlds[i] = this;
      m_uiIndex = i;
      break;
    }
  }

  if (m_uiIndex == nsInvalidIndex)
  {
    m_uiIndex = s_Worlds.GetCount();
    NS_ASSERT_DEV(m_uiIndex < GetMaxNumWorlds(), "Max world index reached: {}", GetMaxNumWorlds());
    static_assert(GetMaxNumWorlds() == NS_MAX_WORLDS);

    s_Worlds.PushBack(this);
  }

  SetGameObjectReferenceResolver(DefaultGameObjectReferenceResolver);
}

nsWorld::~nsWorld()
{
  SetWorldSimulationEnabled(false);

  NS_LOCK(GetWriteMarker());
  m_Data.Clear();

  s_Worlds[m_uiIndex] = nullptr;
  m_uiIndex = nsInvalidIndex;
}


void nsWorld::Clear()
{
  CheckForWriteAccess();

  while (GetObjectCount() > 0)
  {
    for (auto it = GetObjects(); it.IsValid(); ++it)
    {
      DeleteObjectNow(it->GetHandle());
    }

    if (GetObjectCount() > 0)
    {
      nsLog::Dev("Remaining objects after nsWorld::Clear: {}", GetObjectCount());
    }
  }

  for (nsWorldModule* pModule : m_Data.m_Modules)
  {
    if (pModule != nullptr)
    {
      pModule->WorldClear();
    }
  }

  // make sure all dead objects and components are cleared right now
  DeleteDeadObjects();
  DeleteDeadComponents();

  nsEventMessageHandlerComponent::ClearGlobalEventHandlersForWorld(this);
}

void nsWorld::SetCoordinateSystemProvider(const nsSharedPtr<nsCoordinateSystemProvider>& pProvider)
{
  NS_ASSERT_DEV(pProvider != nullptr, "Coordinate System Provider must not be null");

  m_Data.m_pCoordinateSystemProvider = pProvider;
  m_Data.m_pCoordinateSystemProvider->m_pOwnerWorld = this;
}

// a super simple, but also efficient random number generator
inline static nsUInt32 NextStableRandomSeed(nsUInt32& ref_uiSeed)
{
  ref_uiSeed = 214013L * ref_uiSeed + 2531011L;
  return ((ref_uiSeed >> 16) & 0x7FFFF);
}

nsGameObjectHandle nsWorld::CreateObject(const nsGameObjectDesc& desc, nsGameObject*& out_pObject)
{
  CheckForWriteAccess();

  NS_ASSERT_DEV(m_Data.m_Objects.GetCount() < GetMaxNumGameObjects(), "Max number of game objects reached: {}", GetMaxNumGameObjects());

  nsGameObject* pParentObject = nullptr;
  nsGameObject::TransformationData* pParentData = nullptr;
  nsUInt32 uiParentIndex = 0;
  nsUInt64 uiHierarchyLevel = 0;
  bool bDynamic = desc.m_bDynamic;

  if (TryGetObject(desc.m_hParent, pParentObject))
  {
    pParentData = pParentObject->m_pTransformationData;
    uiParentIndex = desc.m_hParent.m_InternalId.m_InstanceIndex;
    uiHierarchyLevel = pParentObject->m_uiHierarchyLevel + 1; // if there is a parent hierarchy level is parent level + 1
    NS_ASSERT_DEV(uiHierarchyLevel < GetMaxNumHierarchyLevels(), "Max hierarchy level reached: {}", GetMaxNumHierarchyLevels());
    bDynamic |= pParentObject->IsDynamic();
  }

  // get storage for the transformation data
  nsGameObject::TransformationData* pTransformationData = m_Data.CreateTransformationData(bDynamic, static_cast<nsUInt32>(uiHierarchyLevel));

  // get storage for the object itself
  nsGameObject* pNewObject = m_Data.m_ObjectStorage.Create();

  // insert the new object into the id mapping table
  nsGameObjectId newId = m_Data.m_Objects.Insert(pNewObject);
  newId.m_WorldIndex = nsGameObjectId::StorageType(m_uiIndex & (NS_MAX_WORLDS - 1));

  // fill out some data
  pNewObject->m_InternalId = newId;
  pNewObject->m_Flags = nsObjectFlags::None;
  pNewObject->m_Flags.AddOrRemove(nsObjectFlags::Dynamic, bDynamic);
  pNewObject->m_Flags.AddOrRemove(nsObjectFlags::ActiveFlag, desc.m_bActiveFlag);
  pNewObject->m_sName = desc.m_sName;
  pNewObject->m_uiParentIndex = uiParentIndex;
  pNewObject->m_Tags = desc.m_Tags;
  pNewObject->m_uiTeamID = desc.m_uiTeamID;

  static_assert((GetMaxNumHierarchyLevels() - 1) <= nsMath::MaxValue<nsUInt16>());
  pNewObject->m_uiHierarchyLevel = static_cast<nsUInt16>(uiHierarchyLevel);

  // fill out the transformation data
  pTransformationData->m_pObject = pNewObject;
  pTransformationData->m_pParentData = pParentData;
  pTransformationData->m_localPosition = nsSimdConversion::ToVec3(desc.m_LocalPosition);
  pTransformationData->m_localRotation = nsSimdConversion::ToQuat(desc.m_LocalRotation);
  pTransformationData->m_localScaling = nsSimdConversion::ToVec4(desc.m_LocalScaling.GetAsVec4(desc.m_LocalUniformScaling));
  pTransformationData->m_globalTransform = nsSimdTransform::MakeIdentity();
#if NS_ENABLED(NS_GAMEOBJECT_VELOCITY)
  pTransformationData->m_lastGlobalTransform = nsSimdTransform::MakeIdentity();
  pTransformationData->m_uiLastGlobalTransformUpdateCounter = nsInvalidIndex;
#endif
  pTransformationData->m_localBounds = nsSimdBBoxSphere::MakeInvalid();
  pTransformationData->m_localBounds.m_BoxHalfExtents.SetW(nsSimdFloat::MakeZero());
  pTransformationData->m_globalBounds = pTransformationData->m_localBounds;
  pTransformationData->m_hSpatialData.Invalidate();
  pTransformationData->m_uiSpatialDataCategoryBitmask = 0;
  pTransformationData->m_uiStableRandomSeed = desc.m_uiStableRandomSeed;

  // if seed is set to 0xFFFFFFFF, use the parent's seed to create a deterministic value for this object
  if (pTransformationData->m_uiStableRandomSeed == 0xFFFFFFFF && pTransformationData->m_pParentData != nullptr)
  {
    nsUInt32 seed = pTransformationData->m_pParentData->m_uiStableRandomSeed + pTransformationData->m_pParentData->m_pObject->GetChildCount();

    do
    {
      pTransformationData->m_uiStableRandomSeed = NextStableRandomSeed(seed);

    } while (pTransformationData->m_uiStableRandomSeed == 0 || pTransformationData->m_uiStableRandomSeed == 0xFFFFFFFF);
  }

  // if the seed is zero (or there was no parent to derive the seed from), assign a random value
  while (pTransformationData->m_uiStableRandomSeed == 0 || pTransformationData->m_uiStableRandomSeed == 0xFFFFFFFF)
  {
    pTransformationData->m_uiStableRandomSeed = GetRandomNumberGenerator().UInt();
  }

  pTransformationData->UpdateGlobalTransformNonRecursive(0);

  // link the transformation data to the game object
  pNewObject->m_pTransformationData = pTransformationData;

  // fix links
  LinkToParent(pNewObject);

  pNewObject->UpdateActiveState(pParentObject == nullptr ? true : pParentObject->IsActive());

  out_pObject = pNewObject;
  return nsGameObjectHandle(newId);
}

void nsWorld::DeleteObjectNow(const nsGameObjectHandle& hObject0, bool bAlsoDeleteEmptyParents /*= true*/)
{
  CheckForWriteAccess();

  nsGameObject* pObject = nullptr;
  if (!m_Data.m_Objects.TryGetValue(hObject0, pObject))
    return;

  nsGameObjectHandle hObject = hObject0;

  if (bAlsoDeleteEmptyParents)
  {
    nsGameObject* pParent = pObject->GetParent();

    while (pParent)
    {
      if (pParent->GetChildCount() != 1 || pParent->GetComponents().GetCount() != 0)
        break;

      pObject = pParent;

      pParent = pParent->GetParent();
    }

    hObject = pObject->GetHandle();
  }

  // inform external systems that we are about to delete this object
  m_Data.m_ObjectDeletionEvent.Broadcast(pObject);

  // set object to inactive so components and children know that they shouldn't access the object anymore.
  pObject->m_Flags.Remove(nsObjectFlags::ActiveFlag | nsObjectFlags::ActiveState);

  // delete children
  for (auto it = pObject->GetChildren(); it.IsValid(); ++it)
  {
    DeleteObjectNow(it->GetHandle(), false);
  }

  // delete attached components
  while (!pObject->m_Components.IsEmpty())
  {
    nsComponent* pComponent = pObject->m_Components[0];
    pComponent->GetOwningManager()->DeleteComponent(pComponent->GetHandle());
  }
  NS_ASSERT_DEV(pObject->m_Components.GetCount() == 0, "Components should already be removed");

  // fix parent and siblings
  UnlinkFromParent(pObject);

  // remove from global key tables
  SetObjectGlobalKey(pObject, nsHashedString());

  // invalidate (but preserve world index) and remove from id table
  pObject->m_InternalId.Invalidate();
  pObject->m_InternalId.m_WorldIndex = m_uiIndex;

  m_Data.m_DeadObjects.Insert(pObject);
  NS_VERIFY(m_Data.m_Objects.Remove(hObject), "Implementation error.");
}

void nsWorld::DeleteObjectDelayed(const nsGameObjectHandle& hObject, bool bAlsoDeleteEmptyParents /*= true*/)
{
  nsMsgDeleteGameObject msg;
  msg.m_bDeleteEmptyParents = bAlsoDeleteEmptyParents;
  PostMessage(hObject, msg, nsTime::MakeZero());
}

nsComponentInitBatchHandle nsWorld::CreateComponentInitBatch(nsStringView sBatchName, bool bMustFinishWithinOneFrame /*= true*/)
{
  auto pInitBatch = NS_NEW(GetAllocator(), nsInternal::WorldData::InitBatch, GetAllocator(), sBatchName, bMustFinishWithinOneFrame);
  return nsComponentInitBatchHandle(m_Data.m_InitBatches.Insert(pInitBatch));
}

void nsWorld::DeleteComponentInitBatch(const nsComponentInitBatchHandle& hBatch)
{
  auto& pInitBatch = m_Data.m_InitBatches[hBatch.GetInternalID()];
  NS_IGNORE_UNUSED(pInitBatch);
  NS_ASSERT_DEV(pInitBatch->m_ComponentsToInitialize.IsEmpty() && pInitBatch->m_ComponentsToStartSimulation.IsEmpty(), "Init batch has not been completely processed");
  m_Data.m_InitBatches.Remove(hBatch.GetInternalID());
}

void nsWorld::BeginAddingComponentsToInitBatch(const nsComponentInitBatchHandle& hBatch)
{
  NS_ASSERT_DEV(m_Data.m_pCurrentInitBatch == m_Data.m_pDefaultInitBatch, "Nested init batches are not supported");
  m_Data.m_pCurrentInitBatch = m_Data.m_InitBatches[hBatch.GetInternalID()].Borrow();
}

void nsWorld::EndAddingComponentsToInitBatch(const nsComponentInitBatchHandle& hBatch)
{
  NS_ASSERT_DEV(m_Data.m_InitBatches[hBatch.GetInternalID()] == m_Data.m_pCurrentInitBatch, "Init batch with id {} is currently not active", hBatch.GetInternalID().m_Data);
  m_Data.m_pCurrentInitBatch = m_Data.m_pDefaultInitBatch;
}

void nsWorld::SubmitComponentInitBatch(const nsComponentInitBatchHandle& hBatch)
{
  m_Data.m_InitBatches[hBatch.GetInternalID()]->m_bIsReady = true;
  m_Data.m_pCurrentInitBatch = m_Data.m_pDefaultInitBatch;
}

bool nsWorld::IsComponentInitBatchCompleted(const nsComponentInitBatchHandle& hBatch, double* pCompletionFactor /*= nullptr*/)
{
  auto& pInitBatch = m_Data.m_InitBatches[hBatch.GetInternalID()];
  NS_ASSERT_DEV(pInitBatch->m_bIsReady, "Batch is not submitted yet");

  if (pCompletionFactor != nullptr)
  {
    if (pInitBatch->m_ComponentsToInitialize.IsEmpty())
    {
      double fStartSimCompletion = pInitBatch->m_ComponentsToStartSimulation.IsEmpty() ? 1.0 : (double)pInitBatch->m_uiNextComponentToStartSimulation / pInitBatch->m_ComponentsToStartSimulation.GetCount();
      *pCompletionFactor = fStartSimCompletion * 0.5 + 0.5;
    }
    else
    {
      double fInitCompletion = pInitBatch->m_ComponentsToInitialize.IsEmpty() ? 1.0 : (double)pInitBatch->m_uiNextComponentToInitialize / pInitBatch->m_ComponentsToInitialize.GetCount();
      *pCompletionFactor = fInitCompletion * 0.5;
    }
  }

  return pInitBatch->m_ComponentsToInitialize.IsEmpty() && pInitBatch->m_ComponentsToStartSimulation.IsEmpty();
}

void nsWorld::CancelComponentInitBatch(const nsComponentInitBatchHandle& hBatch)
{
  auto& pInitBatch = m_Data.m_InitBatches[hBatch.GetInternalID()];
  pInitBatch->m_ComponentsToInitialize.Clear();
  pInitBatch->m_ComponentsToStartSimulation.Clear();
}
void nsWorld::PostMessage(const nsGameObjectHandle& receiverObject, const nsMessage& msg, nsObjectMsgQueueType::Enum queueType, nsTime delay, bool bRecursive) const
{
  // This method is allowed to be called from multiple threads.

  NS_ASSERT_DEBUG((receiverObject.m_InternalId.m_Data >> 62) == 0, "Upper 2 bits in object id must not be set");

  QueuedMsgMetaData metaData;
  metaData.m_uiReceiverObjectOrComponent = receiverObject.m_InternalId.m_Data;
  metaData.m_uiReceiverIsComponent = false;
  metaData.m_uiRecursive = bRecursive;

  if (m_Data.m_ProcessingMessageQueue == queueType)
  {
    delay = nsMath::Max(delay, nsTime::MakeFromMilliseconds(1));
  }

  nsRTTIAllocator* pMsgRTTIAllocator = msg.GetDynamicRTTI()->GetAllocator();
  if (delay.IsPositive())
  {
    nsMessage* pMsgCopy = pMsgRTTIAllocator->Clone<nsMessage>(&msg, &m_Data.m_Allocator);

    metaData.m_Due = m_Data.m_Clock.GetAccumulatedTime() + delay;
    m_Data.m_TimedMessageQueues[queueType].Enqueue(pMsgCopy, metaData);
  }
  else
  {
    nsMessage* pMsgCopy = pMsgRTTIAllocator->Clone<nsMessage>(&msg, m_Data.m_StackAllocator.GetCurrentAllocator());
    m_Data.m_MessageQueues[queueType].Enqueue(pMsgCopy, metaData);
  }
}

void nsWorld::PostMessage(const nsComponentHandle& hReceiverComponent, const nsMessage& msg, nsTime delay, nsObjectMsgQueueType::Enum queueType) const
{
  // This method is allowed to be called from multiple threads.

  NS_ASSERT_DEBUG((hReceiverComponent.m_InternalId.m_Data >> 62) == 0, "Upper 2 bits in component id must not be set");

  QueuedMsgMetaData metaData;
  metaData.m_uiReceiverObjectOrComponent = hReceiverComponent.m_InternalId.m_Data;
  metaData.m_uiReceiverIsComponent = true;
  metaData.m_uiRecursive = false;

  if (m_Data.m_ProcessingMessageQueue == queueType)
  {
    delay = nsMath::Max(delay, nsTime::MakeFromMilliseconds(1));
  }

  nsRTTIAllocator* pMsgRTTIAllocator = msg.GetDynamicRTTI()->GetAllocator();
  if (delay.IsPositive())
  {
    nsMessage* pMsgCopy = pMsgRTTIAllocator->Clone<nsMessage>(&msg, &m_Data.m_Allocator);

    metaData.m_Due = m_Data.m_Clock.GetAccumulatedTime() + delay;
    m_Data.m_TimedMessageQueues[queueType].Enqueue(pMsgCopy, metaData);
  }
  else
  {
    nsMessage* pMsgCopy = pMsgRTTIAllocator->Clone<nsMessage>(&msg, m_Data.m_StackAllocator.GetCurrentAllocator());
    m_Data.m_MessageQueues[queueType].Enqueue(pMsgCopy, metaData);
  }
}

void nsWorld::FindEventMsgHandlers(const nsMessage& msg, nsGameObject* pSearchObject, nsDynamicArray<nsComponent*>& out_components)
{
  FindEventMsgHandlers(*this, msg, pSearchObject, out_components);
}

void nsWorld::FindEventMsgHandlers(const nsMessage& msg, const nsGameObject* pSearchObject, nsDynamicArray<const nsComponent*>& out_components) const
{
  FindEventMsgHandlers(*this, msg, pSearchObject, out_components);
}

void nsWorld::Update()
{
  CheckForWriteAccess();

  NS_LOG_BLOCK(m_Data.m_sName.GetData());

  {
    nsStringBuilder sStatName;
    sStatName.SetFormat("World Update/{0}/Game Object Count", m_Data.m_sName);

    nsStringBuilder sStatValue;
    nsStats::SetStat(sStatName, GetObjectCount());
  }

  ++m_Data.m_uiUpdateCounter;

  if (!m_Data.m_bSimulateWorld)
  {
    // only change the pause mode temporarily
    // so that user choices don't get overridden

    const bool bClockPaused = m_Data.m_Clock.GetPaused();
    m_Data.m_Clock.SetPaused(true);
    m_Data.m_Clock.Update();
    m_Data.m_Clock.SetPaused(bClockPaused);
  }
  else
  {
    m_Data.m_Clock.Update();
  }

  if (m_Data.m_pSpatialSystem != nullptr)
  {
    m_Data.m_pSpatialSystem->StartNewFrame();
  }

  // reload resources
  {
    NS_PROFILE_SCOPE("Reload Resources");
    ProcessResourceReloadFunctions();
  }

  // initialize phase
  {
    NS_PROFILE_SCOPE("Initialize Phase");
    ProcessComponentsToInitialize();
    ProcessUpdateFunctionsToRegister();

    ProcessQueuedMessages(nsObjectMsgQueueType::AfterInitialized);
  }

  // pre-async phase
  {
    NS_PROFILE_SCOPE("Pre-Async Phase");
    ProcessQueuedMessages(nsObjectMsgQueueType::NextFrame);
    UpdateSynchronous(m_Data.m_UpdateFunctions[nsComponentManagerBase::UpdateFunctionDesc::Phase::PreAsync]);
  }

  // async phase
  {
    // remove write marker but keep the read marker. Thus no one can mark the world for writing now. Only reading is allowed in async phase.
    m_Data.m_WriteThreadID = (nsThreadID)0;

    NS_PROFILE_SCOPE("Async Phase");
    UpdateAsynchronous();

    // restore write marker
    m_Data.m_WriteThreadID = nsThreadUtils::GetCurrentThreadID();
  }

  // post-async phase
  {
    NS_PROFILE_SCOPE("Post-Async Phase");
    ProcessQueuedMessages(nsObjectMsgQueueType::PostAsync);
    UpdateSynchronous(m_Data.m_UpdateFunctions[nsComponentManagerBase::UpdateFunctionDesc::Phase::PostAsync]);
  }

  // delete dead objects and update the object hierarchy
  {
    NS_PROFILE_SCOPE("Delete Dead Objects");
    DeleteDeadObjects();
    DeleteDeadComponents();
  }

  // update transforms
  {
    NS_PROFILE_SCOPE("Update Transforms");
    m_Data.UpdateGlobalTransforms();
  }

  // post-transform phase
  {
    NS_PROFILE_SCOPE("Post-Transform Phase");
    ProcessQueuedMessages(nsObjectMsgQueueType::PostTransform);
    UpdateSynchronous(m_Data.m_UpdateFunctions[nsComponentManagerBase::UpdateFunctionDesc::Phase::PostTransform]);
  }

  // Process again so new component can receive render messages, otherwise we introduce a frame delay.
  {
    NS_PROFILE_SCOPE("Initialize Phase 2");
    // Only process the default init batch here since it contains the components created at runtime.
    // Also make sure that all initialization is finished after this call by giving it enough time.
    ProcessInitializationBatch(*m_Data.m_pDefaultInitBatch, nsTime::Now() + nsTime::MakeFromHours(10000));

    ProcessQueuedMessages(nsObjectMsgQueueType::AfterInitialized);
  }

  // Swap our double buffered stack allocator
  m_Data.m_StackAllocator.Swap();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

nsWorldModule* nsWorld::GetOrCreateModule(const nsRTTI* pRtti)
{
  CheckForWriteAccess();

  const nsWorldModuleTypeId uiTypeId = nsWorldModuleFactory::GetInstance()->GetTypeId(pRtti);
  if (uiTypeId == 0xFFFF)
  {
    return nullptr;
  }

  m_Data.m_Modules.EnsureCount(uiTypeId + 1);

  nsWorldModule* pModule = m_Data.m_Modules[uiTypeId];
  if (pModule == nullptr)
  {
    pModule = nsWorldModuleFactory::GetInstance()->CreateWorldModule(uiTypeId, this);
    pModule->Initialize();

    m_Data.m_Modules[uiTypeId] = pModule;
    m_Data.m_ModulesToStartSimulation.PushBack(pModule);
  }

  return pModule;
}

void nsWorld::DeleteModule(const nsRTTI* pRtti)
{
  CheckForWriteAccess();

  const nsWorldModuleTypeId uiTypeId = nsWorldModuleFactory::GetInstance()->GetTypeId(pRtti);
  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    if (nsWorldModule* pModule = m_Data.m_Modules[uiTypeId])
    {
      m_Data.m_Modules[uiTypeId] = nullptr;

      pModule->Deinitialize();
      DeregisterUpdateFunctions(pModule);
      NS_DELETE(&m_Data.m_Allocator, pModule);
    }
  }
}

nsWorldModule* nsWorld::GetModule(const nsRTTI* pRtti)
{
  CheckForWriteAccess();

  const nsWorldModuleTypeId uiTypeId = nsWorldModuleFactory::GetInstance()->GetTypeId(pRtti);
  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    return m_Data.m_Modules[uiTypeId];
  }

  return nullptr;
}

const nsWorldModule* nsWorld::GetModule(const nsRTTI* pRtti) const
{
  CheckForReadAccess();

  const nsWorldModuleTypeId uiTypeId = nsWorldModuleFactory::GetInstance()->GetTypeId(pRtti);
  if (uiTypeId < m_Data.m_Modules.GetCount())
  {
    return m_Data.m_Modules[uiTypeId];
  }

  return nullptr;
}

nsGameObject* nsWorld::Reflection_TryGetObjectWithGlobalKey(nsTempHashedString sGlobalKey)
{
  nsGameObject* pObject = nullptr;
  bool res = TryGetObjectWithGlobalKey(sGlobalKey, pObject);
  NS_IGNORE_UNUSED(res);
  return pObject;
}

nsClock* nsWorld::Reflection_GetClock()
{
  return &m_Data.m_Clock;
}

void nsWorld::SetParent(nsGameObject* pObject, nsGameObject* pNewParent, nsGameObject::TransformPreservation preserve)
{
  NS_ASSERT_DEV(pObject != pNewParent, "Object can't be its own parent!");
  NS_ASSERT_DEV(pNewParent == nullptr || pObject->IsDynamic() || pNewParent->IsStatic(), "Can't attach a static object to a dynamic parent!");
  CheckForWriteAccess();

  if (GetObjectUnchecked(pObject->m_uiParentIndex) == pNewParent)
    return;

  UnlinkFromParent(pObject);
  // UnlinkFromParent does not clear these as they are still needed in DeleteObjectNow to allow deletes while iterating.
  pObject->m_uiNextSiblingIndex = 0;
  pObject->m_uiPrevSiblingIndex = 0;
  if (pNewParent != nullptr)
  {
    // Ensure that the parent's global transform is up-to-date otherwise the object's local transform will be wrong afterwards.
    pNewParent->UpdateGlobalTransform();

    pObject->m_uiParentIndex = pNewParent->m_InternalId.m_InstanceIndex;
    LinkToParent(pObject);
  }

  PatchHierarchyData(pObject, preserve);

  // TODO: the functions above send messages such as nsMsgChildrenChanged, which will not arrive for inactive components, is that a problem ?
  // 1) if a component was active before and now gets deactivated, it may not care about the message anymore anyway
  // 2) if a component was inactive before, it did not get the message, but upon activation it can update the state for which it needed the message
  // so probably it is fine, only components that were active and stay active need the message, and that will be the case
  pObject->UpdateActiveState(pNewParent == nullptr ? true : pNewParent->IsActive());
}

void nsWorld::LinkToParent(nsGameObject* pObject)
{
  NS_ASSERT_DEBUG(pObject->m_uiNextSiblingIndex == 0 && pObject->m_uiPrevSiblingIndex == 0, "Object is either still linked to another parent or data was not cleared.");
  if (nsGameObject* pParentObject = pObject->GetParent())
  {
    const nsUInt32 uiIndex = pObject->m_InternalId.m_InstanceIndex;

    if (pParentObject->m_uiFirstChildIndex != 0)
    {
      pObject->m_uiPrevSiblingIndex = pParentObject->m_uiLastChildIndex;
      GetObjectUnchecked(pParentObject->m_uiLastChildIndex)->m_uiNextSiblingIndex = uiIndex;
    }
    else
    {
      pParentObject->m_uiFirstChildIndex = uiIndex;
    }

    pParentObject->m_uiLastChildIndex = uiIndex;
    pParentObject->m_uiChildCount++;

    pObject->m_pTransformationData->m_pParentData = pParentObject->m_pTransformationData;

    if (pObject->m_Flags.IsSet(nsObjectFlags::ParentChangesNotifications))
    {
      nsMsgParentChanged msg;
      msg.m_Type = nsMsgParentChanged::Type::ParentLinked;
      msg.m_hParent = pParentObject->GetHandle();

      pObject->SendMessage(msg);
    }

    if (pParentObject->m_Flags.IsSet(nsObjectFlags::ChildChangesNotifications))
    {
      nsMsgChildrenChanged msg;
      msg.m_Type = nsMsgChildrenChanged::Type::ChildAdded;
      msg.m_hParent = pParentObject->GetHandle();
      msg.m_hChild = pObject->GetHandle();

      pParentObject->SendNotificationMessage(msg);
    }
  }
}

void nsWorld::UnlinkFromParent(nsGameObject* pObject)
{
  if (nsGameObject* pParentObject = pObject->GetParent())
  {
    const nsUInt32 uiIndex = pObject->m_InternalId.m_InstanceIndex;

    if (uiIndex == pParentObject->m_uiFirstChildIndex)
      pParentObject->m_uiFirstChildIndex = pObject->m_uiNextSiblingIndex;

    if (uiIndex == pParentObject->m_uiLastChildIndex)
      pParentObject->m_uiLastChildIndex = pObject->m_uiPrevSiblingIndex;

    if (nsGameObject* pNextObject = GetObjectUnchecked(pObject->m_uiNextSiblingIndex))
      pNextObject->m_uiPrevSiblingIndex = pObject->m_uiPrevSiblingIndex;

    if (nsGameObject* pPrevObject = GetObjectUnchecked(pObject->m_uiPrevSiblingIndex))
      pPrevObject->m_uiNextSiblingIndex = pObject->m_uiNextSiblingIndex;

    pParentObject->m_uiChildCount--;
    pObject->m_uiParentIndex = 0;
    pObject->m_pTransformationData->m_pParentData = nullptr;

    if (pObject->m_Flags.IsSet(nsObjectFlags::ParentChangesNotifications))
    {
      nsMsgParentChanged msg;
      msg.m_Type = nsMsgParentChanged::Type::ParentUnlinked;
      msg.m_hParent = pParentObject->GetHandle();

      pObject->SendMessage(msg);
    }

    // Note that the sibling indices must not be set to 0 here.
    // They are still needed if we currently iterate over child objects.

    if (pParentObject->m_Flags.IsSet(nsObjectFlags::ChildChangesNotifications))
    {
      nsMsgChildrenChanged msg;
      msg.m_Type = nsMsgChildrenChanged::Type::ChildRemoved;
      msg.m_hParent = pParentObject->GetHandle();
      msg.m_hChild = pObject->GetHandle();

      pParentObject->SendNotificationMessage(msg);
    }
  }
}

void nsWorld::SetObjectGlobalKey(nsGameObject* pObject, const nsHashedString& sGlobalKey)
{
  if (m_Data.m_GlobalKeyToIdTable.Contains(sGlobalKey.GetHash()))
  {
    nsLog::Error("Can't set global key to '{0}' because an object with this global key already exists. Global keys have to be unique.", sGlobalKey);
    return;
  }

  const nsUInt32 uiId = pObject->m_InternalId.m_InstanceIndex;

  // Remove existing entry first.
  nsHashedString* pOldGlobalKey;
  if (m_Data.m_IdToGlobalKeyTable.TryGetValue(uiId, pOldGlobalKey))
  {
    if (sGlobalKey == *pOldGlobalKey)
    {
      return;
    }

    NS_VERIFY(m_Data.m_GlobalKeyToIdTable.Remove(pOldGlobalKey->GetHash()), "Implementation error.");
    NS_VERIFY(m_Data.m_IdToGlobalKeyTable.Remove(uiId), "Implementation error.");
  }

  // Insert new one if key is valid.
  if (!sGlobalKey.IsEmpty())
  {
    m_Data.m_GlobalKeyToIdTable.Insert(sGlobalKey.GetHash(), pObject->m_InternalId);
    m_Data.m_IdToGlobalKeyTable.Insert(uiId, sGlobalKey);
  }
}

nsStringView nsWorld::GetObjectGlobalKey(const nsGameObject* pObject) const
{
  const nsUInt32 uiId = pObject->m_InternalId.m_InstanceIndex;

  const nsHashedString* pGlobalKey;
  if (m_Data.m_IdToGlobalKeyTable.TryGetValue(uiId, pGlobalKey))
  {
    return pGlobalKey->GetView();
  }

  return {};
}

void nsWorld::ProcessQueuedMessage(const nsInternal::WorldData::MessageQueue::Entry& entry)
{
  if (entry.m_MetaData.m_uiReceiverIsComponent)
  {
    nsComponentHandle hComponent(nsComponentId(entry.m_MetaData.m_uiReceiverObjectOrComponent));

    nsComponent* pReceiverComponent = nullptr;
    if (TryGetComponent(hComponent, pReceiverComponent))
    {
      pReceiverComponent->SendMessageInternal(*entry.m_pMessage, true);
    }
    else
    {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
      if (entry.m_pMessage->GetDebugMessageRouting())
      {
        nsLog::Warning("nsWorld::ProcessQueuedMessage: Receiver nsComponent for message of type '{0}' does not exist anymore.", entry.m_pMessage->GetId());
      }
#endif
    }
  }
  else
  {
    nsGameObjectHandle hObject(nsGameObjectId(entry.m_MetaData.m_uiReceiverObjectOrComponent));

    nsGameObject* pReceiverObject = nullptr;
    if (TryGetObject(hObject, pReceiverObject))
    {
      if (entry.m_MetaData.m_uiRecursive)
      {
        pReceiverObject->SendMessageRecursiveInternal(*entry.m_pMessage, true);
      }
      else
      {
        pReceiverObject->SendMessageInternal(*entry.m_pMessage, true);
      }
    }
    else
    {
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
      if (entry.m_pMessage->GetDebugMessageRouting())
      {
        nsLog::Warning("nsWorld::ProcessQueuedMessage: Receiver nsGameObject for message of type '{0}' does not exist anymore.", entry.m_pMessage->GetId());
      }
#endif
    }
  }
}

void nsWorld::ProcessQueuedMessages(nsObjectMsgQueueType::Enum queueType)
{
  NS_PROFILE_SCOPE("Process Queued Messages");

  struct MessageComparer
  {
    NS_FORCE_INLINE bool Less(const nsInternal::WorldData::MessageQueue::Entry& a, const nsInternal::WorldData::MessageQueue::Entry& b) const
    {
      if (a.m_MetaData.m_Due != b.m_MetaData.m_Due)
        return a.m_MetaData.m_Due < b.m_MetaData.m_Due;

      const nsInt32 iKeyA = a.m_pMessage->GetSortingKey();
      const nsInt32 iKeyB = b.m_pMessage->GetSortingKey();
      if (iKeyA != iKeyB)
        return iKeyA < iKeyB;

      if (a.m_pMessage->GetId() != b.m_pMessage->GetId())
        return a.m_pMessage->GetId() < b.m_pMessage->GetId();

      if (a.m_MetaData.m_uiReceiverData != b.m_MetaData.m_uiReceiverData)
        return a.m_MetaData.m_uiReceiverData < b.m_MetaData.m_uiReceiverData;

      if (a.m_uiMessageHash == 0)
      {
        a.m_uiMessageHash = a.m_pMessage->GetHash();
      }

      if (b.m_uiMessageHash == 0)
      {
        b.m_uiMessageHash = b.m_pMessage->GetHash();
      }

      return a.m_uiMessageHash < b.m_uiMessageHash;
    }
  };

  // regular messages
  {
    nsInternal::WorldData::MessageQueue& queue = m_Data.m_MessageQueues[queueType];
    queue.Sort(MessageComparer());

    m_Data.m_ProcessingMessageQueue = queueType;
    for (nsUInt32 i = 0; i < queue.GetCount(); ++i)
    {
      ProcessQueuedMessage(queue[i]);

      // no need to deallocate these messages, they are allocated through a frame allocator
    }
    m_Data.m_ProcessingMessageQueue = nsObjectMsgQueueType::COUNT;

    queue.Clear();
  }

  // timed messages
  {
    nsInternal::WorldData::MessageQueue& queue = m_Data.m_TimedMessageQueues[queueType];
    queue.Sort(MessageComparer());

    const nsTime now = m_Data.m_Clock.GetAccumulatedTime();

    m_Data.m_ProcessingMessageQueue = queueType;
    while (!queue.IsEmpty())
    {
      auto& entry = queue.Peek();
      if (entry.m_MetaData.m_Due > now)
        break;

      ProcessQueuedMessage(entry);

      NS_DELETE(&m_Data.m_Allocator, entry.m_pMessage);

      queue.Dequeue();
    }
    m_Data.m_ProcessingMessageQueue = nsObjectMsgQueueType::COUNT;
  }
}

// static
template <typename World, typename GameObject, typename Component>
void nsWorld::FindEventMsgHandlers(World& world, const nsMessage& msg, GameObject pSearchObject, nsDynamicArray<Component>& out_components)
{
  using EventMessageHandlerComponentType = typename std::conditional<std::is_const<World>::value, const nsEventMessageHandlerComponent*, nsEventMessageHandlerComponent*>::type;

  out_components.Clear();

  // walk the graph upwards until an object is found with at least one nsComponent that handles this type of message
  {
    auto pCurrentObject = pSearchObject;

    while (pCurrentObject != nullptr)
    {
      bool bContinueSearch = true;
      for (auto pComponent : pCurrentObject->GetComponents())
      {
        if constexpr (std::is_const<World>::value == false)
        {
          pComponent->EnsureInitialized();
        }

        if (pComponent->HandlesMessage(msg))
        {
          out_components.PushBack(pComponent);
          bContinueSearch = false;
        }
        else
        {
          if constexpr (std::is_const<World>::value)
          {
            if (pComponent->IsInitialized() == false)
            {
              nsLog::Warning("Component of type '{}' was not initialized (yet) and thus might have reported an incorrect result in HandlesMessage(). "
                             "To allow this component to be automatically initialized at this point in time call the non-const variant of SendEventMessage.",
                pComponent->GetDynamicRTTI()->GetTypeName());
            }
          }

          // only continue to search on parent objects if all event handlers on the current object have the "pass through unhandled events" flag set.
          if (auto pEventMessageHandlerComponent = nsDynamicCast<EventMessageHandlerComponentType>(pComponent))
          {
            bContinueSearch &= pEventMessageHandlerComponent->GetPassThroughUnhandledEvents();
          }
        }
      }

      if (!bContinueSearch)
      {
        // stop searching as we found at least one nsEventMessageHandlerComponent or one doesn't have the "pass through" flag set.
        return;
      }

      pCurrentObject = pCurrentObject->GetParent();
    }
  }

  // if no components have been found, check all event handler components that are registered as 'global event handlers'
  if (out_components.IsEmpty())
  {
    auto globalEventMessageHandler = nsEventMessageHandlerComponent::GetAllGlobalEventHandler(&world);
    for (auto hEventMessageHandlerComponent : globalEventMessageHandler)
    {
      EventMessageHandlerComponentType pEventMessageHandlerComponent = nullptr;
      if (world.TryGetComponent(hEventMessageHandlerComponent, pEventMessageHandlerComponent))
      {
        if (pEventMessageHandlerComponent->HandlesMessage(msg))
        {
          out_components.PushBack(pEventMessageHandlerComponent);
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void nsWorld::RegisterUpdateFunction(const nsComponentManagerBase::UpdateFunctionDesc& desc)
{
  CheckForWriteAccess();

  NS_ASSERT_DEV(desc.m_Phase == nsComponentManagerBase::UpdateFunctionDesc::Phase::Async || desc.m_uiGranularity == 0, "Granularity must be 0 for synchronous update functions");
  NS_ASSERT_DEV(desc.m_Phase != nsComponentManagerBase::UpdateFunctionDesc::Phase::Async || desc.m_DependsOn.GetCount() == 0, "Asynchronous update functions must not have dependencies");
  NS_ASSERT_DEV(desc.m_Function.IsComparable(), "Delegates with captures are not allowed as nsWorld update functions.");

  m_Data.m_UpdateFunctionsToRegister.PushBack(desc);
}

void nsWorld::DeregisterUpdateFunction(const nsComponentManagerBase::UpdateFunctionDesc& desc)
{
  CheckForWriteAccess();

  nsDynamicArrayBase<nsInternal::WorldData::RegisteredUpdateFunction>& updateFunctions = m_Data.m_UpdateFunctions[desc.m_Phase.GetValue()];

  for (nsUInt32 i = updateFunctions.GetCount(); i-- > 0;)
  {
    if (updateFunctions[i].m_Function.IsEqualIfComparable(desc.m_Function))
    {
      updateFunctions.RemoveAtAndCopy(i);
    }
  }
}

void nsWorld::DeregisterUpdateFunctions(nsWorldModule* pModule)
{
  CheckForWriteAccess();

  for (nsUInt32 phase = nsWorldModule::UpdateFunctionDesc::Phase::PreAsync; phase < nsWorldModule::UpdateFunctionDesc::Phase::COUNT; ++phase)
  {
    nsDynamicArrayBase<nsInternal::WorldData::RegisteredUpdateFunction>& updateFunctions = m_Data.m_UpdateFunctions[phase];

    for (nsUInt32 i = updateFunctions.GetCount(); i-- > 0;)
    {
      if (updateFunctions[i].m_Function.GetClassInstance() == pModule)
      {
        updateFunctions.RemoveAtAndCopy(i);
      }
    }
  }
}

void nsWorld::AddComponentToInitialize(nsComponentHandle hComponent)
{
  m_Data.m_pCurrentInitBatch->m_ComponentsToInitialize.PushBack(hComponent);
}

void nsWorld::UpdateFromThread()
{
  NS_LOCK(GetWriteMarker());

  Update();
}

void nsWorld::UpdateSynchronous(const nsArrayPtr<nsInternal::WorldData::RegisteredUpdateFunction>& updateFunctions)
{
  nsWorldModule::UpdateContext context;
  context.m_uiFirstComponentIndex = 0;
  context.m_uiComponentCount = nsInvalidIndex;

  for (auto& updateFunction : updateFunctions)
  {
    if (updateFunction.m_bOnlyUpdateWhenSimulating && !m_Data.m_bSimulateWorld)
      continue;

    {
      NS_PROFILE_SCOPE(updateFunction.m_sFunctionName);
      updateFunction.m_Function(context);
    }
  }
}

void nsWorld::UpdateAsynchronous()
{
  nsTaskGroupID taskGroupId = nsTaskSystem::CreateTaskGroup(nsTaskPriority::EarlyThisFrame);

  nsDynamicArrayBase<nsInternal::WorldData::RegisteredUpdateFunction>& updateFunctions = m_Data.m_UpdateFunctions[nsComponentManagerBase::UpdateFunctionDesc::Phase::Async];

  nsUInt32 uiCurrentTaskIndex = 0;

  for (auto& updateFunction : updateFunctions)
  {
    if (updateFunction.m_bOnlyUpdateWhenSimulating && !m_Data.m_bSimulateWorld)
      continue;

    nsWorldModule* pModule = static_cast<nsWorldModule*>(updateFunction.m_Function.GetClassInstance());
    nsComponentManagerBase* pManager = nsDynamicCast<nsComponentManagerBase*>(pModule);

    // a world module can also register functions in the async phase so we want at least one task
    const nsUInt32 uiTotalCount = pManager != nullptr ? pManager->GetComponentCount() : 1;
    const nsUInt32 uiGranularity = (updateFunction.m_uiGranularity != 0) ? updateFunction.m_uiGranularity : uiTotalCount;

    nsUInt32 uiStartIndex = 0;
    while (uiStartIndex < uiTotalCount)
    {
      nsSharedPtr<nsInternal::WorldData::UpdateTask> pTask;
      if (uiCurrentTaskIndex < m_Data.m_UpdateTasks.GetCount())
      {
        pTask = m_Data.m_UpdateTasks[uiCurrentTaskIndex];
      }
      else
      {
        pTask = NS_NEW(&m_Data.m_Allocator, nsInternal::WorldData::UpdateTask);
        m_Data.m_UpdateTasks.PushBack(pTask);
      }

      pTask->ConfigureTask(updateFunction.m_sFunctionName, nsTaskNesting::Maybe);
      pTask->m_Function = updateFunction.m_Function;
      pTask->m_uiStartIndex = uiStartIndex;
      pTask->m_uiCount = (uiStartIndex + uiGranularity < uiTotalCount) ? uiGranularity : nsInvalidIndex;
      nsTaskSystem::AddTaskToGroup(taskGroupId, pTask);

      ++uiCurrentTaskIndex;
      uiStartIndex += uiGranularity;
    }
  }

  nsTaskSystem::StartTaskGroup(taskGroupId);
  nsTaskSystem::WaitForGroup(taskGroupId);
}

bool nsWorld::ProcessInitializationBatch(nsInternal::WorldData::InitBatch& batch, nsTime endTime)
{
  CheckForWriteAccess();

  // ensure that all components that are created during this batch (e.g. from prefabs)
  // will also get initialized within this batch
  m_Data.m_pCurrentInitBatch = &batch;
  NS_SCOPE_EXIT(m_Data.m_pCurrentInitBatch = m_Data.m_pDefaultInitBatch);

  if (!batch.m_ComponentsToInitialize.IsEmpty())
  {
    nsStringBuilder profileScopeName("Init ", batch.m_sName);
    NS_PROFILE_SCOPE(profileScopeName);

    // Reserve for later use
    batch.m_ComponentsToStartSimulation.Reserve(batch.m_ComponentsToInitialize.GetCount());

    // Can't use foreach here because the array might be resized during iteration.
    for (; batch.m_uiNextComponentToInitialize < batch.m_ComponentsToInitialize.GetCount(); ++batch.m_uiNextComponentToInitialize)
    {
      nsComponentHandle hComponent = batch.m_ComponentsToInitialize[batch.m_uiNextComponentToInitialize];

      // if it is in the editor, the component might have been added and already deleted, without ever running the simulation
      nsComponent* pComponent = nullptr;
      if (!TryGetComponent(hComponent, pComponent))
        continue;

      NS_ASSERT_DEBUG(pComponent->GetOwner() != nullptr, "Component must have a valid owner");

      // make sure the object's transform is up to date before the component is initialized.
      pComponent->GetOwner()->UpdateGlobalTransform();

      pComponent->EnsureInitialized();

      if (pComponent->IsActive())
      {
        pComponent->OnActivated();

        batch.m_ComponentsToStartSimulation.PushBack(hComponent);
      }

      // Check if there is still time left to initialize more components
      if (nsTime::Now() >= endTime)
      {
        ++batch.m_uiNextComponentToInitialize;
        return false;
      }
    }

    batch.m_ComponentsToInitialize.Clear();
    batch.m_uiNextComponentToInitialize = 0;
  }

  if (m_Data.m_bSimulateWorld)
  {
    nsStringBuilder startSimName("Start Sim ", batch.m_sName);
    NS_PROFILE_SCOPE(startSimName);

    // Can't use foreach here because the array might be resized during iteration.
    for (; batch.m_uiNextComponentToStartSimulation < batch.m_ComponentsToStartSimulation.GetCount(); ++batch.m_uiNextComponentToStartSimulation)
    {
      nsComponentHandle hComponent = batch.m_ComponentsToStartSimulation[batch.m_uiNextComponentToStartSimulation];

      // if it is in the editor, the component might have been added and already deleted,  without ever running the simulation
      nsComponent* pComponent = nullptr;
      if (!TryGetComponent(hComponent, pComponent))
        continue;

      if (pComponent->IsActiveAndInitialized())
      {
        pComponent->EnsureSimulationStarted();
      }

      // Check if there is still time left to initialize more components
      if (nsTime::Now() >= endTime)
      {
        ++batch.m_uiNextComponentToStartSimulation;
        return false;
      }
    }

    batch.m_ComponentsToStartSimulation.Clear();
    batch.m_uiNextComponentToStartSimulation = 0;
  }

  return true;
}

void nsWorld::ProcessComponentsToInitialize()
{
  CheckForWriteAccess();

  if (m_Data.m_bSimulateWorld)
  {
    NS_PROFILE_SCOPE("Modules Start Simulation");

    // Can't use foreach here because the array might be resized during iteration.
    for (nsUInt32 i = 0; i < m_Data.m_ModulesToStartSimulation.GetCount(); ++i)
    {
      m_Data.m_ModulesToStartSimulation[i]->OnSimulationStarted();
    }

    m_Data.m_ModulesToStartSimulation.Clear();
  }

  NS_PROFILE_SCOPE("Initialize Components");

  nsTime endTime = nsTime::Now() + m_Data.m_MaxInitializationTimePerFrame;

  // First process all component init batches that have to finish within this frame
  for (auto it = m_Data.m_InitBatches.GetIterator(); it.IsValid(); ++it)
  {
    auto& pInitBatch = it.Value();
    if (pInitBatch->m_bIsReady && pInitBatch->m_bMustFinishWithinOneFrame)
    {
      ProcessInitializationBatch(*pInitBatch, nsTime::Now() + nsTime::MakeFromHours(10000));
    }
  }

  // If there is still time left process other component init batches
  if (nsTime::Now() < endTime)
  {
    for (auto it = m_Data.m_InitBatches.GetIterator(); it.IsValid(); ++it)
    {
      auto& pInitBatch = it.Value();
      if (!pInitBatch->m_bIsReady || pInitBatch->m_bMustFinishWithinOneFrame)
        continue;

      if (!ProcessInitializationBatch(*pInitBatch, endTime))
        return;
    }
  }
}

void nsWorld::ProcessUpdateFunctionsToRegister()
{
  CheckForWriteAccess();

  if (m_Data.m_UpdateFunctionsToRegister.IsEmpty())
    return;

  NS_PROFILE_SCOPE("Register update functions");

  while (!m_Data.m_UpdateFunctionsToRegister.IsEmpty())
  {
    const nsUInt32 uiNumFunctionsToRegister = m_Data.m_UpdateFunctionsToRegister.GetCount();

    for (nsUInt32 i = uiNumFunctionsToRegister; i-- > 0;)
    {
      if (RegisterUpdateFunctionInternal(m_Data.m_UpdateFunctionsToRegister[i]).Succeeded())
      {
        m_Data.m_UpdateFunctionsToRegister.RemoveAtAndCopy(i);
      }
    }

    NS_ASSERT_DEV(m_Data.m_UpdateFunctionsToRegister.GetCount() < uiNumFunctionsToRegister, "No functions have been registered because the dependencies could not be found.");
  }
}

nsResult nsWorld::RegisterUpdateFunctionInternal(const nsWorldModule::UpdateFunctionDesc& desc)
{
  nsDynamicArrayBase<nsInternal::WorldData::RegisteredUpdateFunction>& updateFunctions = m_Data.m_UpdateFunctions[desc.m_Phase.GetValue()];
  nsUInt32 uiInsertionIndex = 0;

  for (nsUInt32 i = 0; i < desc.m_DependsOn.GetCount(); ++i)
  {
    nsUInt32 uiDependencyIndex = nsInvalidIndex;

    for (nsUInt32 j = 0; j < updateFunctions.GetCount(); ++j)
    {
      if (updateFunctions[j].m_sFunctionName == desc.m_DependsOn[i])
      {
        uiDependencyIndex = j;
        break;
      }
    }

    if (uiDependencyIndex == nsInvalidIndex) // dependency not found
    {
      return NS_FAILURE;
    }
    else
    {
      uiInsertionIndex = nsMath::Max(uiInsertionIndex, uiDependencyIndex + 1);
    }
  }

  nsInternal::WorldData::RegisteredUpdateFunction newFunction;
  newFunction.FillFromDesc(desc);

  while (uiInsertionIndex < updateFunctions.GetCount())
  {
    const auto& existingFunction = updateFunctions[uiInsertionIndex];
    if (newFunction < existingFunction)
    {
      break;
    }

    ++uiInsertionIndex;
  }

  updateFunctions.InsertAt(uiInsertionIndex, newFunction);

  return NS_SUCCESS;
}

void nsWorld::DeleteDeadObjects()
{
  while (!m_Data.m_DeadObjects.IsEmpty())
  {
    nsGameObject* pObject = m_Data.m_DeadObjects.GetIterator().Key();

    if (!pObject->m_pTransformationData->m_hSpatialData.IsInvalidated())
    {
      m_Data.m_pSpatialSystem->DeleteSpatialData(pObject->m_pTransformationData->m_hSpatialData);
    }

    m_Data.DeleteTransformationData(pObject->IsDynamic(), pObject->m_uiHierarchyLevel, pObject->m_pTransformationData);

    nsGameObject* pMovedObject = nullptr;
    m_Data.m_ObjectStorage.Delete(pObject, pMovedObject);

    if (pObject != pMovedObject)
    {
      // patch the id table: the last element in the storage has been moved to deleted object's location,
      // thus the pointer now points to another object
      nsGameObjectId id = pObject->m_InternalId;
      if (id.m_InstanceIndex != nsGameObjectId::INVALID_INSTANCE_INDEX)
        m_Data.m_Objects[id] = pObject;

      // The moved object might be deleted as well so we remove it from the dead objects set instead.
      // If that is not the case we remove the original object from the set.
      if (m_Data.m_DeadObjects.Remove(pMovedObject))
      {
        continue;
      }
    }

    m_Data.m_DeadObjects.Remove(pObject);
  }
}

void nsWorld::DeleteDeadComponents()
{
  while (!m_Data.m_DeadComponents.IsEmpty())
  {
    nsComponent* pComponent = m_Data.m_DeadComponents.GetIterator().Key();

    nsComponentManagerBase* pManager = pComponent->GetOwningManager();
    nsComponent* pMovedComponent = nullptr;
    pManager->DeleteComponentStorage(pComponent, pMovedComponent);

    // another component has been moved to the deleted component location
    if (pComponent != pMovedComponent)
    {
      pManager->PatchIdTable(pComponent);

      if (nsGameObject* pOwner = pComponent->GetOwner())
      {
        pOwner->FixComponentPointer(pMovedComponent, pComponent);
      }

      // The moved component might be deleted as well so we remove it from the dead components set instead.
      // If that is not the case we remove the original component from the set.
      if (m_Data.m_DeadComponents.Remove(pMovedComponent))
      {
        continue;
      }
    }

    m_Data.m_DeadComponents.Remove(pComponent);
  }
}

void nsWorld::PatchHierarchyData(nsGameObject* pObject, nsGameObject::TransformPreservation preserve)
{
  nsGameObject* pParent = pObject->GetParent();

  RecreateHierarchyData(pObject, pObject->IsDynamic());

  pObject->m_pTransformationData->m_pParentData = pParent != nullptr ? pParent->m_pTransformationData : nullptr;

  if (preserve == nsGameObject::TransformPreservation::PreserveGlobal)
  {
    // SetGlobalTransform will internally trigger bounds update for static objects
    pObject->SetGlobalTransform(pObject->m_pTransformationData->m_globalTransform);
  }
  else
  {
    // Explicitly trigger transform AND bounds update, otherwise bounds would be outdated for static objects
    // Don't call pObject->UpdateGlobalTransformAndBounds() here since that would recursively update the parent global transform which is already up-to-date.
    pObject->m_pTransformationData->UpdateGlobalTransformNonRecursive(GetUpdateCounter());

    pObject->m_pTransformationData->UpdateGlobalBounds(GetSpatialSystem());
  }

  for (auto it = pObject->GetChildren(); it.IsValid(); ++it)
  {
    PatchHierarchyData(it, preserve);
  }
  NS_ASSERT_DEBUG(pObject->m_pTransformationData != pObject->m_pTransformationData->m_pParentData, "Hierarchy corrupted!");
}

void nsWorld::RecreateHierarchyData(nsGameObject* pObject, bool bWasDynamic)
{
  nsGameObject* pParent = pObject->GetParent();

  const nsUInt32 uiNewHierarchyLevel = pParent != nullptr ? pParent->m_uiHierarchyLevel + 1 : 0;
  const nsUInt32 uiOldHierarchyLevel = pObject->m_uiHierarchyLevel;

  const bool bIsDynamic = pObject->IsDynamic();

  if (uiNewHierarchyLevel != uiOldHierarchyLevel || bIsDynamic != bWasDynamic)
  {
    nsGameObject::TransformationData* pOldTransformationData = pObject->m_pTransformationData;

    nsGameObject::TransformationData* pNewTransformationData = m_Data.CreateTransformationData(bIsDynamic, uiNewHierarchyLevel);
    nsMemoryUtils::Copy(pNewTransformationData, pOldTransformationData, 1);

    pObject->m_uiHierarchyLevel = static_cast<nsUInt16>(uiNewHierarchyLevel);
    pObject->m_pTransformationData = pNewTransformationData;

    // fix parent transform data for children as well
    for (auto it = pObject->GetChildren(); it.IsValid(); ++it)
    {
      nsGameObject::TransformationData* pTransformData = it->m_pTransformationData;
      pTransformData->m_pParentData = pNewTransformationData;
    }

    m_Data.DeleteTransformationData(bWasDynamic, uiOldHierarchyLevel, pOldTransformationData);
  }
}

void nsWorld::ProcessResourceReloadFunctions()
{
  ResourceReloadContext context;
  context.m_pWorld = this;

  for (auto& hResource : m_Data.m_NeedReload)
  {
    if (m_Data.m_ReloadFunctions.TryGetValue(hResource, m_Data.m_TempReloadFunctions))
    {
      for (auto& data : m_Data.m_TempReloadFunctions)
      {
        NS_VERIFY(data.m_hComponent.IsInvalidated() || TryGetComponent(data.m_hComponent, context.m_pComponent), "Reload function called on dead component");
        context.m_pUserData = data.m_pUserData;

        data.m_Func(context);
      }
    }
  }

  m_Data.m_NeedReload.Clear();
}

void nsWorld::SetMaxInitializationTimePerFrame(nsTime maxInitTime)
{
  CheckForWriteAccess();

  m_Data.m_MaxInitializationTimePerFrame = maxInitTime;
}

void nsWorld::SetGameObjectReferenceResolver(const ReferenceResolver& resolver)
{
  m_Data.m_GameObjectReferenceResolver = resolver;
}

const nsWorld::ReferenceResolver& nsWorld::GetGameObjectReferenceResolver() const
{
  return m_Data.m_GameObjectReferenceResolver;
}

void nsWorld::AddResourceReloadFunction(nsTypelessResourceHandle hResource, nsComponentHandle hComponent, void* pUserData, ResourceReloadFunc function)
{
  CheckForWriteAccess();

  if (hResource.IsValid() == false)
    return;

  auto& data = m_Data.m_ReloadFunctions[hResource].ExpandAndGetRef();
  data.m_hComponent = hComponent;
  data.m_pUserData = pUserData;
  data.m_Func = function;
}

void nsWorld::RemoveResourceReloadFunction(nsTypelessResourceHandle hResource, nsComponentHandle hComponent, void* pUserData)
{
  CheckForWriteAccess();

  nsInternal::WorldData::ReloadFunctionList* pReloadFunctions = nullptr;
  if (m_Data.m_ReloadFunctions.TryGetValue(hResource, pReloadFunctions))
  {
    for (nsUInt32 i = 0; i < pReloadFunctions->GetCount(); ++i)
    {
      auto& data = (*pReloadFunctions)[i];
      if (data.m_hComponent == hComponent && data.m_pUserData == pUserData)
      {
        pReloadFunctions->RemoveAtAndSwap(i);
        break;
      }
    }
  }
}

NS_STATICLINK_FILE(Core, Core_World_Implementation_World);
