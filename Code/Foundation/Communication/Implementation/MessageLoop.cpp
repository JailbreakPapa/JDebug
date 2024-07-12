#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Implementation/MessageLoop.h>
#include <Foundation/Communication/IpcChannel.h>
#include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/Configuration/Startup.h>

NS_IMPLEMENT_SINGLETON(nsMessageLoop);

#if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
#  include <Foundation/Platform/Win/MessageLoop_Win.h>
#elif NS_ENABLED(NS_PLATFORM_LINUX)
#  include <Foundation/Platform/Linux/MessageLoop_Linux.h>
#else
#  include <Foundation/Communication/Implementation/MessageLoop_Fallback.h>
#endif

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, MessageLoop)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "TaskSystem",
    "ThreadUtils"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    if (nsStartup::HasApplicationTag("NoMessageLoop"))
      return;

    #if NS_ENABLED(NS_PLATFORM_WINDOWS_DESKTOP)
      NS_DEFAULT_NEW(nsMessageLoop_win);
    #elif NS_ENABLED(NS_PLATFORM_LINUX)
      NS_DEFAULT_NEW(nsMessageLoop_linux);
    #else
      NS_DEFAULT_NEW(nsMessageLoop_Fallback);
    #endif
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsMessageLoop* pDummy = nsMessageLoop::GetSingleton();
    NS_DEFAULT_DELETE(pDummy);
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

class nsLoopThread : public nsThread
{
public:
  nsLoopThread()
    : nsThread("nsMessageLoopThread")
  {
  }
  nsMessageLoop* m_pRemoteInterface = nullptr;
  virtual nsUInt32 Run() override
  {
    m_pRemoteInterface->RunLoop();
    return 0;
  }
};

nsMessageLoop::nsMessageLoop()
  : m_SingletonRegistrar(this)
{
}

void nsMessageLoop::StartUpdateThread()
{
  NS_LOCK(m_Mutex);
  if (m_pUpdateThread == nullptr)
  {
    m_pUpdateThread = NS_DEFAULT_NEW(nsLoopThread);
    m_pUpdateThread->m_pRemoteInterface = this;
    m_pUpdateThread->Start();
  }
}

void nsMessageLoop::StopUpdateThread()
{
  NS_LOCK(m_Mutex);
  if (m_pUpdateThread != nullptr)
  {
    m_bShouldQuit = true;
    WakeUp();
    m_pUpdateThread->Join();

    NS_DEFAULT_DELETE(m_pUpdateThread);
  }
}

void nsMessageLoop::RunLoop()
{
#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
  m_ThreadId = nsThreadUtils::GetCurrentThreadID();
#endif

  while (true)
  {
    if (m_bCallTickFunction)
    {
      for (nsIpcChannel* pChannel : m_AllAddedChannels)
      {
        if (pChannel->RequiresRegularTick())
        {
          pChannel->Tick();
        }
      }
    }

    // process all available data until all is processed and we wait for new messages.
    bool didwork = ProcessTasks();
    if (m_bShouldQuit)
      break;

    if (didwork)
      continue;

    didwork |= WaitForMessages(0, nullptr);
    if (m_bShouldQuit)
      break;

    if (didwork)
      continue;

    // wait until we have work again
    WaitForMessages(m_bCallTickFunction ? 50 : -1, nullptr); // timeout 20 times per second, if we need to call the tick function
  }
}

bool nsMessageLoop::ProcessTasks()
{
  {
    NS_LOCK(m_TasksMutex);
    // Swap out the queues under the lock so we can process them without holding the lock
    m_ConnectQueueTask.Swap(m_ConnectQueue);
    m_SendQueueTask.Swap(m_SendQueue);
    m_DisconnectQueueTask.Swap(m_DisconnectQueue);
  }

  for (nsIpcChannel* pChannel : m_ConnectQueueTask)
  {
    pChannel->InternalConnect();
  }
  for (nsIpcChannel* pChannel : m_SendQueueTask)
  {
    pChannel->InternalSend();
  }
  for (nsIpcChannel* pChannel : m_DisconnectQueueTask)
  {
    pChannel->InternalDisconnect();
  }

  bool bDidWork = !m_ConnectQueueTask.IsEmpty() || !m_SendQueueTask.IsEmpty() || !m_DisconnectQueueTask.IsEmpty();
  m_ConnectQueueTask.Clear();
  m_SendQueueTask.Clear();
  m_DisconnectQueueTask.Clear();
  return bDidWork;
}

void nsMessageLoop::Quit()
{
  m_bShouldQuit = true;
}

void nsMessageLoop::AddChannel(nsIpcChannel* pChannel)
{
  {
    NS_LOCK(m_TasksMutex);
    m_AllAddedChannels.PushBack(pChannel);

    m_bCallTickFunction = false;
    for (auto pThisChannel : m_AllAddedChannels)
    {
      if (pThisChannel->RequiresRegularTick())
      {
        m_bCallTickFunction = true;
        break;
      }
    }
  }

  StartUpdateThread();
  pChannel->m_pOwner = this;
}

void nsMessageLoop::RemoveChannel(nsIpcChannel* pChannel)
{
  NS_LOCK(m_TasksMutex);

  m_AllAddedChannels.RemoveAndSwap(pChannel);
  m_ConnectQueue.RemoveAndSwap(pChannel);
  m_DisconnectQueue.RemoveAndSwap(pChannel);
  m_SendQueue.RemoveAndSwap(pChannel);
}

NS_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_MessageLoop);
