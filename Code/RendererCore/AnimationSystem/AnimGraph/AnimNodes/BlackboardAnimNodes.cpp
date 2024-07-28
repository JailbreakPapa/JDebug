#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/BlackboardAnimNodes.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSetBlackboardNumberAnimNode, 1, nsRTTIDefaultAllocator<nsSetBlackboardNumberAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("BlackboardEntry", GetBlackboardEntry, SetBlackboardEntry),
    NS_MEMBER_PROPERTY("Number", m_fNumber),

    NS_MEMBER_PROPERTY("InActivate", m_InActivate)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("InNumber", m_InNumber)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsTitleAttribute("Set Number: '{BlackboardEntry}' to {Number}"),
    new nsCategoryAttribute("Blackboard"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Red)),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsSetBlackboardNumberAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sBlackboardEntry;
  stream << m_fNumber;

  NS_SUCCEED_OR_RETURN(m_InActivate.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InNumber.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSetBlackboardNumberAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sBlackboardEntry;
  stream >> m_fNumber;

  NS_SUCCEED_OR_RETURN(m_InActivate.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InNumber.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSetBlackboardNumberAnimNode::SetBlackboardEntry(const char* szFile)
{
  m_sBlackboardEntry.Assign(szFile);
}

const char* nsSetBlackboardNumberAnimNode::GetBlackboardEntry() const
{
  return m_sBlackboardEntry.GetData();
}

void nsSetBlackboardNumberAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_InActivate.IsTriggered(ref_graph))
    return;

  auto pBlackboard = ref_controller.GetBlackboard();
  if (pBlackboard == nullptr)
    return;

  pBlackboard->SetEntryValue(m_sBlackboardEntry, m_InNumber.GetNumber(ref_graph, m_fNumber));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsGetBlackboardNumberAnimNode, 1, nsRTTIDefaultAllocator<nsGetBlackboardNumberAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("BlackboardEntry", GetBlackboardEntry, SetBlackboardEntry),

    NS_MEMBER_PROPERTY("OutNumber", m_OutNumber)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Blackboard"),
    new nsTitleAttribute("Get Number: '{BlackboardEntry}'"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Lime)),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsGetBlackboardNumberAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sBlackboardEntry;

  NS_SUCCEED_OR_RETURN(m_OutNumber.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsGetBlackboardNumberAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sBlackboardEntry;

  NS_SUCCEED_OR_RETURN(m_OutNumber.Deserialize(stream));

  return NS_SUCCESS;
}

void nsGetBlackboardNumberAnimNode::SetBlackboardEntry(const char* szFile)
{
  m_sBlackboardEntry.Assign(szFile);
}

const char* nsGetBlackboardNumberAnimNode::GetBlackboardEntry() const
{
  return m_sBlackboardEntry.GetData();
}

void nsGetBlackboardNumberAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  auto pBlackboard = ref_controller.GetBlackboard();
  if (pBlackboard == nullptr)
    return;

  if (m_sBlackboardEntry.IsEmpty())
    return;

  nsVariant value = pBlackboard->GetEntryValue(m_sBlackboardEntry);

  if (!value.IsValid() || !value.IsNumber())
  {
    nsLog::Warning("AnimController::GetBlackboardNumber: '{}' doesn't exist or isn't a number type.", m_sBlackboardEntry);
    return;
  }

  m_OutNumber.SetNumber(ref_graph, value.ConvertTo<double>());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCompareBlackboardNumberAnimNode, 1, nsRTTIDefaultAllocator<nsCompareBlackboardNumberAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("BlackboardEntry", GetBlackboardEntry, SetBlackboardEntry),
    NS_MEMBER_PROPERTY("ReferenceValue", m_fReferenceValue),
    NS_ENUM_MEMBER_PROPERTY("Comparison", nsComparisonOperator, m_Comparison),

    NS_MEMBER_PROPERTY("OutOnTrue", m_OutOnTrue)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("OutOnFalse", m_OutOnFalse)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("OutIsTrue", m_OutIsTrue)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Blackboard"),
    new nsTitleAttribute("Check: '{BlackboardEntry}' {Comparison} {ReferenceValue}"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Lime)),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsCompareBlackboardNumberAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(2);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sBlackboardEntry;
  stream << m_fReferenceValue;
  stream << m_Comparison;

  NS_SUCCEED_OR_RETURN(m_OutOnTrue.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFalse.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutIsTrue.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsCompareBlackboardNumberAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(2);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sBlackboardEntry;
  stream >> m_fReferenceValue;
  stream >> m_Comparison;

  NS_SUCCEED_OR_RETURN(m_OutOnTrue.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFalse.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutIsTrue.Deserialize(stream));

  return NS_SUCCESS;
}

void nsCompareBlackboardNumberAnimNode::SetBlackboardEntry(const char* szFile)
{
  m_sBlackboardEntry.Assign(szFile);
}

const char* nsCompareBlackboardNumberAnimNode::GetBlackboardEntry() const
{
  return m_sBlackboardEntry.GetData();
}

void nsCompareBlackboardNumberAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  auto pBlackboard = ref_controller.GetBlackboard();
  if (pBlackboard == nullptr)
    return;

  if (m_sBlackboardEntry.IsEmpty())
    return;

  const nsVariant value = pBlackboard->GetEntryValue(m_sBlackboardEntry);

  if (!value.IsValid() || !value.IsNumber())
  {
    nsLog::Warning("AnimController::CompareBlackboardNumber: '{}' doesn't exist or isn't a number type.", m_sBlackboardEntry);
    return;
  }

  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  const double fValue = value.ConvertTo<double>();
  const bool bIsTrueNow = nsComparisonOperator::Compare(m_Comparison, fValue, m_fReferenceValue);
  const nsInt8 iIsTrueNow = bIsTrueNow ? 1 : 0;

  m_OutIsTrue.SetBool(ref_graph, bIsTrueNow);

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

bool nsCompareBlackboardNumberAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsCheckBlackboardBoolAnimNode, 1, nsRTTIDefaultAllocator<nsCheckBlackboardBoolAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("BlackboardEntry", GetBlackboardEntry, SetBlackboardEntry),

    NS_MEMBER_PROPERTY("OutOnTrue", m_OutOnTrue)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("OutOnFalse", m_OutOnFalse)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("OutBool", m_OutBool)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Blackboard"),
    new nsTitleAttribute("Check Bool: '{BlackboardEntry}'"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Lime)),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsCheckBlackboardBoolAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sBlackboardEntry;

  NS_SUCCEED_OR_RETURN(m_OutOnTrue.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFalse.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutBool.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsCheckBlackboardBoolAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sBlackboardEntry;

  NS_SUCCEED_OR_RETURN(m_OutOnTrue.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutOnFalse.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_OutBool.Deserialize(stream));

  return NS_SUCCESS;
}

void nsCheckBlackboardBoolAnimNode::SetBlackboardEntry(const char* szFile)
{
  m_sBlackboardEntry.Assign(szFile);
}

const char* nsCheckBlackboardBoolAnimNode::GetBlackboardEntry() const
{
  return m_sBlackboardEntry.GetData();
}

void nsCheckBlackboardBoolAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  auto pBlackboard = ref_controller.GetBlackboard();
  if (pBlackboard == nullptr)
    return;

  if (m_sBlackboardEntry.IsEmpty())
    return;

  const nsVariant value = pBlackboard->GetEntryValue(m_sBlackboardEntry);

  if (!value.IsValid() || !value.CanConvertTo<bool>())
  {
    nsLog::Warning("AnimController::CheckBlackboardBool: '{}' doesn't exist or isn't a bool type.", m_sBlackboardEntry);
    return;
  }

  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  const bool bValue = value.ConvertTo<bool>();
  const nsInt8 iIsTrueNow = bValue ? 1 : 0;

  m_OutBool.SetBool(ref_graph, bValue);

  // we use a tri-state bool here to ensure that OnTrue or OnFalse get fired right away
  if (pInstance->m_iIsTrue != iIsTrueNow)
  {
    pInstance->m_iIsTrue = iIsTrueNow;

    if (bValue)
    {
      m_OutOnTrue.SetTriggered(ref_graph);
    }
    else
    {
      m_OutOnFalse.SetTriggered(ref_graph);
    }
  }
}

bool nsCheckBlackboardBoolAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSetBlackboardBoolAnimNode, 1, nsRTTIDefaultAllocator<nsSetBlackboardBoolAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("BlackboardEntry", GetBlackboardEntry, SetBlackboardEntry),
    NS_MEMBER_PROPERTY("Bool", m_bBool),

    NS_MEMBER_PROPERTY("InActivate", m_InActivate)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("InBool", m_InBool)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsTitleAttribute("Set Bool: '{BlackboardEntry}' to {Bool}"),
    new nsCategoryAttribute("Blackboard"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Red)),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsSetBlackboardBoolAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sBlackboardEntry;
  stream << m_bBool;

  NS_SUCCEED_OR_RETURN(m_InActivate.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InBool.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsSetBlackboardBoolAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sBlackboardEntry;
  stream >> m_bBool;

  NS_SUCCEED_OR_RETURN(m_InActivate.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InBool.Deserialize(stream));

  return NS_SUCCESS;
}

void nsSetBlackboardBoolAnimNode::SetBlackboardEntry(const char* szFile)
{
  m_sBlackboardEntry.Assign(szFile);
}

const char* nsSetBlackboardBoolAnimNode::GetBlackboardEntry() const
{
  return m_sBlackboardEntry.GetData();
}

void nsSetBlackboardBoolAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_InActivate.IsTriggered(ref_graph))
    return;

  auto pBlackboard = ref_controller.GetBlackboard();
  if (pBlackboard == nullptr)
    return;

  pBlackboard->SetEntryValue(m_sBlackboardEntry, m_InBool.GetBool(ref_graph, m_bBool));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsGetBlackboardBoolAnimNode, 1, nsRTTIDefaultAllocator<nsGetBlackboardBoolAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("BlackboardEntry", GetBlackboardEntry, SetBlackboardEntry),

    NS_MEMBER_PROPERTY("OutBool", m_OutBool)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Blackboard"),
    new nsTitleAttribute("Get Bool: '{BlackboardEntry}'"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Lime)),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsGetBlackboardBoolAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sBlackboardEntry;

  NS_SUCCEED_OR_RETURN(m_OutBool.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsGetBlackboardBoolAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sBlackboardEntry;

  NS_SUCCEED_OR_RETURN(m_OutBool.Deserialize(stream));

  return NS_SUCCESS;
}

void nsGetBlackboardBoolAnimNode::SetBlackboardEntry(const char* szFile)
{
  m_sBlackboardEntry.Assign(szFile);
}

const char* nsGetBlackboardBoolAnimNode::GetBlackboardEntry() const
{
  return m_sBlackboardEntry.GetData();
}

void nsGetBlackboardBoolAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  auto pBlackboard = ref_controller.GetBlackboard();
  if (pBlackboard == nullptr)
    return;

  if (m_sBlackboardEntry.IsEmpty())
    return;

  nsVariant value = pBlackboard->GetEntryValue(m_sBlackboardEntry);

  if (!value.IsValid() || !value.CanConvertTo<bool>())
  {
    nsLog::Warning("AnimController::GetBlackboardBool: '{}' doesn't exist or can't be converted to bool.", m_sBlackboardEntry);
    return;
  }

  m_OutBool.SetBool(ref_graph, value.ConvertTo<bool>());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsOnBlackboardValueChangedAnimNode, 1, nsRTTIDefaultAllocator<nsOnBlackboardValueChangedAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("BlackboardEntry", GetBlackboardEntry, SetBlackboardEntry),

    NS_MEMBER_PROPERTY("OutOnValueChanged", m_OutOnValueChanged)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Blackboard"),
    new nsTitleAttribute("OnChanged: '{BlackboardEntry}'"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Lime)),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsOnBlackboardValueChangedAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sBlackboardEntry;

  NS_SUCCEED_OR_RETURN(m_OutOnValueChanged.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsOnBlackboardValueChangedAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sBlackboardEntry;

  NS_SUCCEED_OR_RETURN(m_OutOnValueChanged.Deserialize(stream));

  return NS_SUCCESS;
}

void nsOnBlackboardValueChangedAnimNode::SetBlackboardEntry(const char* szFile)
{
  m_sBlackboardEntry.Assign(szFile);
}

const char* nsOnBlackboardValueChangedAnimNode::GetBlackboardEntry() const
{
  return m_sBlackboardEntry.GetData();
}

void nsOnBlackboardValueChangedAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  auto pBlackboard = ref_controller.GetBlackboard();
  if (pBlackboard == nullptr)
    return;

  if (m_sBlackboardEntry.IsEmpty())
    return;

  const nsBlackboard::Entry* pEntry = pBlackboard->GetEntry(m_sBlackboardEntry);

  if (pEntry == nullptr)
  {
    nsLog::Warning("AnimController::OnBlackboardValueChanged: '{}' doesn't exist.", m_sBlackboardEntry);
    return;
  }

  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  if (pInstance->m_uiChangeCounter == pEntry->m_uiChangeCounter)
    return;

  if (pInstance->m_uiChangeCounter != nsInvalidIndex)
  {
    m_OutOnValueChanged.SetTriggered(ref_graph);
  }

  pInstance->m_uiChangeCounter = pEntry->m_uiChangeCounter;
}

bool nsOnBlackboardValueChangedAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}

//////////////////////////////////////////////////////////////////////////


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_BlackboardAnimNodes);
