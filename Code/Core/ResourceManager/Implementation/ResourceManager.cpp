#include <Core/CorePCH.h>

#include <Core/ResourceManager/Implementation/ResourceManagerState.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Profiling/Profiling.h>

/// \todo Do not unload resources while they are acquired
/// \todo Resource Type Memory Thresholds
/// \todo Preload does not load all quality levels

/// Infos to Display:
///   Ref Count (max)
///   Fallback: Type / Instance
///   Loading Time

/// Resource Flags:
/// Category / Group (Texture Sets)

/// Resource Loader
///   Requires No File Access -> on non-File Thread

nsUniquePtr<nsResourceManagerState> nsResourceManager::s_pState;
nsMutex nsResourceManager::s_ResourceMutex;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Core, ResourceManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsResourceManager::OnCoreStartup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsResourceManager::OnCoreShutdown();
  }

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    nsResourceManager::OnEngineShutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on


nsResourceTypeLoader* nsResourceManager::GetResourceTypeLoader(const nsRTTI* pRTTI)
{
  return s_pState->m_ResourceTypeLoader[pRTTI];
}

nsMap<const nsRTTI*, nsResourceTypeLoader*>& nsResourceManager::GetResourceTypeLoaders()
{
  return s_pState->m_ResourceTypeLoader;
}

void nsResourceManager::AddResourceCleanupCallback(ResourceCleanupCB cb)
{
  NS_ASSERT_DEV(cb.IsComparable(), "Delegates with captures are not allowed");

  for (nsUInt32 i = 0; i < s_pState->m_ResourceCleanupCallbacks.GetCount(); ++i)
  {
    if (s_pState->m_ResourceCleanupCallbacks[i].IsEqualIfComparable(cb))
      return;
  }

  s_pState->m_ResourceCleanupCallbacks.PushBack(cb);
}

void nsResourceManager::ClearResourceCleanupCallback(ResourceCleanupCB cb)
{
  for (nsUInt32 i = 0; i < s_pState->m_ResourceCleanupCallbacks.GetCount(); ++i)
  {
    if (s_pState->m_ResourceCleanupCallbacks[i].IsEqualIfComparable(cb))
    {
      s_pState->m_ResourceCleanupCallbacks.RemoveAtAndSwap(i);
      return;
    }
  }
}

void nsResourceManager::ExecuteAllResourceCleanupCallbacks()
{
  if (s_pState == nullptr)
  {
    // In case resource manager wasn't initialized, nothing to do
    return;
  }

  nsDynamicArray<ResourceCleanupCB> callbacks = s_pState->m_ResourceCleanupCallbacks;
  s_pState->m_ResourceCleanupCallbacks.Clear();

  for (auto& cb : callbacks)
  {
    cb();
  }

  NS_ASSERT_DEV(s_pState->m_ResourceCleanupCallbacks.IsEmpty(), "During resource cleanup, new resource cleanup callbacks were registered.");
}

nsMap<const nsRTTI*, nsResourcePriority>& nsResourceManager::GetResourceTypePriorities()
{
  return s_pState->m_ResourceTypePriorities;
}

void nsResourceManager::BroadcastResourceEvent(const nsResourceEvent& e)
{
  NS_LOCK(s_ResourceMutex);

  // broadcast it through the resource to everyone directly interested in that specific resource
  e.m_pResource->m_ResourceEvents.Broadcast(e);

  // and then broadcast it to everyone else through the general event
  s_pState->m_ResourceEvents.Broadcast(e);
}

void nsResourceManager::RegisterResourceForAssetType(nsStringView sAssetTypeName, const nsRTTI* pResourceType)
{
  nsStringBuilder s = sAssetTypeName;
  s.ToLower();

  s_pState->m_AssetToResourceType[s] = pResourceType;
}

const nsRTTI* nsResourceManager::FindResourceForAssetType(nsStringView sAssetTypeName)
{
  nsStringBuilder s = sAssetTypeName;
  s.ToLower();

  return s_pState->m_AssetToResourceType.GetValueOrDefault(s, nullptr);
}

void nsResourceManager::ForceNoFallbackAcquisition(nsUInt32 uiNumFrames /*= 0xFFFFFFFF*/)
{
  s_pState->m_uiForceNoFallbackAcquisition = nsMath::Max(s_pState->m_uiForceNoFallbackAcquisition, uiNumFrames);
}

nsUInt32 nsResourceManager::FreeAllUnusedResources()
{
  NS_LOG_BLOCK("nsResourceManager::FreeAllUnusedResources");

  NS_PROFILE_SCOPE("FreeAllUnusedResources");

  if (s_pState == nullptr)
  {
    // In case resource manager wasn't initialized, no resources to unload
    return 0;
  }

  const bool bFreeAllUnused = true;

  nsUInt32 uiUnloaded = 0;
  bool bUnloadedAny = false;
  bool bAnyFailed = false;

  do
  {
    {
      NS_LOCK(s_ResourceMutex);

      bUnloadedAny = false;

      for (auto itType = s_pState->m_LoadedResources.GetIterator(); itType.IsValid(); ++itType)
      {
        LoadedResources& lr = itType.Value();

        for (auto it = lr.m_Resources.GetIterator(); it.IsValid(); /* empty */)
        {
          nsResource* pReference = it.Value();

          if (pReference->m_iReferenceCount == 0)
          {
            bUnloadedAny = true; // make sure to try again, even if DeallocateResource() fails; need to release our lock for that to prevent dead-locks

            if (DeallocateResource(pReference).Succeeded())
            {
              ++uiUnloaded;

              it = lr.m_Resources.Remove(it);
              continue;
            }
            else
            {
              bAnyFailed = true;
            }
          }

          ++it;
        }
      }
    }

    if (bAnyFailed)
    {
      // When this happens, it is possible that the resource that failed to be deleted
      // is dependent on a task that needs to be executed on THIS thread (main thread).
      // Therefore, help executing some tasks here, to unblock the task system.

      bAnyFailed = false;

      nsInt32 iHelpExecTasksRounds = 1;
      nsTaskSystem::WaitForCondition([&iHelpExecTasksRounds]()
        { return iHelpExecTasksRounds-- <= 0; });
    }

  } while (bFreeAllUnused && bUnloadedAny);

  return uiUnloaded;
}

nsUInt32 nsResourceManager::FreeUnusedResources(nsTime timeout, nsTime lastAcquireThreshold)
{
  if (timeout.IsZeroOrNegative())
    return 0;

  NS_LOCK(s_ResourceMutex);
  NS_LOG_BLOCK("nsResourceManager::FreeUnusedResources");
  NS_PROFILE_SCOPE("FreeUnusedResources");

  auto itResourceType = s_pState->m_LoadedResources.Find(s_pState->m_pFreeUnusedLastType);
  if (!itResourceType.IsValid())
  {
    itResourceType = s_pState->m_LoadedResources.GetIterator();
  }

  if (!itResourceType.IsValid())
    return 0;

  auto itResourceID = itResourceType.Value().m_Resources.Find(s_pState->m_sFreeUnusedLastResourceID);
  if (!itResourceID.IsValid())
  {
    itResourceID = itResourceType.Value().m_Resources.GetIterator();
  }

  const nsTime tStart = nsTime::Now();

  nsUInt32 uiDeallocatedCount = 0;

  nsStringBuilder sResourceName;

  const nsRTTI* pLastTypeCheck = nullptr;

  // stop once we wasted enough time
  while (nsTime::Now() - tStart < timeout)
  {
    if (!itResourceID.IsValid())
    {
      // reached the end of this resource type
      // advance to the next resource type
      ++itResourceType;

      if (!itResourceType.IsValid())
      {
        // if we reached the end, reset everything and stop

        s_pState->m_pFreeUnusedLastType = nullptr;
        s_pState->m_sFreeUnusedLastResourceID = nsTempHashedString();
        return uiDeallocatedCount;
      }


      // reset resource ID to the beginning of this type and start over
      itResourceID = itResourceType.Value().m_Resources.GetIterator();
      continue;
    }

    s_pState->m_pFreeUnusedLastType = itResourceType.Key();
    s_pState->m_sFreeUnusedLastResourceID = itResourceID.Key();

    if (pLastTypeCheck != itResourceType.Key())
    {
      pLastTypeCheck = itResourceType.Key();

      if (GetResourceTypeInfo(pLastTypeCheck).m_bIncrementalUnload == false)
      {
        itResourceID = itResourceType.Value().m_Resources.GetEndIterator();
        continue;
      }
    }

    nsResource* pResource = itResourceID.Value();

    if ((pResource->GetReferenceCount() == 0) && (tStart - pResource->GetLastAcquireTime() > lastAcquireThreshold))
    {
      sResourceName = pResource->GetResourceID();

      if (DeallocateResource(pResource).Succeeded())
      {
        nsLog::Debug("Freed '{}'", nsArgSensitive(sResourceName, "ResourceID"));

        ++uiDeallocatedCount;
        itResourceID = itResourceType.Value().m_Resources.Remove(itResourceID);
        continue;
      }
    }

    ++itResourceID;
  }

  return uiDeallocatedCount;
}

void nsResourceManager::SetAutoFreeUnused(nsTime timeout, nsTime lastAcquireThreshold)
{
  s_pState->m_AutoFreeUnusedTimeout = timeout;
  s_pState->m_AutoFreeUnusedThreshold = lastAcquireThreshold;
}

void nsResourceManager::AllowResourceTypeAcquireDuringUpdateContent(const nsRTTI* pTypeBeingUpdated, const nsRTTI* pTypeItWantsToAcquire)
{
  auto& info = s_pState->m_TypeInfo[pTypeBeingUpdated];

  NS_ASSERT_DEV(info.m_bAllowNestedAcquireCached == false, "AllowResourceTypeAcquireDuringUpdateContent for type '{}' must be called before the resource info has been requested.", pTypeBeingUpdated->GetTypeName());

  if (info.m_NestedTypes.IndexOf(pTypeItWantsToAcquire) == nsInvalidIndex)
  {
    info.m_NestedTypes.PushBack(pTypeItWantsToAcquire);
  }
}

bool nsResourceManager::IsResourceTypeAcquireDuringUpdateContentAllowed(const nsRTTI* pTypeBeingUpdated, const nsRTTI* pTypeItWantsToAcquire)
{
  NS_ASSERT_DEBUG(s_ResourceMutex.IsLocked(), "");

  auto& info = s_pState->m_TypeInfo[pTypeBeingUpdated];

  if (!info.m_bAllowNestedAcquireCached)
  {
    info.m_bAllowNestedAcquireCached = true;

    nsSet<const nsRTTI*> visited;
    nsSet<const nsRTTI*> todo;
    nsSet<const nsRTTI*> deps;

    for (const nsRTTI* pRtti : info.m_NestedTypes)
    {
      nsRTTI::ForEachDerivedType(pRtti, [&](const nsRTTI* pDerived)
        { todo.Insert(pDerived); });
    }

    while (!todo.IsEmpty())
    {
      auto it = todo.GetIterator();
      const nsRTTI* pRtti = it.Key();
      todo.Remove(it);

      if (visited.Contains(pRtti))
        continue;

      visited.Insert(pRtti);
      deps.Insert(pRtti);

      for (const nsRTTI* pNestedRtti : s_pState->m_TypeInfo[pRtti].m_NestedTypes)
      {
        if (!visited.Contains(pNestedRtti))
        {
          nsRTTI::ForEachDerivedType(pNestedRtti, [&](const nsRTTI* pDerived)
            { todo.Insert(pDerived); });
        }
      }
    }

    info.m_NestedTypes.Clear();
    for (const nsRTTI* pRtti : deps)
    {
      info.m_NestedTypes.PushBack(pRtti);
    }
    info.m_NestedTypes.Sort();
  }

  return info.m_NestedTypes.IndexOf(pTypeItWantsToAcquire) != nsInvalidIndex;
}

nsResult nsResourceManager::DeallocateResource(nsResource* pResource)
{
  // NS_ASSERT_DEBUG(pResource->m_iLockCount == 0, "Resource '{0}' has a refcount of zero, but is still in an acquired state.", pResource->GetResourceID());

  if (RemoveFromLoadingQueue(pResource).Failed())
  {
    // cannot deallocate resources that are currently queued for loading,
    // especially when they are already picked up by a task
    return NS_FAILURE;
  }

  pResource->CallUnloadData(nsResource::Unload::AllQualityLevels);

  NS_ASSERT_DEBUG(pResource->GetLoadingState() <= nsResourceState::LoadedResourceMissing, "Resource '{0}' should be in an unloaded state now.", pResource->GetResourceID());

  // broadcast that we are going to delete the resource
  {
    nsResourceEvent e;
    e.m_pResource = pResource;
    e.m_Type = nsResourceEvent::Type::ResourceDeleted;
    nsResourceManager::BroadcastResourceEvent(e);
  }

  NS_ASSERT_DEV(pResource->GetReferenceCount() == 0, "The resource '{}' ({}) is being deallocated, you just stored a handle to it, which won't work! If you are listening to nsResourceEvent::Type::ResourceContentUnloading then additionally listen to nsResourceEvent::Type::ResourceDeleted to clean up handles to dead resources.", pResource->GetResourceID(), pResource->GetResourceDescription());

  // delete the resource via the RTTI provided allocator
  pResource->GetDynamicRTTI()->GetAllocator()->Deallocate(pResource);

  return NS_SUCCESS;
}

// To allow triggering this event without a link dependency
// Used by Fileserve, to trigger this event, even though Fileserve should not have a link dependency on Core
NS_ON_GLOBAL_EVENT(nsResourceManager_ReloadAllResources)
{
  nsResourceManager::ReloadAllResources(false);
}
void nsResourceManager::ResetAllResources()
{
  NS_LOCK(s_ResourceMutex);
  NS_LOG_BLOCK("nsResourceManager::ReloadAllResources");

  for (auto itType = s_pState->m_LoadedResources.GetIterator(); itType.IsValid(); ++itType)
  {
    for (auto it = itType.Value().m_Resources.GetIterator(); it.IsValid(); ++it)
    {
      nsResource* pResource = it.Value();
      pResource->ResetResource();
    }
  }
}

void nsResourceManager::PerFrameUpdate()
{
  NS_PROFILE_SCOPE("nsResourceManagerUpdate");

  s_pState->m_LastFrameUpdate = nsTime::Now();

  if (s_pState->m_bBroadcastExistsEvent)
  {
    NS_LOCK(s_ResourceMutex);

    s_pState->m_bBroadcastExistsEvent = false;

    for (auto itType = s_pState->m_LoadedResources.GetIterator(); itType.IsValid(); ++itType)
    {
      for (auto it = itType.Value().m_Resources.GetIterator(); it.IsValid(); ++it)
      {
        nsResourceEvent e;
        e.m_Type = nsResourceEvent::Type::ResourceExists;
        e.m_pResource = it.Value();

        nsResourceManager::BroadcastResourceEvent(e);
      }
    }
  }

  {
    NS_LOCK(s_ResourceMutex);

    for (auto it = s_pState->m_ResourcesToUnloadOnMainThread.GetIterator(); it.IsValid(); it.Next())
    {
      // Identify the container of loaded resource for the type of resource we want to unload.
      LoadedResources loadedResourcesForType;
      if (s_pState->m_LoadedResources.TryGetValue(it.Value(), loadedResourcesForType) == false)
      {
        continue;
      }

      // See, if the resource we want to unload still exists.
      nsResource* resourceToUnload = nullptr;

      if (loadedResourcesForType.m_Resources.TryGetValue(it.Key(), resourceToUnload) == false)
      {
        continue;
      }

      NS_ASSERT_DEV(resourceToUnload != nullptr, "Found a resource above, should not be nullptr.");

      // If the resource was still loaded, we are going to unload it now.
      resourceToUnload->CallUnloadData(nsResource::Unload::AllQualityLevels);

      NS_ASSERT_DEV(resourceToUnload->GetLoadingState() <= nsResourceState::LoadedResourceMissing, "Resource '{0}' should be in an unloaded state now.", resourceToUnload->GetResourceID());
    }

    s_pState->m_ResourcesToUnloadOnMainThread.Clear();
  }

  if (s_pState->m_AutoFreeUnusedTimeout.IsPositive())
  {
    FreeUnusedResources(s_pState->m_AutoFreeUnusedTimeout, s_pState->m_AutoFreeUnusedThreshold);
  }
}

const nsEvent<const nsResourceEvent&, nsMutex>& nsResourceManager::GetResourceEvents()
{
  return s_pState->m_ResourceEvents;
}

const nsEvent<const nsResourceManagerEvent&, nsMutex>& nsResourceManager::GetManagerEvents()
{
  return s_pState->m_ManagerEvents;
}

void nsResourceManager::BroadcastExistsEvent()
{
  s_pState->m_bBroadcastExistsEvent = true;
}

void nsResourceManager::PluginEventHandler(const nsPluginEvent& e)
{
  switch (e.m_EventType)
  {
    case nsPluginEvent::AfterStartupShutdown:
    {
      // unload all resources until there are no more that can be unloaded
      // this is to prevent having resources allocated that came from a dynamic plugin
      FreeAllUnusedResources();
    }
    break;

    default:
      break;
  }
}

void nsResourceManager::OnCoreStartup()
{
  s_pState = NS_DEFAULT_NEW(nsResourceManagerState);

  NS_LOCK(s_ResourceMutex);
  s_pState->m_bAllowLaunchDataLoadTask = true;
  s_pState->m_bShutdown = false;

  nsPlugin::Events().AddEventHandler(PluginEventHandler);
}

void nsResourceManager::EngineAboutToShutdown()
{
  {
    NS_LOCK(s_ResourceMutex);

    if (s_pState == nullptr)
    {
      // In case resource manager wasn't initialized, nothing to do
      return;
    }

    s_pState->m_bAllowLaunchDataLoadTask = false; // prevent a new one from starting
    s_pState->m_bShutdown = true;
  }

  for (nsUInt32 i = 0; i < s_pState->m_WorkerTasksDataLoad.GetCount(); ++i)
  {
    nsTaskSystem::CancelTask(s_pState->m_WorkerTasksDataLoad[i].m_pTask).IgnoreResult();
  }

  for (nsUInt32 i = 0; i < s_pState->m_WorkerTasksUpdateContent.GetCount(); ++i)
  {
    nsTaskSystem::CancelTask(s_pState->m_WorkerTasksUpdateContent[i].m_pTask).IgnoreResult();
  }

  {
    NS_LOCK(s_ResourceMutex);

    for (auto entry : s_pState->m_LoadingQueue)
    {
      entry.m_pResource->m_Flags.Remove(nsResourceFlags::IsQueuedForLoading);
    }

    s_pState->m_LoadingQueue.Clear();

    // Since we just canceled all loading tasks above and cleared the loading queue,
    // some resources may still be flagged as 'loading', but can never get loaded.
    // That can deadlock the 'FreeAllUnused' function, because it won't delete 'loading' resources.
    // Therefore we need to make sure no resource has the IsQueuedForLoading flag set anymore.
    for (auto itTypes : s_pState->m_LoadedResources)
    {
      for (auto itRes : itTypes.Value().m_Resources)
      {
        nsResource* pRes = itRes.Value();

        if (pRes->GetBaseResourceFlags().IsSet(nsResourceFlags::IsQueuedForLoading))
        {
          pRes->m_Flags.Remove(nsResourceFlags::IsQueuedForLoading);
        }
      }
    }
  }
}

bool nsResourceManager::IsAnyLoadingInProgress()
{
  NS_LOCK(s_ResourceMutex);

  if (s_pState->m_LoadingQueue.GetCount() > 0)
  {
    return true;
  }

  for (nsUInt32 i = 0; i < s_pState->m_WorkerTasksDataLoad.GetCount(); ++i)
  {
    if (!s_pState->m_WorkerTasksDataLoad[i].m_pTask->IsTaskFinished())
    {
      return true;
    }
  }

  for (nsUInt32 i = 0; i < s_pState->m_WorkerTasksUpdateContent.GetCount(); ++i)
  {
    if (!s_pState->m_WorkerTasksUpdateContent[i].m_pTask->IsTaskFinished())
    {
      return true;
    }
  }
  return false;
}

void nsResourceManager::OnEngineShutdown()
{
  nsResourceManagerEvent e;
  e.m_Type = nsResourceManagerEvent::Type::ManagerShuttingDown;

  // in case of a crash inside the event broadcast or ExecuteAllResourceCleanupCallbacks():
  // you might have a resource type added through a dynamic plugin that has already been unloaded,
  // but the event handler is still referenced
  // to fix this, call nsResource::CleanupDynamicPluginReferences() on that resource type during engine shutdown (see nsStartup)
  s_pState->m_ManagerEvents.Broadcast(e);

  ExecuteAllResourceCleanupCallbacks();

  EngineAboutToShutdown();

  // unload all resources until there are no more that can be unloaded
  FreeAllUnusedResources();
}

void nsResourceManager::OnCoreShutdown()
{
  OnEngineShutdown();

  NS_LOG_BLOCK("Referenced Resources");

  for (auto itType = s_pState->m_LoadedResources.GetIterator(); itType.IsValid(); ++itType)
  {
    const nsRTTI* pRtti = itType.Key();
    LoadedResources& lr = itType.Value();

    if (!lr.m_Resources.IsEmpty())
    {
      NS_LOG_BLOCK("Type", pRtti->GetTypeName());

      nsLog::Error("{0} resource of type '{1}' are still referenced.", lr.m_Resources.GetCount(), pRtti->GetTypeName());

      for (auto it = lr.m_Resources.GetIterator(); it.IsValid(); ++it)
      {
        nsResource* pReference = it.Value();

        nsLog::Info("RC = {0}, ID = '{1}'", pReference->GetReferenceCount(), nsArgSensitive(pReference->GetResourceID(), "ResourceID"));

#if NS_ENABLED(NS_RESOURCEHANDLE_STACK_TRACES)
        pReference->PrintHandleStackTraces();
#endif
      }
    }
  }

  nsPlugin::Events().RemoveEventHandler(PluginEventHandler);

  s_pState.Clear();
}

nsResource* nsResourceManager::GetResource(const nsRTTI* pRtti, nsStringView sResourceID, bool bIsReloadable)
{
  if (sResourceID.IsEmpty())
    return nullptr;

  NS_ASSERT_DEV(s_ResourceMutex.IsLocked(), "Calling code must lock the mutex until the resource pointer is stored in a handle");

  // redirect requested type to override type, if available
  pRtti = FindResourceTypeOverride(pRtti, sResourceID);

  NS_ASSERT_DEBUG(pRtti != nullptr, "There is no RTTI information available for the given resource type '{0}'", NS_STRINGIZE(ResourceType));

  if (pRtti->GetTypeFlags().IsSet(nsTypeFlags::Abstract))
  {
    // this can happen for assets that use a resource type override (such as scripts) shortly after they have been created
    return nullptr;
  }

  NS_ASSERT_DEBUG(pRtti->GetAllocator() != nullptr && pRtti->GetAllocator()->CanAllocate(), "There is no RTTI allocator available for the given resource type '{0}'", NS_STRINGIZE(ResourceType));

  nsResource* pResource = nullptr;
  nsTempHashedString sHashedResourceID(sResourceID);

  nsHashedString* redirection;
  if (s_pState->m_NamedResources.TryGetValue(sHashedResourceID, redirection))
  {
    sHashedResourceID = *redirection;
    sResourceID = redirection->GetView();
  }

  LoadedResources& lr = s_pState->m_LoadedResources[pRtti];

  if (lr.m_Resources.TryGetValue(sHashedResourceID, pResource))
    return pResource;

  nsResource* pNewResource = pRtti->GetAllocator()->Allocate<nsResource>();
  pNewResource->m_Priority = s_pState->m_ResourceTypePriorities.GetValueOrDefault(pRtti, nsResourcePriority::Medium);
  pNewResource->SetUniqueID(sResourceID, bIsReloadable);
  pNewResource->m_Flags.AddOrRemove(nsResourceFlags::ResourceHasTypeFallback, pNewResource->HasResourceTypeLoadingFallback());

  lr.m_Resources.Insert(sHashedResourceID, pNewResource);

  return pNewResource;
}

void nsResourceManager::RegisterResourceOverrideType(const nsRTTI* pDerivedTypeToUse, nsDelegate<bool(const nsStringBuilder&)> overrideDecider)
{
  const nsRTTI* pParentType = pDerivedTypeToUse->GetParentType();
  while (pParentType != nullptr && pParentType != nsGetStaticRTTI<nsResource>())
  {
    auto& info = s_pState->m_DerivedTypeInfos[pParentType].ExpandAndGetRef();
    info.m_pDerivedType = pDerivedTypeToUse;
    info.m_Decider = overrideDecider;

    pParentType = pParentType->GetParentType();
  }
}

void nsResourceManager::UnregisterResourceOverrideType(const nsRTTI* pDerivedTypeToUse)
{
  const nsRTTI* pParentType = pDerivedTypeToUse->GetParentType();
  while (pParentType != nullptr && pParentType != nsGetStaticRTTI<nsResource>())
  {
    auto it = s_pState->m_DerivedTypeInfos.Find(pParentType);
    pParentType = pParentType->GetParentType();

    if (!it.IsValid())
      break;

    auto& infos = it.Value();

    for (nsUInt32 i = infos.GetCount(); i > 0; --i)
    {
      if (infos[i - 1].m_pDerivedType == pDerivedTypeToUse)
        infos.RemoveAtAndSwap(i - 1);
    }
  }
}

const nsRTTI* nsResourceManager::FindResourceTypeOverride(const nsRTTI* pRtti, nsStringView sResourceID)
{
  auto it = s_pState->m_DerivedTypeInfos.Find(pRtti);

  if (!it.IsValid())
    return pRtti;

  nsStringBuilder sRedirectedPath;
  nsFileSystem::ResolveAssetRedirection(sResourceID, sRedirectedPath);

  while (it.IsValid())
  {
    for (const auto& info : it.Value())
    {
      if (info.m_Decider(sRedirectedPath))
      {
        pRtti = info.m_pDerivedType;
        it = s_pState->m_DerivedTypeInfos.Find(pRtti);
        continue;
      }
    }

    break;
  }

  return pRtti;
}

nsString nsResourceManager::GenerateUniqueResourceID(nsStringView sResourceIDPrefix)
{
  nsStringBuilder resourceID;
  resourceID.SetFormat("{}-{}", sResourceIDPrefix, s_pState->m_uiNextResourceID++);
  return resourceID;
}

nsTypelessResourceHandle nsResourceManager::GetExistingResourceByType(const nsRTTI* pResourceType, nsStringView sResourceID)
{
  nsResource* pResource = nullptr;

  const nsTempHashedString sResourceHash(sResourceID);

  NS_LOCK(s_ResourceMutex);

  const nsRTTI* pRtti = FindResourceTypeOverride(pResourceType, sResourceID);

  if (s_pState->m_LoadedResources[pRtti].m_Resources.TryGetValue(sResourceHash, pResource))
    return nsTypelessResourceHandle(pResource);

  return nsTypelessResourceHandle();
}

nsTypelessResourceHandle nsResourceManager::GetExistingResourceOrCreateAsync(const nsRTTI* pResourceType, nsStringView sResourceID, nsUniquePtr<nsResourceTypeLoader>&& pLoader)
{
  NS_LOCK(s_ResourceMutex);

  nsTypelessResourceHandle hResource = GetExistingResourceByType(pResourceType, sResourceID);

  if (hResource.IsValid())
    return hResource;

  hResource = GetResource(pResourceType, sResourceID, false);
  nsResource* pResource = hResource.m_pResource;

  pResource->m_Flags.Add(nsResourceFlags::HasCustomDataLoader | nsResourceFlags::IsCreatedResource);
  s_pState->m_CustomLoaders[pResource] = std::move(pLoader);

  return hResource;
}

void nsResourceManager::ForceLoadResourceNow(const nsTypelessResourceHandle& hResource)
{
  NS_ASSERT_DEV(hResource.IsValid(), "Cannot access an invalid resource");

  nsResource* pResource = hResource.m_pResource;

  if (pResource->GetLoadingState() != nsResourceState::LoadedResourceMissing && pResource->GetLoadingState() != nsResourceState::Loaded)
  {
    InternalPreloadResource(pResource, true);

    EnsureResourceLoadingState(hResource.m_pResource, nsResourceState::Loaded);
  }
}

void nsResourceManager::RegisterNamedResource(nsStringView sLookupName, nsStringView sRedirectionResource)
{
  NS_LOCK(s_ResourceMutex);

  nsTempHashedString lookup(sLookupName);

  nsHashedString redirection;
  redirection.Assign(sRedirectionResource);

  s_pState->m_NamedResources[lookup] = redirection;
}

void nsResourceManager::UnregisterNamedResource(nsStringView sLookupName)
{
  NS_LOCK(s_ResourceMutex);

  nsTempHashedString hash(sLookupName);
  s_pState->m_NamedResources.Remove(hash);
}

void nsResourceManager::SetResourceLowResData(const nsTypelessResourceHandle& hResource, nsStreamReader* pStream)
{
  nsResource* pResource = hResource.m_pResource;

  if (pResource->GetBaseResourceFlags().IsSet(nsResourceFlags::HasLowResData))
    return;

  if (!pResource->GetBaseResourceFlags().IsSet(nsResourceFlags::IsReloadable))
    return;

  NS_LOCK(s_ResourceMutex);

  // set this, even if we don't end up using the data (because some thread is already loading the full thing)
  pResource->m_Flags.Add(nsResourceFlags::HasLowResData);

  if (IsQueuedForLoading(pResource))
  {
    // if we cannot find it in the queue anymore, some thread already started loading it
    // in this case, do not try to modify it
    if (RemoveFromLoadingQueue(pResource).Failed())
      return;
  }

  pResource->CallUpdateContent(pStream);

  NS_ASSERT_DEV(pResource->GetLoadingState() != nsResourceState::Unloaded, "The resource should have changed its loading state.");

  // Update Memory Usage
  {
    nsResource::MemoryUsage MemUsage;
    MemUsage.m_uiMemoryCPU = 0xFFFFFFFF;
    MemUsage.m_uiMemoryGPU = 0xFFFFFFFF;
    pResource->UpdateMemoryUsage(MemUsage);

    NS_ASSERT_DEV(MemUsage.m_uiMemoryCPU != 0xFFFFFFFF, "Resource '{0}' did not properly update its CPU memory usage", pResource->GetResourceID());
    NS_ASSERT_DEV(MemUsage.m_uiMemoryGPU != 0xFFFFFFFF, "Resource '{0}' did not properly update its GPU memory usage", pResource->GetResourceID());

    pResource->m_MemoryUsage = MemUsage;
  }
}

nsResourceTypeLoader* nsResourceManager::GetDefaultResourceLoader()
{
  return s_pState->m_pDefaultResourceLoader;
}

void nsResourceManager::EnableExportMode(bool bEnable)
{
  NS_ASSERT_DEV(s_pState != nullptr, "nsStartup::StartupCoreSystems() must be called before using the nsResourceManager.");

  s_pState->m_bExportMode = bEnable;
}

bool nsResourceManager::IsExportModeEnabled()
{
  NS_ASSERT_DEV(s_pState != nullptr, "nsStartup::StartupCoreSystems() must be called before using the nsResourceManager.");

  return s_pState->m_bExportMode;
}

void nsResourceManager::RestoreResource(const nsTypelessResourceHandle& hResource)
{
  NS_ASSERT_DEV(hResource.IsValid(), "Cannot access an invalid resource");

  nsResource* pResource = hResource.m_pResource;
  pResource->m_Flags.Remove(nsResourceFlags::PreventFileReload);

  ReloadResource(pResource, true);
}

nsUInt32 nsResourceManager::GetForceNoFallbackAcquisition()
{
  return s_pState->m_uiForceNoFallbackAcquisition;
}

nsTime nsResourceManager::GetLastFrameUpdate()
{
  return s_pState->m_LastFrameUpdate;
}

nsHashTable<const nsRTTI*, nsResourceManager::LoadedResources>& nsResourceManager::GetLoadedResources()
{
  return s_pState->m_LoadedResources;
}

nsDynamicArray<nsResource*>& nsResourceManager::GetLoadedResourceOfTypeTempContainer()
{
  return s_pState->m_LoadedResourceOfTypeTempContainer;
}

void nsResourceManager::SetDefaultResourceLoader(nsResourceTypeLoader* pDefaultLoader)
{
  NS_LOCK(s_ResourceMutex);

  s_pState->m_pDefaultResourceLoader = pDefaultLoader;
}

nsResourceManager::ResourceTypeInfo& nsResourceManager::GetResourceTypeInfo(const nsRTTI* pRtti)
{
  return s_pState->m_TypeInfo[pRtti];
}

NS_STATICLINK_FILE(Core, Core_ResourceManager_Implementation_ResourceManager);
