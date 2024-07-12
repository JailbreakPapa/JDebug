#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Threading/Implementation/TaskGroup.h>
#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/Implementation/TaskWorkerThread.h>
#include <Foundation/Threading/TaskSystem.h>

nsMutex nsTaskSystem::s_TaskSystemMutex;
nsUniquePtr<nsTaskSystemState> nsTaskSystem::s_pState;
nsUniquePtr<nsTaskSystemThreadState> nsTaskSystem::s_pThreadState;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, TaskSystem)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ThreadUtils",
    "Time"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    if (nsStartup::HasApplicationTag("NoTaskSystem"))
      return;

    nsTaskSystem::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsTaskSystem::Shutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

void nsTaskSystem::Startup()
{
  s_pThreadState = NS_DEFAULT_NEW(nsTaskSystemThreadState);
  s_pState = NS_DEFAULT_NEW(nsTaskSystemState);

  tl_TaskWorkerInfo.m_WorkerType = nsWorkerThreadType::MainThread;
  tl_TaskWorkerInfo.m_iWorkerIndex = 0;

  // initialize with the default number of worker threads
  SetWorkerThreadCount();
}

void nsTaskSystem::Shutdown()
{
  if (s_pThreadState == nullptr)
    return;

  StopWorkerThreads();

  s_pState.Clear();
  s_pThreadState.Clear();
}

void nsTaskSystem::SetTargetFrameTime(nsTime targetFrameTime)
{
  s_pState->m_TargetFrameTime = targetFrameTime;
}

NS_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_TaskSystem);
