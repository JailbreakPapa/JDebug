#pragma once

#include <Foundation/Basics.h>

/// \brief This helper class can reserve and allocate whole memory pages.
class NS_FOUNDATION_DLL nsPageAllocator
{
public:
  static void* AllocatePage(size_t uiSize);
  static void DeallocatePage(void* pPtr);

  static nsAllocatorId GetId();
};
