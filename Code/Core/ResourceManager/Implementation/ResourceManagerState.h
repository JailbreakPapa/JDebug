#pragma once

#include <Core/CoreInternal.h>
NS_CORE_INTERNAL_HEADER

#include <Core/ResourceManager/ResourceManager.h>

class nsResourceManagerState
{
private:
  friend class nsResource;
  friend class nsResourceManager;
  friend class nsResourceManagerWorkerDataLoad;
  friend class nsResourceManagerWorkerUpdateContent;
  friend class nsResourceHandleReadContext;

  /// \name Events
  ///@{

  nsEvent<const nsResourceEvent&, nsMutex> m_ResourceEvents;
  nsEvent<const nsResourceManagerEvent&, nsMutex> m_ManagerEvents;

  ///@}
  /// \name Resource Fallbacks
  ///@{

  nsDynamicArray<nsResourceManager::ResourceCleanupCB> m_ResourceCleanupCallbacks;

  ///@}
  /// \name Resource Priorities
  ///@{

  nsMap<const nsRTTI*, nsResourcePriority> m_ResourceTypePriorities;

  ///@}

  struct TaskDataUpdateContent
  {
    nsSharedPtr<nsResourceManagerWorkerUpdateContent> m_pTask;
    nsTaskGroupID m_GroupId;
  };

  struct TaskDataDataLoad
  {
    nsSharedPtr<nsResourceManagerWorkerDataLoad> m_pTask;
    nsTaskGroupID m_GroupId;
  };

  bool m_bTaskNamesInitialized = false;
  bool m_bBroadcastExistsEvent = false;
  nsUInt32 m_uiForceNoFallbackAcquisition = 0;

  // resources in this queue are waiting for a task to load them
  nsDeque<nsResourceManager::LoadingInfo> m_LoadingQueue;

  nsHashTable<const nsRTTI*, nsResourceManager::LoadedResources> m_LoadedResources;

  bool m_bAllowLaunchDataLoadTask = true;
  bool m_bShutdown = false;

  nsHybridArray<TaskDataUpdateContent, 24> m_WorkerTasksUpdateContent;
  nsHybridArray<TaskDataDataLoad, 8> m_WorkerTasksDataLoad;

  nsTime m_LastFrameUpdate;
  nsUInt32 m_uiLastResourcePriorityUpdateIdx = 0;

  nsDynamicArray<nsResource*> m_LoadedResourceOfTypeTempContainer;
  nsHashTable<nsTempHashedString, const nsRTTI*> m_ResourcesToUnloadOnMainThread;

  const nsRTTI* m_pFreeUnusedLastType = nullptr;
  nsTempHashedString m_sFreeUnusedLastResourceID;

  // Type Loaders

  nsMap<const nsRTTI*, nsResourceTypeLoader*> m_ResourceTypeLoader;
  nsResourceLoaderFromFile m_FileResourceLoader;
  nsResourceTypeLoader* m_pDefaultResourceLoader = &m_FileResourceLoader;
  nsMap<nsResource*, nsUniquePtr<nsResourceTypeLoader>> m_CustomLoaders;


  // Override / derived resources

  nsMap<const nsRTTI*, nsHybridArray<nsResourceManager::DerivedTypeInfo, 4>> m_DerivedTypeInfos;


  // Named resources

  nsHashTable<nsTempHashedString, nsHashedString> m_NamedResources;

  // Asset system interaction

  nsMap<nsString, const nsRTTI*> m_AssetToResourceType;


  // Export mode

  bool m_bExportMode = false;
  nsUInt32 m_uiNextResourceID = 0;

  // Resource Unloading
  nsTime m_AutoFreeUnusedTimeout = nsTime::MakeZero();
  nsTime m_AutoFreeUnusedThreshold = nsTime::MakeZero();

  nsMap<const nsRTTI*, nsResourceManager::ResourceTypeInfo> m_TypeInfo;
};
