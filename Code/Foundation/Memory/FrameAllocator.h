#pragma once

#include <Foundation/Memory/LinearAllocator.h>

/// \brief A double buffered stack allocator
class NS_FOUNDATION_DLL nsDoubleBufferedLinearAllocator
{
public:
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  static constexpr bool OverwriteMemoryOnReset = true;
#else
  static constexpr bool OverwriteMemoryOnReset = false;
#endif
  using StackAllocatorType = nsLinearAllocator<nsAllocatorTrackingMode::Basics, OverwriteMemoryOnReset>;

  nsDoubleBufferedLinearAllocator(nsStringView sName, nsAllocator* pParent);
  ~nsDoubleBufferedLinearAllocator();

  NS_ALWAYS_INLINE nsAllocator* GetCurrentAllocator() const { return m_pCurrentAllocator; }

  void Swap();
  void Reset();

private:
  StackAllocatorType* m_pCurrentAllocator;
  StackAllocatorType* m_pOtherAllocator;
};

class NS_FOUNDATION_DLL nsFrameAllocator
{
public:
  NS_ALWAYS_INLINE static nsAllocator* GetCurrentAllocator() { return s_pAllocator->GetCurrentAllocator(); }

  static void Swap();
  static void Reset();

private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, FrameAllocator);

  static void Startup();
  static void Shutdown();

  static nsDoubleBufferedLinearAllocator* s_pAllocator;
};
