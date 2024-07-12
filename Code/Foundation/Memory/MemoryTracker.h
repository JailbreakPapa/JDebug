#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Types/Bitflags.h>

enum class nsAllocatorTrackingMode : nsUInt32
{
  Nothing,                       ///< The allocator doesn't track anything. Use this for best performance.
  Basics,                        ///< The allocator will be known to the system, so it can show up in debugging tools, but barely anything more.
  AllocationStats,               ///< The allocator keeps track of how many allocations and deallocations it did and how large its memory usage is.
  AllocationStatsIgnoreLeaks,    ///< Same as AllocationStats, but any remaining allocations at shutdown are not reported as leaks.
  AllocationStatsAndStacktraces, ///< The allocator will record stack traces for each allocation, which can be used to find memory leaks.

  Default = NS_ALLOC_TRACKING_DEFAULT,
};

/// \brief Memory tracker which keeps track of all allocations and constructions
class NS_FOUNDATION_DLL nsMemoryTracker
{
public:
  struct AllocationInfo
  {
    NS_DECLARE_POD_TYPE();

    NS_FORCE_INLINE AllocationInfo()

      = default;

    void** m_pStackTrace = nullptr;
    size_t m_uiSize = 0;
    nsUInt16 m_uiAlignment = 0;
    nsUInt16 m_uiStackTraceLength = 0;

    NS_ALWAYS_INLINE const nsArrayPtr<void*> GetStackTrace() const { return nsArrayPtr<void*>(m_pStackTrace, (nsUInt32)m_uiStackTraceLength); }

    NS_ALWAYS_INLINE nsArrayPtr<void*> GetStackTrace() { return nsArrayPtr<void*>(m_pStackTrace, (nsUInt32)m_uiStackTraceLength); }

    NS_FORCE_INLINE void SetStackTrace(nsArrayPtr<void*> stackTrace)
    {
      m_pStackTrace = stackTrace.GetPtr();
      NS_ASSERT_DEV(stackTrace.GetCount() < 0xFFFF, "stack trace too long");
      m_uiStackTraceLength = (nsUInt16)stackTrace.GetCount();
    }
  };

  class NS_FOUNDATION_DLL Iterator
  {
  public:
    ~Iterator();

    nsAllocatorId Id() const;
    nsStringView Name() const;
    nsAllocatorId ParentId() const;
    const nsAllocator::Stats& Stats() const;

    void Next();
    bool IsValid() const;

    NS_ALWAYS_INLINE void operator++() { Next(); }

  private:
    friend class nsMemoryTracker;

    NS_ALWAYS_INLINE Iterator(void* pData)
      : m_pData(pData)
    {
    }

    void* m_pData;
  };

  static nsAllocatorId RegisterAllocator(nsStringView sName, nsAllocatorTrackingMode mode, nsAllocatorId parentId);
  static void DeregisterAllocator(nsAllocatorId allocatorId);

  static void AddAllocation(nsAllocatorId allocatorId, nsAllocatorTrackingMode mode, const void* pPtr, size_t uiSize, size_t uiAlign, nsTime allocationTime);
  static void RemoveAllocation(nsAllocatorId allocatorId, const void* pPtr);
  static void RemoveAllAllocations(nsAllocatorId allocatorId);
  static void SetAllocatorStats(nsAllocatorId allocatorId, const nsAllocator::Stats& stats);

  static void ResetPerFrameAllocatorStats();

  static nsStringView GetAllocatorName(nsAllocatorId allocatorId);
  static const nsAllocator::Stats& GetAllocatorStats(nsAllocatorId allocatorId);
  static nsAllocatorId GetAllocatorParentId(nsAllocatorId allocatorId);
  static const AllocationInfo& GetAllocationInfo(nsAllocatorId allocatorId, const void* pPtr);

  static Iterator GetIterator();

  /// \brief Callback for printing strings.
  using PrintFunc = void (*)(const char* szLine);

  /// \brief Reports back information about all currently known root memory leaks.
  ///
  /// Returns the number of found memory leaks.
  static nsUInt32 PrintMemoryLeaks(PrintFunc printfunc);

  /// \brief Prints the known memory leaks to nsLog and triggers an assert if there are any.
  ///
  /// This is useful to call at the end of an application, to get a debug breakpoint in case of memory leaks.
  static void DumpMemoryLeaks();
};
