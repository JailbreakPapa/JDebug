#include <Foundation/FoundationPCH.h>

#include <Foundation/Algorithm/HashStream.h>

#define XXH_INLINE_ALL
#include <Foundation/ThirdParty/xxHash/xxhash.h>

wdHashStreamWriter32::wdHashStreamWriter32(wdUInt32 uiSeed)
{
  m_pState = XXH32_createState();
  WD_VERIFY(XXH_OK == XXH32_reset((XXH32_state_t*)m_pState, uiSeed), "");
}

wdHashStreamWriter32::~wdHashStreamWriter32()
{
  XXH32_freeState((XXH32_state_t*)m_pState);
}

wdResult wdHashStreamWriter32::WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite)
{
  if (uiBytesToWrite > std::numeric_limits<size_t>::max())
    return WD_FAILURE;

  if (XXH_OK == XXH32_update((XXH32_state_t*)m_pState, pWriteBuffer, static_cast<size_t>(uiBytesToWrite)))
    return WD_SUCCESS;

  return WD_FAILURE;
}

wdUInt32 wdHashStreamWriter32::GetHashValue() const
{
  return XXH32_digest((XXH32_state_t*)m_pState);
}


wdHashStreamWriter64::wdHashStreamWriter64(wdUInt64 uiSeed)
{
  m_pState = XXH64_createState();
  WD_VERIFY(XXH_OK == XXH64_reset((XXH64_state_t*)m_pState, uiSeed), "");
}

wdHashStreamWriter64::~wdHashStreamWriter64()
{
  XXH64_freeState((XXH64_state_t*)m_pState);
}

wdResult wdHashStreamWriter64::WriteBytes(const void* pWriteBuffer, wdUInt64 uiBytesToWrite)
{
  if (uiBytesToWrite > std::numeric_limits<size_t>::max())
    return WD_FAILURE;

  if (XXH_OK == XXH64_update((XXH64_state_t*)m_pState, pWriteBuffer, static_cast<size_t>(uiBytesToWrite)))
    return WD_SUCCESS;

  return WD_FAILURE;
}

wdUInt64 wdHashStreamWriter64::GetHashValue() const
{
  return XXH64_digest((XXH64_state_t*)m_pState);
}

WD_STATICLINK_FILE(Foundation, Foundation_Algorithm_Implementation_HashStream);
