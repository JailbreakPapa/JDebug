#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/RootMotionAnimNodes.h>

// clang-format off
 NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsRootRotationAnimNode, 1, nsRTTIDefaultAllocator<nsRootRotationAnimNode>)
{
   NS_BEGIN_PROPERTIES
   {
     NS_MEMBER_PROPERTY("InRotateX", m_InRotateX)->AddAttributes(new nsHiddenAttribute),
     NS_MEMBER_PROPERTY("InRotateY", m_InRotateY)->AddAttributes(new nsHiddenAttribute),
     NS_MEMBER_PROPERTY("InRotateZ", m_InRotateZ)->AddAttributes(new nsHiddenAttribute),
   }
   NS_END_PROPERTIES;
   NS_BEGIN_ATTRIBUTES
   {
     new nsCategoryAttribute("Output"),
     new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Grape)),
     new nsTitleAttribute("Root Rotation"),
   }
   NS_END_ATTRIBUTES;
 }
 NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsRootRotationAnimNode::nsRootRotationAnimNode() = default;
nsRootRotationAnimNode::~nsRootRotationAnimNode() = default;

nsResult nsRootRotationAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_InRotateX.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InRotateY.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InRotateZ.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsRootRotationAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_InRotateX.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InRotateY.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InRotateZ.Deserialize(stream));

  return NS_SUCCESS;
}

void nsRootRotationAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  nsVec3 vRootMotion = nsVec3::MakeZero();
  nsAngle rootRotationX;
  nsAngle rootRotationY;
  nsAngle rootRotationZ;

  ref_controller.GetRootMotion(vRootMotion, rootRotationX, rootRotationY, rootRotationZ);

  if (m_InRotateX.IsConnected())
  {
    rootRotationX += nsAngle::MakeFromDegree(static_cast<float>(m_InRotateX.GetNumber(ref_graph)));
  }
  if (m_InRotateY.IsConnected())
  {
    rootRotationY += nsAngle::MakeFromDegree(static_cast<float>(m_InRotateY.GetNumber(ref_graph)));
  }
  if (m_InRotateZ.IsConnected())
  {
    rootRotationZ += nsAngle::MakeFromDegree(static_cast<float>(m_InRotateZ.GetNumber(ref_graph)));
  }

  ref_controller.SetRootMotion(vRootMotion, rootRotationX, rootRotationY, rootRotationZ);
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_RootMotionAnimNodes);
