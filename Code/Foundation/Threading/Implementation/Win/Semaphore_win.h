#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#include <Foundation/Basics/Platform/Win/MinWindows.h>
#include <Foundation/Strings/StringBuilder.h>

wdSemaphore::wdSemaphore()
{
  m_hSemaphore = nullptr;
}

wdSemaphore::~wdSemaphore()
{
  if (m_hSemaphore != nullptr)
  {
    CloseHandle(m_hSemaphore);
    m_hSemaphore = nullptr;
  }
}

wdResult wdSemaphore::Create(wdUInt32 uiInitialTokenCount, wdStringView sSharedName /*= nullptr*/)
{
  WD_ASSERT_DEV(m_hSemaphore == nullptr, "Semaphore can't be recreated.");

  LPSECURITY_ATTRIBUTES secAttr = nullptr; // default
  const DWORD flags = 0;                   // reserved but unused
  const DWORD access = STANDARD_RIGHTS_ALL | SEMAPHORE_MODIFY_STATE /* needed for ReleaseSemaphore */;

  if (sSharedName.IsEmpty())
  {
    // create an unnamed semaphore

    m_hSemaphore = CreateSemaphoreExW(secAttr, uiInitialTokenCount, wdMath::MaxValue<wdInt32>(), nullptr, flags, access);
  }
  else
  {
    // create a named semaphore in the 'Local' namespace
    // these are visible session wide, ie. all processes by the same user account can see these, but not across users

    const wdStringBuilder semaphoreName("Local\\", sSharedName);

    m_hSemaphore = CreateSemaphoreExW(secAttr, uiInitialTokenCount, wdMath::MaxValue<wdInt32>(), wdStringWChar(semaphoreName).GetData(), flags, access);
  }

  if (m_hSemaphore == nullptr)
  {
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

wdResult wdSemaphore::Open(wdStringView sSharedName)
{
  WD_ASSERT_DEV(m_hSemaphore == nullptr, "Semaphore can't be recreated.");

  LPSECURITY_ATTRIBUTES secAttr = nullptr; // default
  const DWORD flags = 0;                   // reserved but unused
  const DWORD access = SYNCHRONIZE /* needed for WaitForSingleObject */ | SEMAPHORE_MODIFY_STATE /* needed for ReleaseSemaphore */;
  const BOOL inheriteHandle = FALSE;

  WD_ASSERT_DEV(!sSharedName.IsEmpty(), "Name of semaphore to open mustn't be empty.");

  const wdStringBuilder semaphoreName("Local\\", sSharedName);

  m_hSemaphore = OpenSemaphoreW(access, inheriteHandle, wdStringWChar(semaphoreName).GetData());

  if (m_hSemaphore == nullptr)
  {
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

void wdSemaphore::AcquireToken()
{
  WD_ASSERT_DEV(m_hSemaphore != nullptr, "Invalid semaphore.");
  WD_VERIFY(WaitForSingleObject(m_hSemaphore, INFINITE) == WAIT_OBJECT_0, "Semaphore token acquisition failed.");
}

void wdSemaphore::ReturnToken()
{
  WD_ASSERT_DEV(m_hSemaphore != nullptr, "Invalid semaphore.");
  WD_VERIFY(ReleaseSemaphore(m_hSemaphore, 1, nullptr) != 0, "Returning a semaphore token failed, most likely due to a AcquireToken() / ReturnToken() mismatch.");
}

wdResult wdSemaphore::TryAcquireToken()
{
  WD_ASSERT_DEV(m_hSemaphore != nullptr, "Invalid semaphore.");

  const wdUInt32 res = WaitForSingleObject(m_hSemaphore, 0 /* timeout of zero milliseconds */);

  if (res == WAIT_OBJECT_0)
  {
    return WD_SUCCESS;
  }

  WD_ASSERT_DEV(res == WAIT_OBJECT_0 || res == WAIT_TIMEOUT, "Semaphore TryAcquireToken (WaitForSingleObject) failed with error code {}.", res);

  return WD_FAILURE;
}
