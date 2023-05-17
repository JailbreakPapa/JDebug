#include <Foundation/FoundationPCH.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
#  include <Foundation/Application/Implementation/Uwp/ApplicationEntryPoint_uwp.h>
#  include <roapi.h>

namespace wdApplicationDetails
{
  wdResult InitializeWinrt()
  {
    HRESULT result = RoInitialize(RO_INIT_MULTITHREADED);
    if (FAILED(result))
    {
      wdLog::Printf("Failed to init WinRT: %i", result);
      return WD_FAILURE;
    }

    return WD_SUCCESS;
  }

  void UninitializeWinrt() { RoUninitialize(); }
} // namespace wdApplicationDetails
#endif


WD_STATICLINK_FILE(Foundation, Foundation_Application_Implementation_uwp_ApplicationEntryPoint_uwp);
