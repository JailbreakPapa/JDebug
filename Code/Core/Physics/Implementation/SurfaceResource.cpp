#include <Core/CorePCH.h>

#include <Core/Messages/ApplyOnlyToMessage.h>
#include <Core/Messages/CommonMessages.h>
#include <Core/Physics/SurfaceResource.h>
#include <Core/Prefabs/PrefabResource.h>
#include <Foundation/Utilities/AssetFileHeader.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsSurfaceResource, 1, nsRTTIDefaultAllocator<nsSurfaceResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsSurfaceResource);
// clang-format on

nsEvent<const nsSurfaceResourceEvent&, nsMutex> nsSurfaceResource::s_Events;

nsSurfaceResource::nsSurfaceResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

nsSurfaceResource::~nsSurfaceResource()
{
  NS_ASSERT_DEV(m_pPhysicsMaterialPhysX == nullptr, "Physics material has not been cleaned up properly");
  NS_ASSERT_DEV(m_pPhysicsMaterialJolt == nullptr, "Physics material has not been cleaned up properly");
}

nsResourceLoadDesc nsSurfaceResource::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  nsSurfaceResourceEvent e;
  e.m_pSurface = this;
  e.m_Type = nsSurfaceResourceEvent::Type::Destroyed;
  s_Events.Broadcast(e);

  return res;
}

nsResourceLoadDesc nsSurfaceResource::UpdateContent(nsStreamReader* Stream)
{
  NS_LOG_BLOCK("nsSurfaceResource::UpdateContent", GetResourceIdOrDescription());

  m_Interactions.Clear();

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsStringBuilder sAbsFilePath;
    (*Stream) >> sAbsFilePath;
  }


  nsAssetFileHeader AssetHash;
  AssetHash.Read(*Stream).IgnoreResult();

  {
    nsSurfaceResourceDescriptor dummy;
    dummy.Load(*Stream);

    CreateResource(std::move(dummy));
  }

  // configure the lookup table
  {
    m_Interactions.Reserve(m_Descriptor.m_Interactions.GetCount());
    for (const auto& i : m_Descriptor.m_Interactions)
    {
      nsTempHashedString s(i.m_sInteractionType.GetData());
      auto& item = m_Interactions.ExpandAndGetRef();
      item.m_uiInteractionTypeHash = s.GetHash();
      item.m_pInteraction = &i;
    }

    m_Interactions.Sort([](const SurfInt& lhs, const SurfInt& rhs) -> bool
      {
      if (lhs.m_uiInteractionTypeHash != rhs.m_uiInteractionTypeHash)
        return lhs.m_uiInteractionTypeHash < rhs.m_uiInteractionTypeHash;

      return lhs.m_pInteraction->m_fImpulseThreshold > rhs.m_pInteraction->m_fImpulseThreshold; });
  }

  res.m_State = nsResourceState::Loaded;
  return res;
}

void nsSurfaceResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryCPU = sizeof(nsSurfaceResource);
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsSurfaceResource, nsSurfaceResourceDescriptor)
{
  m_Descriptor = descriptor;

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Loaded;

  nsSurfaceResourceEvent e;
  e.m_pSurface = this;
  e.m_Type = nsSurfaceResourceEvent::Type::Created;
  s_Events.Broadcast(e);

  return res;
}

const nsSurfaceInteraction* nsSurfaceResource::FindInteraction(const nsSurfaceResource* pCurSurf, nsUInt64 uiHash, float fImpulseSqr, float& out_fImpulseParamValue)
{
  while (true)
  {
    bool bFoundAny = false;

    // try to find a matching interaction
    for (const auto& interaction : pCurSurf->m_Interactions)
    {
      if (interaction.m_uiInteractionTypeHash > uiHash)
        break;

      if (interaction.m_uiInteractionTypeHash == uiHash)
      {
        bFoundAny = true;

        // only use it if the threshold is large enough
        if (fImpulseSqr >= nsMath::Square(interaction.m_pInteraction->m_fImpulseThreshold))
        {
          const float fImpulse = nsMath::Sqrt(fImpulseSqr);
          out_fImpulseParamValue = (fImpulse - interaction.m_pInteraction->m_fImpulseThreshold) * interaction.m_pInteraction->m_fImpulseScale;

          return interaction.m_pInteraction;
        }
      }
    }

    // if we did find something, we just never exceeded the threshold, then do not search in the base surface
    if (bFoundAny)
      break;

    if (pCurSurf->m_Descriptor.m_hBaseSurface.IsValid())
    {
      nsResourceLock<nsSurfaceResource> pBase(pCurSurf->m_Descriptor.m_hBaseSurface, nsResourceAcquireMode::BlockTillLoaded);
      pCurSurf = pBase.GetPointer();
    }
    else
    {
      break;
    }
  }

  return nullptr;
}

bool nsSurfaceResource::InteractWithSurface(nsWorld* pWorld, nsGameObjectHandle hObject, const nsVec3& vPosition, const nsVec3& vSurfaceNormal, const nsVec3& vIncomingDirection, const nsTempHashedString& sInteraction, const nsUInt16* pOverrideTeamID, float fImpulseSqr /*= 0.0f*/) const
{
  float fImpulseParam = 0;
  const nsSurfaceInteraction* pIA = FindInteraction(this, sInteraction.GetHash(), fImpulseSqr, fImpulseParam);

  if (pIA == nullptr)
    return false;

  // defined, but set to be empty
  if (!pIA->m_hPrefab.IsValid())
    return false;

  nsResourceLock<nsPrefabResource> pPrefab(pIA->m_hPrefab, nsResourceAcquireMode::BlockTillLoaded);

  nsVec3 vDir;

  switch (pIA->m_Alignment)
  {
    case nsSurfaceInteractionAlignment::SurfaceNormal:
      vDir = vSurfaceNormal;
      break;

    case nsSurfaceInteractionAlignment::IncidentDirection:
      vDir = -vIncomingDirection;
      ;
      break;

    case nsSurfaceInteractionAlignment::ReflectedDirection:
      vDir = vIncomingDirection.GetReflectedVector(vSurfaceNormal);
      break;

    case nsSurfaceInteractionAlignment::ReverseSurfaceNormal:
      vDir = -vSurfaceNormal;
      break;

    case nsSurfaceInteractionAlignment::ReverseIncidentDirection:
      vDir = vIncomingDirection;
      ;
      break;

    case nsSurfaceInteractionAlignment::ReverseReflectedDirection:
      vDir = -vIncomingDirection.GetReflectedVector(vSurfaceNormal);
      break;
  }

  vDir.Normalize();
  nsVec3 vTangent = vDir.GetOrthogonalVector().GetNormalized();

  // random rotation around the spawn direction
  {
    double randomAngle = pWorld->GetRandomNumberGenerator().DoubleInRange(0.0, nsMath::Pi<double>() * 2.0);

    nsMat3 rotMat = nsMat3::MakeAxisRotation(vDir, nsAngle::MakeFromRadian((float)randomAngle));

    vTangent = rotMat * vTangent;
  }

  if (pIA->m_Deviation > nsAngle::MakeFromRadian(0.0f))
  {
    nsAngle maxDeviation;

    /// \todo do random deviation, make sure to clamp max deviation angle
    switch (pIA->m_Alignment)
    {
      case nsSurfaceInteractionAlignment::IncidentDirection:
      case nsSurfaceInteractionAlignment::ReverseReflectedDirection:
      {
        const float fCosAngle = vDir.Dot(-vSurfaceNormal);
        const float fMaxDeviation = nsMath::Pi<float>() - nsMath::ACos(fCosAngle).GetRadian();

        maxDeviation = nsMath::Min(pIA->m_Deviation, nsAngle::MakeFromRadian(fMaxDeviation));
      }
      break;

      case nsSurfaceInteractionAlignment::ReflectedDirection:
      case nsSurfaceInteractionAlignment::ReverseIncidentDirection:
      {
        const float fCosAngle = vDir.Dot(vSurfaceNormal);
        const float fMaxDeviation = nsMath::Pi<float>() - nsMath::ACos(fCosAngle).GetRadian();

        maxDeviation = nsMath::Min(pIA->m_Deviation, nsAngle::MakeFromRadian(fMaxDeviation));
      }
      break;

      default:
        maxDeviation = pIA->m_Deviation;
        break;
    }

    const nsAngle deviation = nsAngle::MakeFromRadian((float)pWorld->GetRandomNumberGenerator().DoubleMinMax(-maxDeviation.GetRadian(), maxDeviation.GetRadian()));

    // tilt around the tangent (we don't want to compute another random rotation here)
    nsMat3 matTilt = nsMat3::MakeAxisRotation(vTangent, deviation);

    vDir = matTilt * vDir;
  }


  // finally compute the bi-tangent
  const nsVec3 vBiTangent = vDir.CrossRH(vTangent);

  nsMat3 mRot;
  mRot.SetColumn(0, vDir); // we always use X as the main axis, so align X with the direction
  mRot.SetColumn(1, vTangent);
  mRot.SetColumn(2, vBiTangent);

  nsTransform t;
  t.m_vPosition = vPosition;
  t.m_qRotation = nsQuat::MakeFromMat3(mRot);
  t.m_vScale.Set(1.0f);

  // attach to dynamic objects
  nsGameObjectHandle hParent;

  nsGameObject* pObject = nullptr;
  if (pWorld->TryGetObject(hObject, pObject) && pObject->IsDynamic())
  {
    hParent = hObject;
    t = nsTransform::MakeLocalTransform(pObject->GetGlobalTransform(), t);
  }

  nsHybridArray<nsGameObject*, 8> rootObjects;

  nsPrefabInstantiationOptions options;
  options.m_hParent = hParent;
  options.m_pCreatedRootObjectsOut = &rootObjects;
  options.m_pOverrideTeamID = pOverrideTeamID;

  pPrefab->InstantiatePrefab(*pWorld, t, options, &pIA->m_Parameters);

  {
    nsMsgSetFloatParameter msgSetFloat;
    msgSetFloat.m_sParameterName = "Impulse";
    msgSetFloat.m_fValue = fImpulseParam;

    for (auto pRootObject : rootObjects)
    {
      pRootObject->PostMessageRecursive(msgSetFloat, nsTime::MakeZero(), nsObjectMsgQueueType::AfterInitialized);
    }
  }

  if (pObject != nullptr && pObject->IsDynamic())
  {
    nsMsgOnlyApplyToObject msg;
    msg.m_hObject = hParent;

    for (auto pRootObject : rootObjects)
    {
      pRootObject->PostMessageRecursive(msg, nsTime::MakeZero(), nsObjectMsgQueueType::AfterInitialized);
    }
  }

  return true;
}

bool nsSurfaceResource::IsBasedOn(const nsSurfaceResource* pThisOrBaseSurface) const
{
  if (pThisOrBaseSurface == this)
    return true;

  if (m_Descriptor.m_hBaseSurface.IsValid())
  {
    nsResourceLock<nsSurfaceResource> pBase(m_Descriptor.m_hBaseSurface, nsResourceAcquireMode::BlockTillLoaded);

    return pBase->IsBasedOn(pThisOrBaseSurface);
  }

  return false;
}

bool nsSurfaceResource::IsBasedOn(const nsSurfaceResourceHandle hThisOrBaseSurface) const
{
  nsResourceLock<nsSurfaceResource> pThisOrBaseSurface(hThisOrBaseSurface, nsResourceAcquireMode::BlockTillLoaded);

  return IsBasedOn(pThisOrBaseSurface.GetPointer());
}


NS_STATICLINK_FILE(Core, Core_Physics_Implementation_SurfaceResource);
