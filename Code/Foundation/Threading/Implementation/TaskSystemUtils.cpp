#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Threading/Implementation/TaskGroup.h>
#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Time/Timestamp.h>
#include <Foundation/Utilities/DGMLWriter.h>

const char* wdWorkerThreadType::GetThreadTypeName(wdWorkerThreadType::Enum threadType)
{
  switch (threadType)
  {
    case wdWorkerThreadType::ShortTasks:
      return "Short Task";

    case wdWorkerThreadType::LongTasks:
      return "Long Task";

    case wdWorkerThreadType::FileAccess:
      return "File Access";

    default:
      WD_REPORT_FAILURE("Invalid Thread Type");
      return "unknown";
  }
}

void wdTaskSystem::WriteStateSnapshotToDGML(wdDGMLGraph& ref_graph)
{
  WD_LOCK(s_TaskSystemMutex);

  wdHashTable<const wdTaskGroup*, wdDGMLGraph::NodeId> groupNodeIds;

  wdStringBuilder title, tmp;

  wdDGMLGraph::NodeDesc taskGroupND;
  taskGroupND.m_Color = wdColor::CornflowerBlue;
  taskGroupND.m_Shape = wdDGMLGraph::NodeShape::Rectangle;

  wdDGMLGraph::NodeDesc taskNodeND;
  taskNodeND.m_Color = wdColor::OrangeRed;
  taskNodeND.m_Shape = wdDGMLGraph::NodeShape::RoundedRectangle;

  const wdDGMLGraph::PropertyId startedByUserId = ref_graph.AddPropertyType("StartByUser");
  const wdDGMLGraph::PropertyId activeDepsId = ref_graph.AddPropertyType("ActiveDependencies");
  const wdDGMLGraph::PropertyId scheduledId = ref_graph.AddPropertyType("Scheduled");
  const wdDGMLGraph::PropertyId finishedId = ref_graph.AddPropertyType("Finished");
  const wdDGMLGraph::PropertyId multiplicityId = ref_graph.AddPropertyType("Multiplicity");
  const wdDGMLGraph::PropertyId remainingRunsId = ref_graph.AddPropertyType("RemainingRuns");
  const wdDGMLGraph::PropertyId priorityId = ref_graph.AddPropertyType("GroupPriority");

  const char* szTaskPriorityNames[wdTaskPriority::ENUM_COUNT] = {};
  szTaskPriorityNames[wdTaskPriority::EarlyThisFrame] = "EarlyThisFrame";
  szTaskPriorityNames[wdTaskPriority::ThisFrame] = "ThisFrame";
  szTaskPriorityNames[wdTaskPriority::LateThisFrame] = "LateThisFrame";
  szTaskPriorityNames[wdTaskPriority::EarlyNextFrame] = "EarlyNextFrame";
  szTaskPriorityNames[wdTaskPriority::NextFrame] = "NextFrame";
  szTaskPriorityNames[wdTaskPriority::LateNextFrame] = "LateNextFrame";
  szTaskPriorityNames[wdTaskPriority::In2Frames] = "In 2 Frames";
  szTaskPriorityNames[wdTaskPriority::In2Frames] = "In 3 Frames";
  szTaskPriorityNames[wdTaskPriority::In4Frames] = "In 4 Frames";
  szTaskPriorityNames[wdTaskPriority::In5Frames] = "In 5 Frames";
  szTaskPriorityNames[wdTaskPriority::In6Frames] = "In 6 Frames";
  szTaskPriorityNames[wdTaskPriority::In7Frames] = "In 7 Frames";
  szTaskPriorityNames[wdTaskPriority::In8Frames] = "In 8 Frames";
  szTaskPriorityNames[wdTaskPriority::In9Frames] = "In 9 Frames";
  szTaskPriorityNames[wdTaskPriority::LongRunningHighPriority] = "LongRunningHighPriority";
  szTaskPriorityNames[wdTaskPriority::LongRunning] = "LongRunning";
  szTaskPriorityNames[wdTaskPriority::FileAccessHighPriority] = "FileAccessHighPriority";
  szTaskPriorityNames[wdTaskPriority::FileAccess] = "FileAccess";
  szTaskPriorityNames[wdTaskPriority::ThisFrameMainThread] = "ThisFrameMainThread";
  szTaskPriorityNames[wdTaskPriority::SomeFrameMainThread] = "SomeFrameMainThread";

  for (wdUInt32 g = 0; g < s_pState->m_TaskGroups.GetCount(); ++g)
  {
    const wdTaskGroup& tg = s_pState->m_TaskGroups[g];

    if (!tg.m_bInUse)
      continue;

    title.Format("Group {}", g);

    const wdDGMLGraph::NodeId taskGroupId = ref_graph.AddGroup(title, wdDGMLGraph::GroupType::Expanded, &taskGroupND);
    groupNodeIds[&tg] = taskGroupId;

    ref_graph.AddNodeProperty(taskGroupId, startedByUserId, tg.m_bStartedByUser ? "true" : "false");
    ref_graph.AddNodeProperty(taskGroupId, priorityId, szTaskPriorityNames[tg.m_Priority]);
    ref_graph.AddNodeProperty(taskGroupId, activeDepsId, wdFmt("{}", tg.m_iNumActiveDependencies));

    for (wdUInt32 t = 0; t < tg.m_Tasks.GetCount(); ++t)
    {
      const wdTask& task = *tg.m_Tasks[t];
      const wdDGMLGraph::NodeId taskNodeId = ref_graph.AddNode(task.m_sTaskName, &taskNodeND);

      ref_graph.AddNodeToGroup(taskNodeId, taskGroupId);

      ref_graph.AddNodeProperty(taskNodeId, scheduledId, task.m_bTaskIsScheduled ? "true" : "false");
      ref_graph.AddNodeProperty(taskNodeId, finishedId, task.IsTaskFinished() ? "true" : "false");

      tmp.Format("{}", task.GetMultiplicity());
      ref_graph.AddNodeProperty(taskNodeId, multiplicityId, tmp);

      tmp.Format("{}", task.m_iRemainingRuns);
      ref_graph.AddNodeProperty(taskNodeId, remainingRunsId, tmp);
    }
  }

  for (wdUInt32 g = 0; g < s_pState->m_TaskGroups.GetCount(); ++g)
  {
    const wdTaskGroup& tg = s_pState->m_TaskGroups[g];

    if (!tg.m_bInUse)
      continue;

    const wdDGMLGraph::NodeId ownNodeId = groupNodeIds[&tg];

    for (const wdTaskGroupID& dependsOn : tg.m_DependsOnGroups)
    {
      wdDGMLGraph::NodeId otherNodeId;

      // filter out already fulfilled dependencies
      if (dependsOn.m_pTaskGroup->m_uiGroupCounter != dependsOn.m_uiGroupCounter)
        continue;

      // filter out already fulfilled dependencies
      if (!groupNodeIds.TryGetValue(dependsOn.m_pTaskGroup, otherNodeId))
        continue;

      WD_ASSERT_DEBUG(otherNodeId != ownNodeId, "");

      ref_graph.AddConnection(otherNodeId, ownNodeId);
    }
  }
}

void wdTaskSystem::WriteStateSnapshotToFile(const char* szPath /*= nullptr*/)
{
  wdStringBuilder sPath = szPath;

  if (sPath.IsEmpty())
  {
    sPath = ":appdata/TaskGraphs/";

    const wdDateTime dt = wdTimestamp::CurrentTimestamp();

    sPath.AppendFormat("{0}-{1}-{2}_{3}-{4}-{5}-{6}", dt.GetYear(), wdArgU(dt.GetMonth(), 2, true), wdArgU(dt.GetDay(), 2, true), wdArgU(dt.GetHour(), 2, true), wdArgU(dt.GetMinute(), 2, true), wdArgU(dt.GetSecond(), 2, true), wdArgU(dt.GetMicroseconds() / 1000, 3, true));

    sPath.ChangeFileExtension("dgml");
  }

  wdDGMLGraph graph;
  wdTaskSystem::WriteStateSnapshotToDGML(graph);

  wdDGMLGraphWriter::WriteGraphToFile(sPath, graph).IgnoreResult();

  wdStringBuilder absPath;
  wdFileSystem::ResolvePath(sPath, &absPath, nullptr).IgnoreResult();
  wdLog::Info("Task graph snapshot saved to '{}'", absPath);
}


WD_STATICLINK_FILE(Foundation, Foundation_Threading_Implementation_TaskSystemUtils);
