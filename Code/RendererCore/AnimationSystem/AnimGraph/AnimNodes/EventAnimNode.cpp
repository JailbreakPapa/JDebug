#include <RendererCore/RendererCorePCH.h>

#include <Core/Messages/CommonMessages.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/EventAnimNode.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSendEventAnimNode, 1, nsRTTIDefaultAllocator<nsSendEventAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("EventName", GetEventName, SetEventName),

    NS_MEMBER_PROPERTY("InActivate", m_InActivate)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Events"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Orange)),
    new nsTitleAttribute("Send Event: '{EventName}'"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsSendEventAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sEventName;

  NS_SUCCEED_OR_RETURN(m_InActivate.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSendEventAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sEventName;

  NS_SUCCEED_OR_RETURN(m_InActivate.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSendEventAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (m_sEventName.IsEmpty())
    return;

  if (!m_InActivate.IsTriggered(ref_graph))
    return;

  nsMsgGenericEvent msg;
  msg.m_sMessage = m_sEventName;

  pTarget->SendEventMessage(msg, nullptr);
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_EventAnimNode);
