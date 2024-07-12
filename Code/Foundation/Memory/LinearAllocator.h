#pragma once

#include <Foundation/Containers/DynamicArray.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Memory/AllocatorWithPolicy.h>
#include <Foundation/Memory/Policies/AllocPolicyLinear.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

template <nsAllocatorTrackingMode TrackingMode = nsAllocatorTrackingMode::Default, bool OverwriteMemoryOnReset = false>
class nsLinearAllocator : public nsAllocatorWithPolicy<nsAllocPolicyLinear<OverwriteMemoryOnReset>, TrackingMode>
{
  using PolicyStack = nsAllocPolicyLinear<OverwriteMemoryOnReset>;

public:
  nsLinearAllocator(nsStringView sName, nsAllocator* pParent);
  ~nsLinearAllocator();

  virtual void* Allocate(size_t uiSize, size_t uiAlign, nsMemoryUtils::DestructorFunction destructorFunc) override;
  virtual void Deallocate(void* pPtr) override;

  /// \brief
  ///   Resets the allocator freeing all memory.
  void Reset();

private:
  struct DestructData
  {
    NS_DECLARE_POD_TYPE();

    nsMemoryUtils::DestructorFunction m_Func;
    void* m_Ptr;
  };

  nsMutex m_Mutex;
  nsDynamicArray<DestructData> m_DestructData;
  nsHashTable<void*, nsUInt32> m_PtrToDestructDataIndexTable;
};

#include <Foundation/Memory/Implementation/LinearAllocator_inl.h>
