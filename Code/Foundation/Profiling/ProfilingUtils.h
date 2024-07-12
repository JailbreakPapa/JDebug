#pragma once

#include <Foundation/Basics.h>

class NS_FOUNDATION_DLL nsProfilingUtils
{
public:
  /// \brief Captures profiling data via nsProfilingSystem::Capture and saves it to the giben file location.
  static nsResult SaveProfilingCapture(nsStringView sCapturePath);
  /// \brief Reads two profiling captures and merges them into one.
  static nsResult MergeProfilingCaptures(nsStringView sCapturePath1, nsStringView sCapturePath2, nsStringView sMergedCapturePath);
};
