
namespace wdMemoryPolicies
{
  wdGuardedAllocation::wdGuardedAllocation(wdAllocatorBase* pParent) { WD_ASSERT_NOT_IMPLEMENTED; }

  void* wdGuardedAllocation::Allocate(size_t uiSize, size_t uiAlign)
  {
    WD_ASSERT_NOT_IMPLEMENTED;
    return nullptr;
  }

  void wdGuardedAllocation::Deallocate(void* ptr) { WD_ASSERT_NOT_IMPLEMENTED; }
} // namespace wdMemoryPolicies
