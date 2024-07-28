#include <Core/CorePCH.h>

#include <Core/ResourceManager/Resource.h>

nsTypelessResourceHandle::nsTypelessResourceHandle(nsResource* pResource)
{
  m_pResource = pResource;

  if (m_pResource)
  {
    IncreaseResourceRefCount(m_pResource, this);
  }
}

void nsTypelessResourceHandle::Invalidate()
{
  if (m_pResource)
  {
    DecreaseResourceRefCount(m_pResource, this);
  }

  m_pResource = nullptr;
}

nsUInt64 nsTypelessResourceHandle::GetResourceIDHash() const
{
  return IsValid() ? m_pResource->GetResourceIDHash() : 0;
}

const nsString& nsTypelessResourceHandle::GetResourceID() const
{
  return m_pResource->GetResourceID();
}

const nsRTTI* nsTypelessResourceHandle::GetResourceType() const
{
  return IsValid() ? m_pResource->GetDynamicRTTI() : nullptr;
}

void nsTypelessResourceHandle::operator=(const nsTypelessResourceHandle& rhs)
{
  NS_ASSERT_DEBUG(this != &rhs, "Cannot assign a resource handle to itself! This would invalidate the handle.");

  Invalidate();

  m_pResource = rhs.m_pResource;

  if (m_pResource)
  {
    IncreaseResourceRefCount(reinterpret_cast<nsResource*>(m_pResource), this);
  }
}

void nsTypelessResourceHandle::operator=(nsTypelessResourceHandle&& rhs)
{
  Invalidate();

  m_pResource = rhs.m_pResource;
  rhs.m_pResource = nullptr;

  if (m_pResource)
  {
    MigrateResourceRefCount(m_pResource, &rhs, this);
  }
}

// static
void nsResourceHandleStreamOperations::WriteHandle(nsStreamWriter& Stream, const nsResource* pResource)
{
  if (pResource != nullptr)
  {
    Stream << pResource->GetDynamicRTTI()->GetTypeName();
    Stream << pResource->GetResourceID();
  }
  else
  {
    const char* szEmpty = "";
    Stream << szEmpty;
  }
}

// static
void nsResourceHandleStreamOperations::ReadHandle(nsStreamReader& Stream, nsTypelessResourceHandle& ResourceHandle)
{
  nsStringBuilder sTemp;

  Stream >> sTemp;
  if (sTemp.IsEmpty())
  {
    ResourceHandle.Invalidate();
    return;
  }

  const nsRTTI* pRtti = nsResourceManager::FindResourceForAssetType(sTemp);

  if (pRtti == nullptr)
  {
    pRtti = nsRTTI::FindTypeByName(sTemp);
  }

  if (pRtti == nullptr)
  {
    nsLog::Error("Unknown resource type '{0}'", sTemp);
    ResourceHandle.Invalidate();
  }

  // read unique ID for restoring the resource (from file)
  Stream >> sTemp;

  if (pRtti != nullptr)
  {
    ResourceHandle = nsResourceManager::LoadResourceByType(pRtti, sTemp);
  }
}
