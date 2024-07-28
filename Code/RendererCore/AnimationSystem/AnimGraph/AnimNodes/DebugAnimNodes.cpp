#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/DebugAnimNodes.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLogAnimNode, 1, nsRTTINoAllocator)
  {
    NS_BEGIN_PROPERTIES
    {
      NS_MEMBER_PROPERTY("Text", m_sText)->AddAttributes(new nsDefaultValueAttribute("Values: {0}/{1}-{3}/{4}")),

      NS_MEMBER_PROPERTY("InActivate", m_InActivate)->AddAttributes(new nsHiddenAttribute()),
      NS_MEMBER_PROPERTY("NumberCount", m_uiNumberCount)->AddAttributes(new nsNoTemporaryTransactionsAttribute(), new nsDynamicPinAttribute(), new nsDefaultValueAttribute(1)),
      NS_ARRAY_MEMBER_PROPERTY("InNumbers", m_InNumbers)->AddAttributes(new nsHiddenAttribute(), new nsDynamicPinAttribute("NumberCount")),
    }
    NS_END_PROPERTIES;
    NS_BEGIN_ATTRIBUTES
    {
      new nsCategoryAttribute("Debug"),
      new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Pink)),
      new nsTitleAttribute("Log: '{Text}'"),
    }
    NS_END_ATTRIBUTES;
  }
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsResult nsLogAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  stream << m_sText;
  stream << m_uiNumberCount;

  NS_SUCCEED_OR_RETURN(m_InActivate.Serialize(stream));
  NS_SUCCEED_OR_RETURN(stream.WriteArray(m_InNumbers));

  return NS_SUCCESS;
}

nsResult nsLogAnimNode::DeserializeNode(nsStreamReader& stream)
{
  const auto version = stream.ReadVersion(1);
  NS_IGNORE_UNUSED(version);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  stream >> m_sText;
  stream >> m_uiNumberCount;

  NS_SUCCEED_OR_RETURN(m_InActivate.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(stream.ReadArray(m_InNumbers));

  return NS_SUCCESS;
}

static nsStringView BuildFormattedText(nsStringView sText, const nsVariantArray& params, nsStringBuilder& ref_sStorage)
{
  nsHybridArray<nsString, 12> stringStorage;
  stringStorage.Reserve(params.GetCount());
  for (auto& param : params)
  {
    stringStorage.PushBack(param.ConvertTo<nsString>());
  }

  nsHybridArray<nsStringView, 12> stringViews;
  stringViews.Reserve(stringStorage.GetCount());
  for (auto& s : stringStorage)
  {
    stringViews.PushBack(s);
  }

  nsFormatString fs(sText);
  return fs.BuildFormattedText(ref_sStorage, stringViews.GetData(), stringViews.GetCount());
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLogInfoAnimNode, 1, nsRTTIDefaultAllocator<nsLogInfoAnimNode>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsTitleAttribute("Log Info: '{Text}'"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsLogInfoAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_InActivate.IsTriggered(ref_graph))
    return;

  nsVariantArray params;
  for (auto& n : m_InNumbers)
  {
    params.PushBack(n.GetNumber(ref_graph));
  }

  nsStringBuilder sStorage;
  nsLog::Info(BuildFormattedText(m_sText, params, sStorage));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsLogErrorAnimNode, 1, nsRTTIDefaultAllocator<nsLogErrorAnimNode>)
{
  NS_BEGIN_ATTRIBUTES
  {
    new nsTitleAttribute("Log Error: '{Text}'"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsLogErrorAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_InActivate.IsTriggered(ref_graph))
    return;

  nsVariantArray params;
  for (auto& n : m_InNumbers)
  {
    params.PushBack(n.GetNumber(ref_graph));
  }

  nsStringBuilder sStorage;
  nsLog::Error(BuildFormattedText(m_sText, params, sStorage));
}


NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_DebugAnimNodes);
