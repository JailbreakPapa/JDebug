#include <Foundation/FoundationPCH.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
#  include <Foundation/Application/Implementation/Uwp/ApplicationEntryPoint_uwp.h>
#  include <roapi.h>

namespace nsApplicationDetails
{
  nsResult InitializeWinrt()
  {
    HRESULT result = RoInitialize(RO_INIT_MULTITHREADED);
    if (FAILED(result))
    {
      nsLog::Printf("Failed to init WinRT: %i", result);
      return NS_FAILURE;
    }

    return NS_SUCCESS;
  }

  void UninitializeWinrt()
  {
    RoUninitialize();
  }
} // namespace nsApplicationDetails
#endif
