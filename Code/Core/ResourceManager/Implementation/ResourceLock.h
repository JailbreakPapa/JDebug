#pragma once

#include <Core/ResourceManager/Implementation/Declarations.h>

/// \brief Helper class to acquire and release a resource safely.
///
/// The constructor calls nsResourceManager::BeginAcquireResource, the destructor makes sure to call nsResourceManager::EndAcquireResource.
/// The instance of this class can be used like a pointer to the resource.
///
/// Whether the acquisition succeeded or returned a loading fallback, missing fallback or even no result, at all,
/// can be retrieved through GetAcquireResult().
/// \note If a resource is missing, but no missing fallback is specified for the resource type, the code will fail with an assertion,
/// unless you used nsResourceAcquireMode::BlockTillLoaded_NeverFail. Only then will the error be silently ignored and the acquire result
/// will be nsResourceAcquireResult::None.
///
/// \sa nsResourceManager::BeginAcquireResource()
/// \sa nsResourceAcquireMode
/// \sa nsResourceAcquireResult
template <class RESOURCE_TYPE>
class nsResourceLock
{
public:
  NS_ALWAYS_INLINE nsResourceLock(const nsTypedResourceHandle<RESOURCE_TYPE>& hResource, nsResourceAcquireMode mode,
    const nsTypedResourceHandle<RESOURCE_TYPE>& hFallbackResource = nsTypedResourceHandle<RESOURCE_TYPE>())
  {
    m_pResource = nsResourceManager::BeginAcquireResource(hResource, mode, hFallbackResource, &m_AcquireResult);
  }

  nsResourceLock(const nsResourceLock&) = delete;

  nsResourceLock(nsResourceLock&& other)
    : m_AcquireResult(other.m_AcquireResult)
    , m_pResource(other.m_pResource)
  {
    other.m_pResource = nullptr;
    other.m_AcquireResult = nsResourceAcquireResult::None;
  }

  NS_ALWAYS_INLINE ~nsResourceLock()
  {
    if (m_pResource)
    {
      nsResourceManager::EndAcquireResource(m_pResource);
    }
  }

  NS_ALWAYS_INLINE RESOURCE_TYPE* operator->() { return m_pResource; }
  NS_ALWAYS_INLINE const RESOURCE_TYPE* operator->() const { return m_pResource; }

  NS_ALWAYS_INLINE bool IsValid() const { return m_pResource != nullptr; }
  NS_ALWAYS_INLINE explicit operator bool() const { return m_pResource != nullptr; }

  NS_ALWAYS_INLINE nsResourceAcquireResult GetAcquireResult() const { return m_AcquireResult; }

  NS_ALWAYS_INLINE const RESOURCE_TYPE* GetPointer() const { return m_pResource; }
  NS_ALWAYS_INLINE RESOURCE_TYPE* GetPointerNonConst() const { return m_pResource; }

private:
  nsResourceAcquireResult m_AcquireResult;
  RESOURCE_TYPE* m_pResource;
};
