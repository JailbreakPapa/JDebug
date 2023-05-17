#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Containers/IdTable.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Memory/Allocator.h>
#include <Foundation/Memory/Policies/HeapAllocation.h>
#include <Foundation/Strings/String.h>
#include <Foundation/System/StackTracer.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#endif

namespace
{
  // no tracking for the tracker data itself
  typedef wdAllocator<wdMemoryPolicies::wdHeapAllocation, 0> TrackerDataAllocator;

  static TrackerDataAllocator* s_pTrackerDataAllocator;

  struct TrackerDataAllocatorWrapper
  {
    WD_ALWAYS_INLINE static wdAllocatorBase* GetAllocator() { return s_pTrackerDataAllocator; }
  };


  struct AllocatorData
  {
    WD_ALWAYS_INLINE AllocatorData() {}

    wdHybridString<32, TrackerDataAllocatorWrapper> m_sName;
    wdBitflags<wdMemoryTrackingFlags> m_Flags;

    wdAllocatorId m_ParentId;

    wdAllocatorBase::Stats m_Stats;

    wdHashTable<const void*, wdMemoryTracker::AllocationInfo, wdHashHelper<const void*>, TrackerDataAllocatorWrapper> m_Allocations;
  };

  struct TrackerData
  {
    WD_ALWAYS_INLINE void Lock() { m_Mutex.Lock(); }
    WD_ALWAYS_INLINE void Unlock() { m_Mutex.Unlock(); }

    wdMutex m_Mutex;

    typedef wdIdTable<wdAllocatorId, AllocatorData, TrackerDataAllocatorWrapper> AllocatorTable;
    AllocatorTable m_AllocatorData;

    wdAllocatorId m_StaticAllocatorId;
  };

  static TrackerData* s_pTrackerData;
  static bool s_bIsInitialized = false;
  static bool s_bIsInitializing = false;

  static void Initialize()
  {
    if (s_bIsInitialized)
      return;

    WD_ASSERT_DEV(!s_bIsInitializing, "MemoryTracker initialization entered recursively");
    s_bIsInitializing = true;

    if (s_pTrackerDataAllocator == nullptr)
    {
      alignas(WD_ALIGNMENT_OF(TrackerDataAllocator)) static wdUInt8 TrackerDataAllocatorBuffer[sizeof(TrackerDataAllocator)];
      s_pTrackerDataAllocator = new (TrackerDataAllocatorBuffer) TrackerDataAllocator("MemoryTracker");
      WD_ASSERT_DEV(s_pTrackerDataAllocator != nullptr, "MemoryTracker initialization failed");
    }

    if (s_pTrackerData == nullptr)
    {
      alignas(WD_ALIGNMENT_OF(TrackerData)) static wdUInt8 TrackerDataBuffer[sizeof(TrackerData)];
      s_pTrackerData = new (TrackerDataBuffer) TrackerData();
      WD_ASSERT_DEV(s_pTrackerData != nullptr, "MemoryTracker initialization failed");
    }

    s_bIsInitialized = true;
    s_bIsInitializing = false;
  }

  static void DumpLeak(const wdMemoryTracker::AllocationInfo& info, const char* szAllocatorName)
  {
    char szBuffer[512];
    wdUInt64 uiSize = info.m_uiSize;
    wdStringUtils::snprintf(szBuffer, WD_ARRAY_SIZE(szBuffer), "Leaked %llu bytes allocated by '%s'\n", uiSize, szAllocatorName);

    wdLog::Print(szBuffer);

    if (info.GetStackTrace().GetPtr() != nullptr)
    {
      wdStackTracer::ResolveStackTrace(info.GetStackTrace(), &wdLog::Print);
    }

    wdLog::Print("--------------------------------------------------------------------\n\n");
  }
} // namespace

// Iterator
#define CAST_ITER(ptr) static_cast<TrackerData::AllocatorTable::Iterator*>(ptr)

wdAllocatorId wdMemoryTracker::Iterator::Id() const
{
  return CAST_ITER(m_pData)->Id();
}

const char* wdMemoryTracker::Iterator::Name() const
{
  return CAST_ITER(m_pData)->Value().m_sName.GetData();
}

wdAllocatorId wdMemoryTracker::Iterator::ParentId() const
{
  return CAST_ITER(m_pData)->Value().m_ParentId;
}

const wdAllocatorBase::Stats& wdMemoryTracker::Iterator::Stats() const
{
  return CAST_ITER(m_pData)->Value().m_Stats;
}

void wdMemoryTracker::Iterator::Next()
{
  CAST_ITER(m_pData)->Next();
}

bool wdMemoryTracker::Iterator::IsValid() const
{
  return CAST_ITER(m_pData)->IsValid();
}

wdMemoryTracker::Iterator::~Iterator()
{
  auto it = CAST_ITER(m_pData);
  WD_DELETE(s_pTrackerDataAllocator, it);
  m_pData = nullptr;
}


// static
wdAllocatorId wdMemoryTracker::RegisterAllocator(const char* szName, wdBitflags<wdMemoryTrackingFlags> flags, wdAllocatorId parentId)
{
  Initialize();

  WD_LOCK(*s_pTrackerData);

  AllocatorData data;
  data.m_sName = szName;
  data.m_Flags = flags;
  data.m_ParentId = parentId;

  wdAllocatorId id = s_pTrackerData->m_AllocatorData.Insert(data);

  if (data.m_sName == WD_STATIC_ALLOCATOR_NAME)
  {
    s_pTrackerData->m_StaticAllocatorId = id;
  }

  return id;
}

// static
void wdMemoryTracker::DeregisterAllocator(wdAllocatorId allocatorId)
{
  WD_LOCK(*s_pTrackerData);

  const AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];

  wdUInt32 uiLiveAllocations = data.m_Allocations.GetCount();
  if (uiLiveAllocations != 0)
  {
    for (auto it = data.m_Allocations.GetIterator(); it.IsValid(); ++it)
    {
      DumpLeak(it.Value(), data.m_sName.GetData());
    }

    WD_REPORT_FAILURE("Allocator '{0}' leaked {1} allocation(s)", data.m_sName.GetData(), uiLiveAllocations);
  }

  s_pTrackerData->m_AllocatorData.Remove(allocatorId);
}

// static
void wdMemoryTracker::AddAllocation(wdAllocatorId allocatorId, wdBitflags<wdMemoryTrackingFlags> flags, const void* pPtr, size_t uiSize, size_t uiAlign, wdTime allocationTime)
{
  WD_ASSERT_DEV((flags & wdMemoryTrackingFlags::EnableAllocationTracking) != 0, "Allocation tracking is turned off, but wdMemoryTracker::AddAllocation() is called anyway.");

  WD_ASSERT_DEV(uiAlign < 0xFFFF, "Alignment too big");

  wdArrayPtr<void*> stackTrace;
  if (flags.IsSet(wdMemoryTrackingFlags::EnableStackTrace))
  {
    void* pBuffer[64];
    wdArrayPtr<void*> tempTrace(pBuffer);
    const wdUInt32 uiNumTraces = wdStackTracer::GetStackTrace(tempTrace);

    stackTrace = WD_NEW_ARRAY(s_pTrackerDataAllocator, void*, uiNumTraces);
    wdMemoryUtils::Copy(stackTrace.GetPtr(), pBuffer, uiNumTraces);
  }

  {
    WD_LOCK(*s_pTrackerData);

    AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];
    data.m_Stats.m_uiNumAllocations++;
    data.m_Stats.m_uiAllocationSize += uiSize;
    data.m_Stats.m_uiPerFrameAllocationSize += uiSize;
    data.m_Stats.m_PerFrameAllocationTime += allocationTime;

    WD_ASSERT_DEBUG(data.m_Flags == flags, "Given flags have to be identical to allocator flags");
    auto pInfo = &data.m_Allocations[pPtr];
    pInfo->m_uiSize = uiSize;
    pInfo->m_uiAlignment = (wdUInt16)uiAlign;
    pInfo->SetStackTrace(stackTrace);
  }
}

// static
void wdMemoryTracker::RemoveAllocation(wdAllocatorId allocatorId, const void* pPtr)
{
  wdArrayPtr<void*> stackTrace;

  {
    WD_LOCK(*s_pTrackerData);

    AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];

    AllocationInfo info;
    if (data.m_Allocations.Remove(pPtr, &info))
    {
      data.m_Stats.m_uiNumDeallocations++;
      data.m_Stats.m_uiAllocationSize -= info.m_uiSize;

      stackTrace = info.GetStackTrace();
    }
    else
    {
      WD_REPORT_FAILURE("Invalid Allocation '{0}'. Memory corruption?", wdArgP(pPtr));
    }
  }

  WD_DELETE_ARRAY(s_pTrackerDataAllocator, stackTrace);
}

// static
void wdMemoryTracker::RemoveAllAllocations(wdAllocatorId allocatorId)
{
  WD_LOCK(*s_pTrackerData);
  AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];
  for (auto it = data.m_Allocations.GetIterator(); it.IsValid(); ++it)
  {
    auto& info = it.Value();
    data.m_Stats.m_uiNumDeallocations++;
    data.m_Stats.m_uiAllocationSize -= info.m_uiSize;

    WD_DELETE_ARRAY(s_pTrackerDataAllocator, info.GetStackTrace());
  }
  data.m_Allocations.Clear();
}

// static
void wdMemoryTracker::SetAllocatorStats(wdAllocatorId allocatorId, const wdAllocatorBase::Stats& stats)
{
  WD_LOCK(*s_pTrackerData);

  s_pTrackerData->m_AllocatorData[allocatorId].m_Stats = stats;
}

// static
void wdMemoryTracker::ResetPerFrameAllocatorStats()
{
  WD_LOCK(*s_pTrackerData);

  for (auto it = s_pTrackerData->m_AllocatorData.GetIterator(); it.IsValid(); ++it)
  {
    AllocatorData& data = it.Value();
    data.m_Stats.m_uiPerFrameAllocationSize = 0;
    data.m_Stats.m_PerFrameAllocationTime.SetZero();
  }
}

// static
const char* wdMemoryTracker::GetAllocatorName(wdAllocatorId allocatorId)
{
  WD_LOCK(*s_pTrackerData);

  return s_pTrackerData->m_AllocatorData[allocatorId].m_sName.GetData();
}

// static
const wdAllocatorBase::Stats& wdMemoryTracker::GetAllocatorStats(wdAllocatorId allocatorId)
{
  WD_LOCK(*s_pTrackerData);

  return s_pTrackerData->m_AllocatorData[allocatorId].m_Stats;
}

// static
wdAllocatorId wdMemoryTracker::GetAllocatorParentId(wdAllocatorId allocatorId)
{
  WD_LOCK(*s_pTrackerData);

  return s_pTrackerData->m_AllocatorData[allocatorId].m_ParentId;
}

// static
const wdMemoryTracker::AllocationInfo& wdMemoryTracker::GetAllocationInfo(wdAllocatorId allocatorId, const void* pPtr)
{
  WD_LOCK(*s_pTrackerData);

  const AllocatorData& data = s_pTrackerData->m_AllocatorData[allocatorId];
  const AllocationInfo* info = nullptr;
  if (data.m_Allocations.TryGetValue(pPtr, info))
  {
    return *info;
  }

  static AllocationInfo invalidInfo;

  WD_REPORT_FAILURE("Could not find info for allocation {0}", wdArgP(pPtr));
  return invalidInfo;
}


struct LeakInfo
{
  WD_DECLARE_POD_TYPE();

  wdAllocatorId m_AllocatorId;
  size_t m_uiSize = 0;
  const void* m_pParentLeak = nullptr;

  WD_ALWAYS_INLINE bool IsRootLeak() const { return m_pParentLeak == nullptr && m_AllocatorId != s_pTrackerData->m_StaticAllocatorId; }
};

// static
void wdMemoryTracker::DumpMemoryLeaks()
{
  if (s_pTrackerData == nullptr) // if both tracking and tracing is disabled there is no tracker data
    return;
  WD_LOCK(*s_pTrackerData);

  static wdHashTable<const void*, LeakInfo, wdHashHelper<const void*>, TrackerDataAllocatorWrapper> leakTable;
  leakTable.Clear();

  // first collect all leaks
  for (auto it = s_pTrackerData->m_AllocatorData.GetIterator(); it.IsValid(); ++it)
  {
    const AllocatorData& data = it.Value();
    for (auto it2 = data.m_Allocations.GetIterator(); it2.IsValid(); ++it2)
    {
      LeakInfo leak;
      leak.m_AllocatorId = it.Id();
      leak.m_uiSize = it2.Value().m_uiSize;
      leak.m_pParentLeak = nullptr;

      leakTable.Insert(it2.Key(), leak);
    }
  }

  // find dependencies
  for (auto it = leakTable.GetIterator(); it.IsValid(); ++it)
  {
    const void* ptr = it.Key();
    const LeakInfo& leak = it.Value();

    const void* curPtr = ptr;
    const void* endPtr = wdMemoryUtils::AddByteOffset(ptr, leak.m_uiSize);

    while (curPtr < endPtr)
    {
      const void* testPtr = *reinterpret_cast<const void* const*>(curPtr);

      LeakInfo* dependentLeak = nullptr;
      if (leakTable.TryGetValue(testPtr, dependentLeak))
      {
        dependentLeak->m_pParentLeak = ptr;
      }

      curPtr = wdMemoryUtils::AddByteOffset(curPtr, sizeof(void*));
    }
  }

  // dump leaks
  wdUInt64 uiNumLeaks = 0;

  for (auto it = leakTable.GetIterator(); it.IsValid(); ++it)
  {
    const void* ptr = it.Key();
    const LeakInfo& leak = it.Value();

    if (leak.IsRootLeak())
    {
      if (uiNumLeaks == 0)
      {
        wdLog::Print("\n\n--------------------------------------------------------------------\n"
                     "Memory Leak Report:"
                     "\n--------------------------------------------------------------------\n\n");
      }

      const AllocatorData& data = s_pTrackerData->m_AllocatorData[leak.m_AllocatorId];
      wdMemoryTracker::AllocationInfo info;
      data.m_Allocations.TryGetValue(ptr, info);

      DumpLeak(info, data.m_sName.GetData());

      ++uiNumLeaks;
    }
  }

  if (uiNumLeaks > 0)
  {
    wdLog::Printf("\n--------------------------------------------------------------------\n"
                  "Found %llu root memory leak(s)."
                  "\n--------------------------------------------------------------------\n\n",
      uiNumLeaks);

    WD_REPORT_FAILURE("Found {0} root memory leak(s).", uiNumLeaks);
  }
}

// static
wdMemoryTracker::Iterator wdMemoryTracker::GetIterator()
{
  auto pInnerIt = WD_NEW(s_pTrackerDataAllocator, TrackerData::AllocatorTable::Iterator, s_pTrackerData->m_AllocatorData.GetIterator());
  return Iterator(pInnerIt);
}


WD_STATICLINK_FILE(Foundation, Foundation_Memory_Implementation_MemoryTracker);
