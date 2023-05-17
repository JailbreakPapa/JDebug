#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Bitflags.h>

struct wdMemoryTrackingFlags
{
  typedef wdUInt32 StorageType;

  enum Enum
  {
    None,
    RegisterAllocator = WD_BIT(0),        ///< Register the allocator with the memory tracker. If EnableAllocationTracking is not set as well it is up to the
                                          ///< allocator implementation whether it collects usable stats or not.
    EnableAllocationTracking = WD_BIT(1), ///< Enable tracking of individual allocations
    EnableStackTrace = WD_BIT(2),         ///< Enable stack traces for each allocation

    All = RegisterAllocator | EnableAllocationTracking | EnableStackTrace,

    Default = 0
#if WD_ENABLED(WD_USE_ALLOCATION_TRACKING)
              | RegisterAllocator | EnableAllocationTracking
#endif
#if WD_ENABLED(WD_USE_ALLOCATION_STACK_TRACING)
              | EnableStackTrace
#endif
  };

  struct Bits
  {
    StorageType RegisterAllocator : 1;
    StorageType EnableAllocationTracking : 1;
    StorageType EnableStackTrace : 1;
  };
};

// WD_DECLARE_FLAGS_OPERATORS(wdMemoryTrackingFlags);

#define WD_STATIC_ALLOCATOR_NAME "Statics"

/// \brief Memory tracker which keeps track of all allocations and constructions
class WD_FOUNDATION_DLL wdMemoryTracker
{
public:
  struct AllocationInfo
  {
    WD_DECLARE_POD_TYPE();

    WD_FORCE_INLINE AllocationInfo()
      : m_pStackTrace(nullptr)
      , m_uiSize(0)
      , m_uiAlignment(0)
      , m_uiStackTraceLength(0)
    {
    }

    void** m_pStackTrace;
    size_t m_uiSize;
    wdUInt16 m_uiAlignment;
    wdUInt16 m_uiStackTraceLength;

    WD_ALWAYS_INLINE const wdArrayPtr<void*> GetStackTrace() const { return wdArrayPtr<void*>(m_pStackTrace, (wdUInt32)m_uiStackTraceLength); }

    WD_ALWAYS_INLINE wdArrayPtr<void*> GetStackTrace() { return wdArrayPtr<void*>(m_pStackTrace, (wdUInt32)m_uiStackTraceLength); }

    WD_FORCE_INLINE void SetStackTrace(wdArrayPtr<void*> stackTrace)
    {
      m_pStackTrace = stackTrace.GetPtr();
      WD_ASSERT_DEV(stackTrace.GetCount() < 0xFFFF, "stack trace too long");
      m_uiStackTraceLength = (wdUInt16)stackTrace.GetCount();
    }
  };

  class WD_FOUNDATION_DLL Iterator
  {
  public:
    ~Iterator();

    wdAllocatorId Id() const;
    const char* Name() const;
    wdAllocatorId ParentId() const;
    const wdAllocatorBase::Stats& Stats() const;

    void Next();
    bool IsValid() const;

    WD_ALWAYS_INLINE void operator++() { Next(); }

  private:
    friend class wdMemoryTracker;

    WD_ALWAYS_INLINE Iterator(void* pData)
      : m_pData(pData)
    {
    }

    void* m_pData;
  };

  static wdAllocatorId RegisterAllocator(const char* szName, wdBitflags<wdMemoryTrackingFlags> flags, wdAllocatorId parentId);
  static void DeregisterAllocator(wdAllocatorId allocatorId);

  static void AddAllocation(
    wdAllocatorId allocatorId, wdBitflags<wdMemoryTrackingFlags> flags, const void* pPtr, size_t uiSize, size_t uiAlign, wdTime allocationTime);
  static void RemoveAllocation(wdAllocatorId allocatorId, const void* pPtr);
  static void RemoveAllAllocations(wdAllocatorId allocatorId);
  static void SetAllocatorStats(wdAllocatorId allocatorId, const wdAllocatorBase::Stats& stats);

  static void ResetPerFrameAllocatorStats();

  static const char* GetAllocatorName(wdAllocatorId allocatorId);
  static const wdAllocatorBase::Stats& GetAllocatorStats(wdAllocatorId allocatorId);
  static wdAllocatorId GetAllocatorParentId(wdAllocatorId allocatorId);
  static const AllocationInfo& GetAllocationInfo(wdAllocatorId allocatorId, const void* pPtr);

  static void DumpMemoryLeaks();

  static Iterator GetIterator();
};
