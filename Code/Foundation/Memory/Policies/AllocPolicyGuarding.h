#pragma once

#include <Foundation/Containers/StaticRingBuffer.h>
#include <Foundation/Math/Math.h>
#include <Foundation/Threading/Lock.h>
#include <Foundation/Threading/Mutex.h>

class nsAllocPolicyGuarding
{
public:
  nsAllocPolicyGuarding(nsAllocator* pParent);
  NS_ALWAYS_INLINE ~nsAllocPolicyGuarding() = default;

  void* Allocate(size_t uiSize, size_t uiAlign);
  void Deallocate(void* pPtr);

  NS_ALWAYS_INLINE nsAllocator* GetParent() const { return nullptr; }

private:
  nsMutex m_Mutex;

  nsUInt32 m_uiPageSize;

  nsStaticRingBuffer<void*, (1 << 16)> m_AllocationsToFreeLater;
};
