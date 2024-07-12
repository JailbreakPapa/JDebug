
NS_FORCE_INLINE void* nsAllocPolicyAlignedHeap::Allocate(size_t uiSize, size_t uiAlign)
{
  // alignment has to be at least sizeof(void*) otherwise posix_memalign will fail
  uiAlign = nsMath::Max<size_t>(uiAlign, 16u);

  void* ptr = nullptr;

  int res = posix_memalign(&ptr, uiAlign, uiSize);
  NS_IGNORE_UNUSED(res);
  NS_ASSERT_DEV(res == 0, "posix_memalign failed with error: {0}", res);

  NS_CHECK_ALIGNMENT(ptr, uiAlign);

  return ptr;
}

NS_ALWAYS_INLINE void nsAllocPolicyAlignedHeap::Deallocate(void* ptr)
{
  free(ptr);
}
