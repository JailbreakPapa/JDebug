#pragma once

#include <Core/CoreDLL.h>
#include <Core/ResourceManager/Implementation/Declarations.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

/// \brief If this is set to NS_ON, stack traces are recorded for every resource handle.
///
/// This can be used to find the places that create resource handles but do not properly clean them up.
#define NS_RESOURCEHANDLE_STACK_TRACES NS_OFF

class nsResource;

template <typename T>
class nsResourceLock;

// These out-of-line helper functions allow to forward declare resource handles without knowledge about the resource class.
NS_CORE_DLL void IncreaseResourceRefCount(nsResource* pResource, const void* pOwner);
NS_CORE_DLL void DecreaseResourceRefCount(nsResource* pResource, const void* pOwner);

#if NS_ENABLED(NS_RESOURCEHANDLE_STACK_TRACES)
NS_CORE_DLL void MigrateResourceRefCount(nsResource* pResource, const void* pOldOwner, const void* pNewOwner);
#else
NS_ALWAYS_INLINE void MigrateResourceRefCount(nsResource* pResource, const void* pOldOwner, const void* pNewOwner)
{
}
#endif

/// \brief The typeless implementation of resource handles. A typed interface is provided by nsTypedResourceHandle.
class NS_CORE_DLL nsTypelessResourceHandle
{
public:
  NS_ALWAYS_INLINE nsTypelessResourceHandle() = default;

  /// \brief [internal] Increases the refcount of the given resource.
  nsTypelessResourceHandle(nsResource* pResource);

  /// \brief Increases the refcount of the given resource
  NS_ALWAYS_INLINE nsTypelessResourceHandle(const nsTypelessResourceHandle& rhs)
  {
    m_pResource = rhs.m_pResource;

    if (m_pResource)
    {
      IncreaseResourceRefCount(m_pResource, this);
    }
  }

  /// \brief Move constructor, no refcount change is necessary.
  NS_ALWAYS_INLINE nsTypelessResourceHandle(nsTypelessResourceHandle&& rhs)
  {
    m_pResource = rhs.m_pResource;
    rhs.m_pResource = nullptr;

    if (m_pResource)
    {
      MigrateResourceRefCount(m_pResource, &rhs, this);
    }
  }

  /// \brief Releases any referenced resource.
  NS_ALWAYS_INLINE ~nsTypelessResourceHandle() { Invalidate(); }

  /// \brief Returns whether the handle stores a valid pointer to a resource.
  NS_ALWAYS_INLINE bool IsValid() const { return m_pResource != nullptr; }

  /// \brief Clears any reference to a resource and reduces its refcount.
  void Invalidate();

  /// \brief Returns the Resource ID hash of the exact resource that this handle points to, without acquiring the resource.
  /// The handle must be valid.
  nsUInt64 GetResourceIDHash() const;

  /// \brief Returns the Resource ID of the exact resource that this handle points to, without acquiring the resource.
  /// The handle must be valid.
  const nsString& GetResourceID() const;

  /// \brief Releases the current reference and increases the refcount of the given resource.
  void operator=(const nsTypelessResourceHandle& rhs);

  /// \brief Move operator, no refcount change is necessary.
  void operator=(nsTypelessResourceHandle&& rhs);

  /// \brief Checks whether the two handles point to the same resource.
  NS_ALWAYS_INLINE bool operator==(const nsTypelessResourceHandle& rhs) const { return m_pResource == rhs.m_pResource; }

  /// \brief Checks whether the two handles point to the same resource.
  NS_ALWAYS_INLINE bool operator!=(const nsTypelessResourceHandle& rhs) const { return m_pResource != rhs.m_pResource; }

  /// \brief For storing handles as keys in maps
  NS_ALWAYS_INLINE bool operator<(const nsTypelessResourceHandle& rhs) const { return m_pResource < rhs.m_pResource; }

  /// \brief Checks whether the handle points to the given resource.
  NS_ALWAYS_INLINE bool operator==(const nsResource* rhs) const { return m_pResource == rhs; }

  /// \brief Checks whether the handle points to the given resource.
  NS_ALWAYS_INLINE bool operator!=(const nsResource* rhs) const { return m_pResource != rhs; }

  /// \brief Returns the type information of the resource or nullptr if the handle is invalid.
  const nsRTTI* GetResourceType() const;

protected:
  nsResource* m_pResource = nullptr;

private:
  // you must go through the resource manager to get access to the resource pointer
  friend class nsResourceManager;
  friend class nsResourceHandleWriteContext;
  friend class nsResourceHandleReadContext;
  friend class nsResourceHandleStreamOperations;
};

template <>
struct nsHashHelper<nsTypelessResourceHandle>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsTypelessResourceHandle& value) { return nsHashingUtils::StringHashTo32(value.GetResourceIDHash()); }

  NS_ALWAYS_INLINE static bool Equal(const nsTypelessResourceHandle& a, const nsTypelessResourceHandle& b) { return a == b; }
};

/// \brief The nsTypedResourceHandle controls access to an nsResource.
///
/// All resources must be referenced using nsTypedResourceHandle instances (instantiated with the proper resource type as the template
/// argument). You must not store a direct pointer to a resource anywhere. Instead always store resource handles. To actually access a
/// resource, use nsResourceManager::BeginAcquireResource and nsResourceManager::EndAcquireResource after you have finished using it.
///
/// nsTypedResourceHandle implements reference counting on resources. It also allows to redirect resources to fallback resources when they
/// are not yet loaded (if possible).
///
/// As long as there is one resource handle that references a resource, it is considered 'in use' and thus might not get unloaded.
/// So be careful where you store resource handles.
/// If necessary you can call Invalidate() to clear a resource handle and thus also remove the reference to the resource.
template <typename RESOURCE_TYPE>
class nsTypedResourceHandle
{
public:
  using ResourceType = RESOURCE_TYPE;

  /// \brief A default constructed handle is invalid and does not reference any resource.
  nsTypedResourceHandle() = default;

  /// \brief Increases the refcount of the given resource.
  explicit nsTypedResourceHandle(ResourceType* pResource)
    : m_hTypeless(pResource)
  {
  }

  /// \brief Increases the refcount of the given resource.
  nsTypedResourceHandle(const nsTypedResourceHandle<ResourceType>& rhs)
    : m_hTypeless(rhs.m_hTypeless)
  {
  }

  /// \brief Move constructor, no refcount change is necessary.
  nsTypedResourceHandle(nsTypedResourceHandle<ResourceType>&& rhs)
    : m_hTypeless(std::move(rhs.m_hTypeless))
  {
  }

  template <typename BaseOrDerivedType>
  nsTypedResourceHandle(const nsTypedResourceHandle<BaseOrDerivedType>& rhs)
    : m_hTypeless(rhs.m_hTypeless)
  {
    static_assert(std::is_base_of<ResourceType, BaseOrDerivedType>::value || std::is_base_of<BaseOrDerivedType, ResourceType>::value, "Only related types can be assigned to handles of this type");

#if NS_ENABLED(NS_COMPILE_FOR_DEBUG)
    if (std::is_base_of<BaseOrDerivedType, ResourceType>::value)
    {
      NS_ASSERT_DEBUG(rhs.IsValid(), "Cannot cast invalid base handle to derived type!");
      nsResourceLock<BaseOrDerivedType> lock(rhs, nsResourceAcquireMode::PointerOnly);
      NS_ASSERT_DEBUG(nsDynamicCast<const ResourceType*>(lock.GetPointer()) != nullptr, "Types are not related!");
    }
#endif
  }

  /// \brief Releases the current reference and increases the refcount of the given resource.
  void operator=(const nsTypedResourceHandle<ResourceType>& rhs) { m_hTypeless = rhs.m_hTypeless; }

  /// \brief Move operator, no refcount change is necessary.
  void operator=(nsTypedResourceHandle<ResourceType>&& rhs) { m_hTypeless = std::move(rhs.m_hTypeless); }

  /// \brief Checks whether the two handles point to the same resource.
  NS_ALWAYS_INLINE bool operator==(const nsTypedResourceHandle<ResourceType>& rhs) const { return m_hTypeless == rhs.m_hTypeless; }

  /// \brief Checks whether the two handles point to the same resource.
  NS_ALWAYS_INLINE bool operator!=(const nsTypedResourceHandle<ResourceType>& rhs) const { return m_hTypeless != rhs.m_hTypeless; }

  /// \brief For storing handles as keys in maps
  NS_ALWAYS_INLINE bool operator<(const nsTypedResourceHandle<ResourceType>& rhs) const { return m_hTypeless < rhs.m_hTypeless; }

  /// \brief Checks whether the handle points to the given resource.
  NS_ALWAYS_INLINE bool operator==(const nsResource* rhs) const { return m_hTypeless == rhs; }

  /// \brief Checks whether the handle points to the given resource.
  NS_ALWAYS_INLINE bool operator!=(const nsResource* rhs) const { return m_hTypeless != rhs; }


  /// \brief Returns the corresponding typeless resource handle.
  NS_ALWAYS_INLINE operator const nsTypelessResourceHandle() const { return m_hTypeless; }

  /// \brief Returns the corresponding typeless resource handle.
  NS_ALWAYS_INLINE operator nsTypelessResourceHandle() { return m_hTypeless; }

  /// \brief Returns whether the handle stores a valid pointer to a resource.
  NS_ALWAYS_INLINE bool IsValid() const { return m_hTypeless.IsValid(); }

  /// \brief Returns whether the handle stores a valid pointer to a resource.
  NS_ALWAYS_INLINE explicit operator bool() const { return m_hTypeless.IsValid(); }

  /// \brief Clears any reference to a resource and reduces its refcount.
  NS_ALWAYS_INLINE void Invalidate() { m_hTypeless.Invalidate(); }

  /// \brief Returns the Resource ID hash of the exact resource that this handle points to, without acquiring the resource.
  /// The handle must be valid.
  NS_ALWAYS_INLINE nsUInt64 GetResourceIDHash() const { return m_hTypeless.GetResourceIDHash(); }

  /// \brief Returns the Resource ID of the exact resource that this handle points to, without acquiring the resource.
  /// The handle must be valid.
  NS_ALWAYS_INLINE const nsString& GetResourceID() const { return m_hTypeless.GetResourceID(); }

  /// \brief Attempts to copy the given typeless handle to this handle.
  ///
  /// It is an error to assign a typeless handle that references a resource with a mismatching type.
  void AssignFromTypelessHandle(const nsTypelessResourceHandle& hHandle)
  {
    if (!hHandle.IsValid())
      return;

    NS_ASSERT_DEV(hHandle.GetResourceType()->IsDerivedFrom<RESOURCE_TYPE>(), "Type '{}' does not match resource type '{}' in typeless handle.", nsGetStaticRTTI<RESOURCE_TYPE>()->GetTypeName(), hHandle.GetResourceType()->GetTypeName());

    m_hTypeless = hHandle;
  }

private:
  template <typename T>
  friend class nsTypedResourceHandle;

  // you must go through the resource manager to get access to the resource pointer
  friend class nsResourceManager;
  friend class nsResourceHandleWriteContext;
  friend class nsResourceHandleReadContext;
  friend class nsResourceHandleStreamOperations;

  nsTypelessResourceHandle m_hTypeless;
};

template <typename T>
struct nsHashHelper<nsTypedResourceHandle<T>>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const nsTypedResourceHandle<T>& value) { return nsHashingUtils::StringHashTo32(value.GetResourceIDHash()); }

  NS_ALWAYS_INLINE static bool Equal(const nsTypedResourceHandle<T>& a, const nsTypedResourceHandle<T>& b) { return a == b; }
};

// Stream operations
class nsResource;

class NS_CORE_DLL nsResourceHandleStreamOperations
{
public:
  template <typename ResourceType>
  static void WriteHandle(nsStreamWriter& inout_stream, const nsTypedResourceHandle<ResourceType>& hResource)
  {
    WriteHandle(inout_stream, hResource.m_hTypeless.m_pResource);
  }

  template <typename ResourceType>
  static void ReadHandle(nsStreamReader& inout_stream, nsTypedResourceHandle<ResourceType>& ref_hResourceHandle)
  {
    ReadHandle(inout_stream, ref_hResourceHandle.m_hTypeless);
  }

private:
  static void WriteHandle(nsStreamWriter& Stream, const nsResource* pResource);
  static void ReadHandle(nsStreamReader& Stream, nsTypelessResourceHandle& ResourceHandle);
};

/// \brief Operator to serialize resource handles
template <typename ResourceType>
void operator<<(nsStreamWriter& inout_stream, const nsTypedResourceHandle<ResourceType>& hValue)
{
  nsResourceHandleStreamOperations::WriteHandle(inout_stream, hValue);
}

/// \brief Operator to deserialize resource handles
template <typename ResourceType>
void operator>>(nsStreamReader& inout_stream, nsTypedResourceHandle<ResourceType>& ref_hValue)
{
  nsResourceHandleStreamOperations::ReadHandle(inout_stream, ref_hValue);
}
