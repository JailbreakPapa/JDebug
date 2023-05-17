#pragma once

#include <Foundation/Basics.h>

// Deactivate Doxygen document generation for the following block.
/// \cond

struct wdOSFileData
{
  wdOSFileData() { m_pFileHandle = nullptr; }

  FILE* m_pFileHandle;
};

#if WD_ENABLED(WD_SUPPORTS_FILE_ITERATORS)

struct wdFileIterationData
{
  // This is storing DIR*, which we can't forward declare
  wdHybridArray<void*, 16> m_Handles;
  wdString m_wildcardSearch;
};

#endif

/// \endcond
