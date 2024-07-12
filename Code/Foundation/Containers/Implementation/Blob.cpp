#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/Blob.h>

#include <Foundation/Memory/AllocatorWithPolicy.h>

nsBlob::nsBlob() = default;

nsBlob::nsBlob(nsBlob&& other)
{
  m_pStorage = other.m_pStorage;
  m_uiSize = other.m_uiSize;

  other.m_pStorage = nullptr;
  other.m_uiSize = 0;
}

void nsBlob::operator=(nsBlob&& rhs)
{
  Clear();

  m_pStorage = rhs.m_pStorage;
  m_uiSize = rhs.m_uiSize;

  rhs.m_pStorage = nullptr;
  rhs.m_uiSize = 0;
}

nsBlob::~nsBlob()
{
  Clear();
}

void nsBlob::SetFrom(const void* pSource, nsUInt64 uiSize)
{
  SetCountUninitialized(uiSize);
  nsMemoryUtils::Copy(static_cast<nsUInt8*>(m_pStorage), static_cast<const nsUInt8*>(pSource), static_cast<size_t>(uiSize));
}

void nsBlob::Clear()
{
  if (m_pStorage)
  {
    nsFoundation::GetAlignedAllocator()->Deallocate(m_pStorage);
    m_pStorage = nullptr;
    m_uiSize = 0;
  }
}

void nsBlob::SetCountUninitialized(nsUInt64 uiCount)
{
  if (m_uiSize != uiCount)
  {
    Clear();

    m_pStorage = nsFoundation::GetAlignedAllocator()->Allocate(nsMath::SafeConvertToSizeT(uiCount), 64u);
    m_uiSize = uiCount;
  }
}

void nsBlob::ZeroFill()
{
  if (m_pStorage)
  {
    nsMemoryUtils::ZeroFill(static_cast<nsUInt8*>(m_pStorage), static_cast<size_t>(m_uiSize));
  }
}

bool nsBlob::IsEmpty() const
{
  return 0 == m_uiSize;
}

NS_STATICLINK_FILE(Foundation, Foundation_Containers_Implementation_Blob);
