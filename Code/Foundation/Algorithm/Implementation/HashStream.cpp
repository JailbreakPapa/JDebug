#include <Foundation/FoundationPCH.h>

#include <Foundation/Algorithm/HashStream.h>

NS_WARNING_PUSH()
NS_WARNING_DISABLE_CLANG("-Wunused-function")

#define XXH_INLINE_ALL
#include <Foundation/ThirdParty/xxHash/xxhash.h>

NS_WARNING_POP()

nsHashStreamWriter32::nsHashStreamWriter32(nsUInt32 uiSeed)
{
  m_pState = XXH32_createState();
  NS_VERIFY(XXH_OK == XXH32_reset((XXH32_state_t*)m_pState, uiSeed), "");
}

nsHashStreamWriter32::~nsHashStreamWriter32()
{
  XXH32_freeState((XXH32_state_t*)m_pState);
}

nsResult nsHashStreamWriter32::WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
{
  if (uiBytesToWrite > std::numeric_limits<size_t>::max())
    return NS_FAILURE;

  if (XXH_OK == XXH32_update((XXH32_state_t*)m_pState, pWriteBuffer, static_cast<size_t>(uiBytesToWrite)))
    return NS_SUCCESS;

  return NS_FAILURE;
}

nsUInt32 nsHashStreamWriter32::GetHashValue() const
{
  return XXH32_digest((XXH32_state_t*)m_pState);
}


nsHashStreamWriter64::nsHashStreamWriter64(nsUInt64 uiSeed)
{
  m_pState = XXH64_createState();
  NS_VERIFY(XXH_OK == XXH64_reset((XXH64_state_t*)m_pState, uiSeed), "");
}

nsHashStreamWriter64::~nsHashStreamWriter64()
{
  XXH64_freeState((XXH64_state_t*)m_pState);
}

nsResult nsHashStreamWriter64::WriteBytes(const void* pWriteBuffer, nsUInt64 uiBytesToWrite)
{
  if (uiBytesToWrite > std::numeric_limits<size_t>::max())
    return NS_FAILURE;

  if (XXH_OK == XXH64_update((XXH64_state_t*)m_pState, pWriteBuffer, static_cast<size_t>(uiBytesToWrite)))
    return NS_SUCCESS;

  return NS_FAILURE;
}

nsUInt64 nsHashStreamWriter64::GetHashValue() const
{
  return XXH64_digest((XXH64_state_t*)m_pState);
}
