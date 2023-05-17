#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/AllocatorWrapper.h>

static thread_local wdAllocatorBase* s_pAllocator = nullptr;

wdLocalAllocatorWrapper::wdLocalAllocatorWrapper(wdAllocatorBase* pAllocator)
{
  s_pAllocator = pAllocator;
}

void wdLocalAllocatorWrapper::Reset()
{
  s_pAllocator = nullptr;
}

wdAllocatorBase* wdLocalAllocatorWrapper::GetAllocator()
{
  return s_pAllocator;
}

WD_STATICLINK_FILE(Foundation, Foundation_Memory_Implementation_AllocatorWrapper);
