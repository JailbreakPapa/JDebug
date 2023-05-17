#pragma once

#include <Foundation/Basics.h>

namespace wdMemoryPolicies
{
  /// \brief This Allocation policy redirects all operations to its parent.
  ///
  /// \note Note that the stats are taken on the proxy as well as on the parent.
  ///
  /// \see wdAllocator
  class wdProxyAllocation
  {
  public:
    WD_FORCE_INLINE wdProxyAllocation(wdAllocatorBase* pParent)
      : m_pParent(pParent)
    {
      WD_ASSERT_ALWAYS(m_pParent != nullptr, "Parent allocator must not be nullptr");
    }

    WD_FORCE_INLINE void* Allocate(size_t uiSize, size_t uiAlign) { return m_pParent->Allocate(uiSize, uiAlign); }

    WD_FORCE_INLINE void* Reallocate(void* pPtr, size_t uiCurrentSize, size_t uiNewSize, size_t uiAlign)
    {
      return m_pParent->Reallocate(pPtr, uiCurrentSize, uiNewSize, uiAlign);
    }

    WD_FORCE_INLINE void Deallocate(void* pPtr) { m_pParent->Deallocate(pPtr); }

    WD_FORCE_INLINE size_t AllocatedSize(const void* pPtr) { return m_pParent->AllocatedSize(pPtr); }

    WD_ALWAYS_INLINE wdAllocatorBase* GetParent() const { return m_pParent; }

  private:
    wdAllocatorBase* m_pParent;
  };
} // namespace wdMemoryPolicies
