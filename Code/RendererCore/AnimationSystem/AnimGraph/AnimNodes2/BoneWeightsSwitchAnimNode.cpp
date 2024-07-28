#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes2/BoneWeightsSwitchAnimNode.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSwitchBoneWeightsAnimNode, 1, nsRTTIDefaultAllocator<nsSwitchBoneWeightsAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("InIndex", m_InIndex)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("WeightsCount", m_uiWeightsCount)->AddAttributes(new nsNoTemporaryTransactionsAttribute(), new nsDynamicPinAttribute(), new nsDefaultValueAttribute(2)),
    NS_ARRAY_MEMBER_PROPERTY("InWeights", m_InWeights)->AddAttributes(new nsHiddenAttribute(), new nsDynamicPinAttribute("WeightsCount")),
    NS_MEMBER_PROPERTY("OutWeights", m_OutWeights)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Weights"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Teal)),
    new nsTitleAttribute("Bone Weights Switch"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsSwitchBoneWeightsAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_uiWeightsCount;

  NS_SUCCEED_OR_RETURN(m_InIndex.Serialize(stream));
  NS_SUCCEED_OR_RETURN(stream.WriteArray(m_InWeights));
  NS_SUCCEED_OR_RETURN(m_OutWeights.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSwitchBoneWeightsAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);
  NS_IGNORE_UNUSED(version);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_uiWeightsCount;

  NS_SUCCEED_OR_RETURN(m_InIndex.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(stream.ReadArray(m_InWeights));
  NS_SUCCEED_OR_RETURN(m_OutWeights.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSwitchBoneWeightsAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_OutWeights.IsConnected() || !m_InIndex.IsConnected() || m_InWeights.IsEmpty())
    return;

  const nsInt32 iIndex = nsMath::Clamp((nsInt32)m_InIndex.GetNumber(ref_graph), 0, (nsInt32)m_InWeights.GetCount() - 1);

  if (!m_InWeights[iIndex].IsConnected())
    return;

  m_OutWeights.SetWeights(ref_graph, m_InWeights[iIndex].GetWeights(ref_controller, ref_graph));
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes2_BoneWeightsSwitchAnimNode);
