#pragma once

#include <Foundation/Memory/StackAllocator.h>

/// \brief A double buffered stack allocator
class WD_FOUNDATION_DLL wdDoubleBufferedStackAllocator
{
public:
  typedef wdStackAllocator<wdMemoryTrackingFlags::RegisterAllocator> StackAllocatorType;

  wdDoubleBufferedStackAllocator(const char* szName, wdAllocatorBase* pParent);
  ~wdDoubleBufferedStackAllocator();

  WD_ALWAYS_INLINE wdAllocatorBase* GetCurrentAllocator() const { return m_pCurrentAllocator; }

  void Swap();
  void Reset();

private:
  StackAllocatorType* m_pCurrentAllocator;
  StackAllocatorType* m_pOtherAllocator;
};

class WD_FOUNDATION_DLL wdFrameAllocator
{
public:
  WD_ALWAYS_INLINE static wdAllocatorBase* GetCurrentAllocator() { return s_pAllocator->GetCurrentAllocator(); }

  static void Swap();
  static void Reset();

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, FrameAllocator);

  static void Startup();
  static void Shutdown();

  static wdDoubleBufferedStackAllocator* s_pAllocator;
};
