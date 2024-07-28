#include <Core/CorePCH.h>

#include <Core/ResourceManager/Implementation/ResourceManagerState.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Profiling/Profiling.h>

nsResourceManagerWorkerDataLoad::nsResourceManagerWorkerDataLoad() = default;
nsResourceManagerWorkerDataLoad::~nsResourceManagerWorkerDataLoad() = default;

void nsResourceManagerWorkerDataLoad::Execute()
{
  NS_PROFILE_SCOPE("LoadResourceFromDisk");

  nsResource* pResourceToLoad = nullptr;
  nsResourceTypeLoader* pLoader = nullptr;
  nsUniquePtr<nsResourceTypeLoader> pCustomLoader;

  {
    NS_LOCK(nsResourceManager::s_ResourceMutex);

    if (nsResourceManager::s_pState->m_LoadingQueue.IsEmpty())
    {
      nsResourceManager::s_pState->m_bAllowLaunchDataLoadTask = true;
      return;
    }

    nsResourceManager::UpdateLoadingDeadlines();

    auto it = nsResourceManager::s_pState->m_LoadingQueue.PeekFront();
    pResourceToLoad = it.m_pResource;
    nsResourceManager::s_pState->m_LoadingQueue.PopFront();

    if (pResourceToLoad->m_Flags.IsSet(nsResourceFlags::HasCustomDataLoader))
    {
      pCustomLoader = std::move(nsResourceManager::s_pState->m_CustomLoaders[pResourceToLoad]);
      pLoader = pCustomLoader.Borrow();
      pResourceToLoad->m_Flags.Remove(nsResourceFlags::HasCustomDataLoader);
      pResourceToLoad->m_Flags.Add(nsResourceFlags::PreventFileReload);
    }
  }

  if (pLoader == nullptr)
    pLoader = nsResourceManager::GetResourceTypeLoader(pResourceToLoad->GetDynamicRTTI());

  if (pLoader == nullptr)
    pLoader = pResourceToLoad->GetDefaultResourceTypeLoader();

  NS_ASSERT_DEV(pLoader != nullptr, "No Loader function available for Resource Type '{0}'", pResourceToLoad->GetDynamicRTTI()->GetTypeName());

  nsResourceLoadData LoaderData = pLoader->OpenDataStream(pResourceToLoad);

  // we need this info later to do some work in a lock, all the directly following code is outside the lock
  const bool bResourceIsLoadedOnMainThread = pResourceToLoad->GetBaseResourceFlags().IsAnySet(nsResourceFlags::UpdateOnMainThread);

  nsSharedPtr<nsResourceManagerWorkerUpdateContent> pUpdateContentTask;
  nsTaskGroupID* pUpdateContentGroup = nullptr;

  NS_LOCK(nsResourceManager::s_ResourceMutex);

  // try to find an update content task that has finished and can be reused
  for (nsUInt32 i = 0; i < nsResourceManager::s_pState->m_WorkerTasksUpdateContent.GetCount(); ++i)
  {
    auto& td = nsResourceManager::s_pState->m_WorkerTasksUpdateContent[i];

    if (nsTaskSystem::IsTaskGroupFinished(td.m_GroupId))
    {
      pUpdateContentTask = td.m_pTask;
      pUpdateContentGroup = &td.m_GroupId;
      break;
    }
  }

  // if no such task could be found, we must allocate a new one
  if (pUpdateContentTask == nullptr)
  {
    nsStringBuilder s;
    s.SetFormat("Resource Content Updater {0}", nsResourceManager::s_pState->m_WorkerTasksUpdateContent.GetCount());

    auto& td = nsResourceManager::s_pState->m_WorkerTasksUpdateContent.ExpandAndGetRef();
    td.m_pTask = NS_DEFAULT_NEW(nsResourceManagerWorkerUpdateContent);
    td.m_pTask->ConfigureTask(s, nsTaskNesting::Maybe);

    pUpdateContentTask = td.m_pTask;
    pUpdateContentGroup = &td.m_GroupId;
  }

  // always updated together with pUpdateContentTask
  NS_MSVC_ANALYSIS_ASSUME(pUpdateContentGroup != nullptr);

  // set up the data load task and launch it
  {
    pUpdateContentTask->m_LoaderData = LoaderData;
    pUpdateContentTask->m_pLoader = pLoader;
    pUpdateContentTask->m_pCustomLoader = std::move(pCustomLoader);
    pUpdateContentTask->m_pResourceToLoad = pResourceToLoad;

    // schedule the task to run, either on the main thread or on some other thread
    *pUpdateContentGroup = nsTaskSystem::StartSingleTask(
      pUpdateContentTask, bResourceIsLoadedOnMainThread ? nsTaskPriority::SomeFrameMainThread : nsTaskPriority::LateNextFrame);

    // restart the next loading task (this one is about to finish)
    nsResourceManager::s_pState->m_bAllowLaunchDataLoadTask = true;
    nsResourceManager::RunWorkerTask(nullptr);

    pCustomLoader.Clear();
  }
}


//////////////////////////////////////////////////////////////////////////

nsResourceManagerWorkerUpdateContent::nsResourceManagerWorkerUpdateContent() = default;
nsResourceManagerWorkerUpdateContent::~nsResourceManagerWorkerUpdateContent() = default;

void nsResourceManagerWorkerUpdateContent::Execute()
{
  if (!m_LoaderData.m_sResourceDescription.IsEmpty())
    m_pResourceToLoad->SetResourceDescription(m_LoaderData.m_sResourceDescription);

  m_pResourceToLoad->CallUpdateContent(m_LoaderData.m_pDataStream);

  if (m_pResourceToLoad->m_uiQualityLevelsLoadable > 0)
  {
    // if the resource can have more details loaded, put it into the preload queue right away again
    nsResourceManager::PreloadResource(m_pResourceToLoad);
  }

  // update the file modification date, if available
  if (m_LoaderData.m_LoadedFileModificationDate.IsValid())
    m_pResourceToLoad->m_LoadedFileModificationTime = m_LoaderData.m_LoadedFileModificationDate;

  NS_ASSERT_DEV(m_pResourceToLoad->GetLoadingState() != nsResourceState::Unloaded, "The resource should have changed its loading state.");

  // Update Memory Usage
  {
    nsResource::MemoryUsage MemUsage;
    MemUsage.m_uiMemoryCPU = 0xFFFFFFFF;
    MemUsage.m_uiMemoryGPU = 0xFFFFFFFF;
    m_pResourceToLoad->UpdateMemoryUsage(MemUsage);

    NS_ASSERT_DEV(
      MemUsage.m_uiMemoryCPU != 0xFFFFFFFF, "Resource '{0}' did not properly update its CPU memory usage", m_pResourceToLoad->GetResourceID());
    NS_ASSERT_DEV(
      MemUsage.m_uiMemoryGPU != 0xFFFFFFFF, "Resource '{0}' did not properly update its GPU memory usage", m_pResourceToLoad->GetResourceID());

    m_pResourceToLoad->m_MemoryUsage = MemUsage;
  }

  m_pLoader->CloseDataStream(m_pResourceToLoad, m_LoaderData);

  {
    NS_LOCK(nsResourceManager::s_ResourceMutex);
    NS_ASSERT_DEV(nsResourceManager::IsQueuedForLoading(m_pResourceToLoad), "Multi-threaded access detected");
    m_pResourceToLoad->m_Flags.Remove(nsResourceFlags::IsQueuedForLoading);
    m_pResourceToLoad->m_LastAcquire = nsResourceManager::GetLastFrameUpdate();
  }

  m_pLoader = nullptr;
  m_pResourceToLoad = nullptr;
}
