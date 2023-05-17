#ifdef WD_MUTEX_WIN_INL_H_INCLUDED
#  error "This file must not be included twice."
#endif

#define WD_MUTEX_WIN_INL_H_INCLUDED

#if WD_ENABLED(WD_COMPILER_MSVC) && WD_ENABLED(WD_PLATFORM_ARCH_X86)

extern "C"
{
  // The main purpose of this little hack here is to have Mutex::Lock and Mutex::Unlock inline-able without including windows.h
  // The hack however does only work on the MSVC compiler. See fall back code below.

  // First define two functions which are binary compatible with EnterCriticalSection and LeaveCriticalSection
  __declspec(dllimport) void __stdcall wdWinEnterCriticalSection(wdMutexHandle* handle);
  __declspec(dllimport) void __stdcall wdWinLeaveCriticalSection(wdMutexHandle* handle);
  __declspec(dllimport) wdMinWindows::BOOL __stdcall wdWinTryEnterCriticalSection(wdMutexHandle* handle);

  // Now redirect them through linker flags to the correct implementation
#  if WD_ENABLED(WD_PLATFORM_32BIT)
#    pragma comment(linker, "/alternatename:__imp__wdWinEnterCriticalSection@4=__imp__EnterCriticalSection@4")
#    pragma comment(linker, "/alternatename:__imp__wdWinLeaveCriticalSection@4=__imp__LeaveCriticalSection@4")
#    pragma comment(linker, "/alternatename:__imp__wdWinTryEnterCriticalSection@4=__imp__TryEnterCriticalSection@4")
#  else
#    pragma comment(linker, "/alternatename:__imp_wdWinEnterCriticalSection=__imp_EnterCriticalSection")
#    pragma comment(linker, "/alternatename:__imp_wdWinLeaveCriticalSection=__imp_LeaveCriticalSection")
#    pragma comment(linker, "/alternatename:__imp_wdWinTryEnterCriticalSection=__imp_TryEnterCriticalSection")
#  endif
}

inline void wdMutex::Lock()
{
  wdWinEnterCriticalSection(&m_hHandle);
  ++m_iLockCount;
}

inline void wdMutex::Unlock()
{
  --m_iLockCount;
  wdWinLeaveCriticalSection(&m_hHandle);
}

inline wdResult wdMutex::TryLock()
{
  if (wdWinTryEnterCriticalSection(&m_hHandle) != 0)
  {
    ++m_iLockCount;
    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

#else

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

inline void wdMutex::Lock()
{
  EnterCriticalSection((CRITICAL_SECTION*)&m_hHandle);
  ++m_iLockCount;
}

inline void wdMutex::Unlock()
{
  --m_iLockCount;
  LeaveCriticalSection((CRITICAL_SECTION*)&m_hHandle);
}

inline wdResult wdMutex::TryLock()
{
  if (TryEnterCriticalSection((CRITICAL_SECTION*)&m_hHandle) != 0)
  {
    ++m_iLockCount;
    return WD_SUCCESS;
  }

  return WD_FAILURE;
}

#endif
