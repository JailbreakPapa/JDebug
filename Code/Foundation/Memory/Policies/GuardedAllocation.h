#pragma once

#include <Foundation/Containers/StaticRingBuffer.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

namespace wdMemoryPolicies
{
  class wdGuardedAllocation
  {
  public:
    wdGuardedAllocation(wdAllocatorBase* pParent);
    WD_ALWAYS_INLINE ~wdGuardedAllocation() {}

    void* Allocate(size_t uiSize, size_t uiAlign);
    void Deallocate(void* pPtr);

    WD_ALWAYS_INLINE wdAllocatorBase* GetParent() const { return nullptr; }

  private:
    wdMutex m_Mutex;

    wdUInt32 m_uiPageSize;

    wdStaticRingBuffer<void*, (1 << 16)> m_AllocationsToFreeLater;
  };
} // namespace wdMemoryPolicies
