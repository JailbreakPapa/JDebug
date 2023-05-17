#pragma once

#include <Foundation/Basics.h>

#if WD_DISABLED(WD_PLATFORM_WINDOWS)
#  error "WinRT util header should only be included in Windows builds!"
#endif

// For 10.0.10240, there are circular includes in some headers, fix by including first:
#include <corecrt.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
// All WRL
#  include <wrl.h>
#else
// Only include a subset that is C++ conformant
#  include <wrl/def.h>
#  include <wrl/ftm.h>
#  include <wrl/internal.h>
#  include <wrl/module.h>
#  include <wrl/wrappers/corewrappers.h>
#endif

#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.devices.enumeration.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

// warning C4530: C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
WD_MSVC_ANALYSIS_WARNING_DISABLE(4530);
