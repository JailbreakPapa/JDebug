#pragma once

#include <Foundation/Logging/Log.h>

template <typename ResourceType>
ResourceType* nsResourceManager::GetResource(nsStringView sResourceID, bool bIsReloadable)
{
  return static_cast<ResourceType*>(GetResource(nsGetStaticRTTI<ResourceType>(), sResourceID, bIsReloadable));
}

template <typename ResourceType>
nsTypedResourceHandle<ResourceType> nsResourceManager::LoadResource(nsStringView sResourceID)
{
  // the mutex here is necessary to prevent a race between resource unloading and storing the pointer in the handle
  NS_LOCK(s_ResourceMutex);
  return nsTypedResourceHandle<ResourceType>(GetResource<ResourceType>(sResourceID, true));
}

template <typename ResourceType>
nsTypedResourceHandle<ResourceType> nsResourceManager::LoadResource(nsStringView sResourceID, nsTypedResourceHandle<ResourceType> hLoadingFallback)
{
  nsTypedResourceHandle<ResourceType> hResource;
  {
    // the mutex here is necessary to prevent a race between resource unloading and storing the pointer in the handle
    NS_LOCK(s_ResourceMutex);
    hResource = nsTypedResourceHandle<ResourceType>(GetResource<ResourceType>(sResourceID, true));
  }

  if (hLoadingFallback.IsValid())
  {
    hResource.m_pResource->SetLoadingFallbackResource(hLoadingFallback);
  }

  return hResource;
}

template <typename ResourceType>
nsTypedResourceHandle<ResourceType> nsResourceManager::GetExistingResource(nsStringView sResourceID)
{
  nsResource* pResource = nullptr;

  const nsTempHashedString sResourceHash(sResourceID);

  NS_LOCK(s_ResourceMutex);

  const nsRTTI* pRtti = FindResourceTypeOverride(nsGetStaticRTTI<ResourceType>(), sResourceID);

  if (GetLoadedResources()[pRtti].m_Resources.TryGetValue(sResourceHash, pResource))
    return nsTypedResourceHandle<ResourceType>((ResourceType*)pResource);

  return nsTypedResourceHandle<ResourceType>();
}

template <typename ResourceType, typename DescriptorType>
nsTypedResourceHandle<ResourceType> nsResourceManager::CreateResource(nsStringView sResourceID, DescriptorType&& descriptor, nsStringView sResourceDescription)
{
  static_assert(std::is_rvalue_reference<DescriptorType&&>::value, "Please std::move the descriptor into this function");

  NS_LOG_BLOCK("nsResourceManager::CreateResource", sResourceID);

  NS_LOCK(s_ResourceMutex);

  nsTypedResourceHandle<ResourceType> hResource(GetResource<ResourceType>(sResourceID, false));

  ResourceType* pResource = BeginAcquireResource(hResource, nsResourceAcquireMode::PointerOnly);
  pResource->SetResourceDescription(sResourceDescription);
  pResource->m_Flags.Add(nsResourceFlags::IsCreatedResource);

  NS_ASSERT_DEV(pResource->GetLoadingState() == nsResourceState::Unloaded, "CreateResource was called on a resource that is already created");

  // If this does not compile, you either passed in the wrong descriptor type for the given resource type
  // or you forgot to std::move the descriptor when calling CreateResource
  {
    auto localDescriptor = std::move(descriptor);
    nsResourceLoadDesc ld = pResource->CreateResource(std::move(localDescriptor));
    pResource->VerifyAfterCreateResource(ld);
  }

  NS_ASSERT_DEV(pResource->GetLoadingState() != nsResourceState::Unloaded, "CreateResource did not set the loading state properly.");

  EndAcquireResource(pResource);

  return hResource;
}

template <typename ResourceType, typename DescriptorType>
nsTypedResourceHandle<ResourceType>
nsResourceManager::GetOrCreateResource(nsStringView sResourceID, DescriptorType&& descriptor, nsStringView sResourceDescription)
{
  NS_LOCK(s_ResourceMutex);
  nsTypedResourceHandle<ResourceType> hResource = GetExistingResource<ResourceType>(sResourceID);
  if (!hResource.IsValid())
  {
    hResource = CreateResource<ResourceType, DescriptorType>(sResourceID, std::move(descriptor), sResourceDescription);
  }

  return hResource;
}

NS_FORCE_INLINE nsResource* nsResourceManager::BeginAcquireResourcePointer(const nsRTTI* pType, const nsTypelessResourceHandle& hResource)
{
  NS_ASSERT_DEV(hResource.IsValid(), "Cannot acquire a resource through an invalid handle!");

  nsResource* pResource = (nsResource*)hResource.m_pResource;

  NS_ASSERT_DEBUG(pResource->GetDynamicRTTI()->IsDerivedFrom(pType),
    "The requested resource does not have the same type ('{0}') as the resource handle ('{1}').", pResource->GetDynamicRTTI()->GetTypeName(),
    pType->GetTypeName());

  // pResource->m_iLockCount.Increment();
  return pResource;
}

template <typename ResourceType>
ResourceType* nsResourceManager::BeginAcquireResource(const nsTypedResourceHandle<ResourceType>& hResource, nsResourceAcquireMode mode,
  const nsTypedResourceHandle<ResourceType>& hFallbackResource, nsResourceAcquireResult* out_pAcquireResult /*= nullptr*/)
{
  NS_ASSERT_DEV(hResource.IsValid(), "Cannot acquire a resource through an invalid handle!");

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  const nsResource* pCurrentlyUpdatingContent = nsResource::GetCurrentlyUpdatingContent();
  if (pCurrentlyUpdatingContent != nullptr)
  {
    NS_LOCK(s_ResourceMutex);
    NS_ASSERT_DEV(mode == nsResourceAcquireMode::PointerOnly || IsResourceTypeAcquireDuringUpdateContentAllowed(pCurrentlyUpdatingContent->GetDynamicRTTI(), nsGetStaticRTTI<ResourceType>()),
      "Trying to acquire a resource of type '{0}' during '{1}::UpdateContent()'. This has to be enabled by calling "
      "nsResourceManager::AllowResourceTypeAcquireDuringUpdateContent<{1}, {0}>(); at engine startup, for example in "
      "nsGameApplication::Init_SetupDefaultResources().",
      nsGetStaticRTTI<ResourceType>()->GetTypeName(), pCurrentlyUpdatingContent->GetDynamicRTTI()->GetTypeName());
  }
#endif

  ResourceType* pResource = (ResourceType*)hResource.m_hTypeless.m_pResource;

  // NS_ASSERT_DEV(pResource->m_iLockCount < 20, "You probably forgot somewhere to call 'EndAcquireResource' in sync with 'BeginAcquireResource'.");
  NS_ASSERT_DEBUG(pResource->GetDynamicRTTI()->template IsDerivedFrom<ResourceType>(),
    "The requested resource does not have the same type ('{0}') as the resource handle ('{1}').", pResource->GetDynamicRTTI()->GetTypeName(),
    nsGetStaticRTTI<ResourceType>()->GetTypeName());

  if (mode == nsResourceAcquireMode::AllowLoadingFallback && GetForceNoFallbackAcquisition() > 0)
  {
    mode = nsResourceAcquireMode::BlockTillLoaded;
  }

  if (mode == nsResourceAcquireMode::PointerOnly)
  {
    if (out_pAcquireResult)
      *out_pAcquireResult = nsResourceAcquireResult::Final;

    // pResource->m_iLockCount.Increment();
    return pResource;
  }

  // only set the last accessed time stamp, if it is actually needed, pointer-only access might not mean that the resource is used
  // productively
  pResource->m_LastAcquire = GetLastFrameUpdate();

  if (pResource->GetLoadingState() != nsResourceState::LoadedResourceMissing)
  {
    if (pResource->GetLoadingState() != nsResourceState::Loaded)
    {
      // if BlockTillLoaded is specified, it will prepended to the preload array, thus will be loaded immediately
      InternalPreloadResource(pResource, mode >= nsResourceAcquireMode::BlockTillLoaded);

      if (mode == nsResourceAcquireMode::AllowLoadingFallback &&
          (pResource->m_hLoadingFallback.IsValid() || hFallbackResource.IsValid() || GetResourceTypeLoadingFallback<ResourceType>().IsValid()))
      {
        // return the fallback resource for now, if there is one
        if (out_pAcquireResult)
          *out_pAcquireResult = nsResourceAcquireResult::LoadingFallback;

        // Fallback order is as follows:
        //  1) Prefer any resource specific fallback resource
        //  2) If not available, use the fallback that is given to BeginAcquireResource, as that is at least specific to the situation
        //  3) If nothing else is available, take the fallback for the whole resource type

        if (pResource->m_hLoadingFallback.IsValid())
          return (ResourceType*)BeginAcquireResource(pResource->m_hLoadingFallback, nsResourceAcquireMode::BlockTillLoaded);
        else if (hFallbackResource.IsValid())
          return (ResourceType*)BeginAcquireResource(hFallbackResource, nsResourceAcquireMode::BlockTillLoaded);
        else
          return (ResourceType*)BeginAcquireResource(GetResourceTypeLoadingFallback<ResourceType>(), nsResourceAcquireMode::BlockTillLoaded);
      }

      EnsureResourceLoadingState(pResource, nsResourceState::Loaded);
    }
    else
    {
      // as long as there are more quality levels available, schedule the resource for more loading
      // accessing IsQueuedForLoading without a lock here is save because InternalPreloadResource() will lock and early out if necessary
      // and accidentally skipping InternalPreloadResource() is no problem
      if (IsQueuedForLoading(pResource) == false && pResource->GetNumQualityLevelsLoadable() > 0)
        InternalPreloadResource(pResource, false);
    }
  }

  if (pResource->GetLoadingState() == nsResourceState::LoadedResourceMissing)
  {
    // When you get a crash with a stack overflow in this code path, then the resource to be used as the
    // 'missing resource' replacement might be missing itself.

    if (nsResourceManager::GetResourceTypeMissingFallback<ResourceType>().IsValid())
    {
      if (out_pAcquireResult)
        *out_pAcquireResult = nsResourceAcquireResult::MissingFallback;

      return (ResourceType*)BeginAcquireResource(
        nsResourceManager::GetResourceTypeMissingFallback<ResourceType>(), nsResourceAcquireMode::BlockTillLoaded);
    }

    if (mode != nsResourceAcquireMode::AllowLoadingFallback_NeverFail && mode != nsResourceAcquireMode::BlockTillLoaded_NeverFail)
    {
      NS_REPORT_FAILURE("The resource '{0}' of type '{1}' is missing and no fallback is available", pResource->GetResourceID(),
        nsGetStaticRTTI<ResourceType>()->GetTypeName());
    }

    if (out_pAcquireResult)
      *out_pAcquireResult = nsResourceAcquireResult::None;

    return nullptr;
  }

  if (out_pAcquireResult)
    *out_pAcquireResult = nsResourceAcquireResult::Final;

  // pResource->m_iLockCount.Increment();
  return pResource;
}

template <typename ResourceType>
void nsResourceManager::EndAcquireResource(ResourceType* pResource)
{
  // NS_ASSERT_DEV(pResource->m_iLockCount > 0, "The resource lock counter is incorrect: {0}", (nsInt32)pResource->m_iLockCount);
  // pResource->m_iLockCount.Decrement();
}

NS_FORCE_INLINE void nsResourceManager::EndAcquireResourcePointer(nsResource* pResource)
{
  // NS_ASSERT_DEV(pResource->m_iLockCount > 0, "The resource lock counter is incorrect: {0}", (nsInt32)pResource->m_iLockCount);
  // pResource->m_iLockCount.Decrement();
}

template <typename ResourceType>
nsLockedObject<nsMutex, nsDynamicArray<nsResource*>> nsResourceManager::GetAllResourcesOfType()
{
  const nsRTTI* pBaseType = nsGetStaticRTTI<ResourceType>();

  auto& container = GetLoadedResourceOfTypeTempContainer();

  // We use a static container here to ensure its life-time is extended beyond
  // calls to this function as the locked object does not own the passed-in object
  // and thus does not extend the data life-time. It is safe to do this, as the
  // locked object holding the container ensures the container will not be
  // accessed concurrently.
  nsLockedObject<nsMutex, nsDynamicArray<nsResource*>> loadedResourcesLock(s_ResourceMutex, &container);

  container.Clear();

  for (auto itType = GetLoadedResources().GetIterator(); itType.IsValid(); itType.Next())
  {
    const nsRTTI* pDerivedType = itType.Key();

    if (pDerivedType->IsDerivedFrom(pBaseType))
    {
      const LoadedResources& lr = GetLoadedResources()[pDerivedType];

      container.Reserve(container.GetCount() + lr.m_Resources.GetCount());

      for (auto itResource : lr.m_Resources)
      {
        container.PushBack(itResource.Value());
      }
    }
  }

  return loadedResourcesLock;
}

template <typename ResourceType>
bool nsResourceManager::ReloadResource(const nsTypedResourceHandle<ResourceType>& hResource, bool bForce)
{
  ResourceType* pResource = BeginAcquireResource(hResource, nsResourceAcquireMode::PointerOnly);

  bool res = ReloadResource(pResource, bForce);

  EndAcquireResource(pResource);

  return res;
}

NS_FORCE_INLINE bool nsResourceManager::ReloadResource(const nsRTTI* pType, const nsTypelessResourceHandle& hResource, bool bForce)
{
  nsResource* pResource = BeginAcquireResourcePointer(pType, hResource);

  bool res = ReloadResource(pResource, bForce);

  EndAcquireResourcePointer(pResource);

  return res;
}

template <typename ResourceType>
nsUInt32 nsResourceManager::ReloadResourcesOfType(bool bForce)
{
  return ReloadResourcesOfType(nsGetStaticRTTI<ResourceType>(), bForce);
}

template <typename ResourceType>
void nsResourceManager::SetResourceTypeLoader(nsResourceTypeLoader* pCreator)
{
  NS_LOCK(s_ResourceMutex);

  GetResourceTypeLoaders()[nsGetStaticRTTI<ResourceType>()] = pCreator;
}

template <typename ResourceType>
nsTypedResourceHandle<ResourceType> nsResourceManager::GetResourceHandleForExport(nsStringView sResourceID)
{
  NS_ASSERT_DEV(IsExportModeEnabled(), "Export mode needs to be enabled");

  return LoadResource<ResourceType>(sResourceID);
}

template <typename ResourceType>
void nsResourceManager::SetIncrementalUnloadForResourceType(bool bActive)
{
  GetResourceTypeInfo(nsGetStaticRTTI<ResourceType>()).m_bIncrementalUnload = bActive;
}
