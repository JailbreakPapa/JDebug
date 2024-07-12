
NS_FORCE_INLINE void* nsAllocPolicyAlignedHeap::Allocate(size_t uiSize, size_t uiAlign)
{
  uiAlign = nsMath::Max<size_t>(uiAlign, 16u);

  void* ptr = _aligned_malloc(uiSize, uiAlign);
  NS_CHECK_ALIGNMENT(ptr, uiAlign);

  return ptr;
}

NS_ALWAYS_INLINE void nsAllocPolicyAlignedHeap::Deallocate(void* pPtr)
{
  _aligned_free(pPtr);
}
