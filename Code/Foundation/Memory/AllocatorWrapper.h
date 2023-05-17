#pragma once

struct wdNullAllocatorWrapper
{
  WD_FORCE_INLINE static wdAllocatorBase* GetAllocator()
  {
    WD_REPORT_FAILURE("This method should never be called");
    return nullptr;
  }
};

struct wdDefaultAllocatorWrapper
{
  WD_ALWAYS_INLINE static wdAllocatorBase* GetAllocator() { return wdFoundation::GetDefaultAllocator(); }
};

struct wdStaticAllocatorWrapper
{
  WD_ALWAYS_INLINE static wdAllocatorBase* GetAllocator() { return wdFoundation::GetStaticAllocator(); }
};

struct wdAlignedAllocatorWrapper
{
  WD_ALWAYS_INLINE static wdAllocatorBase* GetAllocator() { return wdFoundation::GetAlignedAllocator(); }
};

struct WD_FOUNDATION_DLL wdLocalAllocatorWrapper
{
  wdLocalAllocatorWrapper(wdAllocatorBase* pAllocator);

  void Reset();

  static wdAllocatorBase* GetAllocator();
};
