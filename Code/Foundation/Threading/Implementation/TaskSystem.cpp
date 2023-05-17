#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Threading/Implementation/TaskGroup.h>
#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/Implementation/TaskWorkerThread.h>
#include <Foundation/Threading/TaskSystem.h>

wdMutex wdTaskSystem::s_TaskSystemMutex;
wdUniquePtr<wdTaskSystemState> wdTaskSystem::s_pState;
wdUniquePtr<wdTaskSystemThreadState> wdTaskSystem::s_pThreadState;

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, TaskSystem)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "ThreadUtils",
    "Time"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdTaskSystem::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdTaskSystem::Shutdown();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

void wdTaskSystem::Startup()
{
  s_pThreadState = WD_DEFAULT_NEW(wdTaskSystemThreadState);
  s_pState = WD_DEFAULT_NEW(wdTaskSystemState);

  tl_TaskWorkerInfo.m_WorkerType = wdWorkerThreadType::MainThread;
  tl_TaskWorkerInfo.m_iWorkerIndex = 0;

  // initialize with the default number of worker threads
  SetWorkerThreadCount();
}

void wdTaskSystem::Shutdown()
{
  StopWorkerThreads();

  s_pState.Clear();
  s_pThreadState.Clear();
}

void wdTaskSystem::SetTargetFrameTime(wdTime targetFrameTime)
{
  s_pState->m_TargetFrameTime = targetFrameTime;
}

WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_TaskSystem);
