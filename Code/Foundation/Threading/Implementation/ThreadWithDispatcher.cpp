#include <Foundation/FoundationPCH.h>

#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Threading/ThreadWithDispatcher.h>

nsThreadWithDispatcher::nsThreadWithDispatcher(const char* szName /*= "nsThreadWithDispatcher"*/, nsUInt32 uiStackSize /*= 128 * 1024*/)
  : nsThread(szName, uiStackSize)
{
}

nsThreadWithDispatcher::~nsThreadWithDispatcher() = default;

void nsThreadWithDispatcher::Dispatch(DispatchFunction&& delegate)
{
  NS_LOCK(m_QueueMutex);
  m_ActiveQueue.PushBack(std::move(delegate));
}

void nsThreadWithDispatcher::DispatchQueue()
{
  {
    NS_LOCK(m_QueueMutex);
    std::swap(m_ActiveQueue, m_CurrentlyBeingDispatchedQueue);
  }

  for (const auto& pDelegate : m_CurrentlyBeingDispatchedQueue)
  {
    pDelegate();
  }

  m_CurrentlyBeingDispatchedQueue.Clear();
}
