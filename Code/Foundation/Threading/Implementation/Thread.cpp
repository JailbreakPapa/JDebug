#include <Foundation/FoundationPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/Thread.h>

nsEvent<const nsThreadEvent&, nsMutex> nsThread::s_ThreadEvents;

thread_local nsThread* g_pCurrentThread = nullptr;

const nsThread* nsThread::GetCurrentThread()
{
  return g_pCurrentThread;
}

nsThread::nsThread(nsStringView sName /*= "nsThread"*/, nsUInt32 uiStackSize /*= 128 * 1024*/)
  : nsOSThread(nsThreadClassEntryPoint, this, sName, uiStackSize)
  , m_sName(sName)
{
  nsThreadEvent e;
  e.m_pThread = this;
  e.m_Type = nsThreadEvent::Type::ThreadCreated;
  nsThread::s_ThreadEvents.Broadcast(e, 255);
}

nsThread::~nsThread()
{
  NS_ASSERT_DEV(!IsRunning(), "Thread deletion while still running detected!");

  nsThreadEvent e;
  e.m_pThread = this;
  e.m_Type = nsThreadEvent::Type::ThreadDestroyed;
  nsThread::s_ThreadEvents.Broadcast(e, 255);
}

nsUInt32 RunThread(nsThread* pThread)
{
  if (pThread == nullptr)
    return 0;

  g_pCurrentThread = pThread;
  nsProfilingSystem::SetThreadName(pThread->m_sName.GetView());

  {
    nsThreadEvent e;
    e.m_pThread = pThread;
    e.m_Type = nsThreadEvent::Type::StartingExecution;
    nsThread::s_ThreadEvents.Broadcast(e, 255);
  }

  pThread->m_ThreadStatus = nsThread::Running;

  // Run the worker thread function
  nsUInt32 uiReturnCode = pThread->Run();

  {
    nsThreadEvent e;
    e.m_pThread = pThread;
    e.m_Type = nsThreadEvent::Type::FinishedExecution;
    nsThread::s_ThreadEvents.Broadcast(e, 255);
  }

  pThread->m_ThreadStatus = nsThread::Finished;

  nsProfilingSystem::RemoveThread();

  return uiReturnCode;
}
