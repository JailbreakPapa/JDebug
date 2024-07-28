#include <Core/CorePCH.h>

#include <Core/Prefabs/PrefabResource.h>
#include <Foundation/Reflection/PropertyPath.h>
#include <Foundation/Reflection/ReflectionUtils.h>
#include <Foundation/Utilities/AssetFileHeader.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsPrefabResource, 1, nsRTTIDefaultAllocator<nsPrefabResource>)
NS_END_DYNAMIC_REFLECTED_TYPE;

NS_RESOURCE_IMPLEMENT_COMMON_CODE(nsPrefabResource);
// clang-format on

nsPrefabResource::nsPrefabResource()
  : nsResource(DoUpdate::OnAnyThread, 1)
{
}

void nsPrefabResource::InstantiatePrefab(nsWorld& ref_world, const nsTransform& rootTransform, nsPrefabInstantiationOptions options, const nsArrayMap<nsHashedString, nsVariant>* pExposedParamValues)
{
  if (GetLoadingState() != nsResourceState::Loaded)
    return;

  if (pExposedParamValues != nullptr && !pExposedParamValues->IsEmpty())
  {
    nsHybridArray<nsGameObject*, 8> createdRootObjects;
    nsHybridArray<nsGameObject*, 8> createdChildObjects;

    if (options.m_pCreatedRootObjectsOut == nullptr)
    {
      options.m_pCreatedRootObjectsOut = &createdRootObjects;
    }

    if (options.m_pCreatedChildObjectsOut == nullptr)
    {
      options.m_pCreatedChildObjectsOut = &createdChildObjects;
    }

    m_WorldReader.InstantiatePrefab(ref_world, rootTransform, options);

    NS_ASSERT_DEBUG(options.m_pCreatedRootObjectsOut != options.m_pCreatedChildObjectsOut, "These pointers must point to different arrays, otherwise applying exposed properties doesn't work correctly.");
    ApplyExposedParameterValues(pExposedParamValues, *options.m_pCreatedChildObjectsOut, *options.m_pCreatedRootObjectsOut);
  }
  else
  {
    m_WorldReader.InstantiatePrefab(ref_world, rootTransform, options);
  }
}

nsPrefabResource::InstantiateResult nsPrefabResource::InstantiatePrefab(const nsPrefabResourceHandle& hPrefab, bool bBlockTillLoaded, nsWorld& ref_world, const nsTransform& rootTransform, nsPrefabInstantiationOptions options, const nsArrayMap<nsHashedString, nsVariant>* pExposedParamValues /*= nullptr*/)
{
  nsResourceLock<nsPrefabResource> pPrefab(hPrefab, bBlockTillLoaded ? nsResourceAcquireMode::BlockTillLoaded_NeverFail : nsResourceAcquireMode::AllowLoadingFallback_NeverFail);

  switch (pPrefab.GetAcquireResult())
  {
    case nsResourceAcquireResult::Final:
      pPrefab->InstantiatePrefab(ref_world, rootTransform, options, pExposedParamValues);
      return InstantiateResult::Success;

    case nsResourceAcquireResult::LoadingFallback:
      return InstantiateResult::NotYetLoaded;

    default:
      return InstantiateResult::Error;
  }
}

void nsPrefabResource::ApplyExposedParameterValues(const nsArrayMap<nsHashedString, nsVariant>* pExposedParamValues, const nsDynamicArray<nsGameObject*>& createdChildObjects, const nsDynamicArray<nsGameObject*>& createdRootObjects) const
{
  const nsUInt32 uiNumParamDescs = m_PrefabParamDescs.GetCount();

  for (nsUInt32 i = 0; i < pExposedParamValues->GetCount(); ++i)
  {
    const nsHashedString& name = pExposedParamValues->GetKey(i);
    const nsUInt64 uiNameHash = name.GetHash();

    for (nsUInt32 uiCurParam = FindFirstParamWithName(uiNameHash); uiCurParam < uiNumParamDescs; ++uiCurParam)
    {
      const auto& ppd = m_PrefabParamDescs[uiCurParam];

      if (ppd.m_sExposeName.GetHash() != uiNameHash)
        break;

      nsGameObject* pTarget = ppd.m_uiWorldReaderChildObject ? createdChildObjects[ppd.m_uiWorldReaderObjectIndex] : createdRootObjects[ppd.m_uiWorldReaderObjectIndex];

      if (ppd.m_CachedPropertyPath.IsValid())
      {
        if (ppd.m_sComponentType.IsEmpty())
        {
          ppd.m_CachedPropertyPath.SetValue(pTarget, pExposedParamValues->GetValue(i));
        }
        else
        {
          for (nsComponent* pComp : pTarget->GetComponents())
          {
            const nsRTTI* pRtti = pComp->GetDynamicRTTI();

            // TODO: use component index instead
            // atm if the same component type is attached multiple times, they will all get the value applied
            if (pRtti->GetTypeNameHash() == ppd.m_sComponentType.GetHash())
            {
              ppd.m_CachedPropertyPath.SetValue(pComp, pExposedParamValues->GetValue(i));
            }
          }
        }
      }

      // Allow to bind multiple properties to the same exposed parameter name
      // Therefore, do not break here, but continue iterating
    }
  }
}

nsResourceLoadDesc nsPrefabResource::UnloadData(Unload WhatToUnload)
{
  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;
  res.m_State = nsResourceState::Unloaded;

  if (WhatToUnload == nsResource::Unload::AllQualityLevels)
  {
    m_WorldReader.ClearAndCompact();
  }

  return res;
}

nsResourceLoadDesc nsPrefabResource::UpdateContent(nsStreamReader* Stream)
{
  NS_LOG_BLOCK("nsPrefabResource::UpdateContent", GetResourceIdOrDescription());

  nsResourceLoadDesc res;
  res.m_uiQualityLevelsDiscardable = 0;
  res.m_uiQualityLevelsLoadable = 0;

  if (Stream == nullptr)
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  nsStreamReader& s = *Stream;

  // skip the absolute file path data that the standard file reader writes into the stream
  {
    nsString sAbsFilePath;
    s >> sAbsFilePath;
  }

  nsAssetFileHeader assetHeader;
  assetHeader.Read(s).IgnoreResult();

  char szSceneTag[16];
  s.ReadBytes(szSceneTag, sizeof(char) * 16);
  NS_ASSERT_DEV(nsStringUtils::IsEqualN(szSceneTag, "[nsBinaryScene]", 16), "The given file is not a valid prefab file");

  if (!nsStringUtils::IsEqualN(szSceneTag, "[nsBinaryScene]", 16))
  {
    res.m_State = nsResourceState::LoadedResourceMissing;
    return res;
  }

  m_WorldReader.ReadWorldDescription(s).IgnoreResult();

  if (assetHeader.GetFileVersion() >= 4)
  {
    nsUInt32 uiExposedParams = 0;

    s >> uiExposedParams;

    m_PrefabParamDescs.SetCount(uiExposedParams);

    for (nsUInt32 i = 0; i < uiExposedParams; ++i)
    {
      auto& ppd = m_PrefabParamDescs[i];

      NS_ASSERT_DEV(assetHeader.GetFileVersion() >= 6, "Old resource version not supported anymore");
      ppd.Load(s);

      // initialize the cached property path here once
      // so we can only apply it later as often as needed
      {
        if (ppd.m_sComponentType.IsEmpty())
        {
          ppd.m_CachedPropertyPath.InitializeFromPath(*nsGetStaticRTTI<nsGameObject>(), ppd.m_sProperty).IgnoreResult();
        }
        else
        {
          if (const nsRTTI* pRtti = nsRTTI::FindTypeByNameHash(ppd.m_sComponentType.GetHash()))
          {
            ppd.m_CachedPropertyPath.InitializeFromPath(*pRtti, ppd.m_sProperty).IgnoreResult();
          }
        }
      }
    }

    // sort exposed parameter descriptions by name hash for quicker access
    m_PrefabParamDescs.Sort([](const nsExposedPrefabParameterDesc& lhs, const nsExposedPrefabParameterDesc& rhs) -> bool
      { return lhs.m_sExposeName.GetHash() < rhs.m_sExposeName.GetHash(); });
  }

  res.m_State = nsResourceState::Loaded;
  return res;
}

void nsPrefabResource::UpdateMemoryUsage(MemoryUsage& out_NewMemoryUsage)
{
  out_NewMemoryUsage.m_uiMemoryGPU = 0;
  out_NewMemoryUsage.m_uiMemoryCPU = m_WorldReader.GetHeapMemoryUsage() + sizeof(this);
}

NS_RESOURCE_IMPLEMENT_CREATEABLE(nsPrefabResource, nsPrefabResourceDescriptor)
{
  nsResourceLoadDesc desc;
  desc.m_State = nsResourceState::Loaded;
  desc.m_uiQualityLevelsDiscardable = 0;
  desc.m_uiQualityLevelsLoadable = 0;
  return desc;
}

nsUInt32 nsPrefabResource::FindFirstParamWithName(nsUInt64 uiNameHash) const
{
  nsUInt32 lb = 0;
  nsUInt32 ub = m_PrefabParamDescs.GetCount();

  while (lb < ub)
  {
    const nsUInt32 middle = lb + ((ub - lb) >> 1);

    if (m_PrefabParamDescs[middle].m_sExposeName.GetHash() < uiNameHash)
    {
      lb = middle + 1;
    }
    else
    {
      ub = middle;
    }
  }

  return lb;
}

void nsExposedPrefabParameterDesc::Save(nsStreamWriter& inout_stream) const
{
  nsUInt32 comb = m_uiWorldReaderObjectIndex | (m_uiWorldReaderChildObject << 31);

  inout_stream << m_sExposeName;
  inout_stream << comb;
  inout_stream << m_sComponentType;
  inout_stream << m_sProperty;
}

void nsExposedPrefabParameterDesc::Load(nsStreamReader& inout_stream)
{
  nsUInt32 comb = 0;

  inout_stream >> m_sExposeName;
  inout_stream >> comb;
  inout_stream >> m_sComponentType;
  inout_stream >> m_sProperty;

  m_uiWorldReaderObjectIndex = comb & 0x7FFFFFFF;
  m_uiWorldReaderChildObject = (comb >> 31);
}

NS_STATICLINK_FILE(Core, Core_Prefabs_Implementation_PrefabResource);
