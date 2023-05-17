
WD_FORCE_INLINE void* wdAlignedHeapAllocation::Allocate(size_t uiSize, size_t uiAlign)
{
  // alignment has to be at least sizeof(void*) otherwise posix_memalign will fail
  uiAlign = wdMath::Max<size_t>(uiAlign, 16u);

  void* ptr = nullptr;

  int res = posix_memalign(&ptr, uiAlign, uiSize);
  WD_IGNORE_UNUSED(res);
  WD_ASSERT_DEV(res == 0, "posix_memalign failed with error: {0}", res);

  WD_CHECK_ALIGNMENT(ptr, uiAlign);

  return ptr;
}

WD_ALWAYS_INLINE void wdAlignedHeapAllocation::Deallocate(void* ptr)
{
  free(ptr);
}
