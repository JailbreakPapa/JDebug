#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/MathAnimNodes.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsMathExpressionAnimNode, 1, nsRTTIDefaultAllocator<nsMathExpressionAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("Expression", GetExpression, SetExpression)->AddAttributes(new nsDefaultValueAttribute("a*a + (b-c) / abs(d)")),
    NS_MEMBER_PROPERTY("a", m_ValueAPin)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("b", m_ValueBPin)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("c", m_ValueCPin)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("d", m_ValueDPin)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("Result", m_ResultPin)->AddAttributes(new nsHiddenAttribute),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Math"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Lime)),
    new nsTitleAttribute("= {Expression}"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsMathExpressionAnimNode::nsMathExpressionAnimNode() = default;
nsMathExpressionAnimNode::~nsMathExpressionAnimNode() = default;

void nsMathExpressionAnimNode::SetExpression(nsString sExpr)
{
  m_sExpression = sExpr;
}

nsString nsMathExpressionAnimNode::GetExpression() const
{
  return m_sExpression;
}

nsResult nsMathExpressionAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sExpression;
  NS_SUCCEED_OR_RETURN(m_ValueAPin.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_ValueBPin.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_ValueCPin.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_ValueDPin.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_ResultPin.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsMathExpressionAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sExpression;
  NS_SUCCEED_OR_RETURN(m_ValueAPin.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_ValueBPin.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_ValueCPin.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_ValueDPin.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_ResultPin.Deserialize(stream));

  return NS_SUCCESS;
}

static nsHashedString s_sA = nsMakeHashedString("a");
static nsHashedString s_sB = nsMakeHashedString("b");
static nsHashedString s_sC = nsMakeHashedString("c");
static nsHashedString s_sD = nsMakeHashedString("d");

void nsMathExpressionAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  if (pInstance->m_mExpression.GetExpressionString().IsEmpty())
  {
    pInstance->m_mExpression.Reset(m_sExpression);
  }

  if (!pInstance->m_mExpression.IsValid())
  {
    m_ResultPin.SetNumber(ref_graph, 0);
    return;
  }

  nsMathExpression::Input inputs[] =
    {
      {s_sA, static_cast<float>(m_ValueAPin.GetNumber(ref_graph))},
      {s_sB, static_cast<float>(m_ValueBPin.GetNumber(ref_graph))},
      {s_sC, static_cast<float>(m_ValueCPin.GetNumber(ref_graph))},
      {s_sD, static_cast<float>(m_ValueDPin.GetNumber(ref_graph))},
    };

  float result = pInstance->m_mExpression.Evaluate(inputs);
  m_ResultPin.SetNumber(ref_graph, result);
}

bool nsMathExpressionAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCompareNumberAnimNode, 1, nsRTTIDefaultAllocator<nsCompareNumberAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("ReferenceValue", m_fReferenceValue),
    NS_ENUM_MEMBER_PROPERTY("Comparison", nsComparisonOperator, m_Comparison),

    NS_MEMBER_PROPERTY("OutIsTrue", m_OutIsTrue)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("InNumber", m_InNumber)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("InReference", m_InReference)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Logic"),
    new nsTitleAttribute("Compare: Number {Comparison} {ReferenceValue}"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Lime)),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsCompareNumberAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_fReferenceValue;
  stream << m_Comparison;

  NS_SUCCEED_OR_RETURN(m_InNumber.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InReference.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutIsTrue.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsCompareNumberAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_fReferenceValue;
  stream >> m_Comparison;

  NS_SUCCEED_OR_RETURN(m_InNumber.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InReference.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutIsTrue.Deserialize(stream));

  return NS_SUCCESS;
}

void nsCompareNumberAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  const bool bIsTrue = nsComparisonOperator::Compare<double>(m_Comparison, m_InNumber.GetNumber(ref_graph), m_InReference.GetNumber(ref_graph, m_fReferenceValue));

  m_OutIsTrue.SetBool(ref_graph, bIsTrue);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBoolToNumberAnimNode, 1, nsRTTIDefaultAllocator<nsBoolToNumberAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("FalseValue", m_fFalseValue)->AddAttributes(new nsDefaultValueAttribute(0.0)),
    NS_MEMBER_PROPERTY("TrueValue", m_fTrueValue)->AddAttributes(new nsDefaultValueAttribute(1.0)),
    NS_MEMBER_PROPERTY("InValue", m_InValue)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutNumber", m_OutNumber)->AddAttributes(new nsHiddenAttribute),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Logic"),
    new nsTitleAttribute("Bool To Number"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBoolToNumberAnimNode::nsBoolToNumberAnimNode() = default;
nsBoolToNumberAnimNode::~nsBoolToNumberAnimNode() = default;

nsResult nsBoolToNumberAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_fFalseValue;
  stream << m_fTrueValue;

  NS_SUCCEED_OR_RETURN(m_InValue.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutNumber.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsBoolToNumberAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_fFalseValue;
  stream >> m_fTrueValue;

  NS_SUCCEED_OR_RETURN(m_InValue.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutNumber.Deserialize(stream));

  return NS_SUCCESS;
}

void nsBoolToNumberAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  m_OutNumber.SetNumber(ref_graph, m_InValue.GetBool(ref_graph) ? m_fTrueValue : m_fFalseValue);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBoolToTriggerAnimNode, 1, nsRTTIDefaultAllocator<nsBoolToTriggerAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("InValue", m_InValue)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutOnTrue", m_OutOnTrue)->AddAttributes(new nsHiddenAttribute),
    NS_MEMBER_PROPERTY("OutOnFalse", m_OutOnFalse)->AddAttributes(new nsHiddenAttribute),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Logic"),
    new nsTitleAttribute("Bool To Event"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBoolToTriggerAnimNode::nsBoolToTriggerAnimNode() = default;
nsBoolToTriggerAnimNode::~nsBoolToTriggerAnimNode() = default;

nsResult nsBoolToTriggerAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_InValue.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnTrue.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFalse.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsBoolToTriggerAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  NS_SUCCEED_OR_RETURN(m_InValue.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnTrue.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFalse.Deserialize(stream));

  return NS_SUCCESS;
}

bool nsBoolToTriggerAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}

void nsBoolToTriggerAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  const bool bIsTrueNow = m_InValue.GetBool(ref_graph);
  const nsInt8 iIsTrueNow = bIsTrueNow ? 1 : 0;

  // we use a tri-state bool here to ensure that OnTrue or OnFalse get fired right away
  if (pInstance->m_iIsTrue != iIsTrueNow)
  {
    pInstance->m_iIsTrue = iIsTrueNow;

    if (bIsTrueNow)
    {
      m_OutOnTrue.SetTriggered(ref_graph);
    }
    else
    {
      m_OutOnFalse.SetTriggered(ref_graph);
    }
  }
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_MathAnimNodes);
