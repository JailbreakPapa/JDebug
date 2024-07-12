#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Memory/FrameAllocator.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Strings/StringBuilder.h>

nsDoubleBufferedLinearAllocator::nsDoubleBufferedLinearAllocator(nsStringView sName0, nsAllocator* pParent)
{
  nsStringBuilder sName = sName0;
  sName.Append("0");

  m_pCurrentAllocator = NS_DEFAULT_NEW(StackAllocatorType, sName, pParent);

  sName = sName0;
  sName.Append("1");

  m_pOtherAllocator = NS_DEFAULT_NEW(StackAllocatorType, sName, pParent);
}

nsDoubleBufferedLinearAllocator::~nsDoubleBufferedLinearAllocator()
{
  NS_DEFAULT_DELETE(m_pCurrentAllocator);
  NS_DEFAULT_DELETE(m_pOtherAllocator);
}

void nsDoubleBufferedLinearAllocator::Swap()
{
  nsMath::Swap(m_pCurrentAllocator, m_pOtherAllocator);

  m_pCurrentAllocator->Reset();
}

void nsDoubleBufferedLinearAllocator::Reset()
{
  m_pCurrentAllocator->Reset();
  m_pOtherAllocator->Reset();
}


// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, FrameAllocator)

  ON_CORESYSTEMS_STARTUP
  {
    nsFrameAllocator::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsFrameAllocator::Shutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsDoubleBufferedLinearAllocator* nsFrameAllocator::s_pAllocator;

// static
void nsFrameAllocator::Swap()
{
  NS_PROFILE_SCOPE("FrameAllocator.Swap");

  s_pAllocator->Swap();
}

// static
void nsFrameAllocator::Reset()
{
  if (s_pAllocator)
  {
    s_pAllocator->Reset();
  }
}

// static
void nsFrameAllocator::Startup()
{
  s_pAllocator = NS_DEFAULT_NEW(nsDoubleBufferedLinearAllocator, "FrameAllocator", nsFoundation::GetAlignedAllocator());
}

// static
void nsFrameAllocator::Shutdown()
{
  NS_DEFAULT_DELETE(s_pAllocator);
}

NS_STATICLINK_FILE(Foundation, Foundation_Memory_Implementation_FrameAllocator);
