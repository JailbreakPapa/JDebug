#pragma once

// Deactivate Doxygen document generation for the following block.
/// \cond

#include <Foundation/Basics/Platform/Win/MinWindows.h>

#if WD_ENABLED(WD_PLATFORM_32BIT)
struct alignas(4) wdMutexHandle
{
  wdUInt8 data[24];
};
#else
struct alignas(8) wdMutexHandle
{
  wdUInt8 data[40];
};
#endif


#if WD_ENABLED(WD_PLATFORM_32BIT)
struct alignas(4) wdConditionVariableHandle
{
  wdUInt8 data[4];
};
#else
struct alignas(8) wdConditionVariableHandle
{
  wdUInt8 data[8];
};
#endif



using wdThreadHandle = wdMinWindows::HANDLE;
using wdThreadID = wdMinWindows::DWORD;
using wdOSThreadEntryPoint = wdMinWindows::DWORD(__stdcall*)(void* lpThreadParameter);
using wdSemaphoreHandle = wdMinWindows::HANDLE;

#define WD_THREAD_CLASS_ENTRY_POINT wdMinWindows::DWORD __stdcall wdThreadClassEntryPoint(void* lpThreadParameter);

struct wdConditionVariableData
{
  wdConditionVariableHandle m_ConditionVariable;
};

/// \endcond
