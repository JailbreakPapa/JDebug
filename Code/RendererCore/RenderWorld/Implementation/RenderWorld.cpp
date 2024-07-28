#include <RendererCore/RendererCorePCH.h>

#include <Core/Console/ConsoleFunction.h>
#include <Core/World/World.h>
#include <Foundation/Application/Application.h>
#include <Foundation/Configuration/CVar.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Memory/CommonAllocators.h>
#include <Foundation/Utilities/DGMLWriter.h>
#include <RendererCore/Pipeline/RenderPipeline.h>
#include <RendererCore/Pipeline/View.h>
#include <RendererCore/RenderWorld/RenderWorld.h>
#include <RendererFoundation/Device/Device.h>
#include <RendererFoundation/Profiling/Profiling.h>

nsCVarBool cvar_RenderingMultithreading("Rendering.Multithreading", true, nsCVarFlags::Default, "Enables multi-threaded update and rendering");
nsCVarBool cvar_RenderingCachingStaticObjects("Rendering.Caching.StaticObjects", true, nsCVarFlags::Default, "Enables render data caching of static objects");

nsEvent<nsView*, nsMutex> nsRenderWorld::s_ViewCreatedEvent;
nsEvent<nsView*, nsMutex> nsRenderWorld::s_ViewDeletedEvent;

nsEvent<void*> nsRenderWorld::s_CameraConfigsModifiedEvent;
bool nsRenderWorld::s_bModifyingCameraConfigs = false;
nsMap<nsString, nsRenderWorld::CameraConfig> nsRenderWorld::s_CameraConfigs;

nsEvent<const nsRenderWorldExtractionEvent&, nsMutex> nsRenderWorld::s_ExtractionEvent;
nsEvent<const nsRenderWorldRenderEvent&, nsMutex> nsRenderWorld::s_RenderEvent;
nsUInt64 nsRenderWorld::s_uiFrameCounter;

namespace
{
  static bool s_bInExtract;
  static nsThreadID s_RenderingThreadID;

  static nsMutex s_ExtractTasksMutex;
  static nsDynamicArray<nsTaskGroupID> s_ExtractTasks;

  static nsMutex s_ViewsMutex;
  static nsIdTable<nsViewId, nsView*> s_Views;

  static nsDynamicArray<nsViewHandle> s_MainViews;

  static nsMutex s_ViewsToRenderMutex;
  static nsDynamicArray<nsView*> s_ViewsToRender;

  static nsDynamicArray<nsSharedPtr<nsRenderPipeline>> s_FilteredRenderPipelines[2];

  struct PipelineToRebuild
  {
    NS_DECLARE_POD_TYPE();

    nsRenderPipeline* m_pPipeline;
    nsViewHandle m_hView;
  };

  static nsMutex s_PipelinesToRebuildMutex;
  static nsDynamicArray<PipelineToRebuild> s_PipelinesToRebuild;

  static nsProxyAllocator* s_pCacheAllocator;

  static nsMutex s_CachedRenderDataMutex;
  using CachedRenderDataPerComponent = nsHybridArray<const nsRenderData*, 4>;
  static nsHashTable<nsComponentHandle, CachedRenderDataPerComponent> s_CachedRenderData;
  static nsDynamicArray<const nsRenderData*> s_DeletedRenderData;

  enum
  {
    MaxNumNewCacheEntries = 32
  };

  static bool s_bWriteRenderPipelineDgml = false;
  static nsConsoleFunction<void()> s_ConFunc_WriteRenderPipelineDgml("WriteRenderPipelineDgml", "()", []()
    { s_bWriteRenderPipelineDgml = true; });
} // namespace

namespace nsInternal
{
  struct RenderDataCache
  {
    RenderDataCache(nsAllocator* pAllocator)
      : m_PerObjectCaches(pAllocator)
    {
      for (nsUInt32 i = 0; i < MaxNumNewCacheEntries; ++i)
      {
        m_NewEntriesPerComponent.PushBack(NewEntryPerComponent(pAllocator));
      }
    }

    struct PerObjectCache
    {
      PerObjectCache() = default;

      PerObjectCache(nsAllocator* pAllocator)
        : m_Entries(pAllocator)
      {
      }

      nsHybridArray<RenderDataCacheEntry, 4> m_Entries;
      nsUInt16 m_uiVersion = 0;
    };

    nsDynamicArray<PerObjectCache> m_PerObjectCaches;

    struct NewEntryPerComponent
    {
      NewEntryPerComponent(nsAllocator* pAllocator)
        : m_Cache(pAllocator)
      {
      }

      nsGameObjectHandle m_hOwnerObject;
      nsComponentHandle m_hOwnerComponent;
      PerObjectCache m_Cache;
    };

    nsStaticArray<NewEntryPerComponent, MaxNumNewCacheEntries> m_NewEntriesPerComponent;
    nsAtomicInteger32 m_NewEntriesCount;
  };

#if NS_ENABLED(NS_PLATFORM_64BIT)
  NS_CHECK_AT_COMPILETIME(sizeof(RenderDataCacheEntry) == 16);
#endif
} // namespace nsInternal

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(RendererCore, RenderWorld)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Foundation",
    "Core"
  END_SUBSYSTEM_DEPENDENCIES

  ON_HIGHLEVELSYSTEMS_STARTUP
  {
    nsRenderWorld::OnEngineStartup();
  }

  ON_HIGHLEVELSYSTEMS_SHUTDOWN
  {
    nsRenderWorld::OnEngineShutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsViewHandle nsRenderWorld::CreateView(const char* szName, nsView*& out_pView)
{
  nsView* pView = NS_DEFAULT_NEW(nsView);

  {
    NS_LOCK(s_ViewsMutex);
    pView->m_InternalId = s_Views.Insert(pView);
  }

  pView->SetName(szName);
  pView->InitializePins();

  pView->m_pRenderDataCache = NS_NEW(s_pCacheAllocator, nsInternal::RenderDataCache, s_pCacheAllocator);

  s_ViewCreatedEvent.Broadcast(pView);

  out_pView = pView;
  return pView->GetHandle();
}

void nsRenderWorld::DeleteView(const nsViewHandle& hView)
{
  nsView* pView = nullptr;

  {
    NS_LOCK(s_ViewsMutex);
    if (!s_Views.Remove(hView, &pView))
      return;
  }

  s_ViewDeletedEvent.Broadcast(pView);

  NS_DELETE(s_pCacheAllocator, pView->m_pRenderDataCache);

  {
    NS_LOCK(s_PipelinesToRebuildMutex);

    for (nsUInt32 i = s_PipelinesToRebuild.GetCount(); i-- > 0;)
    {
      if (s_PipelinesToRebuild[i].m_hView == hView)
      {
        s_PipelinesToRebuild.RemoveAtAndCopy(i);
      }
    }
  }

  RemoveMainView(hView);

  NS_DEFAULT_DELETE(pView);
}

bool nsRenderWorld::TryGetView(const nsViewHandle& hView, nsView*& out_pView)
{
  NS_LOCK(s_ViewsMutex);
  return s_Views.TryGetValue(hView, out_pView);
}

nsView* nsRenderWorld::GetViewByUsageHint(nsCameraUsageHint::Enum usageHint, nsCameraUsageHint::Enum alternativeUsageHint /*= nsCameraUsageHint::None*/, const nsWorld* pWorld /*= nullptr*/)
{
  NS_LOCK(s_ViewsMutex);

  nsView* pAlternativeView = nullptr;

  for (auto it = s_Views.GetIterator(); it.IsValid(); ++it)
  {
    nsView* pView = it.Value();
    if (pWorld != nullptr && pView->GetWorld() != pWorld)
      continue;

    if (pView->GetCameraUsageHint() == usageHint)
    {
      return pView;
    }
    else if (alternativeUsageHint != nsCameraUsageHint::None && pView->GetCameraUsageHint() == alternativeUsageHint)
    {
      pAlternativeView = pView;
    }
  }

  return pAlternativeView;
}

void nsRenderWorld::AddMainView(const nsViewHandle& hView)
{
  NS_ASSERT_DEV(!s_bInExtract, "Cannot add main view during extraction");

  if (!s_MainViews.Contains(hView))
    s_MainViews.PushBack(hView);
}

void nsRenderWorld::RemoveMainView(const nsViewHandle& hView)
{
  nsUInt32 uiIndex = s_MainViews.IndexOf(hView);
  if (uiIndex != nsInvalidIndex)
  {
    NS_ASSERT_DEV(!s_bInExtract, "Cannot remove main view during extraction");
    s_MainViews.RemoveAtAndCopy(uiIndex);
  }
}

void nsRenderWorld::ClearMainViews()
{
  NS_ASSERT_DEV(!s_bInExtract, "Cannot clear main views during extraction");

  s_MainViews.Clear();
}

nsArrayPtr<nsViewHandle> nsRenderWorld::GetMainViews()
{
  return s_MainViews;
}

void nsRenderWorld::CacheRenderData(const nsView& view, const nsGameObjectHandle& hOwnerObject, const nsComponentHandle& hOwnerComponent, nsUInt16 uiComponentVersion, nsArrayPtr<nsInternal::RenderDataCacheEntry> cacheEntries)
{
  if (cvar_RenderingCachingStaticObjects)
  {
    nsUInt32 uiNewEntriesCount = view.m_pRenderDataCache->m_NewEntriesCount;
    if (uiNewEntriesCount >= MaxNumNewCacheEntries)
    {
      return;
    }

    uiNewEntriesCount = view.m_pRenderDataCache->m_NewEntriesCount.Increment();
    if (uiNewEntriesCount <= MaxNumNewCacheEntries)
    {
      auto& newEntry = view.m_pRenderDataCache->m_NewEntriesPerComponent[uiNewEntriesCount - 1];
      newEntry.m_hOwnerObject = hOwnerObject;
      newEntry.m_hOwnerComponent = hOwnerComponent;
      newEntry.m_Cache.m_Entries = cacheEntries;
      newEntry.m_Cache.m_uiVersion = uiComponentVersion;
    }
  }
}

void nsRenderWorld::DeleteAllCachedRenderData()
{
  NS_PROFILE_SCOPE("DeleteAllCachedRenderData");

  NS_ASSERT_DEV(!s_bInExtract, "Cannot delete cached render data during extraction");

  {
    NS_LOCK(s_ViewsMutex);

    for (auto it = s_Views.GetIterator(); it.IsValid(); ++it)
    {
      nsView* pView = it.Value();
      pView->m_pRenderDataCache->m_PerObjectCaches.Clear();
    }
  }

  {
    NS_LOCK(s_CachedRenderDataMutex);

    for (auto it = s_CachedRenderData.GetIterator(); it.IsValid(); ++it)
    {
      auto& cachedRenderDataPerComponent = it.Value();

      for (auto pCachedRenderData : cachedRenderDataPerComponent)
      {
        s_DeletedRenderData.PushBack(pCachedRenderData);
      }

      cachedRenderDataPerComponent.Clear();
    }
  }
}

void nsRenderWorld::DeleteCachedRenderData(const nsGameObjectHandle& hOwnerObject, const nsComponentHandle& hOwnerComponent)
{
  NS_ASSERT_DEV(!s_bInExtract, "Cannot delete cached render data during extraction");

  DeleteCachedRenderDataInternal(hOwnerObject);

  NS_LOCK(s_CachedRenderDataMutex);

  CachedRenderDataPerComponent* pCachedRenderDataPerComponent = nullptr;
  if (s_CachedRenderData.TryGetValue(hOwnerComponent, pCachedRenderDataPerComponent))
  {
    for (auto pCachedRenderData : *pCachedRenderDataPerComponent)
    {
      s_DeletedRenderData.PushBack(pCachedRenderData);
    }

    s_CachedRenderData.Remove(hOwnerComponent);
  }
}

void nsRenderWorld::ResetRenderDataCache(nsView& ref_view)
{
  ref_view.m_pRenderDataCache->m_PerObjectCaches.Clear();
  ref_view.m_pRenderDataCache->m_NewEntriesCount = 0;

  if (ref_view.GetWorld() != nullptr)
  {
    if (ref_view.GetWorld()->GetObjectDeletionEvent().HasEventHandler(&nsRenderWorld::DeleteCachedRenderDataForObject) == false)
    {
      ref_view.GetWorld()->GetObjectDeletionEvent().AddEventHandler(&nsRenderWorld::DeleteCachedRenderDataForObject);
    }
  }
}

void nsRenderWorld::DeleteCachedRenderDataForObject(const nsGameObject* pOwnerObject)
{
  NS_ASSERT_DEV(!s_bInExtract, "Cannot delete cached render data during extraction");

  DeleteCachedRenderDataInternal(pOwnerObject->GetHandle());

  NS_LOCK(s_CachedRenderDataMutex);

  auto components = pOwnerObject->GetComponents();
  for (auto pComponent : components)
  {
    nsComponentHandle hComponent = pComponent->GetHandle();

    CachedRenderDataPerComponent* pCachedRenderDataPerComponent = nullptr;
    if (s_CachedRenderData.TryGetValue(hComponent, pCachedRenderDataPerComponent))
    {
      for (auto pCachedRenderData : *pCachedRenderDataPerComponent)
      {
        s_DeletedRenderData.PushBack(pCachedRenderData);
      }

      s_CachedRenderData.Remove(hComponent);
    }
  }
}

void nsRenderWorld::DeleteCachedRenderDataForObjectRecursive(const nsGameObject* pOwnerObject)
{
  DeleteCachedRenderDataForObject(pOwnerObject);

  for (auto it = pOwnerObject->GetChildren(); it.IsValid(); ++it)
  {
    DeleteCachedRenderDataForObjectRecursive(it);
  }
}

nsArrayPtr<const nsInternal::RenderDataCacheEntry> nsRenderWorld::GetCachedRenderData(const nsView& view, const nsGameObjectHandle& hOwner, nsUInt16 uiComponentVersion)
{
  if (cvar_RenderingCachingStaticObjects)
  {
    const auto& perObjectCaches = view.m_pRenderDataCache->m_PerObjectCaches;
    nsUInt32 uiCacheIndex = hOwner.GetInternalID().m_InstanceIndex;
    if (uiCacheIndex < perObjectCaches.GetCount())
    {
      auto& perObjectCache = perObjectCaches[uiCacheIndex];
      if (perObjectCache.m_uiVersion == uiComponentVersion)
      {
        return perObjectCache.m_Entries;
      }
    }
  }

  return nsArrayPtr<const nsInternal::RenderDataCacheEntry>();
}

void nsRenderWorld::AddViewToRender(const nsViewHandle& hView)
{
  nsView* pView = nullptr;
  if (!TryGetView(hView, pView))
    return;

  if (!pView->IsValid())
    return;

  {
    NS_LOCK(s_ViewsToRenderMutex);
    NS_ASSERT_DEV(s_bInExtract, "Render views need to be collected during extraction");

    // make sure the view is put at the end of the array, if it is already there, reorder it
    // this ensures that the views that have been referenced by the last other view, get rendered first
    nsUInt32 uiIndex = s_ViewsToRender.IndexOf(pView);
    if (uiIndex != nsInvalidIndex)
    {
      s_ViewsToRender.RemoveAtAndCopy(uiIndex);
      s_ViewsToRender.PushBack(pView);
      return;
    }

    s_ViewsToRender.PushBack(pView);
  }

  if (cvar_RenderingMultithreading)
  {
    nsTaskGroupID extractTaskID = nsTaskSystem::StartSingleTask(pView->GetExtractTask(), nsTaskPriority::EarlyThisFrame);

    {
      NS_LOCK(s_ExtractTasksMutex);
      s_ExtractTasks.PushBack(extractTaskID);
    }
  }
  else
  {
    pView->ExtractData();
  }
}

void nsRenderWorld::ExtractMainViews()
{
  NS_ASSERT_DEV(!s_bInExtract, "ExtractMainViews must not be called from multiple threads.");

  s_bInExtract = true;

  nsRenderWorldExtractionEvent extractionEvent;
  extractionEvent.m_Type = nsRenderWorldExtractionEvent::Type::BeginExtraction;
  extractionEvent.m_uiFrameCounter = s_uiFrameCounter;
  s_ExtractionEvent.Broadcast(extractionEvent);

  if (cvar_RenderingMultithreading)
  {
    s_ExtractTasks.Clear();

    nsTaskGroupID extractTaskID = nsTaskSystem::CreateTaskGroup(nsTaskPriority::EarlyThisFrame);
    s_ExtractTasks.PushBack(extractTaskID);

    {
      NS_LOCK(s_ViewsMutex);

      for (nsUInt32 i = 0; i < s_MainViews.GetCount(); ++i)
      {
        nsView* pView = nullptr;
        if (s_Views.TryGetValue(s_MainViews[i], pView) && pView->IsValid())
        {
          s_ViewsToRender.PushBack(pView);
          nsTaskSystem::AddTaskToGroup(extractTaskID, pView->GetExtractTask());
        }
      }
    }

    nsTaskSystem::StartTaskGroup(extractTaskID);

    {
      NS_PROFILE_SCOPE("Wait for Extraction");

      while (true)
      {
        nsTaskGroupID taskID;

        {
          NS_LOCK(s_ExtractTasksMutex);
          if (s_ExtractTasks.IsEmpty())
            break;

          taskID = s_ExtractTasks.PeekBack();
          s_ExtractTasks.PopBack();
        }

        nsTaskSystem::WaitForGroup(taskID);
      }
    }
  }
  else
  {
    for (nsUInt32 i = 0; i < s_MainViews.GetCount(); ++i)
    {
      nsView* pView = nullptr;
      if (s_Views.TryGetValue(s_MainViews[i], pView) && pView->IsValid())
      {
        s_ViewsToRender.PushBack(pView);
        pView->ExtractData();
      }
    }
  }

  // filter out duplicates and reverse order so that dependent views are rendered first
  {
    auto& filteredRenderPipelines = s_FilteredRenderPipelines[GetDataIndexForExtraction()];
    filteredRenderPipelines.Clear();

    for (nsUInt32 i = s_ViewsToRender.GetCount(); i-- > 0;)
    {
      auto& pRenderPipeline = s_ViewsToRender[i]->m_pRenderPipeline;
      if (!filteredRenderPipelines.Contains(pRenderPipeline))
      {
        filteredRenderPipelines.PushBack(pRenderPipeline);
      }
    }

    s_ViewsToRender.Clear();
  }

  extractionEvent.m_Type = nsRenderWorldExtractionEvent::Type::EndExtraction;
  s_ExtractionEvent.Broadcast(extractionEvent);

  s_bInExtract = false;
}

void nsRenderWorld::Render(nsRenderContext* pRenderContext)
{
  NS_PROFILE_SCOPE("nsRenderWorld::Render");

  nsRenderWorldRenderEvent renderEvent;
  renderEvent.m_Type = nsRenderWorldRenderEvent::Type::BeginRender;
  renderEvent.m_uiFrameCounter = s_uiFrameCounter;
  {
    NS_PROFILE_SCOPE("BeginRender");
    s_RenderEvent.Broadcast(renderEvent);
  }

  if (!cvar_RenderingMultithreading)
  {
    RebuildPipelines();
  }

  auto& filteredRenderPipelines = s_FilteredRenderPipelines[GetDataIndexForRendering()];

  if (s_bWriteRenderPipelineDgml)
  {
    // Executed via WriteRenderPipelineDgml console command.
    s_bWriteRenderPipelineDgml = false;
    const nsDateTime dt = nsDateTime::MakeFromTimestamp(nsTimestamp::CurrentTimestamp());
    for (nsUInt32 i = 0; i < filteredRenderPipelines.GetCount(); ++i)
    {
      auto& pRenderPipeline = filteredRenderPipelines[i];
      nsStringBuilder sPath(":appdata/Profiling/", nsApplication::GetApplicationInstance()->GetApplicationName());
      sPath.AppendFormat("_{0}-{1}-{2}_{3}-{4}-{5}_Pipeline{}_{}.dgml", dt.GetYear(), nsArgU(dt.GetMonth(), 2, true), nsArgU(dt.GetDay(), 2, true), nsArgU(dt.GetHour(), 2, true), nsArgU(dt.GetMinute(), 2, true), nsArgU(dt.GetSecond(), 2, true), i, pRenderPipeline->GetViewName().GetData());

      nsDGMLGraph graph(nsDGMLGraph::Direction::TopToBottom);
      pRenderPipeline->CreateDgmlGraph(graph);
      if (nsDGMLGraphWriter::WriteGraphToFile(sPath, graph).Failed())
      {
        nsLog::Error("Failed to write render pipeline dgml: {}", sPath);
      }
    }
  }

  for (auto& pRenderPipeline : filteredRenderPipelines)
  {
    // If we are the only one holding a reference to the pipeline skip rendering. The pipeline is not needed anymore and will be deleted soon.
    if (pRenderPipeline->GetRefCount() > 1)
    {
      pRenderPipeline->Render(pRenderContext);
    }
    pRenderPipeline = nullptr;
  }

  filteredRenderPipelines.Clear();
  /// NOTE: (Only Applies When Tracy is Enabled.)Tracy Seems to declare Timers in the same scope, so dual profile macros can throw: '__tracy_scoped_zone' : redefinition; multitple initalization, so we must scope the two events.
  {
    renderEvent.m_Type = nsRenderWorldRenderEvent::Type::EndRender;
    NS_PROFILE_SCOPE("EndRender");
    s_RenderEvent.Broadcast(renderEvent);
  }
}

void nsRenderWorld::BeginFrame()
{
  NS_PROFILE_SCOPE("BeginFrame");

  s_RenderingThreadID = nsThreadUtils::GetCurrentThreadID();

  for (auto it = s_Views.GetIterator(); it.IsValid(); ++it)
  {
    nsView* pView = it.Value();
    pView->EnsureUpToDate();
  }

  RebuildPipelines();
}

void nsRenderWorld::EndFrame()
{
  NS_PROFILE_SCOPE("EndFrame");

  ++s_uiFrameCounter;

  for (auto it = s_Views.GetIterator(); it.IsValid(); ++it)
  {
    nsView* pView = it.Value();
    if (pView->IsValid())
    {
      pView->ReadBackPassProperties();
    }
  }

  ClearRenderDataCache();
  UpdateRenderDataCache();

  s_RenderingThreadID = (nsThreadID)0;
}

bool nsRenderWorld::GetUseMultithreadedRendering()
{
  return cvar_RenderingMultithreading;
}


bool nsRenderWorld::IsRenderingThread()
{
  return s_RenderingThreadID == nsThreadUtils::GetCurrentThreadID();
}

void nsRenderWorld::DeleteCachedRenderDataInternal(const nsGameObjectHandle& hOwnerObject)
{
  nsUInt32 uiCacheIndex = hOwnerObject.GetInternalID().m_InstanceIndex;
  nsWorld* pWorld = nsWorld::GetWorld(hOwnerObject);

  NS_LOCK(s_ViewsMutex);

  for (auto it = s_Views.GetIterator(); it.IsValid(); ++it)
  {
    nsView* pView = it.Value();
    if (pView->GetWorld() != nullptr && pView->GetWorld() == pWorld)
    {
      auto& perObjectCaches = pView->m_pRenderDataCache->m_PerObjectCaches;

      if (uiCacheIndex < perObjectCaches.GetCount())
      {
        perObjectCaches[uiCacheIndex].m_Entries.Clear();
        perObjectCaches[uiCacheIndex].m_uiVersion = 0;
      }
    }
  }
}

void nsRenderWorld::ClearRenderDataCache()
{
  NS_PROFILE_SCOPE("Clear Render Data Cache");

  for (auto pRenderData : s_DeletedRenderData)
  {
    nsRenderData* ptr = const_cast<nsRenderData*>(pRenderData);
    NS_DELETE(s_pCacheAllocator, ptr);
  }

  s_DeletedRenderData.Clear();
}

void nsRenderWorld::UpdateRenderDataCache()
{
  NS_PROFILE_SCOPE("Update Render Data Cache");

  for (auto it = s_Views.GetIterator(); it.IsValid(); ++it)
  {
    nsView* pView = it.Value();
    nsUInt32 uiNumNewEntries = nsMath::Min<nsInt32>(pView->m_pRenderDataCache->m_NewEntriesCount, MaxNumNewCacheEntries);
    pView->m_pRenderDataCache->m_NewEntriesCount = 0;

    auto& perObjectCaches = pView->m_pRenderDataCache->m_PerObjectCaches;

    for (nsUInt32 uiNewEntryIndex = 0; uiNewEntryIndex < uiNumNewEntries; ++uiNewEntryIndex)
    {
      auto& newEntries = pView->m_pRenderDataCache->m_NewEntriesPerComponent[uiNewEntryIndex];
      NS_ASSERT_DEV(!newEntries.m_hOwnerObject.IsInvalidated(), "Implementation error");

      // find or create cached render data
      auto& cachedRenderDataPerComponent = s_CachedRenderData[newEntries.m_hOwnerComponent];

      const nsUInt32 uiNumCachedRenderData = cachedRenderDataPerComponent.GetCount();
      if (uiNumCachedRenderData == 0) // Nothing cached yet
      {
        cachedRenderDataPerComponent = CachedRenderDataPerComponent(s_pCacheAllocator);
      }

      nsUInt32 uiCachedRenderDataIndex = 0;
      for (auto& newEntry : newEntries.m_Cache.m_Entries)
      {
        if (newEntry.m_pRenderData != nullptr)
        {
          if (uiCachedRenderDataIndex >= cachedRenderDataPerComponent.GetCount())
          {
            const nsRTTI* pRtti = newEntry.m_pRenderData->GetDynamicRTTI();
            newEntry.m_pRenderData = pRtti->GetAllocator()->Clone<nsRenderData>(newEntry.m_pRenderData, s_pCacheAllocator);

            cachedRenderDataPerComponent.PushBack(newEntry.m_pRenderData);
          }
          else
          {
            // replace with cached render data
            newEntry.m_pRenderData = cachedRenderDataPerComponent[uiCachedRenderDataIndex];
          }

          ++uiCachedRenderDataIndex;
        }
      }

      // add entry for this view
      const nsUInt32 uiCacheIndex = newEntries.m_hOwnerObject.GetInternalID().m_InstanceIndex;
      perObjectCaches.EnsureCount(uiCacheIndex + 1);

      auto& perObjectCache = perObjectCaches[uiCacheIndex];
      if (perObjectCache.m_uiVersion != newEntries.m_Cache.m_uiVersion)
      {
        perObjectCache.m_Entries.Clear();
        perObjectCache.m_uiVersion = newEntries.m_Cache.m_uiVersion;
      }

      for (auto& newEntry : newEntries.m_Cache.m_Entries)
      {
        if (!perObjectCache.m_Entries.Contains(newEntry))
        {
          perObjectCache.m_Entries.PushBack(newEntry);
        }
      }

      // keep entries sorted, otherwise the logic nsExtractor::ExtractRenderData doesn't work
      perObjectCache.m_Entries.Sort();
    }
  }
}

// static
void nsRenderWorld::AddRenderPipelineToRebuild(nsRenderPipeline* pRenderPipeline, const nsViewHandle& hView)
{
  NS_LOCK(s_PipelinesToRebuildMutex);

  for (auto& pipelineToRebuild : s_PipelinesToRebuild)
  {
    if (pipelineToRebuild.m_hView == hView)
    {
      pipelineToRebuild.m_pPipeline = pRenderPipeline;
      return;
    }
  }

  auto& pipelineToRebuild = s_PipelinesToRebuild.ExpandAndGetRef();
  pipelineToRebuild.m_pPipeline = pRenderPipeline;
  pipelineToRebuild.m_hView = hView;
}

// static
void nsRenderWorld::RebuildPipelines()
{
  NS_PROFILE_SCOPE("RebuildPipelines");

  for (auto& pipelineToRebuild : s_PipelinesToRebuild)
  {
    nsView* pView = nullptr;
    if (s_Views.TryGetValue(pipelineToRebuild.m_hView, pView))
    {
      if (pipelineToRebuild.m_pPipeline->Rebuild(*pView) == nsRenderPipeline::PipelineState::RebuildError)
      {
        nsLog::Error("Failed to rebuild pipeline '{}' for view '{}'", pipelineToRebuild.m_pPipeline->m_sName, pView->GetName());
      }
    }
  }

  s_PipelinesToRebuild.Clear();
}

void nsRenderWorld::OnEngineStartup()
{
  s_pCacheAllocator = NS_DEFAULT_NEW(nsProxyAllocator, "Cached Render Data", nsFoundation::GetDefaultAllocator());

  s_CachedRenderData = nsHashTable<nsComponentHandle, CachedRenderDataPerComponent>(s_pCacheAllocator);
}

void nsRenderWorld::OnEngineShutdown()
{
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  for (auto it : s_CachedRenderData)
  {
    auto& cachedRenderDataPerComponent = it.Value();
    if (cachedRenderDataPerComponent.IsEmpty() == false)
    {
      NS_REPORT_FAILURE("Leaked cached render data of type '{}'", cachedRenderDataPerComponent[0]->GetDynamicRTTI()->GetTypeName());
    }
  }
#endif

  ClearRenderDataCache();

  NS_DEFAULT_DELETE(s_pCacheAllocator);

  s_FilteredRenderPipelines[0].Clear();
  s_FilteredRenderPipelines[1].Clear();

  ClearMainViews();

  for (auto it = s_Views.GetIterator(); it.IsValid(); ++it)
  {
    nsView* pView = it.Value();
    NS_DEFAULT_DELETE(pView);
  }

  s_Views.Clear();
}

void nsRenderWorld::BeginModifyCameraConfigs()
{
  NS_ASSERT_DEBUG(!s_bModifyingCameraConfigs, "Recursive call not allowed.");
  s_bModifyingCameraConfigs = true;
}

void nsRenderWorld::EndModifyCameraConfigs()
{
  NS_ASSERT_DEBUG(s_bModifyingCameraConfigs, "You have to call nsRenderWorld::BeginModifyCameraConfigs first");
  s_bModifyingCameraConfigs = false;
  s_CameraConfigsModifiedEvent.Broadcast(nullptr);
}

void nsRenderWorld::ClearCameraConfigs()
{
  NS_ASSERT_DEBUG(s_bModifyingCameraConfigs, "You have to call nsRenderWorld::BeginModifyCameraConfigs first");
  s_CameraConfigs.Clear();
}

void nsRenderWorld::SetCameraConfig(const char* szName, const CameraConfig& config)
{
  NS_ASSERT_DEBUG(s_bModifyingCameraConfigs, "You have to call nsRenderWorld::BeginModifyCameraConfigs first");
  s_CameraConfigs[szName] = config;
}

const nsRenderWorld::CameraConfig* nsRenderWorld::FindCameraConfig(const char* szName)
{
  auto it = s_CameraConfigs.Find(szName);

  if (!it.IsValid())
    return nullptr;

  return &it.Value();
}

NS_STATICLINK_FILE(RendererCore, RendererCore_RenderWorld_Implementation_RenderWorld);
