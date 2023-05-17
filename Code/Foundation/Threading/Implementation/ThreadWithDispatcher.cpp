#include <Foundation/FoundationPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/ThreadWithDispatcher.h>

wdThreadWithDispatcher::wdThreadWithDispatcher(const char* szName /*= "wdThreadWithDispatcher"*/, wdUInt32 uiStackSize /*= 128 * 1024*/)
  : wdThread(szName, uiStackSize)
{
}

wdThreadWithDispatcher::~wdThreadWithDispatcher() = default;

void wdThreadWithDispatcher::Dispatch(DispatchFunction&& delegate)
{
  WD_LOCK(m_QueueMutex);
  m_ActiveQueue.PushBack(std::move(delegate));
}

void wdThreadWithDispatcher::DispatchQueue()
{
  {
    WD_LOCK(m_QueueMutex);
    std::swap(m_ActiveQueue, m_CurrentlyBeingDispatchedQueue);
  }

  for (const auto& pDelegate : m_CurrentlyBeingDispatchedQueue)
  {
    pDelegate();
  }

  m_CurrentlyBeingDispatchedQueue.Clear();
}

WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_ThreadWithDispatcher);
