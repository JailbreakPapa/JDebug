
WD_FORCE_INLINE void* wdAlignedHeapAllocation::Allocate(size_t uiSize, size_t uiAlign)
{
  uiAlign = wdMath::Max<size_t>(uiAlign, 16u);

  void* ptr = _aligned_malloc(uiSize, uiAlign);
  WD_CHECK_ALIGNMENT(ptr, uiAlign);

  return ptr;
}

WD_ALWAYS_INLINE void wdAlignedHeapAllocation::Deallocate(void* pPtr)
{
  _aligned_free(pPtr);
}
