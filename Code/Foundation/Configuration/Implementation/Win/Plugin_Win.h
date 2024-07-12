
#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#if NS_ENABLED(NS_PLATFORM_WINDOWS)

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Configuration/Plugin.h>
#  include <Foundation/IO/OSFile.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Strings/StringBuilder.h>

using nsPluginModule = HMODULE;

bool nsPlugin::PlatformNeedsPluginCopy()
{
  return true;
}

void nsPlugin::GetPluginPaths(nsStringView sPluginName, nsStringBuilder& ref_sOriginalFile, nsStringBuilder& ref_sCopiedFile, nsUInt8 uiFileCopyNumber)
{
  ref_sOriginalFile = nsOSFile::GetApplicationDirectory();
  ref_sOriginalFile.AppendPath(sPluginName);
  ref_sOriginalFile.Append(".dll");

  ref_sCopiedFile = nsOSFile::GetApplicationDirectory();
  ref_sCopiedFile.AppendPath(sPluginName);

  if (!nsOSFile::ExistsFile(ref_sOriginalFile))
  {
    ref_sOriginalFile = nsOSFile::GetCurrentWorkingDirectory();
    ref_sOriginalFile.AppendPath(sPluginName);
    ref_sOriginalFile.Append(".dll");

    ref_sCopiedFile = nsOSFile::GetCurrentWorkingDirectory();
    ref_sCopiedFile.AppendPath(sPluginName);
  }

  if (uiFileCopyNumber > 0)
    ref_sCopiedFile.AppendFormat("{0}", uiFileCopyNumber);

  ref_sCopiedFile.Append(".loaded");
}

nsResult UnloadPluginModule(nsPluginModule& ref_pModule, nsStringView sPluginFile)
{
  // reset last error code
  SetLastError(ERROR_SUCCESS);

  if (FreeLibrary(ref_pModule) == FALSE)
  {
    nsLog::Error("Could not unload plugin '{0}'. Error-Code {1}", sPluginFile, nsArgErrorCode(GetLastError()));
    return NS_FAILURE;
  }

  ref_pModule = nullptr;
  return NS_SUCCESS;
}

nsResult LoadPluginModule(nsStringView sFileToLoad, nsPluginModule& ref_pModule, nsStringView sPluginFile)
{
  // reset last error code
  SetLastError(ERROR_SUCCESS);

#  if NS_ENABLED(NS_PLATFORM_WINDOWS_UWP)
  nsStringBuilder relativePath = sFileToLoad;
  NS_SUCCEED_OR_RETURN(relativePath.MakeRelativeTo(nsOSFile::GetApplicationDirectory()));
  ref_pModule = LoadPackagedLibrary(nsStringWChar(relativePath).GetData(), 0);
#  else
  ref_pModule = LoadLibraryW(nsStringWChar(sFileToLoad).GetData());
#  endif

  if (ref_pModule == nullptr)
  {
    const DWORD err = GetLastError();
    nsLog::Error("Could not load plugin '{0}'. Error-Code {1}", sPluginFile, nsArgErrorCode(err));

    if (err == 126)
    {
      nsLog::Error("Please Note: This means that the plugin exists, but a DLL dependency of the plugin is missing. You probably need to copy 3rd "
                   "party DLLs next to the plugin.");
    }

    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

#else
#  error "This file should not have been included."
#endif
