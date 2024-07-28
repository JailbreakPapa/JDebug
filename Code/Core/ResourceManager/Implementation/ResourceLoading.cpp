#include <Core/CorePCH.h>

#include <Core/ResourceManager/Implementation/ResourceManagerState.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Foundation/Profiling/Profiling.h>

nsTypelessResourceHandle nsResourceManager::LoadResourceByType(const nsRTTI* pResourceType, nsStringView sResourceID)
{
  // the mutex here is necessary to prevent a race between resource unloading and storing the pointer in the handle
  NS_LOCK(s_ResourceMutex);
  return nsTypelessResourceHandle(GetResource(pResourceType, sResourceID, true));
}

void nsResourceManager::InternalPreloadResource(nsResource* pResource, bool bHighestPriority)
{
  if (s_pState->m_bShutdown)
    return;

  NS_PROFILE_SCOPE("InternalPreloadResource");

  NS_LOCK(s_ResourceMutex);

  // if there is nothing else that could be loaded, just return right away
  if (pResource->GetLoadingState() == nsResourceState::Loaded && pResource->GetNumQualityLevelsLoadable() == 0)
  {
    // due to the threading this can happen for all resource types and is valid
    // NS_ASSERT_DEV(!IsQueuedForLoading(pResource), "Invalid flag on resource type '{0}'",
    // pResource->GetDynamicRTTI()->GetTypeName());
    return;
  }

  NS_ASSERT_DEV(!s_pState->m_bExportMode, "Resources should not be loaded in export mode");

  // if we are already loading this resource, early out
  if (IsQueuedForLoading(pResource))
  {
    // however, if it now has highest priority and is still in the loading queue (so not yet started)
    // move it to the front of the queue
    if (bHighestPriority)
    {
      // if it is not in the queue anymore, it has already been started by some thread
      if (RemoveFromLoadingQueue(pResource).Succeeded())
      {
        AddToLoadingQueue(pResource, bHighestPriority);
      }
    }

    return;
  }
  else
  {
    AddToLoadingQueue(pResource, bHighestPriority);

    if (bHighestPriority && nsTaskSystem::GetCurrentThreadWorkerType() == nsWorkerThreadType::FileAccess)
    {
      nsResourceManager::s_pState->m_bAllowLaunchDataLoadTask = true;
    }

    RunWorkerTask(pResource);
  }
}

void nsResourceManager::SetupWorkerTasks()
{
  if (!s_pState->m_bTaskNamesInitialized)
  {
    s_pState->m_bTaskNamesInitialized = true;
    nsStringBuilder s;

    {
      static constexpr nsUInt32 InitialDataLoadTasks = 4;

      for (nsUInt32 i = 0; i < InitialDataLoadTasks; ++i)
      {
        s.SetFormat("Resource Data Loader {0}", i);
        auto& data = s_pState->m_WorkerTasksDataLoad.ExpandAndGetRef();
        data.m_pTask = NS_DEFAULT_NEW(nsResourceManagerWorkerDataLoad);
        data.m_pTask->ConfigureTask(s, nsTaskNesting::Maybe);
      }
    }

    {
      static constexpr nsUInt32 InitialUpdateContentTasks = 16;

      for (nsUInt32 i = 0; i < InitialUpdateContentTasks; ++i)
      {
        s.SetFormat("Resource Content Updater {0}", i);
        auto& data = s_pState->m_WorkerTasksUpdateContent.ExpandAndGetRef();
        data.m_pTask = NS_DEFAULT_NEW(nsResourceManagerWorkerUpdateContent);
        data.m_pTask->ConfigureTask(s, nsTaskNesting::Maybe);
      }
    }
  }
}

void nsResourceManager::RunWorkerTask(nsResource* pResource)
{
  if (s_pState->m_bShutdown)
    return;

  NS_ASSERT_DEV(s_ResourceMutex.IsLocked(), "");

  SetupWorkerTasks();

  if (s_pState->m_bAllowLaunchDataLoadTask && !s_pState->m_LoadingQueue.IsEmpty())
  {
    s_pState->m_bAllowLaunchDataLoadTask = false;

    for (nsUInt32 i = 0; i < s_pState->m_WorkerTasksDataLoad.GetCount(); ++i)
    {
      if (s_pState->m_WorkerTasksDataLoad[i].m_pTask->IsTaskFinished())
      {
        s_pState->m_WorkerTasksDataLoad[i].m_GroupId =
          nsTaskSystem::StartSingleTask(s_pState->m_WorkerTasksDataLoad[i].m_pTask, nsTaskPriority::FileAccess);
        return;
      }
    }

    // could not find any unused task -> need to create a new one
    {
      nsStringBuilder s;
      s.SetFormat("Resource Data Loader {0}", s_pState->m_WorkerTasksDataLoad.GetCount());
      auto& data = s_pState->m_WorkerTasksDataLoad.ExpandAndGetRef();
      data.m_pTask = NS_DEFAULT_NEW(nsResourceManagerWorkerDataLoad);
      data.m_pTask->ConfigureTask(s, nsTaskNesting::Maybe);
      data.m_GroupId = nsTaskSystem::StartSingleTask(data.m_pTask, nsTaskPriority::FileAccess);
    }
  }
}

void nsResourceManager::ReverseBubbleSortStep(nsDeque<LoadingInfo>& data)
{
  // Yep, it's really bubble sort!
  // This will move the entry with the smallest value to the front and move all other values closer to their correct position,
  // which is exactly what we need for the priority queue.
  // We do this once a frame, which gives us nice iterative sorting, with relatively deterministic performance characteristics.

  NS_ASSERT_DEBUG(s_ResourceMutex.IsLocked(), "Calling code must acquire s_ResourceMutex");

  const nsUInt32 uiCount = data.GetCount();

  for (nsUInt32 i = uiCount; i > 1; --i)
  {
    const nsUInt32 idx2 = i - 1;
    const nsUInt32 idx1 = i - 2;

    if (data[idx1].m_fPriority > data[idx2].m_fPriority)
    {
      nsMath::Swap(data[idx1], data[idx2]);
    }
  }
}

void nsResourceManager::UpdateLoadingDeadlines()
{
  if (s_pState->m_LoadingQueue.IsEmpty())
    return;

  NS_ASSERT_DEBUG(s_ResourceMutex.IsLocked(), "Calling code must acquire s_ResourceMutex");

  NS_PROFILE_SCOPE("UpdateLoadingDeadlines");

  const nsUInt32 uiCount = s_pState->m_LoadingQueue.GetCount();
  s_pState->m_uiLastResourcePriorityUpdateIdx = nsMath::Min(s_pState->m_uiLastResourcePriorityUpdateIdx, uiCount);

  nsUInt32 uiUpdateCount = nsMath::Min(50u, uiCount - s_pState->m_uiLastResourcePriorityUpdateIdx);

  if (uiUpdateCount == 0)
  {
    s_pState->m_uiLastResourcePriorityUpdateIdx = 0;
    uiUpdateCount = nsMath::Min(50u, uiCount - s_pState->m_uiLastResourcePriorityUpdateIdx);
  }

  if (uiUpdateCount > 0)
  {
    {
      NS_PROFILE_SCOPE("EvalLoadingDeadlines");

      const nsTime tNow = nsTime::Now();

      for (nsUInt32 i = 0; i < uiUpdateCount; ++i)
      {
        auto& element = s_pState->m_LoadingQueue[s_pState->m_uiLastResourcePriorityUpdateIdx];
        element.m_fPriority = element.m_pResource->GetLoadingPriority(tNow);
        ++s_pState->m_uiLastResourcePriorityUpdateIdx;
      }
    }

    {
      NS_PROFILE_SCOPE("SortLoadingDeadlines");
      ReverseBubbleSortStep(s_pState->m_LoadingQueue);
    }
  }
}

void nsResourceManager::PreloadResource(nsResource* pResource)
{
  InternalPreloadResource(pResource, false);
}

void nsResourceManager::PreloadResource(const nsTypelessResourceHandle& hResource)
{
  NS_ASSERT_DEV(hResource.IsValid(), "Cannot acquire a resource through an invalid handle!");

  nsResource* pResource = hResource.m_pResource;
  PreloadResource(pResource);
}

nsResourceState nsResourceManager::GetLoadingState(const nsTypelessResourceHandle& hResource)
{
  if (hResource.m_pResource == nullptr)
    return nsResourceState::Invalid;

  return hResource.m_pResource->GetLoadingState();
}

nsResult nsResourceManager::RemoveFromLoadingQueue(nsResource* pResource)
{
  NS_ASSERT_DEV(s_ResourceMutex.IsLocked(), "Resource mutex must be locked");

  if (!IsQueuedForLoading(pResource))
    return NS_SUCCESS;

  LoadingInfo li;
  li.m_pResource = pResource;

  if (s_pState->m_LoadingQueue.RemoveAndSwap(li))
  {
    pResource->m_Flags.Remove(nsResourceFlags::IsQueuedForLoading);
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

void nsResourceManager::AddToLoadingQueue(nsResource* pResource, bool bHighestPriority)
{
  NS_ASSERT_DEV(s_ResourceMutex.IsLocked(), "Resource mutex must be locked");
  NS_ASSERT_DEV(IsQueuedForLoading(pResource) == false, "Resource is already in the loading queue");

  pResource->m_Flags.Add(nsResourceFlags::IsQueuedForLoading);

  LoadingInfo li;
  li.m_pResource = pResource;

  if (bHighestPriority)
  {
    pResource->SetPriority(nsResourcePriority::Critical);
    li.m_fPriority = 0.0f;
    s_pState->m_LoadingQueue.PushFront(li);
  }
  else
  {
    li.m_fPriority = pResource->GetLoadingPriority(s_pState->m_LastFrameUpdate);
    s_pState->m_LoadingQueue.PushBack(li);
  }
}

bool nsResourceManager::ReloadResource(nsResource* pResource, bool bForce)
{
  NS_LOCK(s_ResourceMutex);

  if (!pResource->m_Flags.IsAnySet(nsResourceFlags::IsReloadable))
    return false;

  if (!bForce && pResource->m_Flags.IsAnySet(nsResourceFlags::PreventFileReload))
    return false;

  nsResourceTypeLoader* pLoader = nsResourceManager::GetResourceTypeLoader(pResource->GetDynamicRTTI());

  if (pLoader == nullptr)
    pLoader = pResource->GetDefaultResourceTypeLoader();

  if (pLoader == nullptr)
    return false;

  // no need to reload resources that are not loaded so far
  if (pResource->GetLoadingState() == nsResourceState::Unloaded)
    return false;

  bool bAllowPreloading = true;

  // if the resource is already in the loading queue we can just keep it there
  if (IsQueuedForLoading(pResource))
  {
    bAllowPreloading = false;

    LoadingInfo li;
    li.m_pResource = pResource;

    if (s_pState->m_LoadingQueue.IndexOf(li) == nsInvalidIndex)
    {
      // the resource is marked as 'loading' but it is not in the queue anymore
      // that means some task is already working on loading it
      // therefore we should not touch it (especially unload it), it might end up in an inconsistent state

      nsLog::Dev(
        "Resource '{0}' is not being reloaded, because it is currently being loaded", nsArgSensitive(pResource->GetResourceID(), "ResourceID"));
      return false;
    }
  }

  // if bForce, skip the outdated check
  if (!bForce)
  {
    if (!pLoader->IsResourceOutdated(pResource))
      return false;

    if (pResource->GetLoadingState() == nsResourceState::LoadedResourceMissing)
    {
      nsLog::Dev("Resource '{0}' is missing and will be tried to be reloaded ('{1}')", nsArgSensitive(pResource->GetResourceID(), "ResourceID"),
        nsArgSensitive(pResource->GetResourceDescription(), "ResourceDesc"));
    }
    else
    {
      nsLog::Dev("Resource '{0}' is outdated and will be reloaded ('{1}')", nsArgSensitive(pResource->GetResourceID(), "ResourceID"),
        nsArgSensitive(pResource->GetResourceDescription(), "ResourceDesc"));
    }
  }

  if (pResource->GetBaseResourceFlags().IsSet(nsResourceFlags::UpdateOnMainThread) == false || nsThreadUtils::IsMainThread())
  {
    // make sure existing data is purged
    pResource->CallUnloadData(nsResource::Unload::AllQualityLevels);

    NS_ASSERT_DEV(pResource->GetLoadingState() <= nsResourceState::LoadedResourceMissing, "Resource '{0}' should be in an unloaded state now.",
      pResource->GetResourceID());
  }
  else
  {
    s_pState->m_ResourcesToUnloadOnMainThread.Insert(nsTempHashedString(pResource->GetResourceID().GetData()), pResource->GetDynamicRTTI());
  }

  if (bAllowPreloading)
  {
    const nsTime tNow = s_pState->m_LastFrameUpdate;

    // resources that have been in use recently will be put into the preload queue immediately
    // everything else will be loaded on demand
    if (pResource->GetLastAcquireTime() >= tNow - nsTime::MakeFromSeconds(30.0))
    {
      PreloadResource(pResource);
    }
  }

  return true;
}

nsUInt32 nsResourceManager::ReloadResourcesOfType(const nsRTTI* pType, bool bForce)
{
  NS_LOCK(s_ResourceMutex);
  NS_LOG_BLOCK("nsResourceManager::ReloadResourcesOfType", pType->GetTypeName());

  nsUInt32 count = 0;

  LoadedResources& lr = s_pState->m_LoadedResources[pType];

  for (auto it = lr.m_Resources.GetIterator(); it.IsValid(); ++it)
  {
    if (ReloadResource(it.Value(), bForce))
      ++count;
  }

  return count;
}

nsUInt32 nsResourceManager::ReloadAllResources(bool bForce)
{
  NS_PROFILE_SCOPE("ReloadAllResources");

  NS_LOCK(s_ResourceMutex);
  NS_LOG_BLOCK("nsResourceManager::ReloadAllResources");

  nsUInt32 count = 0;

  for (auto itType = s_pState->m_LoadedResources.GetIterator(); itType.IsValid(); ++itType)
  {
    for (auto it = itType.Value().m_Resources.GetIterator(); it.IsValid(); ++it)
    {
      if (ReloadResource(it.Value(), bForce))
        ++count;
    }
  }

  if (count > 0)
  {
    nsResourceManagerEvent e;
    e.m_Type = nsResourceManagerEvent::Type::ReloadAllResources;

    s_pState->m_ManagerEvents.Broadcast(e);
  }

  return count;
}

void nsResourceManager::UpdateResourceWithCustomLoader(const nsTypelessResourceHandle& hResource, nsUniquePtr<nsResourceTypeLoader>&& pLoader)
{
  NS_LOCK(s_ResourceMutex);

  hResource.m_pResource->m_Flags.Add(nsResourceFlags::HasCustomDataLoader);
  s_pState->m_CustomLoaders[hResource.m_pResource] = std::move(pLoader);
  // if there was already a custom loader set, but it got no action yet, it is deleted here and replaced with the newer loader

  ReloadResource(hResource.m_pResource, true);
};

void nsResourceManager::EnsureResourceLoadingState(nsResource* pResourceToLoad, const nsResourceState RequestedState)
{
  const nsRTTI* pOwnRtti = pResourceToLoad->GetDynamicRTTI();

  // help loading until the requested resource is available
  while ((nsInt32)pResourceToLoad->GetLoadingState() < (nsInt32)RequestedState &&
         (pResourceToLoad->GetLoadingState() != nsResourceState::LoadedResourceMissing))
  {
    nsTaskGroupID tgid;

    {
      NS_LOCK(s_ResourceMutex);

      for (nsUInt32 i = 0; i < s_pState->m_WorkerTasksUpdateContent.GetCount(); ++i)
      {
        const nsResource* pQueuedResource = s_pState->m_WorkerTasksUpdateContent[i].m_pTask->m_pResourceToLoad;

        if (pQueuedResource != nullptr && pQueuedResource != pResourceToLoad && !s_pState->m_WorkerTasksUpdateContent[i].m_pTask->IsTaskFinished())
        {
          if (!IsResourceTypeAcquireDuringUpdateContentAllowed(pQueuedResource->GetDynamicRTTI(), pOwnRtti))
          {
            tgid = s_pState->m_WorkerTasksUpdateContent[i].m_GroupId;
            break;
          }
        }
      }
    }

    if (tgid.IsValid())
    {
      nsTaskSystem::WaitForGroup(tgid);
    }
    else
    {
      // do not use nsThreadUtils::YieldTimeSlice here, otherwise the thread is not tagged as 'blocked' in the TaskSystem
      nsTaskSystem::WaitForCondition([=]() -> bool
        { return (nsInt32)pResourceToLoad->GetLoadingState() >= (nsInt32)RequestedState ||
                 (pResourceToLoad->GetLoadingState() == nsResourceState::LoadedResourceMissing); });
    }
  }
}
