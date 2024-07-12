#include <Foundation/Memory/Policies/AllocPolicyGuarding.h>

nsAllocPolicyGuarding::nsAllocPolicyGuarding(nsAllocator* pParent)
{
  NS_ASSERT_NOT_IMPLEMENTED;
  NS_IGNORE_UNUSED(m_uiPageSize);
  NS_IGNORE_UNUSED(m_Mutex);
  NS_IGNORE_UNUSED(m_AllocationsToFreeLater);
}

void* nsAllocPolicyGuarding::Allocate(size_t uiSize, size_t uiAlign)
{
  NS_ASSERT_NOT_IMPLEMENTED;
  return nullptr;
}

void nsAllocPolicyGuarding::Deallocate(void* ptr)
{
  NS_ASSERT_NOT_IMPLEMENTED;
}
