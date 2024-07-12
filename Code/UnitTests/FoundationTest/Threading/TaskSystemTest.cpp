#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/FileSystem/DataDirTypeFolder.h>
#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Utilities/DGMLWriter.h>

class nsTestTask final : public nsTask
{
public:
  nsUInt32 m_uiIterations;
  nsTestTask* m_pDependency;
  bool m_bSupportCancel;
  nsInt32 m_iTaskID;

  nsTestTask()
  {
    m_uiIterations = 50;
    m_pDependency = nullptr;
    m_bStarted = false;
    m_bDone = false;
    m_bSupportCancel = false;
    m_iTaskID = -1;

    ConfigureTask("nsTestTask", nsTaskNesting::Never);
  }

  bool IsStarted() const { return m_bStarted; }
  bool IsDone() const { return m_bDone; }
  bool IsMultiplicityDone() const { return m_iMultiplicityCount == (int)GetMultiplicity(); }

private:
  bool m_bStarted;
  bool m_bDone;
  mutable nsAtomicInteger32 m_iMultiplicityCount;

  virtual void ExecuteWithMultiplicity(nsUInt32 uiInvocation) const override { m_iMultiplicityCount.Increment(); }

  virtual void Execute() override
  {
    if (m_iTaskID >= 0)
      nsLog::Printf("Starting Task %i at %.4f\n", m_iTaskID, nsTime::Now().GetSeconds());

    m_bStarted = true;

    NS_TEST_BOOL(m_pDependency == nullptr || m_pDependency->IsTaskFinished());

    for (nsUInt32 obst = 0; obst < m_uiIterations; ++obst)
    {
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));
      nsTime::Now();

      if (HasBeenCanceled() && m_bSupportCancel)
      {
        if (m_iTaskID >= 0)
          nsLog::Printf("Canceling Task %i at %.4f\n", m_iTaskID, nsTime::Now().GetSeconds());
        return;
      }
    }

    m_bDone = true;

    if (m_iTaskID >= 0)
      nsLog::Printf("Finishing Task %i at %.4f\n", m_iTaskID, nsTime::Now().GetSeconds());
  }
};

class TaskCallbacks
{
public:
  void TaskFinished(const nsSharedPtr<nsTask>& pTask) { m_pInt->Increment(); }

  void TaskGroupFinished(nsTaskGroupID id) { m_pInt->Increment(); }

  nsAtomicInteger32* m_pInt;
};

NS_CREATE_SIMPLE_TEST(Threading, TaskSystem)
{
  nsInt8 iWorkersShort = 4;
  nsInt8 iWorkersLong = 4;

  nsTaskSystem::SetWorkerThreadCount(iWorkersShort, iWorkersLong);
  nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(500));

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Single Tasks")
  {
    nsSharedPtr<nsTestTask> t[3];

    t[0] = NS_DEFAULT_NEW(nsTestTask);
    t[1] = NS_DEFAULT_NEW(nsTestTask);
    t[2] = NS_DEFAULT_NEW(nsTestTask);

    t[0]->ConfigureTask("Task 0", nsTaskNesting::Never);
    t[1]->ConfigureTask("Task 1", nsTaskNesting::Maybe);
    t[2]->ConfigureTask("Task 2", nsTaskNesting::Never);

    auto tg0 = nsTaskSystem::StartSingleTask(t[0], nsTaskPriority::LateThisFrame);
    auto tg1 = nsTaskSystem::StartSingleTask(t[1], nsTaskPriority::ThisFrame);
    auto tg2 = nsTaskSystem::StartSingleTask(t[2], nsTaskPriority::EarlyThisFrame);

    nsTaskSystem::WaitForGroup(tg0);
    nsTaskSystem::WaitForGroup(tg1);
    nsTaskSystem::WaitForGroup(tg2);

    NS_TEST_BOOL(t[0]->IsDone());
    NS_TEST_BOOL(t[1]->IsDone());
    NS_TEST_BOOL(t[2]->IsDone());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Single Tasks with Dependencies")
  {
    nsSharedPtr<nsTestTask> t[4];

    t[0] = NS_DEFAULT_NEW(nsTestTask);
    t[1] = NS_DEFAULT_NEW(nsTestTask);
    t[2] = NS_DEFAULT_NEW(nsTestTask);
    t[3] = NS_DEFAULT_NEW(nsTestTask);

    nsTaskGroupID g[4];

    t[0]->ConfigureTask("Task 0", nsTaskNesting::Never);
    t[1]->ConfigureTask("Task 1", nsTaskNesting::Maybe);
    t[2]->ConfigureTask("Task 2", nsTaskNesting::Never);
    t[3]->ConfigureTask("Task 3", nsTaskNesting::Maybe);

    g[0] = nsTaskSystem::StartSingleTask(t[0], nsTaskPriority::LateThisFrame);
    g[1] = nsTaskSystem::StartSingleTask(t[1], nsTaskPriority::ThisFrame, g[0]);
    g[2] = nsTaskSystem::StartSingleTask(t[2], nsTaskPriority::EarlyThisFrame, g[1]);
    g[3] = nsTaskSystem::StartSingleTask(t[3], nsTaskPriority::EarlyThisFrame, g[0]);

    nsTaskSystem::WaitForGroup(g[2]);
    nsTaskSystem::WaitForGroup(g[3]);

    NS_TEST_BOOL(t[0]->IsDone());
    NS_TEST_BOOL(t[1]->IsDone());
    NS_TEST_BOOL(t[2]->IsDone());
    NS_TEST_BOOL(t[3]->IsDone());
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Grouped Tasks / TaskFinished Callback / GroupFinished Callback")
  {
    nsSharedPtr<nsTestTask> t[8];

    nsTaskGroupID g[4];
    nsAtomicInteger32 GroupsFinished;
    nsAtomicInteger32 TasksFinished;

    TaskCallbacks callbackGroup;
    callbackGroup.m_pInt = &GroupsFinished;

    TaskCallbacks callbackTask;
    callbackTask.m_pInt = &TasksFinished;

    g[0] = nsTaskSystem::CreateTaskGroup(nsTaskPriority::ThisFrame, nsMakeDelegate(&TaskCallbacks::TaskGroupFinished, &callbackGroup));
    g[1] = nsTaskSystem::CreateTaskGroup(nsTaskPriority::ThisFrame, nsMakeDelegate(&TaskCallbacks::TaskGroupFinished, &callbackGroup));
    g[2] = nsTaskSystem::CreateTaskGroup(nsTaskPriority::ThisFrame, nsMakeDelegate(&TaskCallbacks::TaskGroupFinished, &callbackGroup));
    g[3] = nsTaskSystem::CreateTaskGroup(nsTaskPriority::ThisFrame, nsMakeDelegate(&TaskCallbacks::TaskGroupFinished, &callbackGroup));

    for (int i = 0; i < 4; ++i)
      NS_TEST_BOOL(!nsTaskSystem::IsTaskGroupFinished(g[i]));

    nsTaskSystem::AddTaskGroupDependency(g[1], g[0]);
    nsTaskSystem::AddTaskGroupDependency(g[2], g[0]);
    nsTaskSystem::AddTaskGroupDependency(g[3], g[1]);

    for (int i = 0; i < 8; ++i)
    {
      t[i] = NS_DEFAULT_NEW(nsTestTask);
      t[i]->ConfigureTask("Test Task", nsTaskNesting::Maybe, nsMakeDelegate(&TaskCallbacks::TaskFinished, &callbackTask));
    }

    nsTaskSystem::AddTaskToGroup(g[0], t[0]);
    nsTaskSystem::AddTaskToGroup(g[1], t[1]);
    nsTaskSystem::AddTaskToGroup(g[1], t[2]);
    nsTaskSystem::AddTaskToGroup(g[2], t[3]);
    nsTaskSystem::AddTaskToGroup(g[2], t[4]);
    nsTaskSystem::AddTaskToGroup(g[2], t[5]);
    nsTaskSystem::AddTaskToGroup(g[3], t[6]);
    nsTaskSystem::AddTaskToGroup(g[3], t[7]);

    for (int i = 0; i < 8; ++i)
    {
      NS_TEST_BOOL(!t[i]->IsTaskFinished());
      NS_TEST_BOOL(!t[i]->IsDone());
    }

    // do a snapshot
    // we don't validate it, just make sure it doesn't crash
    nsDGMLGraph graph;
    nsTaskSystem::WriteStateSnapshotToDGML(graph);

    nsTaskSystem::StartTaskGroup(g[3]);
    nsTaskSystem::StartTaskGroup(g[2]);
    nsTaskSystem::StartTaskGroup(g[1]);
    nsTaskSystem::StartTaskGroup(g[0]);

    nsTaskSystem::WaitForGroup(g[3]);
    nsTaskSystem::WaitForGroup(g[2]);
    nsTaskSystem::WaitForGroup(g[1]);
    nsTaskSystem::WaitForGroup(g[0]);

    NS_TEST_INT(TasksFinished, 8);

    // It is not guaranteed that group finished callback is called after WaitForGroup returned so we need to wait a bit here.
    for (int i = 0; i < 10; i++)
    {
      if (GroupsFinished == 4)
      {
        break;
      }
      nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    }
    NS_TEST_INT(GroupsFinished, 4);

    for (int i = 0; i < 4; ++i)
      NS_TEST_BOOL(nsTaskSystem::IsTaskGroupFinished(g[i]));

    for (int i = 0; i < 8; ++i)
    {
      NS_TEST_BOOL(t[i]->IsTaskFinished());
      NS_TEST_BOOL(t[i]->IsDone());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "This Frame Tasks / Next Frame Tasks")
  {
    const nsUInt32 uiNumTasks = 20;
    nsSharedPtr<nsTestTask> t[uiNumTasks];
    nsTaskGroupID tg[uiNumTasks];
    bool finished[uiNumTasks];

    for (nsUInt32 i = 0; i < uiNumTasks; i += 2)
    {
      finished[i] = false;
      finished[i + 1] = false;

      t[i] = NS_DEFAULT_NEW(nsTestTask);
      t[i + 1] = NS_DEFAULT_NEW(nsTestTask);

      t[i]->m_uiIterations = 10;
      t[i + 1]->m_uiIterations = 20;

      tg[i] = nsTaskSystem::StartSingleTask(t[i], nsTaskPriority::ThisFrame);
      tg[i + 1] = nsTaskSystem::StartSingleTask(t[i + 1], nsTaskPriority::NextFrame);
    }

    // 'finish' the first frame
    nsTaskSystem::FinishFrameTasks();

    {
      nsUInt32 uiNotAllThisTasksFinished = 0;
      nsUInt32 uiNotAllNextTasksFinished = 0;

      for (nsUInt32 i = 0; i < uiNumTasks; i += 2)
      {
        if (!t[i]->IsTaskFinished())
        {
          NS_TEST_BOOL(!finished[i]);
          ++uiNotAllThisTasksFinished;
        }
        else
        {
          finished[i] = true;
        }

        if (!t[i + 1]->IsTaskFinished())
        {
          NS_TEST_BOOL(!finished[i + 1]);
          ++uiNotAllNextTasksFinished;
        }
        else
        {
          finished[i + 1] = true;
        }
      }

      // up to the number of worker threads tasks can still be active
      NS_TEST_BOOL(uiNotAllThisTasksFinished <= nsTaskSystem::GetNumAllocatedWorkerThreads(nsWorkerThreadType::ShortTasks));
      NS_TEST_BOOL(uiNotAllNextTasksFinished <= uiNumTasks);
    }


    // 'finish' the second frame
    nsTaskSystem::FinishFrameTasks();

    {
      nsUInt32 uiNotAllThisTasksFinished = 0;
      nsUInt32 uiNotAllNextTasksFinished = 0;

      for (int i = 0; i < uiNumTasks; i += 2)
      {
        if (!t[i]->IsTaskFinished())
        {
          NS_TEST_BOOL(!finished[i]);
          ++uiNotAllThisTasksFinished;
        }
        else
        {
          finished[i] = true;
        }

        if (!t[i + 1]->IsTaskFinished())
        {
          NS_TEST_BOOL(!finished[i + 1]);
          ++uiNotAllNextTasksFinished;
        }
        else
        {
          finished[i + 1] = true;
        }
      }

      NS_TEST_BOOL(
        uiNotAllThisTasksFinished + uiNotAllNextTasksFinished <= nsTaskSystem::GetNumAllocatedWorkerThreads(nsWorkerThreadType::ShortTasks));
    }

    // 'finish' all frames
    nsTaskSystem::FinishFrameTasks();

    {
      nsUInt32 uiNotAllThisTasksFinished = 0;
      nsUInt32 uiNotAllNextTasksFinished = 0;

      for (nsUInt32 i = 0; i < uiNumTasks; i += 2)
      {
        if (!t[i]->IsTaskFinished())
        {
          NS_TEST_BOOL(!finished[i]);
          ++uiNotAllThisTasksFinished;
        }
        else
        {
          finished[i] = true;
        }

        if (!t[i + 1]->IsTaskFinished())
        {
          NS_TEST_BOOL(!finished[i + 1]);
          ++uiNotAllNextTasksFinished;
        }
        else
        {
          finished[i + 1] = true;
        }
      }

      // even after finishing multiple frames, the previous frame tasks may still be in execution
      // since no N+x tasks enforce their completion in this test
      NS_TEST_BOOL(
        uiNotAllThisTasksFinished + uiNotAllNextTasksFinished <= nsTaskSystem::GetNumAllocatedWorkerThreads(nsWorkerThreadType::ShortTasks));
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Main Thread Tasks")
  {
    const nsUInt32 uiNumTasks = 20;
    nsSharedPtr<nsTestTask> t[uiNumTasks];

    for (nsUInt32 i = 0; i < uiNumTasks; ++i)
    {
      t[i] = NS_DEFAULT_NEW(nsTestTask);
      t[i]->m_uiIterations = 10;

      nsTaskSystem::StartSingleTask(t[i], nsTaskPriority::ThisFrameMainThread);
    }

    nsTaskSystem::FinishFrameTasks();

    for (nsUInt32 i = 0; i < uiNumTasks; ++i)
    {
      NS_TEST_BOOL(t[i]->IsTaskFinished());
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Canceling Tasks")
  {
    const nsUInt32 uiNumTasks = 20;
    nsSharedPtr<nsTestTask> t[uiNumTasks];
    nsTaskGroupID tg[uiNumTasks];

    for (int i = 0; i < uiNumTasks; ++i)
    {
      t[i] = NS_DEFAULT_NEW(nsTestTask);
      t[i]->m_uiIterations = 50;

      tg[i] = nsTaskSystem::StartSingleTask(t[i], nsTaskPriority::ThisFrame);
    }

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));

    nsUInt32 uiCanceled = 0;

    for (nsUInt32 i0 = uiNumTasks; i0 > 0; --i0)
    {
      const nsUInt32 i = i0 - 1;

      if (nsTaskSystem::CancelTask(t[i], nsOnTaskRunning::ReturnWithoutBlocking) == NS_SUCCESS)
        ++uiCanceled;
    }

    nsUInt32 uiDone = 0;
    nsUInt32 uiStarted = 0;

    for (int i = 0; i < uiNumTasks; ++i)
    {
      nsTaskSystem::WaitForGroup(tg[i]);
      NS_TEST_BOOL(t[i]->IsTaskFinished());

      if (t[i]->IsDone())
        ++uiDone;
      if (t[i]->IsStarted())
        ++uiStarted;
    }

    // at least one task should have run and thus be 'done'
    NS_TEST_BOOL(uiDone > 0);
    NS_TEST_BOOL(uiDone < uiNumTasks);

    NS_TEST_BOOL(uiStarted > 0);
    NS_TEST_BOOL_MSG(uiStarted <= nsTaskSystem::GetNumAllocatedWorkerThreads(nsWorkerThreadType::ShortTasks),
      "This test can fail when the PC is under heavy load."); // should not have managed to start more tasks than there are threads
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Canceling Tasks (forcefully)")
  {
    const nsUInt32 uiNumTasks = 20;
    nsSharedPtr<nsTestTask> t[uiNumTasks];
    nsTaskGroupID tg[uiNumTasks];

    for (int i = 0; i < uiNumTasks; ++i)
    {
      t[i] = NS_DEFAULT_NEW(nsTestTask);
      t[i]->m_uiIterations = 50;
      t[i]->m_bSupportCancel = true;

      tg[i] = nsTaskSystem::StartSingleTask(t[i], nsTaskPriority::ThisFrame);
    }

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));

    nsUInt32 uiCanceled = 0;

    for (int i = uiNumTasks - 1; i >= 0; --i)
    {
      if (nsTaskSystem::CancelTask(t[i], nsOnTaskRunning::ReturnWithoutBlocking) == NS_SUCCESS)
        ++uiCanceled;
    }

    nsUInt32 uiDone = 0;
    nsUInt32 uiStarted = 0;

    for (int i = 0; i < uiNumTasks; ++i)
    {
      nsTaskSystem::WaitForGroup(tg[i]);
      NS_TEST_BOOL(t[i]->IsTaskFinished());

      if (t[i]->IsDone())
        ++uiDone;
      if (t[i]->IsStarted())
        ++uiStarted;
    }

    // not a single thread should have finished the execution
    if (NS_TEST_BOOL_MSG(uiDone == 0, "This test can fail when the PC is under heavy load."))
    {
      NS_TEST_BOOL(uiStarted > 0);
      NS_TEST_BOOL(uiStarted <= nsTaskSystem::GetNumAllocatedWorkerThreads(
                                  nsWorkerThreadType::ShortTasks)); // should not have managed to start more tasks than there are threads
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Canceling Group")
  {
    const nsUInt32 uiNumTasks = 4;
    nsSharedPtr<nsTestTask> t1[uiNumTasks];
    nsSharedPtr<nsTestTask> t2[uiNumTasks];

    nsTaskGroupID g1, g2;
    g1 = nsTaskSystem::CreateTaskGroup(nsTaskPriority::ThisFrame);
    g2 = nsTaskSystem::CreateTaskGroup(nsTaskPriority::ThisFrame);

    nsTaskSystem::AddTaskGroupDependency(g2, g1);

    for (nsUInt32 i = 0; i < uiNumTasks; ++i)
    {
      t1[i] = NS_DEFAULT_NEW(nsTestTask);
      t2[i] = NS_DEFAULT_NEW(nsTestTask);

      nsTaskSystem::AddTaskToGroup(g1, t1[i]);
      nsTaskSystem::AddTaskToGroup(g2, t2[i]);
    }

    nsTaskSystem::StartTaskGroup(g2);
    nsTaskSystem::StartTaskGroup(g1);

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));

    NS_TEST_BOOL(nsTaskSystem::CancelGroup(g2, nsOnTaskRunning::WaitTillFinished) == NS_SUCCESS);

    for (int i = 0; i < uiNumTasks; ++i)
    {
      NS_TEST_BOOL(!t2[i]->IsDone());
      NS_TEST_BOOL(t2[i]->IsTaskFinished());
    }

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(1));

    NS_TEST_BOOL(nsTaskSystem::CancelGroup(g1, nsOnTaskRunning::WaitTillFinished) == NS_FAILURE);

    for (int i = 0; i < uiNumTasks; ++i)
    {
      NS_TEST_BOOL(!t2[i]->IsDone());

      NS_TEST_BOOL(t1[i]->IsTaskFinished());
      NS_TEST_BOOL(t2[i]->IsTaskFinished());
    }

    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(100));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Tasks with Multiplicity")
  {
    nsSharedPtr<nsTestTask> t[3];
    nsTaskGroupID tg[3];

    t[0] = NS_DEFAULT_NEW(nsTestTask);
    t[1] = NS_DEFAULT_NEW(nsTestTask);
    t[2] = NS_DEFAULT_NEW(nsTestTask);

    t[0]->ConfigureTask("Task 0", nsTaskNesting::Maybe);
    t[1]->ConfigureTask("Task 1", nsTaskNesting::Maybe);
    t[2]->ConfigureTask("Task 2", nsTaskNesting::Never);

    t[0]->SetMultiplicity(1);
    t[1]->SetMultiplicity(100);
    t[2]->SetMultiplicity(1000);

    tg[0] = nsTaskSystem::StartSingleTask(t[0], nsTaskPriority::LateThisFrame);
    tg[1] = nsTaskSystem::StartSingleTask(t[1], nsTaskPriority::ThisFrame);
    tg[2] = nsTaskSystem::StartSingleTask(t[2], nsTaskPriority::EarlyThisFrame);

    nsTaskSystem::WaitForGroup(tg[0]);
    nsTaskSystem::WaitForGroup(tg[1]);
    nsTaskSystem::WaitForGroup(tg[2]);

    NS_TEST_BOOL(t[0]->IsMultiplicityDone());
    NS_TEST_BOOL(t[1]->IsMultiplicityDone());
    NS_TEST_BOOL(t[2]->IsMultiplicityDone());
  }

  // capture profiling info for testing
  /*nsStringBuilder sOutputPath = nsTestFramework::GetInstance()->GetAbsOutputPath();

  nsFileSystem::AddDataDirectory(sOutputPath.GetData());

  nsFileWriter fileWriter;
  if (fileWriter.Open("profiling.json") == NS_SUCCESS)
  {
  nsProfilingSystem::Capture(fileWriter);
  }*/
}
