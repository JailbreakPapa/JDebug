#include <Foundation/FoundationPCH.h>

#include <Foundation/IO/FileSystem/FileSystem.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Threading/Implementation/TaskGroup.h>
#include <Foundation/Threading/Implementation/TaskSystemState.h>
#include <Foundation/Threading/TaskSystem.h>
#include <Foundation/Time/Timestamp.h>
#include <Foundation/Utilities/DGMLWriter.h>

const char* nsWorkerThreadType::GetThreadTypeName(nsWorkerThreadType::Enum threadType)
{
  switch (threadType)
  {
    case nsWorkerThreadType::ShortTasks:
      return "Short Task";

    case nsWorkerThreadType::LongTasks:
      return "Long Task";

    case nsWorkerThreadType::FileAccess:
      return "File Access";

    default:
      NS_REPORT_FAILURE("Invalid Thread Type");
      return "unknown";
  }
}

void nsTaskSystem::WriteStateSnapshotToDGML(nsDGMLGraph& ref_graph)
{
  NS_LOCK(s_TaskSystemMutex);

  nsHashTable<const nsTaskGroup*, nsDGMLGraph::NodeId> groupNodeIds;

  nsStringBuilder title, tmp;

  nsDGMLGraph::NodeDesc taskGroupND;
  taskGroupND.m_Color = nsColor::CornflowerBlue;
  taskGroupND.m_Shape = nsDGMLGraph::NodeShape::Rectangle;

  nsDGMLGraph::NodeDesc taskNodeND;
  taskNodeND.m_Color = nsColor::OrangeRed;
  taskNodeND.m_Shape = nsDGMLGraph::NodeShape::RoundedRectangle;

  const nsDGMLGraph::PropertyId startedByUserId = ref_graph.AddPropertyType("StartByUser");
  const nsDGMLGraph::PropertyId activeDepsId = ref_graph.AddPropertyType("ActiveDependencies");
  const nsDGMLGraph::PropertyId scheduledId = ref_graph.AddPropertyType("Scheduled");
  const nsDGMLGraph::PropertyId finishedId = ref_graph.AddPropertyType("Finished");
  const nsDGMLGraph::PropertyId multiplicityId = ref_graph.AddPropertyType("Multiplicity");
  const nsDGMLGraph::PropertyId remainingRunsId = ref_graph.AddPropertyType("RemainingRuns");
  const nsDGMLGraph::PropertyId priorityId = ref_graph.AddPropertyType("GroupPriority");

  const char* szTaskPriorityNames[nsTaskPriority::ENUM_COUNT] = {};
  szTaskPriorityNames[nsTaskPriority::EarlyThisFrame] = "EarlyThisFrame";
  szTaskPriorityNames[nsTaskPriority::ThisFrame] = "ThisFrame";
  szTaskPriorityNames[nsTaskPriority::LateThisFrame] = "LateThisFrame";
  szTaskPriorityNames[nsTaskPriority::EarlyNextFrame] = "EarlyNextFrame";
  szTaskPriorityNames[nsTaskPriority::NextFrame] = "NextFrame";
  szTaskPriorityNames[nsTaskPriority::LateNextFrame] = "LateNextFrame";
  szTaskPriorityNames[nsTaskPriority::In2Frames] = "In 2 Frames";
  szTaskPriorityNames[nsTaskPriority::In3Frames] = "In 3 Frames";
  szTaskPriorityNames[nsTaskPriority::In4Frames] = "In 4 Frames";
  szTaskPriorityNames[nsTaskPriority::In5Frames] = "In 5 Frames";
  szTaskPriorityNames[nsTaskPriority::In6Frames] = "In 6 Frames";
  szTaskPriorityNames[nsTaskPriority::In7Frames] = "In 7 Frames";
  szTaskPriorityNames[nsTaskPriority::In8Frames] = "In 8 Frames";
  szTaskPriorityNames[nsTaskPriority::In9Frames] = "In 9 Frames";
  szTaskPriorityNames[nsTaskPriority::LongRunningHighPriority] = "LongRunningHighPriority";
  szTaskPriorityNames[nsTaskPriority::LongRunning] = "LongRunning";
  szTaskPriorityNames[nsTaskPriority::FileAccessHighPriority] = "FileAccessHighPriority";
  szTaskPriorityNames[nsTaskPriority::FileAccess] = "FileAccess";
  szTaskPriorityNames[nsTaskPriority::ThisFrameMainThread] = "ThisFrameMainThread";
  szTaskPriorityNames[nsTaskPriority::SomeFrameMainThread] = "SomeFrameMainThread";

  for (nsUInt32 g = 0; g < s_pState->m_TaskGroups.GetCount(); ++g)
  {
    const nsTaskGroup& tg = s_pState->m_TaskGroups[g];

    if (!tg.m_bInUse)
      continue;

    title.SetFormat("Group {}", g);

    const nsDGMLGraph::NodeId taskGroupId = ref_graph.AddGroup(title, nsDGMLGraph::GroupType::Expanded, &taskGroupND);
    groupNodeIds[&tg] = taskGroupId;

    ref_graph.AddNodeProperty(taskGroupId, startedByUserId, tg.m_bStartedByUser ? "true" : "false");
    ref_graph.AddNodeProperty(taskGroupId, priorityId, szTaskPriorityNames[tg.m_Priority]);
    ref_graph.AddNodeProperty(taskGroupId, activeDepsId, nsFmt("{}", tg.m_iNumActiveDependencies));

    for (nsUInt32 t = 0; t < tg.m_Tasks.GetCount(); ++t)
    {
      const nsTask& task = *tg.m_Tasks[t];
      const nsDGMLGraph::NodeId taskNodeId = ref_graph.AddNode(task.m_sTaskName, &taskNodeND);

      ref_graph.AddNodeToGroup(taskNodeId, taskGroupId);

      ref_graph.AddNodeProperty(taskNodeId, scheduledId, task.m_bTaskIsScheduled ? "true" : "false");
      ref_graph.AddNodeProperty(taskNodeId, finishedId, task.IsTaskFinished() ? "true" : "false");

      tmp.SetFormat("{}", task.GetMultiplicity());
      ref_graph.AddNodeProperty(taskNodeId, multiplicityId, tmp);

      tmp.SetFormat("{}", task.m_iRemainingRuns);
      ref_graph.AddNodeProperty(taskNodeId, remainingRunsId, tmp);
    }
  }

  for (nsUInt32 g = 0; g < s_pState->m_TaskGroups.GetCount(); ++g)
  {
    const nsTaskGroup& tg = s_pState->m_TaskGroups[g];

    if (!tg.m_bInUse)
      continue;

    const nsDGMLGraph::NodeId ownNodeId = groupNodeIds[&tg];

    for (const nsTaskGroupID& dependsOn : tg.m_DependsOnGroups)
    {
      nsDGMLGraph::NodeId otherNodeId;

      // filter out already fulfilled dependencies
      if (dependsOn.m_pTaskGroup->m_uiGroupCounter != dependsOn.m_uiGroupCounter)
        continue;

      // filter out already fulfilled dependencies
      if (!groupNodeIds.TryGetValue(dependsOn.m_pTaskGroup, otherNodeId))
        continue;

      NS_ASSERT_DEBUG(otherNodeId != ownNodeId, "");

      ref_graph.AddConnection(otherNodeId, ownNodeId);
    }
  }
}

void nsTaskSystem::WriteStateSnapshotToFile(const char* szPath /*= nullptr*/)
{
  nsStringBuilder sPath = szPath;

  if (sPath.IsEmpty())
  {
    sPath = ":appdata/TaskGraphs/";

    const nsDateTime dt = nsDateTime::MakeFromTimestamp(nsTimestamp::CurrentTimestamp());

    sPath.AppendFormat("{0}-{1}-{2}_{3}-{4}-{5}-{6}", dt.GetYear(), nsArgU(dt.GetMonth(), 2, true), nsArgU(dt.GetDay(), 2, true), nsArgU(dt.GetHour(), 2, true), nsArgU(dt.GetMinute(), 2, true), nsArgU(dt.GetSecond(), 2, true), nsArgU(dt.GetMicroseconds() / 1000, 3, true));

    sPath.ChangeFileExtension("dgml");
  }

  nsDGMLGraph graph;
  nsTaskSystem::WriteStateSnapshotToDGML(graph);

  nsDGMLGraphWriter::WriteGraphToFile(sPath, graph).IgnoreResult();

  nsStringBuilder absPath;
  nsFileSystem::ResolvePath(sPath, &absPath, nullptr).IgnoreResult();
  nsLog::Info("Task graph snapshot saved to '{}'", absPath);
}
