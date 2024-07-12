#pragma once

#include <Foundation/Basics.h>

/// \brief This Allocation policy redirects all operations to its parent.
///
/// \note Note that the stats are taken on the proxy as well as on the parent.
///
/// \see nsAllocatorWithPolicy
class nsAllocPolicyProxy
{
public:
  NS_FORCE_INLINE nsAllocPolicyProxy(nsAllocator* pParent)
    : m_pParent(pParent)
  {
    NS_ASSERT_ALWAYS(m_pParent != nullptr, "Parent allocator must not be nullptr");
  }

  NS_FORCE_INLINE void* Allocate(size_t uiSize, size_t uiAlign) { return m_pParent->Allocate(uiSize, uiAlign); }

  NS_FORCE_INLINE void* Reallocate(void* pPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign)
  {
    return m_pParent->Reallocate(pPtr, uiCurrentSize, uiNewSize, uiAlign);
  }

  NS_FORCE_INLINE void Deallocate(void* pPtr) { m_pParent->Deallocate(pPtr); }

  NS_FORCE_INLINE size_t AllocatedSize(const void* pPtr) { return m_pParent->AllocatedSize(pPtr); }

  NS_ALWAYS_INLINE nsAllocator* GetParent() const { return m_pParent; }

private:
  nsAllocator* m_pParent;
};
