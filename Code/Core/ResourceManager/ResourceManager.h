#pragma once

#include <Core/ResourceManager/Implementation/WorkerTasks.h>
#include <Core/ResourceManager/Resource.h>
#include <Core/ResourceManager/ResourceHandle.h>
#include <Core/ResourceManager/ResourceTypeLoader.h>
#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Containers/HashTable.h>
#include <Foundation/Threading/LockedObject.h>
#include <Foundation/Types/UniquePtr.h>

class nsResourceManagerState;

/// \brief The central class for managing all types derived from nsResource
class NS_CORE_DLL nsResourceManager
{
  friend class nsResourceManagerState;
  static nsUniquePtr<nsResourceManagerState> s_pState;

  /// \name Events
  ///@{

public:
  /// Events on individual resources. Subscribe to this to get a notification for events happening on any resource.
  /// If you are only interested in events for a specific resource, subscribe on directly on that instance.
  static const nsEvent<const nsResourceEvent&, nsMutex>& GetResourceEvents();

  /// Events for the resource manager that affect broader things.
  static const nsEvent<const nsResourceManagerEvent&, nsMutex>& GetManagerEvents();

  /// \brief Goes through all existing resources and broadcasts the 'Exists' event.
  ///
  /// Used to announce all currently existing resources to interested event listeners (ie tools).
  static void BroadcastExistsEvent();

  ///@}
  /// \name Loading and creating resources
  ///@{

public:
  /// \brief Returns a handle to the requested resource. szResourceID must uniquely identify the resource, different spellings / casing
  /// will result in different resources.
  ///
  /// After the call to this function the resource definitely exists in memory. Upon access through BeginAcquireResource / nsResourceLock
  /// the resource will be loaded. If it is not possible to load the resource it will change to a 'missing' state. If the code accessing the
  /// resource cannot handle that case, the application will 'terminate' (that means crash).
  template <typename ResourceType>
  static nsTypedResourceHandle<ResourceType> LoadResource(nsStringView sResourceID);

  /// \brief Same as LoadResource(), but additionally allows to set a priority on the resource and a custom fallback resource for this
  /// instance.
  ///
  /// Pass in nsResourcePriority::Unchanged, if you only want to specify a custom fallback resource.
  /// If a resource priority is specified, the target resource will get that priority.
  /// If a valid fallback resource is specified, the resource will store that as its instance specific fallback resource. This will be used
  /// when trying to acquire the resource later.
  template <typename ResourceType>
  static nsTypedResourceHandle<ResourceType> LoadResource(nsStringView sResourceID, nsTypedResourceHandle<ResourceType> hLoadingFallback);


  /// \brief Same as LoadResource(), but instead of a template argument, the resource type to use is given as nsRTTI info. Returns a
  /// typeless handle due to the missing template argument.
  static nsTypelessResourceHandle LoadResourceByType(const nsRTTI* pResourceType, nsStringView sResourceID);

  /// \brief Checks whether any resource loading is in progress
  static bool IsAnyLoadingInProgress();

  /// \brief Generates a unique resource ID with the given prefix.
  ///
  /// Provide a prefix that is preferably not used anywhere else (i.e., closely related to your code).
  /// If the prefix is not also used to manually generate resource IDs, this function is guaranteed to return a unique resource ID.
  static nsString GenerateUniqueResourceID(nsStringView sResourceIDPrefix);

  /// \brief Creates a resource from a descriptor.
  ///
  /// \param szResourceID The unique ID by which the resource is identified. E.g. in GetExistingResource()
  /// \param descriptor A type specific descriptor that holds all the information to create the resource.
  /// \param szResourceDescription An optional description that might help during debugging. Often a human readable name or path is stored
  /// here, to make it easier to identify this resource.
  template <typename ResourceType, typename DescriptorType>
  static nsTypedResourceHandle<ResourceType> CreateResource(nsStringView sResourceID, DescriptorType&& descriptor, nsStringView sResourceDescription = nullptr);

  /// \brief Returns a handle to the resource with the given ID if it exists or creates it from a descriptor.
  ///
  /// \param szResourceID The unique ID by which the resource is identified. E.g. in GetExistingResource()
  /// \param descriptor A type specific descriptor that holds all the information to create the resource.
  /// \param szResourceDescription An optional description that might help during debugging. Often a human readable name or path is stored here, to make it easier to identify this resource.
  template <typename ResourceType, typename DescriptorType>
  static nsTypedResourceHandle<ResourceType> GetOrCreateResource(nsStringView sResourceID, DescriptorType&& descriptor, nsStringView sResourceDescription = nullptr);

  /// \brief Returns a handle to the resource with the given ID. If the resource does not exist, the handle is invalid.
  ///
  /// Use this if a resource needs to be created procedurally (with CreateResource()), but might already have been created.
  /// If the returned handle is invalid, then just go through the resource creation step.
  template <typename ResourceType>
  static nsTypedResourceHandle<ResourceType> GetExistingResource(nsStringView sResourceID);

  /// \brief Same as GetExistingResourceByType() but allows to specify the resource type as an nsRTTI.
  static nsTypelessResourceHandle GetExistingResourceByType(const nsRTTI* pResourceType, nsStringView sResourceID);

  template <typename ResourceType>
  static nsTypedResourceHandle<ResourceType> GetExistingResourceOrCreateAsync(nsStringView sResourceID, nsUniquePtr<nsResourceTypeLoader>&& pLoader, nsTypedResourceHandle<ResourceType> hLoadingFallback = {})
  {
    nsTypelessResourceHandle hTypeless = GetExistingResourceOrCreateAsync(nsGetStaticRTTI<ResourceType>(), sResourceID, std::move(pLoader));

    auto hTyped = nsTypedResourceHandle<ResourceType>((ResourceType*)hTypeless.m_pResource);

    if (hLoadingFallback.IsValid())
    {
      ((ResourceType*)hTypeless.m_pResource)->SetLoadingFallbackResource(hLoadingFallback);
    }

    return hTyped;
  }

  static nsTypelessResourceHandle GetExistingResourceOrCreateAsync(const nsRTTI* pResourceType, nsStringView sResourceID, nsUniquePtr<nsResourceTypeLoader>&& pLoader);

  /// \brief Triggers loading of the given resource. tShouldBeAvailableIn specifies how long the resource is not yet needed, thus allowing
  /// other resources to be loaded first. This is only a hint and there are no guarantees when the resource is available.
  static void PreloadResource(const nsTypelessResourceHandle& hResource);

  /// \brief Similar to locking a resource with 'BlockTillLoaded' acquire mode, but can be done with a typeless handle and does not return a result.
  static void ForceLoadResourceNow(const nsTypelessResourceHandle& hResource);

  /// \brief Returns the current loading state of the given resource.
  static nsResourceState GetLoadingState(const nsTypelessResourceHandle& hResource);

  ///@}
  /// \name Reloading resources
  ///@{

public:
  /// \brief Goes through all resources and makes sure they are reloaded, if they have changed. If bForce is true, all resources
  /// are updated, even if there is no indication that they have changed.
  static nsUInt32 ReloadAllResources(bool bForce);

  /// \brief Goes through all resources of the given type and makes sure they are reloaded, if they have changed. If bForce is true,
  /// resources are updated, even if there is no indication that they have changed.
  template <typename ResourceType>
  static nsUInt32 ReloadResourcesOfType(bool bForce);

  /// \brief Goes through all resources of the given type and makes sure they are reloaded, if they have changed. If bForce is true,
  /// resources are updated, even if there is no indication that they have changed.
  static nsUInt32 ReloadResourcesOfType(const nsRTTI* pType, bool bForce);

  /// \brief Reloads only the one specific resource. If bForce is true, it is updated, even if there is no indication that it has changed.
  template <typename ResourceType>
  static bool ReloadResource(const nsTypedResourceHandle<ResourceType>& hResource, bool bForce);

  /// \brief Reloads only the one specific resource. If bForce is true, it is updated, even if there is no indication that it has changed.
  static bool ReloadResource(const nsRTTI* pType, const nsTypelessResourceHandle& hResource, bool bForce);


  /// \brief Calls ReloadResource() on the given resource, but makes sure that the reload happens with the given custom loader.
  ///
  /// Use this e.g. with a nsResourceLoaderFromMemory to replace an existing resource with new data that was created on-the-fly.
  /// Using this function will set the 'PreventFileReload' flag on the resource and thus prevent further reload actions.
  ///
  /// \sa RestoreResource()
  static void UpdateResourceWithCustomLoader(const nsTypelessResourceHandle& hResource, nsUniquePtr<nsResourceTypeLoader>&& pLoader);

  /// \brief Removes the 'PreventFileReload' flag and forces a reload on the resource.
  ///
  /// \sa UpdateResourceWithCustomLoader()
  static void RestoreResource(const nsTypelessResourceHandle& hResource);

  ///@}
  /// \name Acquiring resources
  ///@{

public:
  /// \brief Acquires a resource pointer from a handle. Prefer to use nsResourceLock, which wraps BeginAcquireResource / EndAcquireResource
  ///
  /// \param hResource The resource to acquire
  /// \param mode The desired way to acquire the resource. See nsResourceAcquireMode for details.
  /// \param hLoadingFallback A custom fallback resource that should be returned if hResource is not yet available. Allows to use domain
  /// specific knowledge to get a better fallback.
  /// \param Priority Allows to adjust the priority of the resource. This will affect how fast
  /// the resource is loaded, in case it is not yet available.
  /// \param out_AcquireResult Returns how successful the acquisition was. See nsResourceAcquireResult for details.
  template <typename ResourceType>
  static ResourceType* BeginAcquireResource(const nsTypedResourceHandle<ResourceType>& hResource, nsResourceAcquireMode mode,
    const nsTypedResourceHandle<ResourceType>& hLoadingFallback = nsTypedResourceHandle<ResourceType>(),
    nsResourceAcquireResult* out_pAcquireResult = nullptr);

  /// \brief Same as BeginAcquireResource but only for the base resource pointer.
  static nsResource* BeginAcquireResourcePointer(const nsRTTI* pType, const nsTypelessResourceHandle& hResource);

  /// \brief Needs to be called in concert with BeginAcquireResource() after accessing a resource has been finished. Prefer to use
  /// nsResourceLock instead.
  template <typename ResourceType>
  static void EndAcquireResource(ResourceType* pResource);

  /// \brief Same as EndAcquireResource but without the template parameter. See also BeginAcquireResourcePointer.
  static void EndAcquireResourcePointer(nsResource* pResource);

  /// \brief Forces the resource manager to treat nsResourceAcquireMode::AllowLoadingFallback as nsResourceAcquireMode::BlockTillLoaded on
  /// BeginAcquireResource.
  static void ForceNoFallbackAcquisition(nsUInt32 uiNumFrames = 0xFFFFFFFF);

  /// \brief If the returned number is greater 0 the resource manager treats nsResourceAcquireMode::AllowLoadingFallback as
  /// nsResourceAcquireMode::BlockTillLoaded on BeginAcquireResource.
  static nsUInt32 GetForceNoFallbackAcquisition();

  /// \brief Retrieves an array of pointers to resources of the indicated type which
  /// are loaded at the moment. Destroy the returned object as soon as possible as it
  /// holds the entire resource manager locked.
  template <typename ResourceType>
  static nsLockedObject<nsMutex, nsDynamicArray<nsResource*>> GetAllResourcesOfType();

  ///@}
  /// \name Unloading resources
  ///@{

public:
  /// \brief Deallocates all resources whose refcount has reached 0. Returns the number of deleted resources.
  static nsUInt32 FreeAllUnusedResources();

  /// \brief Deallocates resources whose refcount has reached 0. Returns the number of deleted resources.
  static nsUInt32 FreeUnusedResources(nsTime timeout, nsTime lastAcquireThreshold);

  /// \brief If timeout is not zero, FreeUnusedResources() is called once every frame with the given parameters.
  static void SetAutoFreeUnused(nsTime timeout, nsTime lastAcquireThreshold);

  /// \brief If set to 'false' resources of the given type will not be incrementally unloaded in the background, when they are not referenced anymore.
  template <typename ResourceType>
  static void SetIncrementalUnloadForResourceType(bool bActive);

  template <typename TypeBeingUpdated, typename TypeItWantsToAcquire>
  static void AllowResourceTypeAcquireDuringUpdateContent()
  {
    AllowResourceTypeAcquireDuringUpdateContent(nsGetStaticRTTI<TypeBeingUpdated>(), nsGetStaticRTTI<TypeItWantsToAcquire>());
  }

  static void AllowResourceTypeAcquireDuringUpdateContent(const nsRTTI* pTypeBeingUpdated, const nsRTTI* pTypeItWantsToAcquire);

  static bool IsResourceTypeAcquireDuringUpdateContentAllowed(const nsRTTI* pTypeBeingUpdated, const nsRTTI* pTypeItWantsToAcquire);

private:
  static nsResult DeallocateResource(nsResource* pResource);

  ///@}
  /// \name Miscellaneous
  ///@{

public:
  /// \brief Returns the resource manager mutex. Allows to lock the manager on a thread when multiple operations need to be done in
  /// sequence.
  static nsMutex& GetMutex() { return s_ResourceMutex; }

  /// \brief Must be called once per frame for some bookkeeping.
  static void PerFrameUpdate();

  /// \brief Makes sure that no further resource loading will take place.
  static void EngineAboutToShutdown();

  /// \brief Calls nsResource::ResetResource() on all resources.
  ///
  /// This is mostly for usage in tools to reset resource whose state can be modified at runtime, to reset them to their original state.
  static void ResetAllResources();

  /// \brief Calls nsResource::UpdateContent() to fill the resource with 'low resolution' data
  ///
  /// This will early out, if the resource has gotten low-res data before.
  /// The resource itself may ignore the data, if it has already gotten low/high res data before.
  ///
  /// The typical use case is, that some other piece of code stores a low-res version of a resource to be able to get
  /// a resource into a usable state. For instance, a material may store low resolution texture data for every texture that it references.
  /// Then when 'loading' the textures, it can pass this low-res data to the textures, such that rendering can give decent results right
  /// away. If the textures have already been loaded before, or some other material already had low-res data, the call exits quickly.
  static void SetResourceLowResData(const nsTypelessResourceHandle& hResource, nsStreamReader* pStream);

  ///@}
  /// \name Type specific loaders
  ///@{

public:
  /// \brief Sets the resource loader to use when no type specific resource loader is available.
  static void SetDefaultResourceLoader(nsResourceTypeLoader* pDefaultLoader);

  /// \brief Returns the resource loader to use when no type specific resource loader is available.
  static nsResourceTypeLoader* GetDefaultResourceLoader();

  /// \brief Sets the resource loader to use for the given resource type.
  ///
  /// \note This is bound to one specific type. Derived types do not inherit the type loader.
  template <typename ResourceType>
  static void SetResourceTypeLoader(nsResourceTypeLoader* pCreator);

  ///@}
  /// \name Named resources
  ///@{

public:
  /// \brief Registers a 'named' resource. When a resource is looked up using \a szLookupName, the lookup will be redirected to \a
  /// szRedirectionResource.
  ///
  /// This can be used to register a resource under an easier to use name. For example one can register "MenuBackground" as the name for "{
  /// E50DCC85-D375-4999-9CFE-42F1377FAC85 }". If the lookup name already exists, it will be overwritten.
  static void RegisterNamedResource(nsStringView sLookupName, nsStringView sRedirectionResource);

  /// \brief Removes a previously registered name from the redirection table.
  static void UnregisterNamedResource(nsStringView sLookupName);


  ///@}
  /// \name Asset system interaction
  ///@{

public:
  /// \brief Registers which resource type to use to load an asset with the given type name
  static void RegisterResourceForAssetType(nsStringView sAssetTypeName, const nsRTTI* pResourceType);

  /// \brief Returns the resource type that was registered to handle the given asset type for loading. nullptr if no resource type was
  /// registered for this asset type.
  static const nsRTTI* FindResourceForAssetType(nsStringView sAssetTypeName);

  ///@}
  /// \name Export mode
  ///@{

public:
  /// \brief Enables export mode. In this mode the resource manager will assert when it actually tries to load a resource.
  /// This can be useful when exporting resource handles but the actual resource content is not needed.
  static void EnableExportMode(bool bEnable);

  /// \brief Returns whether export mode is active.
  static bool IsExportModeEnabled();

  /// \brief Creates a resource handle for the given resource ID. This method can only be used if export mode is enabled.
  /// Internally it will create a resource but does not load the content. This way it can be ensured that the resource handle is always only
  /// the size of a pointer.
  template <typename ResourceType>
  static nsTypedResourceHandle<ResourceType> GetResourceHandleForExport(nsStringView sResourceID);


  ///@}
  /// \name Resource Type Overrides
  ///@{

public:
  /// \brief Registers a resource type to be used instead of any of it's base classes, when loading specific data
  ///
  /// When resource B is derived from A it can be registered to be instantiated when loading data, even if the code specifies to use a
  /// resource of type A.
  /// Whenever LoadResource<A>() is executed, the registered callback \a OverrideDecider is run to figure out whether B should be
  /// instantiated instead. If OverrideDecider returns true, B is used.
  ///
  /// OverrideDecider is given the resource ID after it has been resolved by the nsFileSystem. So it has to be able to make its decision
  /// from the file path, name or extension.
  /// The override is registered for all base classes of \a pDerivedTypeToUse, in case the derivation hierarchy is longer.
  ///
  /// Without calling this at startup, a derived resource type has to be manually requested in code.
  static void RegisterResourceOverrideType(const nsRTTI* pDerivedTypeToUse, nsDelegate<bool(const nsStringBuilder&)> overrideDecider);

  /// \brief Unregisters \a pDerivedTypeToUse as an override resource
  ///
  /// \sa RegisterResourceOverrideType()
  static void UnregisterResourceOverrideType(const nsRTTI* pDerivedTypeToUse);

  ///@}
  /// \name Resource Fallbacks
  ///@{

public:
  /// \brief Specifies which resource to use as a loading fallback for the given type, while a resource is not yet loaded.
  template <typename RESOURCE_TYPE>
  static void SetResourceTypeLoadingFallback(const nsTypedResourceHandle<RESOURCE_TYPE>& hResource)
  {
    RESOURCE_TYPE::SetResourceTypeLoadingFallback(hResource);
  }

  /// \sa SetResourceTypeLoadingFallback()
  template <typename RESOURCE_TYPE>
  static inline const nsTypedResourceHandle<RESOURCE_TYPE>& GetResourceTypeLoadingFallback()
  {
    return RESOURCE_TYPE::GetResourceTypeLoadingFallback();
  }

  /// \brief Specifies which resource to use as a missing fallback for the given type, when a resource cannot be loaded.
  ///
  /// \note If no missing fallback is specified, trying to load a resource that does not exist will assert at runtime.
  template <typename RESOURCE_TYPE>
  static void SetResourceTypeMissingFallback(const nsTypedResourceHandle<RESOURCE_TYPE>& hResource)
  {
    RESOURCE_TYPE::SetResourceTypeMissingFallback(hResource);
  }

  /// \sa SetResourceTypeMissingFallback()
  template <typename RESOURCE_TYPE>
  static inline const nsTypedResourceHandle<RESOURCE_TYPE>& GetResourceTypeMissingFallback()
  {
    return RESOURCE_TYPE::GetResourceTypeMissingFallback();
  }

  using ResourceCleanupCB = nsDelegate<void()>;

  /// \brief [internal] Used by nsResource to register a cleanup function to be called at resource manager shutdown.
  static void AddResourceCleanupCallback(ResourceCleanupCB cb);

  /// \sa AddResourceCleanupCallback()
  static void ClearResourceCleanupCallback(ResourceCleanupCB cb);

  /// \brief This will clear ALL resources that were registered as 'missing' or 'loading' fallback resources. This is called early during
  /// system shutdown to clean up resources.
  static void ExecuteAllResourceCleanupCallbacks();

  ///@}
  /// \name Resource Priorities
  ///@{

public:
  /// \brief Specifies which resource to use as a loading fallback for the given type, while a resource is not yet loaded.
  template <typename RESOURCE_TYPE>
  static void SetResourceTypeDefaultPriority(nsResourcePriority priority)
  {
    GetResourceTypePriorities()[nsGetStaticRTTI<RESOURCE_TYPE>()] = priority;
  }

private:
  static nsMap<const nsRTTI*, nsResourcePriority>& GetResourceTypePriorities();
  ///@}

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

private:
  friend class nsResource;
  friend class nsResourceManagerWorkerDataLoad;
  friend class nsResourceManagerWorkerUpdateContent;
  friend class nsResourceHandleReadContext;

  // Events
private:
  static void BroadcastResourceEvent(const nsResourceEvent& e);

  // Miscellaneous
private:
  static nsMutex s_ResourceMutex;

  // Startup / shutdown
private:
  NS_MAKE_SUBSYSTEM_STARTUP_FRIEND(Core, ResourceManager);
  static void OnEngineShutdown();
  static void OnCoreShutdown();
  static void OnCoreStartup();
  static void PluginEventHandler(const nsPluginEvent& e);

  // Loading / reloading / creating resources
private:
  struct LoadedResources
  {
    nsHashTable<nsTempHashedString, nsResource*> m_Resources;
  };

  struct LoadingInfo
  {
    float m_fPriority = 0;
    nsResource* m_pResource = nullptr;

    NS_ALWAYS_INLINE bool operator==(const LoadingInfo& rhs) const { return m_pResource == rhs.m_pResource; }
    NS_ALWAYS_INLINE bool operator<(const LoadingInfo& rhs) const { return m_fPriority < rhs.m_fPriority; }
  };
  static void EnsureResourceLoadingState(nsResource* pResource, const nsResourceState RequestedState);
  static void PreloadResource(nsResource* pResource);
  static void InternalPreloadResource(nsResource* pResource, bool bHighestPriority);

  template <typename ResourceType>
  static ResourceType* GetResource(nsStringView sResourceID, bool bIsReloadable);
  static nsResource* GetResource(const nsRTTI* pRtti, nsStringView sResourceID, bool bIsReloadable);
  static void RunWorkerTask(nsResource* pResource);
  static void UpdateLoadingDeadlines();
  static void ReverseBubbleSortStep(nsDeque<LoadingInfo>& data);
  static bool ReloadResource(nsResource* pResource, bool bForce);

  static void SetupWorkerTasks();
  static nsTime GetLastFrameUpdate();
  static nsHashTable<const nsRTTI*, LoadedResources>& GetLoadedResources();
  static nsDynamicArray<nsResource*>& GetLoadedResourceOfTypeTempContainer();

  NS_ALWAYS_INLINE static bool IsQueuedForLoading(nsResource* pResource) { return pResource->m_Flags.IsSet(nsResourceFlags::IsQueuedForLoading); }
  [[nodiscard]] static nsResult RemoveFromLoadingQueue(nsResource* pResource);
  static void AddToLoadingQueue(nsResource* pResource, bool bHighPriority);

  struct ResourceTypeInfo
  {
    bool m_bIncrementalUnload = true;
    bool m_bAllowNestedAcquireCached = false;

    nsHybridArray<const nsRTTI*, 8> m_NestedTypes;
  };

  static ResourceTypeInfo& GetResourceTypeInfo(const nsRTTI* pRtti);

  // Type loaders
private:
  static nsResourceTypeLoader* GetResourceTypeLoader(const nsRTTI* pRTTI);

  static nsMap<const nsRTTI*, nsResourceTypeLoader*>& GetResourceTypeLoaders();

  // Override / derived resources
private:
  struct DerivedTypeInfo
  {
    const nsRTTI* m_pDerivedType = nullptr;
    nsDelegate<bool(const nsStringBuilder&)> m_Decider;
  };

  /// \brief Checks whether there is a type override for pRtti given szResourceID and returns that
  static const nsRTTI* FindResourceTypeOverride(const nsRTTI* pRtti, nsStringView sResourceID);
};

#include <Core/ResourceManager/Implementation/ResourceLock.h>
#include <Core/ResourceManager/Implementation/ResourceManager_inl.h>
