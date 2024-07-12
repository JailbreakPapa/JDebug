#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/IdTable.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/AllocatorWithPolicy.h>
#include <Foundation/Memory/Policies/AllocPolicyHeap.h>
#include <Foundation/Strings/String.h>
#include <Foundation/System/StackTracer.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT) && TRACY_ENABLE && TRACY_ENABLE_MEMORY_TRACKING
#  include <tracy/tracy/Tracy.hpp>

#  define NS_TRACY_CALLSTACK_DEPTH 16
#  define NS_TRACY_ALLOC_CS(ptr, size, name) TracyAllocNS(ptr, size, NS_TRACY_CALLSTACK_DEPTH, name)
#  define NS_TRACY_FREE_CS(ptr, name) TracyFreeNS(ptr, NS_TRACY_CALLSTACK_DEPTH, name)
#  define NS_TRACY_ALLOC(ptr, size, name) TracyAllocN(ptr, size, name)
#  define NS_TRACY_FREE(ptr, name) TracyFreeN(ptr, name)
#else
#  define NS_TRACY_ALLOC_CS(ptr, size, name)
#  define NS_TRACY_FREE_CS(ptr, name)
#  define NS_TRACY_ALLOC(ptr, size, name)
#  define NS_TRACY_FREE(ptr, name)
#endif

namespace
{
  // no tracking for the tracker data itself
  using TrackerDataAllocator = nsAllocatorWithPolicy<nsAllocPolicyHeap, nsAllocatorTrackingMode::Nothing>;

  static TrackerDataAllocator* s_pTrackerDataAllocator;

  struct TrackerDataAllocatorWrapper
  {
    NS_ALWAYS_INLINE static nsAllocator* GetAllocator() { return s_pTrackerDataAllocator; }
  };


  struct AllocatorData
  {
    NS_ALWAYS_INLINE AllocatorData() = default;

    nsHybridString<32, TrackerDataAllocatorWrapper> m_sName;
    nsAllocatorTrackingMode m_TrackingMode;

    nsAllocatorId m_ParentId;

    nsAllocator::Stats m_Stats;

    nsHashTable<const void*, nsMemoryTracker::AllocationInfo, nsHashHelper<const void*>, TrackerDataAllocatorWrapper> m_Allocations;
  };

  struct TrackerData
  {
    NS_ALWAYS_INLINE void Lock() { m_Mutex.Lock(); }
    NS_ALWAYS_INLINE void Unlock() { m_Mutex.Unlock(); }

    nsMutex m_Mutex;

    using AllocatorTable = nsIdTable<nsAllocatorId, AllocatorData, TrackerDataAllocatorWrapper>;
    AllocatorTable m_AllocatorData;
  };

  static TrackerData* s_pTrackerData;
  static bool s_bIsInitialized = false;
  static bool s_bIsInitializing = false;

  static void Initialize()
  {
    if (s_bIsInitialized)
      return;

    NS_ASSERT_DEV(!s_bIsInitializing, "MemoryTracker initialization entered recursively");
    s_bIsInitializing = true;

    if (s_pTrackerDataAllocator == nullptr)
    {
      alignas(NS_ALIGNMENT_OF(TrackerDataAllocator)) static nsUInt8 TrackerDataAllocatorBuffer[sizeof(TrackerDataAllocator)];
      s_pTrackerDataAllocator = new (TrackerDataAllocatorBuffer) TrackerDataAllocator("MemoryTracker");
      NS_ASSERT_DEV(s_pTrackerDataAllocator != nullptr, "MemoryTracker initialization failed");
    }

    if (s_pTrackerData == nullptr)
    {
      alignas(NS_ALIGNMENT_OF(TrackerData)) static nsUInt8 TrackerDataBuffer[sizeof(TrackerData)];
      s_pTrackerData = new (TrackerDataBuffer) TrackerData();
      NS_ASSERT_DEV(s_pTrackerData != nullptr, "MemoryTracker initialization failed");
    }

    s_bIsInitialized = true;
    s_bIsInitializing = false;
  }

  static void DumpLeak(const nsMemoryTracker::AllocationInfo& info, const char* szAllocatorName)
  {
    char szBuffer[512];
    nsUInt64 uiSize = info.m_uiSize;
    nsStringUtils::snprintf(szBuffer, NS_ARRAY_SIZE(szBuffer), "Leaked %llu bytes allocated by '%s'\n", uiSize, szAllocatorName);

    nsLog::Print(szBuffer);

    if (info.GetStackTrace().GetPtr() != nullptr)
    {
      nsStackTracer::ResolveStackTrace(info.GetStackTrace(), &nsLog::Print);
    }

    nsLog::Print("--------------------------------------------------------------------\n\n");
  }
} // namespace

// Iterator
#define CAST_ITER(ptr) static_cast<TrackerData::AllocatorTable::Iterator*>(ptr)

nsAllocatorId nsMemoryTracker::Iterator::Id() const
{
  return CAST_ITER(m_pData)->Id();
}

nsStringView nsMemoryTracker::Iterator::Name() const
{
  return CAST_ITER(m_pData)->Value().m_sName;
}

nsAllocatorId nsMemoryTracker::Iterator::ParentId() const
{
  return CAST_ITER(m_pData)->Value().m_ParentId;
}

const nsAllocator::Stats& nsMemoryTracker::Iterator::Stats() const
{
  return CAST_ITER(m_pData)->Value().m_Stats;
}

void nsMemoryTracker::Iterator::Next()
{
  CAST_ITER(m_pData)->Next();
}

bool nsMemoryTracker::Iterator::IsValid() const
{
  return CAST_ITER(m_pData)->IsValid();
}

nsMemoryTracker::Iterator::~Iterator()
{
  auto it = CAST_ITER(m_pData);
  NS_DELETE(s_pTrackerDataAllocator, it);
  m_pData = nullptr;
}


// static
nsAllocatorId nsMemoryTracker::RegisterAllocator(nsStringView sName, nsAllocatorTrackingMode mode, nsAllocatorId parentId)
{
  Initialize();

  NS_LOCK(*s_pTrackerData);

  AllocatorData data;
  data.m_sName = sName;
  data.m_TrackingMode = mode;
  data.m_ParentId = parentId;

  return s_pTrackerData->m_AllocatorData.Insert(data);
}

// static
void nsMemoryTracker::DeregisterAllocator(nsAllocatorId allocatorId)
{
  NS_LOCK(*s_pTrackerData);

  const AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];

  nsUInt32 uiLiveAllocations = data.m_Allocations.GetCount();
  if (uiLiveAllocations != 0)
  {
    for (auto it = data.m_Allocations.GetIterator(); it.IsValid(); ++it)
    {
      DumpLeak(it.Value(), data.m_sName.GetData());
    }

    NS_REPORT_FAILURE("Allocator '{0}' leaked {1} allocation(s)", data.m_sName.GetData(), uiLiveAllocations);
  }

  s_pTrackerData->m_AllocatorData.Remove(allocatorId);
}

// static
void nsMemoryTracker::AddAllocation(nsAllocatorId allocatorId, nsAllocatorTrackingMode mode, const void* pPtr, size_t uiSize, size_t uiAlign, nsTime allocationTime)
{
  NS_ASSERT_DEV(uiAlign < 0xFFFF, "Alignment too big");

  nsArrayPtr<void*> stackTrace;
  if (mode >= nsAllocatorTrackingMode::AllocationStatsAndStacktraces)
  {
    void* pBuffer[64];
    nsArrayPtr<void*> tempTrace(pBuffer);
    const nsUInt32 uiNumTraces = nsStackTracer::GetStackTrace(tempTrace);

    stackTrace = NS_NEW_ARRAY(s_pTrackerDataAllocator, void*, uiNumTraces);
    nsMemoryUtils::Copy(stackTrace.GetPtr(), pBuffer, uiNumTraces);
  }

  {
    NS_LOCK(*s_pTrackerData);

    AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];
    data.m_Stats.m_uiNumAllocations++;
    data.m_Stats.m_uiAllocationSize += uiSize;
    data.m_Stats.m_uiPerFrameAllocationSize += uiSize;
    data.m_Stats.m_PerFrameAllocationTime += allocationTime;

    auto pInfo = &data.m_Allocations[pPtr];
    pInfo->m_uiSize = uiSize;
    pInfo->m_uiAlignment = (nsUInt16)uiAlign;
    pInfo->SetStackTrace(stackTrace);

    if (mode >= nsAllocatorTrackingMode::AllocationStatsAndStacktraces)
    {
      NS_TRACY_ALLOC_CS(pPtr, uiSize, data.m_sName.GetData());
    }
    else
    {
      NS_TRACY_ALLOC(pPtr, uiSize, data.m_sName.GetData());
    }
  }
}

// static
void nsMemoryTracker::RemoveAllocation(nsAllocatorId allocatorId, const void* pPtr)
{
  nsArrayPtr<void*> stackTrace;

  {
    NS_LOCK(*s_pTrackerData);

    AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];

    AllocationInfo info;
    if (data.m_Allocations.Remove(pPtr, &info))
    {
      data.m_Stats.m_uiNumDeallocations++;
      data.m_Stats.m_uiAllocationSize -= info.m_uiSize;

      stackTrace = info.GetStackTrace();

      if (data.m_TrackingMode >= nsAllocatorTrackingMode::AllocationStatsAndStacktraces)
      {
        NS_TRACY_FREE_CS(pPtr, data.m_sName.GetData());
      }
      else
      {
        NS_TRACY_FREE(pPtr, data.m_sName.GetData());
      }
    }
    else
    {
      NS_REPORT_FAILURE("Invalid Allocation '{0}'. Memory corruption?", nsArgP(pPtr));
    }
  }

  NS_DELETE_ARRAY(s_pTrackerDataAllocator, stackTrace);
}

// static
void nsMemoryTracker::RemoveAllAllocations(nsAllocatorId allocatorId)
{
  NS_LOCK(*s_pTrackerData);
  AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];
  for (auto it = data.m_Allocations.GetIterator(); it.IsValid(); ++it)
  {
    auto& info = it.Value();
    data.m_Stats.m_uiNumDeallocations++;
    data.m_Stats.m_uiAllocationSize -= info.m_uiSize;

    if (data.m_TrackingMode >= nsAllocatorTrackingMode::AllocationStatsAndStacktraces)
    {
      for (const auto& alloc : data.m_Allocations)
      {
        NS_IGNORE_UNUSED(alloc);
        NS_TRACY_FREE_CS(alloc.Key(), data.m_sName.GetData());
      }
    }
    else
    {
      for (const auto& alloc : data.m_Allocations)
      {
        NS_IGNORE_UNUSED(alloc);
        NS_TRACY_FREE(alloc.Key(), data.m_sName.GetData());
      }
    }

    NS_DELETE_ARRAY(s_pTrackerDataAllocator, info.GetStackTrace());
  }
  data.m_Allocations.Clear();
}

// static
void nsMemoryTracker::SetAllocatorStats(nsAllocatorId allocatorId, const nsAllocator::Stats& stats)
{
  NS_LOCK(*s_pTrackerData);

  s_pTrackerData->m_AllocatorData[allocatorId].m_Stats = stats;
}

// static
void nsMemoryTracker::ResetPerFrameAllocatorStats()
{
  NS_LOCK(*s_pTrackerData);

  for (auto it = s_pTrackerData->m_AllocatorData.GetIterator(); it.IsValid(); ++it)
  {
    AllocatorData& data = it.Value();
    data.m_Stats.m_uiPerFrameAllocationSize = 0;
    data.m_Stats.m_PerFrameAllocationTime = nsTime::MakeZero();
  }
}

// static
nsStringView nsMemoryTracker::GetAllocatorName(nsAllocatorId allocatorId)
{
  NS_LOCK(*s_pTrackerData);

  return s_pTrackerData->m_AllocatorData[allocatorId].m_sName;
}

// static
const nsAllocator::Stats& nsMemoryTracker::GetAllocatorStats(nsAllocatorId allocatorId)
{
  NS_LOCK(*s_pTrackerData);

  return s_pTrackerData->m_AllocatorData[allocatorId].m_Stats;
}

// static
nsAllocatorId nsMemoryTracker::GetAllocatorParentId(nsAllocatorId allocatorId)
{
  NS_LOCK(*s_pTrackerData);

  return s_pTrackerData->m_AllocatorData[allocatorId].m_ParentId;
}

// static
const nsMemoryTracker::AllocationInfo& nsMemoryTracker::GetAllocationInfo(nsAllocatorId allocatorId, const void* pPtr)
{
  NS_LOCK(*s_pTrackerData);

  const AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];
  const AllocationInfo* info = nullptr;
  if (data.m_Allocations.TryGetValue(pPtr, info))
  {
    return *info;
  }

  static AllocationInfo invalidInfo;

  NS_REPORT_FAILURE("Could not find info for allocation {0}", nsArgP(pPtr));
  return invalidInfo;
}

struct LeakInfo
{
  NS_DECLARE_POD_TYPE();

  nsAllocatorId m_AllocatorId;
  size_t m_uiSize = 0;
  bool m_bIsRootLeak = true;
};

// static
nsUInt32 nsMemoryTracker::PrintMemoryLeaks(PrintFunc printfunc)
{
  if (s_pTrackerData == nullptr) // if both tracking and tracing is disabled there is no tracker data
    return 0;

  NS_LOCK(*s_pTrackerData);

  nsHashTable<const void*, LeakInfo, nsHashHelper<const void*>, TrackerDataAllocatorWrapper> leakTable;

  // first collect all leaks
  for (auto it = s_pTrackerData->m_AllocatorData.GetIterator(); it.IsValid(); ++it)
  {
    const AllocatorData& data = it.Value();
    for (auto it2 = data.m_Allocations.GetIterator(); it2.IsValid(); ++it2)
    {
      LeakInfo leak;
      leak.m_AllocatorId = it.Id();
      leak.m_uiSize = it2.Value().m_uiSize;

      if (data.m_TrackingMode == nsAllocatorTrackingMode::AllocationStatsIgnoreLeaks)
      {
        leak.m_bIsRootLeak = false;
      }

      leakTable.Insert(it2.Key(), leak);
    }
  }

  // find dependencies
  for (auto it = leakTable.GetIterator(); it.IsValid(); ++it)
  {
    const void* ptr = it.Key();
    const LeakInfo& leak = it.Value();

    const void* curPtr = ptr;
    const void* endPtr = nsMemoryUtils::AddByteOffset(ptr, leak.m_uiSize);

    while (curPtr < endPtr)
    {
      const void* testPtr = *reinterpret_cast<const void* const*>(curPtr);

      LeakInfo* dependentLeak = nullptr;
      if (leakTable.TryGetValue(testPtr, dependentLeak))
      {
        dependentLeak->m_bIsRootLeak = false;
      }

      curPtr = nsMemoryUtils::AddByteOffset(curPtr, sizeof(void*));
    }
  }

  // dump leaks
  nsUInt32 uiNumLeaks = 0;

  for (auto it = leakTable.GetIterator(); it.IsValid(); ++it)
  {
    const void* ptr = it.Key();
    const LeakInfo& leak = it.Value();

    if (leak.m_bIsRootLeak)
    {
      const AllocatorData& data = s_pTrackerData->m_AllocatorData[leak.m_AllocatorId];

      if (data.m_TrackingMode != nsAllocatorTrackingMode::AllocationStatsIgnoreLeaks)
      {
        if (uiNumLeaks == 0)
        {
          printfunc("\n\n--------------------------------------------------------------------\n"
                    "Memory Leak Report:"
                    "\n--------------------------------------------------------------------\n\n");
        }

        nsMemoryTracker::AllocationInfo info;
        data.m_Allocations.TryGetValue(ptr, info);

        DumpLeak(info, data.m_sName.GetData());

        ++uiNumLeaks;
      }
    }
  }

  if (uiNumLeaks > 0)
  {
    char tmp[1024];
    nsStringUtils::snprintf(tmp, 1024, "\n--------------------------------------------------------------------\n"
                                       "Found %u root memory leak(s)."
                                       "\n--------------------------------------------------------------------\n\n",
      uiNumLeaks);

    printfunc(tmp);
  }

  return uiNumLeaks;
}

// static
void nsMemoryTracker::DumpMemoryLeaks()
{
  const nsUInt32 uiNumLeaks = PrintMemoryLeaks(nsLog::Print);

  if (uiNumLeaks > 0)
  {
    NS_REPORT_FAILURE("Found {0} root memory leak(s). See console output for details.", uiNumLeaks);
  }
}

// static
nsMemoryTracker::Iterator nsMemoryTracker::GetIterator()
{
  auto pInnerIt = NS_NEW(s_pTrackerDataAllocator, TrackerData::AllocatorTable::Iterator, s_pTrackerData->m_AllocatorData.GetIterator());
  return Iterator(pInnerIt);
}
