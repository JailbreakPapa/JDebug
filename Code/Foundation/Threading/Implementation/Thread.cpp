#include <Foundation/FoundationPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/Thread.h>

wdEvent<const wdThreadEvent&, wdMutex> wdThread::s_ThreadEvents;

wdThread::wdThread(const char* szName /*= "wdThread"*/, wdUInt32 uiStackSize /*= 128 * 1024*/)
  : wdOSThread(wdThreadClassEntryPoint, this, szName, uiStackSize)
  , m_ThreadStatus(Created)
  , m_sName(szName)
{
  wdThreadEvent e;
  e.m_pThread = this;
  e.m_Type = wdThreadEvent::Type::ThreadCreated;
  wdThread::s_ThreadEvents.Broadcast(e, 255);
}

wdThread::~wdThread()
{
  WD_ASSERT_DEV(!IsRunning(), "Thread deletion while still running detected!");

  wdThreadEvent e;
  e.m_pThread = this;
  e.m_Type = wdThreadEvent::Type::ThreadDestroyed;
  wdThread::s_ThreadEvents.Broadcast(e, 255);
}

// Deactivate Doxygen document generation for the following block.
/// \cond

wdUInt32 RunThread(wdThread* pThread)
{
  if (pThread == nullptr)
    return 0;

  wdProfilingSystem::SetThreadName(pThread->m_sName.GetData());

  {
    wdThreadEvent e;
    e.m_pThread = pThread;
    e.m_Type = wdThreadEvent::Type::StartingExecution;
    wdThread::s_ThreadEvents.Broadcast(e, 255);
  }

  pThread->m_ThreadStatus = wdThread::Running;

  // Run the worker thread function
  wdUInt32 uiReturnCode = pThread->Run();

  {
    wdThreadEvent e;
    e.m_pThread = pThread;
    e.m_Type = wdThreadEvent::Type::FinishedExecution;
    wdThread::s_ThreadEvents.Broadcast(e, 255);
  }

  pThread->m_ThreadStatus = wdThread::Finished;

  wdProfilingSystem::RemoveThread();

  return uiReturnCode;
}

/// \endcond

// Include inline file
#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Threading/Implementation/Win/Thread_win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX) || WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Threading/Implementation/Posix/Thread_posix.h>
#else
#  error "Runnable thread entry functions are not implemented on current platform"
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_Thread);
