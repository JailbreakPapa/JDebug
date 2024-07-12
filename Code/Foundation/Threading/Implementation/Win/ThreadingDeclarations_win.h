#pragma once

// Deactivate Doxygen document generation for the following block.
/// \cond

#include <Foundation/Basics/Platform/Win/MinWindows.h>

#if NS_ENABLED(NS_PLATFORM_32BIT)
struct alignas(4) nsMutexHandle
{
  nsUInt8 data[24];
};
#else
struct alignas(8) nsMutexHandle
{
  nsUInt8 data[40];
};
#endif


#if NS_ENABLED(NS_PLATFORM_32BIT)
struct alignas(4) nsConditionVariableHandle
{
  nsUInt8 data[4];
};
#else
struct alignas(8) nsConditionVariableHandle
{
  nsUInt8 data[8];
};
#endif



using nsThreadHandle = nsMinWindows::HANDLE;
using nsThreadID = nsMinWindows::DWORD;
using nsOSThreadEntryPoint = nsMinWindows::DWORD(__stdcall*)(void* lpThreadParameter);
using nsSemaphoreHandle = nsMinWindows::HANDLE;

#define NS_THREAD_CLASS_ENTRY_POINT nsMinWindows::DWORD __stdcall nsThreadClassEntryPoint(void* lpThreadParameter);

struct nsConditionVariableData
{
  nsConditionVariableHandle m_ConditionVariable;
};

/// \endcond
