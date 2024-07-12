#pragma once

#include <Foundation/Threading/Implementation/TaskSystemDeclarations.h>

#include <Foundation/Threading/Thread.h>
#include <Foundation/Threading/ThreadSignal.h>

/// \internal Internal task worker thread class.
class nsTaskWorkerThread final : public nsThread
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsTaskWorkerThread);

  /// \name Execution
  ///@{

public:
  /// \brief Tells the worker thread what tasks to execute and which thread index it has.
  nsTaskWorkerThread(nsWorkerThreadType::Enum threadType, nsUInt32 uiThreadNumber);
  ~nsTaskWorkerThread();

  /// \brief Deactivates the thread. Returns failure, if the thread is currently still running.
  nsResult DeactivateWorker();

private:
  // Which types of tasks this thread should work on.
  nsWorkerThreadType::Enum m_WorkerType;

  // Whether the thread is supposed to continue running.
  volatile bool m_bActive = true;

  // For display purposes.
  nsUInt16 m_uiWorkerThreadNumber = 0xFFFF;

  ///@}

  /// \name Thread Utilization
  ///@{

public:
  /// \brief Returns the last utilization value (0 - 1 range). Optionally returns how many tasks it executed recently.
  double GetThreadUtilization(nsUInt32* pNumTasksExecuted = nullptr);

  /// \brief Computes the thread utilization by dividing the thread active time by the time that has passed since the last update.
  void UpdateThreadUtilization(nsTime timePassed);

private:
  bool m_bExecutingTask = false;
  nsUInt16 m_uiLastNumTasksExecuted = 0;
  nsUInt16 m_uiNumTasksExecuted = 0;
  nsTime m_StartedWorkingTime;
  nsTime m_ThreadActiveTime;
  double m_fLastThreadUtilization = 0.0;

  ///@}

  /// \name Idle State
  ///@{

public:
  /// \brief If the thread is currently idle, this will wake it up and return NS_SUCCESS.
  nsTaskWorkerState WakeUpIfIdle();

private:
  // Puts the thread to sleep (idle state)
  void WaitForWork();

  virtual nsUInt32 Run() override;

  // used to wake up idle threads, see m_WorkerState
  nsThreadSignal m_WakeUpSignal;

  // used to indicate whether this thread is currently idle
  // if so, it can be woken up using m_WakeUpSignal
  // nsAtomicBool m_bIsIdle = false;
  nsAtomicInteger32 m_iWorkerState; // nsTaskWorkerState

  ///@}
};

/// \internal Thread local state used by the task system (and for better debugging)
struct nsTaskWorkerInfo
{
  nsWorkerThreadType::Enum m_WorkerType = nsWorkerThreadType::Unknown;
  bool m_bAllowNestedTasks = true;
  nsInt32 m_iWorkerIndex = -1;
  const char* m_szTaskName = nullptr;
  nsAtomicInteger32* m_pWorkerState = nullptr;
};

extern thread_local nsTaskWorkerInfo tl_TaskWorkerInfo;
