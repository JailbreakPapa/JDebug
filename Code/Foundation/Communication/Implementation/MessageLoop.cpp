#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/Implementation/MessageLoop.h>
#include <Foundation/Communication/IpcChannel.h>
#include <Foundation/Communication/RemoteMessage.h>
#include <Foundation/Configuration/Startup.h>

WD_IMPLEMENT_SINGLETON(wdMessageLoop);

#if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
#  include <Foundation/Communication/Implementation/Win/MessageLoop_win.h>
#elif WD_ENABLED(WD_PLATFORM_LINUX)
#  include <Foundation/Communication/Implementation/Linux/MessageLoop_linux.h>
#else
#  include <Foundation/Communication/Implementation/Mobile/MessageLoop_mobile.h>
#endif

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, MessageLoop)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "TaskSystem",
    "ThreadUtils"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    #if WD_ENABLED(WD_PLATFORM_WINDOWS_DESKTOP)
      WD_DEFAULT_NEW(wdMessageLoop_win);
    #elif WD_ENABLED(WD_PLATFORM_LINUX)
      WD_DEFAULT_NEW(wdMessageLoop_linux);
    #else
      WD_DEFAULT_NEW(wdMessageLoop_mobile);
    #endif
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdMessageLoop* pDummy = wdMessageLoop::GetSingleton();
    WD_DEFAULT_DELETE(pDummy);
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

class wdLoopThread : public wdThread
{
public:
  wdLoopThread()
    : wdThread("wdMessageLoopThread")
  {
  }
  wdMessageLoop* m_pRemoteInterface = nullptr;
  virtual wdUInt32 Run() override
  {
    m_pRemoteInterface->RunLoop();
    return 0;
  }
};

wdMessageLoop::wdMessageLoop()
  : m_SingletonRegistrar(this)
{
}

void wdMessageLoop::StartUpdateThread()
{
  WD_LOCK(m_Mutex);
  if (m_pUpdateThread == nullptr)
  {
    m_pUpdateThread = WD_DEFAULT_NEW(wdLoopThread);
    m_pUpdateThread->m_pRemoteInterface = this;
    m_pUpdateThread->Start();
  }
}

void wdMessageLoop::StopUpdateThread()
{
  WD_LOCK(m_Mutex);
  if (m_pUpdateThread != nullptr)
  {
    m_bShouldQuit = true;
    WakeUp();
    m_pUpdateThread->Join();

    WD_DEFAULT_DELETE(m_pUpdateThread);
  }
}

void wdMessageLoop::RunLoop()
{
#if WD_ENABLED(WD_COMPILE_FOR_DEBUG)
  m_ThreadId = wdThreadUtils::GetCurrentThreadID();
#endif

  while (true)
  {
    if (m_bCallTickFunction)
    {
      for (wdIpcChannel* pChannel : m_AllAddedChannels)
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

bool wdMessageLoop::ProcessTasks()
{
  {
    WD_LOCK(m_TasksMutex);
    // Swap out the queues under the lock so we can process them without holding the lock
    m_ConnectQueueTask.Swap(m_ConnectQueue);
    m_SendQueueTask.Swap(m_SendQueue);
    m_DisconnectQueueTask.Swap(m_DisconnectQueue);
  }

  for (wdIpcChannel* pChannel : m_ConnectQueueTask)
  {
    pChannel->InternalConnect();
  }
  for (wdIpcChannel* pChannel : m_SendQueueTask)
  {
    pChannel->InternalSend();
  }
  for (wdIpcChannel* pChannel : m_DisconnectQueueTask)
  {
    pChannel->InternalDisconnect();
  }

  bool bDidWork = !m_ConnectQueueTask.IsEmpty() || !m_SendQueueTask.IsEmpty() || !m_DisconnectQueueTask.IsEmpty();
  m_ConnectQueueTask.Clear();
  m_SendQueueTask.Clear();
  m_DisconnectQueueTask.Clear();
  return bDidWork;
}

void wdMessageLoop::Quit()
{
  m_bShouldQuit = true;
}

void wdMessageLoop::AddChannel(wdIpcChannel* pChannel)
{
  {
    WD_LOCK(m_TasksMutex);
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
  pChannel->AddToMessageLoop(this);
}

void wdMessageLoop::RemoveChannel(wdIpcChannel* pChannel)
{
  WD_LOCK(m_TasksMutex);

  m_AllAddedChannels.RemoveAndSwap(pChannel);
  m_ConnectQueue.RemoveAndSwap(pChannel);
  m_DisconnectQueue.RemoveAndSwap(pChannel);
  m_SendQueue.RemoveAndSwap(pChannel);
}

WD_STATICLINK_FILE(Foundation, Foundation_Communication_Implementation_MessageLoop);
