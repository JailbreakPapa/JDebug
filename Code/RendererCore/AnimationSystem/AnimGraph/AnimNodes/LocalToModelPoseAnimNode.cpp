#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/LocalToModelPoseAnimNode.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

//// clang-format off
// NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLocalToModelPoseAnimNode, 1, nsRTTIDefaultAllocator<nsLocalToModelPoseAnimNode>)
//{
//   NS_BEGIN_PROPERTIES
//   {
//     NS_MEMBER_PROPERTY("LocalPose", m_LocalPosePin)->AddAttributes(new nsHiddenAttribute),
//     NS_MEMBER_PROPERTY("ModelPose", m_ModelPosePin)->AddAttributes(new nsHiddenAttribute),
//   }
//   NS_END_PROPERTIES;
//   NS_BEGIN_ATTRIBUTES
//   {
//     new nsCategoryAttribute("Pose Processing"),
//     new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Blue)),
//     new nsTitleAttribute("Local To Model Space"),
//   }
//   NS_END_ATTRIBUTES;
// }
// NS_END_DYNAMIC_REFLECTED_TYPE;
//// clang-format on
//
// nsLocalToModelPoseAnimNode::nsLocalToModelPoseAnimNode() = default;
// nsLocalToModelPoseAnimNode::~nsLocalToModelPoseAnimNode() = default;
//
// nsResult nsLocalToModelPoseAnimNode::SerializeNode(nsStreamWriter& stream) const
//{
//  stream.WriteVersion(1);
//
//  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));
//
//  NS_SUCCEED_OR_RETURN(m_LocalPosePin.Serialize(stream));
//  NS_SUCCEED_OR_RETURN(m_ModelPosePin.Serialize(stream));
//
//  return NS_SUCCESS;
//}
//
// nsResult nsLocalToModelPoseAnimNode::DeserializeNode(nsStreamReader& stream)
//{
//  stream.ReadVersion(1);
//
//  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));
//
//  NS_SUCCEED_OR_RETURN(m_LocalPosePin.Deserialize(stream));
//  NS_SUCCEED_OR_RETURN(m_ModelPosePin.Deserialize(stream));
//
//  return NS_SUCCESS;
//}
//
// void nsLocalToModelPoseAnimNode::Step(nsAnimGraphExecutor& executor, nsAnimGraphInstance& graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
//{
//  if (!m_LocalPosePin.IsConnected() || !m_ModelPosePin.IsConnected())
//    return;
//
//  auto pLocalPose = m_LocalPosePin.GetPose(graph);
//  if (pLocalPose == nullptr)
//    return;
//
//  nsAnimGraphPinDataModelTransforms* pModelTransform = graph.AddPinDataModelTransforms();
//
//  if (pLocalPose->m_bUseRootMotion)
//  {
//    pModelTransform->m_bUseRootMotion = true;
//    pModelTransform->m_vRootMotion = pLocalPose->m_vRootMotion;
//  }
//
//  auto& cmd = graph.GetPoseGenerator().AllocCommandLocalToModelPose();
//  cmd.m_Inputs.PushBack(m_LocalPosePin.GetPose(graph)->m_CommandID);
//
//  pModelTransform->m_CommandID = cmd.GetCommandID();
//
//  m_ModelPosePin.SetPose(graph, pModelTransform);
//}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_LocalToModelPoseAnimNode);
