#include <RendererCore/RendererCorePCH.h>

#include <RendererCore/AnimationSystem/AnimGraph/AnimController.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraph.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimGraphInstance.h>
#include <RendererCore/AnimationSystem/AnimGraph/AnimNodes/BoneWeightsAnimNode.h>
#include <RendererCore/AnimationSystem/SkeletonResource.h>

#include <ozz/animation/runtime/skeleton_utils.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsBoneWeightsAnimNode, 1, nsRTTIDefaultAllocator<nsBoneWeightsAnimNode>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Weight", m_fWeight)->AddAttributes(new nsDefaultValueAttribute(1.0f), new nsClampValueAttribute(0.0f, 1.0f)),
    NS_ARRAY_ACCESSOR_PROPERTY("RootBones", RootBones_GetCount, RootBones_GetValue, RootBones_SetValue, RootBones_Insert, RootBones_Remove),

    NS_MEMBER_PROPERTY("Weights", m_WeightsPin)->AddAttributes(new nsHiddenAttribute()),
    NS_MEMBER_PROPERTY("InverseWeights", m_InverseWeightsPin)->AddAttributes(new nsHiddenAttribute()),
  }
  NS_END_PROPERTIES;
  NS_BEGIN_ATTRIBUTES
  {
    new nsCategoryAttribute("Weights"),
    new nsColorAttribute(nsColorScheme::DarkUI(nsColorScheme::Teal)),
    new nsTitleAttribute("Bone Weights '{RootBones[0]}' '{RootBones[1]}' '{RootBones[2]}'"),
  }
  NS_END_ATTRIBUTES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsBoneWeightsAnimNode::nsBoneWeightsAnimNode() = default;
nsBoneWeightsAnimNode::~nsBoneWeightsAnimNode() = default;

nsUInt32 nsBoneWeightsAnimNode::RootBones_GetCount() const
{
  return m_RootBones.GetCount();
}

const char* nsBoneWeightsAnimNode::RootBones_GetValue(nsUInt32 uiIndex) const
{
  return m_RootBones[uiIndex].GetString();
}

void nsBoneWeightsAnimNode::RootBones_SetValue(nsUInt32 uiIndex, const char* value)
{
  m_RootBones[uiIndex].Assign(value);
}

void nsBoneWeightsAnimNode::RootBones_Insert(nsUInt32 uiIndex, const char* value)
{
  nsHashedString tmp;
  tmp.Assign(value);
  m_RootBones.InsertAt(uiIndex, tmp);
}

void nsBoneWeightsAnimNode::RootBones_Remove(nsUInt32 uiIndex)
{
  m_RootBones.RemoveAtAndCopy(uiIndex);
}

nsResult nsBoneWeightsAnimNode::SerializeNode(nsStreamWriter& stream) const
{
  stream.WriteVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::SerializeNode(stream));

  NS_SUCCEED_OR_RETURN(stream.WriteArray(m_RootBones));

  stream << m_fWeight;

  NS_SUCCEED_OR_RETURN(m_WeightsPin.Serialize(stream));
  NS_SUCCEED_OR_RETURN(m_InverseWeightsPin.Serialize(stream));

  return NS_SUCCESS;
}

nsResult nsBoneWeightsAnimNode::DeserializeNode(nsStreamReader& stream)
{
  stream.ReadVersion(1);

  NS_SUCCEED_OR_RETURN(SUPER::DeserializeNode(stream));

  NS_SUCCEED_OR_RETURN(stream.ReadArray(m_RootBones));

  stream >> m_fWeight;

  NS_SUCCEED_OR_RETURN(m_WeightsPin.Deserialize(stream));
  NS_SUCCEED_OR_RETURN(m_InverseWeightsPin.Deserialize(stream));

  return NS_SUCCESS;
}

void nsBoneWeightsAnimNode::Step(nsAnimController& ref_controller, nsAnimGraphInstance& ref_graph, nsTime tDiff, const nsSkeletonResource* pSkeleton, nsGameObject* pTarget) const
{
  if (!m_WeightsPin.IsConnected() && !m_InverseWeightsPin.IsConnected())
    return;

  if (m_RootBones.IsEmpty())
  {
    nsLog::Warning("No root-bones added to bone weight node in animation controller.");
    return;
  }

  InstanceData* pInstance = ref_graph.GetAnimNodeInstanceData<InstanceData>(*this);

  if (pInstance->m_pSharedBoneWeights == nullptr && pInstance->m_pSharedInverseBoneWeights == nullptr)
  {
    const auto pOzzSkeleton = &pSkeleton->GetDescriptor().m_Skeleton.GetOzzSkeleton();

    nsStringBuilder name;
    name.SetFormat("{}", pSkeleton->GetResourceIDHash());

    for (const auto& rootBone : m_RootBones)
    {
      name.AppendFormat("-{}", rootBone);
    }

    pInstance->m_pSharedBoneWeights = ref_controller.CreateBoneWeights(name, *pSkeleton, [this, pOzzSkeleton](nsAnimGraphSharedBoneWeights& ref_bw)
      {
      for (const auto& rootBone : m_RootBones)
      {
        int iRootBone = -1;
        for (int iBone = 0; iBone < pOzzSkeleton->num_joints(); ++iBone)
        {
          if (nsStringUtils::IsEqual(pOzzSkeleton->joint_names()[iBone], rootBone.GetData()))
          {
            iRootBone = iBone;
            break;
          }
        }

        const float fBoneWeight = 1.0f;

        auto setBoneWeight = [&](int iCurrentBone, int) {
          const int iJointIdx0 = iCurrentBone / 4;
          const int iJointIdx1 = iCurrentBone % 4;

          ozz::math::SimdFloat4& soa_weight = ref_bw.m_Weights[iJointIdx0];
          soa_weight = ozz::math::SetI(soa_weight, ozz::math::simd_float4::Load1(fBoneWeight), iJointIdx1);
        };

        ozz::animation::IterateJointsDF(*pOzzSkeleton, setBoneWeight, iRootBone);
      } });

    if (m_InverseWeightsPin.IsConnected())
    {
      name.Append("-inv");

      pInstance->m_pSharedInverseBoneWeights = ref_controller.CreateBoneWeights(name, *pSkeleton, [this, pInstance](nsAnimGraphSharedBoneWeights& ref_bw)
        {
        const ozz::math::SimdFloat4 oneBone = ozz::math::simd_float4::one();

        for (nsUInt32 b = 0; b < ref_bw.m_Weights.GetCount(); ++b)
        {
          ref_bw.m_Weights[b] = ozz::math::MSub(oneBone, oneBone, pInstance->m_pSharedBoneWeights->m_Weights[b]);
        } });
    }

    if (!m_WeightsPin.IsConnected())
    {
      pInstance->m_pSharedBoneWeights.Clear();
    }
  }

  if (m_WeightsPin.IsConnected())
  {
    nsAnimGraphPinDataBoneWeights* pPinData = ref_controller.AddPinDataBoneWeights();
    pPinData->m_fOverallWeight = m_fWeight;
    pPinData->m_pSharedBoneWeights = pInstance->m_pSharedBoneWeights.Borrow();

    m_WeightsPin.SetWeights(ref_graph, pPinData);
  }

  if (m_InverseWeightsPin.IsConnected())
  {
    nsAnimGraphPinDataBoneWeights* pPinData = ref_controller.AddPinDataBoneWeights();
    pPinData->m_fOverallWeight = m_fWeight;
    pPinData->m_pSharedBoneWeights = pInstance->m_pSharedInverseBoneWeights.Borrow();

    m_InverseWeightsPin.SetWeights(ref_graph, pPinData);
  }
}

bool nsBoneWeightsAnimNode::GetInstanceDataDesc(nsInstanceDataDesc& out_desc) const
{
  out_desc.FillFromType<InstanceData>();
  return true;
}

NS_STATICLINK_FILE(RendererCore, RendererCore_AnimationSystem_AnimGraph_AnimNodes_BoneWeightsAnimNode);
