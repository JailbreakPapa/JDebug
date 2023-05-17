#include <Foundation/FoundationPCH.h>

#include <Foundation/Containers/Blob.h>

#include <Foundation/Memory/Allocator.h>

wdBlob::wdBlob() = default;

wdBlob::wdBlob(wdBlob&& other)
{
  m_pStorage = other.m_pStorage;
  m_uiSize = other.m_uiSize;

  other.m_pStorage = nullptr;
  other.m_uiSize = 0;
}

void wdBlob::operator=(wdBlob&& rhs)
{
  Clear();

  m_pStorage = rhs.m_pStorage;
  m_uiSize = rhs.m_uiSize;

  rhs.m_pStorage = nullptr;
  rhs.m_uiSize = 0;
}

wdBlob::~wdBlob()
{
  Clear();
}

void wdBlob::SetFrom(void* pSource, wdUInt64 uiSize)
{
  SetCountUninitialized(uiSize);
  wdMemoryUtils::Copy(static_cast<wdUInt8*>(m_pStorage), static_cast<wdUInt8*>(pSource), static_cast<size_t>(uiSize));
}

void wdBlob::Clear()
{
  if (m_pStorage)
  {
    wdFoundation::GetAlignedAllocator()->Deallocate(m_pStorage);
    m_pStorage = nullptr;
    m_uiSize = 0;
  }
}

void wdBlob::SetCountUninitialized(wdUInt64 uiCount)
{
  if (m_uiSize != uiCount)
  {
    Clear();

    m_pStorage = wdFoundation::GetAlignedAllocator()->Allocate(wdMath::SafeConvertToSizeT(uiCount), 64u);
    m_uiSize = uiCount;
  }
}

void wdBlob::ZeroFill()
{
  if (m_pStorage)
  {
    wdMemoryUtils::ZeroFill(static_cast<wdUInt8*>(m_pStorage), static_cast<size_t>(m_uiSize));
  }
}


WD_STATICLINK_FILE(Foundation, Foundation_Containers_Implementation_Blob);
