#pragma once

#include <Foundation/Basics.h>

/// \brief This helper class can reserve and allocate whole memory pages.
class WD_FOUNDATION_DLL wdPageAllocator
{
public:
  static void* AllocatePage(size_t uiSize);
  static void DeallocatePage(void* pPtr);

  static wdAllocatorId GetId();
};
