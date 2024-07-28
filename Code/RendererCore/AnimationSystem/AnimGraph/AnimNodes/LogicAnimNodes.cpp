#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/LogicAnimNodes.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLogicAndAnimNode, 1, nsRTTIDefaultAllocator<nsLogicAndAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("BoolCount", m_uiBoolCount)->AddAttributes(new nsNoTemporaryTransactionsAttribute(), new nsDynamicPinAttribute(), new nsDefaultValueAttribute(2)),
    NS_ARRAY_MEMBER_PROPERTY("InBool", m_InBool)->AddAttributes(new nsHiddenAttribute(), new nsDynamicPinAttribute("BoolCount")),
    NS_MEMBER_PROPERTY("OutIsTrue", m_OutIsTrue)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutIsFalse", m_OutIsFalse)->AddAttributes(new nsHiddenAttribute),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Logic"),
    new nsTitleAttribute("AND"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsLogicAndAnimNode::nsLogicAndAnimNode() = default;
nsLogicAndAnimNode::~nsLogicAndAnimNode() = default;

nsResult nsLogicAndAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_uiBoolCount;
  NS_SUCCEED_OR_RETURN(stream.WriteArray(m_InBool));
  NS_SUCCEED_OR_RETURN(m_OutIsTrue.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutIsFalse.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsLogicAndAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_uiBoolCount;
  NS_SUCCEED_OR_RETURN(stream.ReadArray(m_InBool));
  NS_SUCCEED_OR_RETURN(m_OutIsTrue.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutIsFalse.Deserialize(stream));

  return NS_SUCCESS;
}

void nsLogicAndAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  bool res = true;

  for (const auto& pin : m_InBool)
  {
    if (!pin.GetBool(ref_graph, true))
    {
      res = false;
      break;
    }
  }

  m_OutIsTrue.SetBool(ref_graph, res);
  m_OutIsFalse.SetBool(ref_graph, !res);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLogicEventAndAnimNode, 1, nsRTTIDefaultAllocator<nsLogicEventAndAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("InActivate", m_InActivate)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("InBool", m_InBool)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutOnActivated", m_OutOnActivated)->AddAttributes(new nsHiddenAttribute),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Logic"),
    new nsTitleAttribute("Event AND"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsLogicEventAndAnimNode::nsLogicEventAndAnimNode() = default;
nsLogicEventAndAnimNode::~nsLogicEventAndAnimNode() = default;

nsResult nsLogicEventAndAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_InActivate.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InBool.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnActivated.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsLogicEventAndAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_InActivate.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InBool.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnActivated.Deserialize(stream));

  return NS_SUCCESS;
}

void nsLogicEventAndAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (m_InActivate.IsTriggered(ref_graph) && m_InBool.GetBool(ref_graph))
  {
    m_OutOnActivated.SetTriggered(ref_graph);
  }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLogicOrAnimNode, 1, nsRTTIDefaultAllocator<nsLogicOrAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("BoolCount", m_uiBoolCount)->AddAttributes(new nsNoTemporaryTransactionsAttribute(), new nsDynamicPinAttribute(), new nsDefaultValueAttribute(2)),
    NS_ARRAY_MEMBER_PROPERTY("InBool", m_InBool)->AddAttributes(new nsHiddenAttribute(), new nsDynamicPinAttribute("BoolCount")),
    NS_MEMBER_PROPERTY("OutIsTrue", m_OutIsTrue)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutIsFalse", m_OutIsFalse)->AddAttributes(new nsHiddenAttribute),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Logic"),
    new nsTitleAttribute("OR"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsLogicOrAnimNode::nsLogicOrAnimNode() = default;
nsLogicOrAnimNode::~nsLogicOrAnimNode() = default;

nsResult nsLogicOrAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_uiBoolCount;
  NS_SUCCEED_OR_RETURN(stream.WriteArray(m_InBool));
  NS_SUCCEED_OR_RETURN(m_OutIsTrue.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutIsFalse.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsLogicOrAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_uiBoolCount;
  NS_SUCCEED_OR_RETURN(stream.ReadArray(m_InBool));
  NS_SUCCEED_OR_RETURN(m_OutIsTrue.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutIsFalse.Deserialize(stream));

  return NS_SUCCESS;
}

void nsLogicOrAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  bool res = false;

  for (const auto& pin : m_InBool)
  {
    if (!pin.GetBool(ref_graph, true))
    {
      res = true;
      break;
    }
  }

  m_OutIsTrue.SetBool(ref_graph, res);
  m_OutIsFalse.SetBool(ref_graph, !res);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLogicNotAnimNode, 1, nsRTTIDefaultAllocator<nsLogicNotAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("InBool", m_InBool)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutBool", m_OutBool)->AddAttributes(new nsHiddenAttribute),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Logic"),
    new nsTitleAttribute("NOT"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsLogicNotAnimNode::nsLogicNotAnimNode() = default;
nsLogicNotAnimNode::~nsLogicNotAnimNode() = default;

nsResult nsLogicNotAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_InBool.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutBool.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsLogicNotAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_InBool.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutBool.Deserialize(stream));

  return NS_SUCCESS;
}

void nsLogicNotAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  const bool value = !m_InBool.GetBool(ref_graph);

  m_OutBool.SetBool(ref_graph, !value);
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_LogicAnimNodes);
