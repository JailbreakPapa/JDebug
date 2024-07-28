#include <Core/CorePCH.h>

#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceManager.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/System/StackTracer.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsResource, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

NS_CORE_DLL void IncreaseResourceRefCount(nsResource* pResource, const void* pOwner)
{
#if NS_ENABLED(NS_RESOURCEHANDLE_STACK_TRACES)
  {
    NS_LOCK(pResource->m_HandleStackTraceMutex);

    auto& info = pResource->m_HandleStackTraces[pOwner];

    nsArrayPtr<void*> ptr(info.m_Ptrs);

    info.m_uiNumPtrs = nsStackTracer::GetStackTrace(ptr);
  }
#endif

  pResource->m_iReferenceCount.Increment();
}

NS_CORE_DLL void DecreaseResourceRefCount(nsResource* pResource, const void* pOwner)
{
#if NS_ENABLED(NS_RESOURCEHANDLE_STACK_TRACES)
  {
    NS_LOCK(pResource->m_HandleStackTraceMutex);

    if (!pResource->m_HandleStackTraces.Remove(pOwner, nullptr))
    {
      NS_REPORT_FAILURE("No associated stack-trace!");
    }
  }
#endif

  pResource->m_iReferenceCount.Decrement();
}

#if NS_ENABLED(NS_RESOURCEHANDLE_STACK_TRACES)
NS_CORE_DLL void MigrateResourceRefCount(nsResource* pResource, const void* pOldOwner, const void* pNewOwner)
{
  NS_LOCK(pResource->m_HandleStackTraceMutex);

  // allocate / resize the hash-table first to ensure the iterator stays valid
  auto& newInfo = pResource->m_HandleStackTraces[pNewOwner];

  auto it = pResource->m_HandleStackTraces.Find(pOldOwner);
  if (!it.IsValid())
  {
    NS_REPORT_FAILURE("No associated stack-trace!");
  }
  else
  {
    newInfo = it.Value();
    pResource->m_HandleStackTraces.Remove(it);
  }
}
#endif

nsResource::~nsResource()
{
  NS_ASSERT_DEV(!nsResourceManager::IsQueuedForLoading(this), "Cannot deallocate a resource while it is still qeued for loading");
}

nsResource::nsResource(DoUpdate ResourceUpdateThread, nsUInt8 uiQualityLevelsLoadable)
{
  m_Flags.AddOrRemove(nsResourceFlags::UpdateOnMainThread, ResourceUpdateThread == DoUpdate::OnMainThread);

  m_uiQualityLevelsLoadable = uiQualityLevelsLoadable;
}

#if NS_ENABLED(NS_RESOURCEHANDLE_STACK_TRACES)
static void LogStackTrace(const char* szText)
{
  nsLog::Info(szText);
};
#endif

void nsResource::PrintHandleStackTraces()
{
#if NS_ENABLED(NS_RESOURCEHANDLE_STACK_TRACES)

  NS_LOCK(m_HandleStackTraceMutex);

  NS_LOG_BLOCK("Resource Handle Stack Traces");

  for (auto& it : m_HandleStackTraces)
  {
    NS_LOG_BLOCK("Handle Trace");

    nsStackTracer::ResolveStackTrace(nsArrayPtr<void*>(it.Value().m_Ptrs, it.Value().m_uiNumPtrs), LogStackTrace);
  }

#else

  nsLog::Warning("Compile with NS_RESOURCEHANDLE_STACK_TRACES set to NS_ON to enable support for resource handle stack traces.");

#endif
}

void nsResource::SetResourceDescription(nsStringView sDescription)
{
  m_sResourceDescription = sDescription;
}

void nsResource::SetUniqueID(nsStringView sUniqueID, bool bIsReloadable)
{
  m_sUniqueID = sUniqueID;
  m_uiUniqueIDHash = nsHashingUtils::StringHash(sUniqueID);
  SetIsReloadable(bIsReloadable);

  nsResourceEvent e;
  e.m_pResource = this;
  e.m_Type = nsResourceEvent::Type::ResourceCreated;
  nsResourceManager::BroadcastResourceEvent(e);
}

void nsResource::CallUnloadData(Unload WhatToUnload)
{
  NS_LOG_BLOCK("nsResource::UnloadData", GetResourceID().GetData());

  nsResourceEvent e;
  e.m_pResource = this;
  e.m_Type = nsResourceEvent::Type::ResourceContentUnloading;
  nsResourceManager::BroadcastResourceEvent(e);

  nsResourceLoadDesc ld = UnloadData(WhatToUnload);

  NS_ASSERT_DEV(ld.m_State != nsResourceState::Invalid, "UnloadData() did not return a valid resource load state");
  NS_ASSERT_DEV(ld.m_uiQualityLevelsDiscardable != 0xFF, "UnloadData() did not fill out m_uiQualityLevelsDiscardable correctly");
  NS_ASSERT_DEV(ld.m_uiQualityLevelsLoadable != 0xFF, "UnloadData() did not fill out m_uiQualityLevelsLoadable correctly");

  m_LoadingState = ld.m_State;
  m_uiQualityLevelsDiscardable = ld.m_uiQualityLevelsDiscardable;
  m_uiQualityLevelsLoadable = ld.m_uiQualityLevelsLoadable;
}

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
thread_local const nsResource* g_pCurrentlyUpdatingContent = nullptr;

const nsResource* nsResource::GetCurrentlyUpdatingContent()
{
  return g_pCurrentlyUpdatingContent;
}
#endif

void nsResource::CallUpdateContent(nsStreamReader* Stream)
{
  NS_PROFILE_SCOPE("CallUpdateContent");

  NS_LOG_BLOCK("nsResource::UpdateContent", GetResourceID().GetData());

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  const nsResource* pPreviouslyUpdatingContent = g_pCurrentlyUpdatingContent;
  g_pCurrentlyUpdatingContent = this;
  nsResourceLoadDesc ld = UpdateContent(Stream);
  g_pCurrentlyUpdatingContent = pPreviouslyUpdatingContent;
#else
  nsResourceLoadDesc ld = UpdateContent(Stream);
#endif

  NS_ASSERT_DEV(ld.m_State != nsResourceState::Invalid, "UpdateContent() did not return a valid resource load state");
  NS_ASSERT_DEV(ld.m_uiQualityLevelsDiscardable != 0xFF, "UpdateContent() did not fill out m_uiQualityLevelsDiscardable correctly");
  NS_ASSERT_DEV(ld.m_uiQualityLevelsLoadable != 0xFF, "UpdateContent() did not fill out m_uiQualityLevelsLoadable correctly");

  if (ld.m_State == nsResourceState::LoadedResourceMissing)
  {
    ReportResourceIsMissing();
  }

  IncResourceChangeCounter();

  m_uiQualityLevelsDiscardable = ld.m_uiQualityLevelsDiscardable;
  m_uiQualityLevelsLoadable = ld.m_uiQualityLevelsLoadable;
  m_LoadingState = ld.m_State;

  nsResourceEvent e;
  e.m_pResource = this;
  e.m_Type = nsResourceEvent::Type::ResourceContentUpdated;
  nsResourceManager::BroadcastResourceEvent(e);

  nsLog::Debug("Updated {0} - '{1}'", GetDynamicRTTI()->GetTypeName(), nsArgSensitive(GetResourceDescription(), "ResourceDesc"));
}

float nsResource::GetLoadingPriority(nsTime now) const
{
  if (m_Priority == nsResourcePriority::Critical)
    return 0.0f;

  // low priority values mean it gets loaded earlier
  float fPriority = static_cast<float>(m_Priority) * 10.0f;

  if (GetLoadingState() == nsResourceState::Loaded)
  {
    // already loaded -> more penalty
    fPriority += 30.0f;

    // the more it could discard, the less important it is to load more of it
    fPriority += GetNumQualityLevelsDiscardable() * 10.0f;
  }
  else
  {
    const nsBitflags<nsResourceFlags> flags = GetBaseResourceFlags();

    if (flags.IsAnySet(nsResourceFlags::ResourceHasFallback))
    {
      // if the resource has a very specific fallback, it is least important to be get loaded
      fPriority += 20.0f;
    }
    else if (flags.IsAnySet(nsResourceFlags::ResourceHasTypeFallback))
    {
      // if it has at least a type fallback, it is less important to get loaded
      fPriority += 10.0f;
    }
  }

  // everything acquired in the last N seconds gets a higher priority
  // by getting the lowest penalty
  const float secondsSinceAcquire = (float)(now - GetLastAcquireTime()).GetSeconds();
  const float fTimePriority = nsMath::Min(10.0f, secondsSinceAcquire);

  return fPriority + fTimePriority;
}

void nsResource::SetPriority(nsResourcePriority priority)
{
  if (m_Priority == priority)
    return;

  m_Priority = priority;

  nsResourceEvent e;
  e.m_pResource = this;
  e.m_Type = nsResourceEvent::Type::ResourcePriorityChanged;
  nsResourceManager::BroadcastResourceEvent(e);
}

nsResourceTypeLoader* nsResource::GetDefaultResourceTypeLoader() const
{
  return nsResourceManager::GetDefaultResourceLoader();
}

void nsResource::ReportResourceIsMissing()
{
  nsLog::SeriousWarning("Missing Resource of Type '{2}': '{0}' ('{1}')", nsArgSensitive(GetResourceID(), "ResourceID"),
    nsArgSensitive(m_sResourceDescription, "ResourceDesc"), GetDynamicRTTI()->GetTypeName());
}

void nsResource::VerifyAfterCreateResource(const nsResourceLoadDesc& ld)
{
  NS_ASSERT_DEV(ld.m_State != nsResourceState::Invalid, "CreateResource() did not return a valid resource load state");
  NS_ASSERT_DEV(ld.m_uiQualityLevelsDiscardable != 0xFF, "CreateResource() did not fill out m_uiQualityLevelsDiscardable correctly");
  NS_ASSERT_DEV(ld.m_uiQualityLevelsLoadable != 0xFF, "CreateResource() did not fill out m_uiQualityLevelsLoadable correctly");

  IncResourceChangeCounter();

  m_LoadingState = ld.m_State;
  m_uiQualityLevelsDiscardable = ld.m_uiQualityLevelsDiscardable;
  m_uiQualityLevelsLoadable = ld.m_uiQualityLevelsLoadable;

  /* Update Memory Usage*/
  {
    nsResource::MemoryUsage MemUsage;
    MemUsage.m_uiMemoryCPU = 0xFFFFFFFF;
    MemUsage.m_uiMemoryGPU = 0xFFFFFFFF;
    UpdateMemoryUsage(MemUsage);

    NS_ASSERT_DEV(MemUsage.m_uiMemoryCPU != 0xFFFFFFFF, "Resource '{0}' did not properly update its CPU memory usage", GetResourceID());
    NS_ASSERT_DEV(MemUsage.m_uiMemoryGPU != 0xFFFFFFFF, "Resource '{0}' did not properly update its GPU memory usage", GetResourceID());

    m_MemoryUsage = MemUsage;
  }

  nsResourceEvent e;
  e.m_pResource = this;
  e.m_Type = nsResourceEvent::Type::ResourceContentUpdated;
  nsResourceManager::BroadcastResourceEvent(e);

  nsLog::Debug("Created {0} - '{1}' ", GetDynamicRTTI()->GetTypeName(), nsArgSensitive(GetResourceDescription(), "ResourceDesc"));
}

NS_STATICLINK_FILE(Core, Core_ResourceManager_Implementation_Resource);
