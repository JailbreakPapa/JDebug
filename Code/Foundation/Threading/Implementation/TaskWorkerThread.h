#pragma once

#include <Foundation/Threading/Implementation/TaskSystemDeclarations.h>

#include <Foundation/Threading/Thread.h>
#include <Foundation/Threading/ThreadSignal.h>

/// \internal Internal task worker thread class.
class wdTaskWorkerThread final : public wdThread
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdTaskWorkerThread);

  /// \name Execution
  ///@{

public:
  /// \brief Tells the worker thread what tasks to execute and which thread index it has.
  wdTaskWorkerThread(wdWorkerThreadType::Enum threadType, wdUInt32 uiThreadNumber);
  ~wdTaskWorkerThread();

  /// \brief Deactivates the thread. Returns failure, if the thread is currently still running.
  wdResult DeactivateWorker();

private:
  // Which types of tasks this thread should work on.
  wdWorkerThreadType::Enum m_WorkerType;

  // Whether the thread is supposed to continue running.
  volatile bool m_bActive = true;

  // For display purposes.
  wdUInt16 m_uiWorkerThreadNumber = 0xFFFF;

  ///@}

  /// \name Thread Utilization
  ///@{

public:
  /// \brief Returns the last utilization value (0 - 1 range). Optionally returns how many tasks it executed recently.
  double GetThreadUtilization(wdUInt32* pNumTasksExecuted = nullptr);

  /// \brief Computes the thread utilization by dividing the thread active time by the time that has passed since the last update.
  void UpdateThreadUtilization(wdTime timePassed);

private:
  bool m_bExecutingTask = false;
  wdUInt16 m_uiLastNumTasksExecuted = 0;
  wdUInt16 m_uiNumTasksExecuted = 0;
  wdTime m_StartedWorkingTime;
  wdTime m_ThreadActiveTime;
  double m_fLastThreadUtilization = 0.0;

  ///@}

  /// \name Idle State
  ///@{

public:
  /// \brief If the thread is currently idle, this will wake it up and return WD_SUCCESS.
  wdTaskWorkerState WakeUpIfIdle();

private:
  // Puts the thread to sleep (idle state)
  void WaitForWork();

  virtual wdUInt32 Run() override;

  // used to wake up idle threads, see m_WorkerState
  wdThreadSignal m_WakeUpSignal;

  // used to indicate whether this thread is currently idle
  // if so, it can be woken up using m_WakeUpSignal
  // wdAtomicBool m_bIsIdle = false;
  wdAtomicInteger32 m_iWorkerState; // wdTaskWorkerState

  ///@}
};

/// \internal Thread local state used by the task system (and for better debugging)
struct wdTaskWorkerInfo
{
  wdWorkerThreadType::Enum m_WorkerType = wdWorkerThreadType::Unknown;
  wdInt32 m_iWorkerIndex = -1;
  bool m_bAllowNestedTasks = true;
  const char* m_szTaskName = nullptr;
  wdAtomicInteger32* m_pWorkerState = nullptr;
};

extern thread_local wdTaskWorkerInfo tl_TaskWorkerInfo;
