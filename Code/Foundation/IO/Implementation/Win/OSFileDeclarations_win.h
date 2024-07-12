#pragma once

#include <Foundation/Basics.h>

// Deactivate Doxygen document generation for the following block.
/// \cond

// Avoid conflicts with windows.h
#ifdef DeleteFile
#  undef DeleteFile
#endif

#ifdef CopyFile
#  undef CopyFile
#endif

#if NS_DISABLED(NS_USE_POSIX_FILE_API)

#  include <Foundation/Basics/Platform/Win/MinWindows.h>

struct nsOSFileData
{
  nsOSFileData() { m_pFileHandle = NS_WINDOWS_INVALID_HANDLE_VALUE; }

  nsMinWindows::HANDLE m_pFileHandle;
};

struct nsFileIterationData
{
  nsHybridArray<nsMinWindows::HANDLE, 16> m_Handles;
};

#endif

/// \endcond
