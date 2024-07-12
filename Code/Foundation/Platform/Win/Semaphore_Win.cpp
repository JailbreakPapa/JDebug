#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Basics/Platform/Win/MinWindows.h>
#  include <Foundation/Strings/StringBuilder.h>
#  include <Foundation/Threading/Semaphore.h>

nsSemaphore::nsSemaphore()
{
  m_hSemaphore = nullptr;
}

nsSemaphore::~nsSemaphore()
{
  if (m_hSemaphore != nullptr)
  {
    CloseHandle(m_hSemaphore);
    m_hSemaphore = nullptr;
  }
}

nsResult nsSemaphore::Create(nsUInt32 uiInitialTokenCount, nsStringView sSharedName /*= nullptr*/)
{
  NS_ASSERT_DEV(m_hSemaphore == nullptr, "Semaphore can't be recreated.");

  LPSECURITY_ATTRIBUTES secAttr = nullptr; // default
  const DWORD flags = 0;                   // reserved but unused
  const DWORD access = STANDARD_RIGHTS_ALL | SEMAPHORE_MODIFY_STATE /* needed for ReleaseSemaphore */;

  if (sSharedName.IsEmpty())
  {
    // create an unnamed semaphore

    m_hSemaphore = CreateSemaphoreExW(secAttr, uiInitialTokenCount, nsMath::MaxValue<nsInt32>(), nullptr, flags, access);
  }
  else
  {
    // create a named semaphore in the 'Local' namespace
    // these are visible session wide, ie. all processes by the same user account can see these, but not across users

    const nsStringBuilder semaphoreName("Local\\", sSharedName);

    m_hSemaphore = CreateSemaphoreExW(secAttr, uiInitialTokenCount, nsMath::MaxValue<nsInt32>(), nsStringWChar(semaphoreName).GetData(), flags, access);
  }

  if (m_hSemaphore == nullptr)
  {
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult nsSemaphore::Open(nsStringView sSharedName)
{
  NS_ASSERT_DEV(m_hSemaphore == nullptr, "Semaphore can't be recreated.");

  const DWORD access = SYNCHRONIZE /* needed for WaitForSingleObject */ | SEMAPHORE_MODIFY_STATE /* needed for ReleaseSemaphore */;
  const BOOL inheriteHandle = FALSE;

  NS_ASSERT_DEV(!sSharedName.IsEmpty(), "Name of semaphore to open mustn't be empty.");

  const nsStringBuilder semaphoreName("Local\\", sSharedName);

  m_hSemaphore = OpenSemaphoreW(access, inheriteHandle, nsStringWChar(semaphoreName).GetData());

  if (m_hSemaphore == nullptr)
  {
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

void nsSemaphore::AcquireToken()
{
  NS_ASSERT_DEV(m_hSemaphore != nullptr, "Invalid semaphore.");
  NS_VERIFY(WaitForSingleObject(m_hSemaphore, INFINITE) == WAIT_OBJECT_0, "Semaphore token acquisition failed.");
}

void nsSemaphore::ReturnToken()
{
  NS_ASSERT_DEV(m_hSemaphore != nullptr, "Invalid semaphore.");
  NS_VERIFY(ReleaseSemaphore(m_hSemaphore, 1, nullptr) != 0, "Returning a semaphore token failed, most likely due to a AcquireToken() / ReturnToken() mismatch.");
}

nsResult nsSemaphore::TryAcquireToken()
{
  NS_ASSERT_DEV(m_hSemaphore != nullptr, "Invalid semaphore.");

  const nsUInt32 res = WaitForSingleObject(m_hSemaphore, 0 /* timeout of zero milliseconds */);

  if (res == WAIT_OBJECT_0)
  {
    return NS_SUCCESS;
  }

  NS_ASSERT_DEV(res == WAIT_OBJECT_0 || res == WAIT_TIMEOUT, "Semaphore TryAcquireToken (WaitForSingleObject) failed with error code {}.", res);

  return NS_FAILURE;
}

#endif
