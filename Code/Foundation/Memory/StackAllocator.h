#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Memory/Allocator.h>
#include <Foundation/Memory/Policies/StackAllocation.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

template <wdUInt32 TrackingFlags = wdMemoryTrackingFlags::Default>
class wdStackAllocator : public wdAllocator<wdMemoryPolicies::wdStackAllocation, TrackingFlags>
{
public:
  wdStackAllocator(const char* szName, wdAllocatorBase* pParent);
  ~wdStackAllocator();

  virtual void* Allocate(size_t uiSize, size_t uiAlign, wdMemoryUtils::DestructorFunction destructorFunc) override;
  virtual void Deallocate(void* pPtr) override;

  /// \brief
  ///   Resets the allocator freeing all memory.
  void Reset();

private:
  struct DestructData
  {
    WD_DECLARE_POD_TYPE();

    wdMemoryUtils::DestructorFunction m_Func;
    void* m_Ptr;
  };

  wdMutex m_Mutex;
  wdDynamicArray<DestructData> m_DestructData;
  wdHashTable<void*, wdUInt32> m_PtrToDestructDataIndexTable;
};

#include <Foundation/Memory/Implementation/StackAllocator_inl.h>
