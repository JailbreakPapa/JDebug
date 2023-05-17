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

#if WD_DISABLED(WD_USE_POSIX_FILE_API)

#  include <Foundation/Basics/Platform/Win/MinWindows.h>

struct wdOSFileData
{
  wdOSFileData() { m_pFileHandle = WD_WINDOWS_INVALID_HANDLE_VALUE; }

  wdMinWindows::HANDLE m_pFileHandle;
};

struct wdFileIterationData
{
  wdHybridArray<wdMinWindows::HANDLE, 16> m_Handles;
};

#endif

/// \endcond
