#include <Foundation/FoundationPCH.h>

#include <Foundation/Memory/AllocatorWrapper.h>

static thread_local nsAllocator* s_pAllocator = nullptr;

nsLocalAllocatorWrapper::nsLocalAllocatorWrapper(nsAllocator* pAllocator)
{
  s_pAllocator = pAllocator;
}

void nsLocalAllocatorWrapper::Reset()
{
  s_pAllocator = nullptr;
}

nsAllocator* nsLocalAllocatorWrapper::GetAllocator()
{
  return s_pAllocator;
}
