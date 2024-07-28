
#pragma once

#include <Foundation/Containers/HashTable.h>
#include <Foundation/Strings/HashedString.h>
#include <RendererFoundation/RendererFoundationDLL.h>

class NS_RENDERERFOUNDATION_DLL nsGALResourceBase : public nsRefCounted
{
public:
  void SetDebugName(const char* szName) const
  {
#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
    m_sDebugName.Assign(szName);
#endif

    SetDebugNamePlatform(szName);
  }

  virtual const nsGALResourceBase* GetParentResource() const { return this; }

protected:
  friend class nsGALDevice;

  inline ~nsGALResourceBase() = default;

  virtual void SetDebugNamePlatform(const char* szName) const = 0;

#if NS_ENABLED(NS_COMPILE_FOR_DEVELOPMENT)
  mutable nsHashedString m_sDebugName;
#endif
};

/// \brief Base class for GAL resources, stores a creation description of the object and also allows for reference counting.
template <typename CreationDescription>
class nsGALResource : public nsGALResourceBase
{
public:
  NS_ALWAYS_INLINE nsGALResource(const CreationDescription& description)
    : m_Description(description)
  {
  }

  NS_ALWAYS_INLINE const CreationDescription& GetDescription() const { return m_Description; }

protected:
  const CreationDescription m_Description;
};
