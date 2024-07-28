#include <Core/CorePCH.h>

#include <Core/Physics/SurfaceResource.h>
#include <Core/Prefabs/PrefabResource.h>
#include <Foundation/Serialization/AbstractObjectGraph.h>

// clang-format off
NS_BEGIN_STATIC_REFLECTED_ENUM(nsSurfaceInteractionAlignment, 2)
  NS_ENUM_CONSTANTS(nsSurfaceInteractionAlignment::SurfaceNormal, nsSurfaceInteractionAlignment::IncidentDirection, nsSurfaceInteractionAlignment::ReflectedDirection)
  NS_ENUM_CONSTANTS(nsSurfaceInteractionAlignment::ReverseSurfaceNormal, nsSurfaceInteractionAlignment::ReverseIncidentDirection, nsSurfaceInteractionAlignment::ReverseReflectedDirection)
NS_END_STATIC_REFLECTED_ENUM;

NS_BEGIN_STATIC_REFLECTED_TYPE(nsSurfaceInteraction, nsNoBase, 1, nsRTTIDefaultAllocator<nsSurfaceInteraction>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_MEMBER_PROPERTY("Type", m_sInteractionType)->AddAttributes(new nsDynamicStringEnumAttribute("SurfaceInteractionTypeEnum")),
    NS_ACCESSOR_PROPERTY("Prefab", GetPrefab, SetPrefab)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Prefab", nsDependencyFlags::Package)),
    NS_MAP_ACCESSOR_PROPERTY("Parameters", GetParameters, GetParameter, SetParameter, RemoveParameter)->AddAttributes(new nsExposedParametersAttribute("Prefab")),
    NS_ENUM_MEMBER_PROPERTY("Alignment", nsSurfaceInteractionAlignment, m_Alignment),
    NS_MEMBER_PROPERTY("Deviation", m_Deviation)->AddAttributes(new nsClampValueAttribute(nsVariant(nsAngle::MakeFromDegree(0.0f)), nsVariant(nsAngle::MakeFromDegree(90.0f)))),
    NS_MEMBER_PROPERTY("ImpulseThreshold", m_fImpulseThreshold),
    NS_MEMBER_PROPERTY("ImpulseScale", m_fImpulseScale)->AddAttributes(new nsDefaultValueAttribute(1.0f)),
  }
  NS_END_PROPERTIES;
}
NS_END_STATIC_REFLECTED_TYPE;

NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSurfaceResourceDescriptor, 2, nsRTTIDefaultAllocator<nsSurfaceResourceDescriptor>)
{
  NS_BEGIN_PROPERTIES
  {
    NS_ACCESSOR_PROPERTY("BaseSurface", GetBaseSurfaceFile, SetBaseSurfaceFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Surface")),// package+thumbnail so that it forbids circular dependencies
    NS_MEMBER_PROPERTY("Restitution", m_fPhysicsRestitution)->AddAttributes(new nsDefaultValueAttribute(0.25f)),
    NS_MEMBER_PROPERTY("StaticFriction", m_fPhysicsFrictionStatic)->AddAttributes(new nsDefaultValueAttribute(0.6f)),
    NS_MEMBER_PROPERTY("DynamicFriction", m_fPhysicsFrictionDynamic)->AddAttributes(new nsDefaultValueAttribute(0.4f)),
    NS_MEMBER_PROPERTY("GroundType", m_iGroundType)->AddAttributes(new nsDefaultValueAttribute(-1), new nsDynamicEnumAttribute("AiGroundType")),
    NS_ACCESSOR_PROPERTY("OnCollideInteraction", GetCollisionInteraction, SetCollisionInteraction)->AddAttributes(new nsDynamicStringEnumAttribute("SurfaceInteractionTypeEnum")),
    NS_ACCESSOR_PROPERTY("SlideReaction", GetSlideReactionPrefabFile, SetSlideReactionPrefabFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Prefab", nsDependencyFlags::Package)),
    NS_ACCESSOR_PROPERTY("RollReaction", GetRollReactionPrefabFile, SetRollReactionPrefabFile)->AddAttributes(new nsAssetBrowserAttribute("CompatibleAsset_Prefab", nsDependencyFlags::Package)),
    NS_ARRAY_MEMBER_PROPERTY("Interactions", m_Interactions),
  }
  NS_END_PROPERTIES;
}
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

void nsSurfaceInteraction::SetPrefab(const char* szPrefab)
{
  nsPrefabResourceHandle hPrefab;

  if (!nsStringUtils::IsNullOrEmpty(szPrefab))
  {
    hPrefab = nsResourceManager::LoadResource<nsPrefabResource>(szPrefab);
  }

  m_hPrefab = hPrefab;
}

const char* nsSurfaceInteraction::GetPrefab() const
{
  if (!m_hPrefab.IsValid())
    return "";

  return m_hPrefab.GetResourceID();
}

const nsRangeView<const char*, nsUInt32> nsSurfaceInteraction::GetParameters() const
{
  return nsRangeView<const char*, nsUInt32>([]() -> nsUInt32
    { return 0; },
    [this]() -> nsUInt32
    { return m_Parameters.GetCount(); },
    [](nsUInt32& ref_uiIt)
    { ++ref_uiIt; },
    [this](const nsUInt32& uiIt) -> const char*
    { return m_Parameters.GetKey(uiIt).GetString().GetData(); });
}

void nsSurfaceInteraction::SetParameter(const char* szKey, const nsVariant& value)
{
  nsHashedString hs;
  hs.Assign(szKey);

  auto it = m_Parameters.Find(hs);
  if (it != nsInvalidIndex && m_Parameters.GetValue(it) == value)
    return;

  m_Parameters[hs] = value;
}

void nsSurfaceInteraction::RemoveParameter(const char* szKey)
{
  m_Parameters.RemoveAndCopy(nsTempHashedString(szKey));
}

bool nsSurfaceInteraction::GetParameter(const char* szKey, nsVariant& out_value) const
{
  nsUInt32 it = m_Parameters.Find(szKey);

  if (it == nsInvalidIndex)
    return false;

  out_value = m_Parameters.GetValue(it);
  return true;
}

void nsSurfaceResourceDescriptor::Load(nsStreamReader& inout_stream)
{
  nsUInt8 uiVersion = 0;

  inout_stream >> uiVersion;
  NS_ASSERT_DEV(uiVersion <= 8, "Invalid version {0} for surface resource", uiVersion);

  inout_stream >> m_fPhysicsRestitution;
  inout_stream >> m_fPhysicsFrictionStatic;
  inout_stream >> m_fPhysicsFrictionDynamic;
  inout_stream >> m_hBaseSurface;

  if (uiVersion >= 4)
  {
    inout_stream >> m_sOnCollideInteraction;
  }

  if (uiVersion >= 7)
  {
    inout_stream >> m_sSlideInteractionPrefab;
    inout_stream >> m_sRollInteractionPrefab;
  }

  if (uiVersion > 2)
  {
    nsUInt32 count = 0;
    inout_stream >> count;
    m_Interactions.SetCount(count);

    nsStringBuilder sTemp;
    for (nsUInt32 i = 0; i < count; ++i)
    {
      auto& ia = m_Interactions[i];

      inout_stream >> sTemp;
      ia.m_sInteractionType = sTemp;

      inout_stream >> ia.m_hPrefab;
      inout_stream >> ia.m_Alignment;
      inout_stream >> ia.m_Deviation;

      if (uiVersion >= 4)
      {
        inout_stream >> ia.m_fImpulseThreshold;
      }

      if (uiVersion >= 5)
      {
        inout_stream >> ia.m_fImpulseScale;
      }

      if (uiVersion >= 6)
      {
        nsUInt8 uiNumParams;
        inout_stream >> uiNumParams;

        ia.m_Parameters.Clear();
        ia.m_Parameters.Reserve(uiNumParams);

        nsHashedString key;
        nsVariant value;

        for (nsUInt32 i2 = 0; i2 < uiNumParams; ++i2)
        {
          inout_stream >> key;
          inout_stream >> value;

          ia.m_Parameters.Insert(key, value);
        }
      }
    }
  }

  if (uiVersion >= 8)
  {
    inout_stream >> m_iGroundType;
  }
}

void nsSurfaceResourceDescriptor::Save(nsStreamWriter& inout_stream) const
{
  const nsUInt8 uiVersion = 8;

  inout_stream << uiVersion;
  inout_stream << m_fPhysicsRestitution;
  inout_stream << m_fPhysicsFrictionStatic;
  inout_stream << m_fPhysicsFrictionDynamic;
  inout_stream << m_hBaseSurface;

  // version 4
  inout_stream << m_sOnCollideInteraction;

  // version 7
  inout_stream << m_sSlideInteractionPrefab;
  inout_stream << m_sRollInteractionPrefab;

  inout_stream << m_Interactions.GetCount();
  for (const auto& ia : m_Interactions)
  {
    inout_stream << ia.m_sInteractionType;
    inout_stream << ia.m_hPrefab;
    inout_stream << ia.m_Alignment;
    inout_stream << ia.m_Deviation;

    // version 4
    inout_stream << ia.m_fImpulseThreshold;

    // version 5
    inout_stream << ia.m_fImpulseScale;

    // version 6
    const nsUInt8 uiNumParams = static_cast<nsUInt8>(ia.m_Parameters.GetCount());
    inout_stream << uiNumParams;
    for (nsUInt32 i = 0; i < uiNumParams; ++i)
    {
      inout_stream << ia.m_Parameters.GetKey(i);
      inout_stream << ia.m_Parameters.GetValue(i);
    }
  }

  // version 8
  inout_stream << m_iGroundType;
}

void nsSurfaceResourceDescriptor::SetBaseSurfaceFile(const char* szFile)
{
  nsSurfaceResourceHandle hResource;

  if (!nsStringUtils::IsNullOrEmpty(szFile))
  {
    hResource = nsResourceManager::LoadResource<nsSurfaceResource>(szFile);
  }

  m_hBaseSurface = hResource;
}

const char* nsSurfaceResourceDescriptor::GetBaseSurfaceFile() const
{
  if (!m_hBaseSurface.IsValid())
    return "";

  return m_hBaseSurface.GetResourceID();
}

void nsSurfaceResourceDescriptor::SetCollisionInteraction(const char* szName)
{
  m_sOnCollideInteraction.Assign(szName);
}

const char* nsSurfaceResourceDescriptor::GetCollisionInteraction() const
{
  return m_sOnCollideInteraction.GetData();
}

void nsSurfaceResourceDescriptor::SetSlideReactionPrefabFile(const char* szFile)
{
  m_sSlideInteractionPrefab.Assign(szFile);
}

const char* nsSurfaceResourceDescriptor::GetSlideReactionPrefabFile() const
{
  return m_sSlideInteractionPrefab.GetData();
}

void nsSurfaceResourceDescriptor::SetRollReactionPrefabFile(const char* szFile)
{
  m_sRollInteractionPrefab.Assign(szFile);
}

const char* nsSurfaceResourceDescriptor::GetRollReactionPrefabFile() const
{
  return m_sRollInteractionPrefab.GetData();
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

#include <Foundation/Serialization/GraphPatch.h>

class nsSurfaceResourceDescriptorPatch_1_2 : public nsGraphPatch
{
public:
  nsSurfaceResourceDescriptorPatch_1_2()
    : nsGraphPatch("nsSurfaceResourceDescriptor", 2)
  {
  }

  virtual void Patch(nsGraphPatchContext& ref_context, nsAbstractObjectGraph* pGraph, nsAbstractObjectNode* pNode) const override
  {
    pNode->RenameProperty("Base Surface", "BaseSurface");
    pNode->RenameProperty("Static Friction", "StaticFriction");
    pNode->RenameProperty("Dynamic Friction", "DynamicFriction");
  }
};

nsSurfaceResourceDescriptorPatch_1_2 g_nsSurfaceResourceDescriptorPatch_1_2;


NS_STATICLINK_FILE(Core, Core_Physics_Implementation_SurfaceResourceDescriptor);
