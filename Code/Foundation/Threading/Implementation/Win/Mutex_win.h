#ifdef NS_MUTEX_WIN_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define NS_MUTEX_WIN_INL_H_INCLUDED

#if NS_ENABLED(NS_COMPILER_MSVC) && NS_ENABLED(NS_PLATFORM_ARCH_X86)

extern "C"
{
  // The main purpose of this little hack here is to have Mutex::Lock and Mutex::Unlock inline-able without including windows.h
  // The hack however does only work on the MSVC compiler. See fall back code below.

  // First define two functions which are binary compatible with EnterCriticalSection and LeaveCriticalSection
  __declspec(dllimport) void __stdcall nsWinEnterCriticalSection(nsMutexHandle* handle);
  __declspec(dllimport) void __stdcall nsWinLeaveCriticalSection(nsMutexHandle* handle);
  __declspec(dllimport) nsMinWindows::BOOL __stdcall nsWinTryEnterCriticalSection(nsMutexHandle* handle);

  // Now redirect them through linker flags to the correct implementation
#  if NS_ENABLED(NS_PLATFORM_32BIT)
#    pragma comment(linker, "/alternatename:__imp__nsWinEnterCriticalSection@4=__imp__EnterCriticalSection@4")
#    pragma comment(linker, "/alternatename:__imp__nsWinLeaveCriticalSection@4=__imp__LeaveCriticalSection@4")
#    pragma comment(linker, "/alternatename:__imp__nsWinTryEnterCriticalSection@4=__imp__TryEnterCriticalSection@4")
#  else
#    pragma comment(linker, "/alternatename:__imp_nsWinEnterCriticalSection=__imp_EnterCriticalSection")
#    pragma comment(linker, "/alternatename:__imp_nsWinLeaveCriticalSection=__imp_LeaveCriticalSection")
#    pragma comment(linker, "/alternatename:__imp_nsWinTryEnterCriticalSection=__imp_TryEnterCriticalSection")
#  endif
}

inline void nsMutex::Lock()
{
  nsWinEnterCriticalSection(&m_hHandle);
  ++m_iLockCount;
}

inline void nsMutex::Unlock()
{
  --m_iLockCount;
  nsWinLeaveCriticalSection(&m_hHandle);
}

inline nsResult nsMutex::TryLock()
{
  if (nsWinTryEnterCriticalSection(&m_hHandle) != 0)
  {
    ++m_iLockCount;
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

#else

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

inline void nsMutex::Lock()
{
  EnterCriticalSection((CRITICAL_SECTION*)&m_hHandle);
  ++m_iLockCount;
}

inline void nsMutex::Unlock()
{
  --m_iLockCount;
  LeaveCriticalSection((CRITICAL_SECTION*)&m_hHandle);
}

inline nsResult nsMutex::TryLock()
{
  if (TryEnterCriticalSection((CRITICAL_SECTION*)&m_hHandle) != 0)
  {
    ++m_iLockCount;
    return NS_SUCCESS;
  }

  return NS_FAILURE;
}

#endif
