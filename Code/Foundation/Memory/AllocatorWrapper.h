#pragma once

struct nsNullAllocatorWrapper
{
  NS_FORCE_INLINE static nsAllocator* GetAllocator()
  {
    NS_REPORT_FAILURE("This method should never be called");
    return nullptr;
  }
};

struct nsDefaultAllocatorWrapper
{
  NS_ALWAYS_INLINE static nsAllocator* GetAllocator() { return nsFoundation::GetDefaultAllocator(); }
};

struct nsStaticsAllocatorWrapper
{
  NS_ALWAYS_INLINE static nsAllocator* GetAllocator() { return nsFoundation::GetStaticsAllocator(); }
};

struct nsAlignedAllocatorWrapper
{
  NS_ALWAYS_INLINE static nsAllocator* GetAllocator() { return nsFoundation::GetAlignedAllocator(); }
};

struct NS_FOUNDATION_DLL nsLocalAllocatorWrapper
{
  nsLocalAllocatorWrapper(nsAllocator* pAllocator);

  void Reset();

  static nsAllocator* GetAllocator();
};
