#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Memory/FrameAllocator.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Strings/StringBuilder.h>

wdDoubleBufferedStackAllocator::wdDoubleBufferedStackAllocator(const char* szName, wdAllocatorBase* pParent)
{
  wdStringBuilder sName = szName;
  sName.Append("0");

  m_pCurrentAllocator = WD_DEFAULT_NEW(StackAllocatorType, sName, pParent);

  sName = szName;
  sName.Append("1");

  m_pOtherAllocator = WD_DEFAULT_NEW(StackAllocatorType, sName, pParent);
}

wdDoubleBufferedStackAllocator::~wdDoubleBufferedStackAllocator()
{
  WD_DEFAULT_DELETE(m_pCurrentAllocator);
  WD_DEFAULT_DELETE(m_pOtherAllocator);
}

void wdDoubleBufferedStackAllocator::Swap()
{
  wdMath::Swap(m_pCurrentAllocator, m_pOtherAllocator);

  m_pCurrentAllocator->Reset();
}

void wdDoubleBufferedStackAllocator::Reset()
{
  m_pCurrentAllocator->Reset();
  m_pOtherAllocator->Reset();
}


// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, FrameAllocator)

  ON_CORESYSTEMS_STARTUP
  {
    wdFrameAllocator::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdFrameAllocator::Shutdown();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdDoubleBufferedStackAllocator* wdFrameAllocator::s_pAllocator;

// static
void wdFrameAllocator::Swap()
{
  WD_PROFILE_SCOPE("FrameAllocator.Swap");

  s_pAllocator->Swap();
}

// static
void wdFrameAllocator::Reset()
{
  if (s_pAllocator)
  {
    s_pAllocator->Reset();
  }
}

// static
void wdFrameAllocator::Startup()
{
  s_pAllocator = WD_DEFAULT_NEW(wdDoubleBufferedStackAllocator, "FrameAllocator", wdFoundation::GetAlignedAllocator());
}

// static
void wdFrameAllocator::Shutdown()
{
  WD_DEFAULT_DELETE(s_pAllocator);
}

WD_STATICLINK_FILE(Foundation, Foundation_Memory_Implementation_FrameAllocator);
