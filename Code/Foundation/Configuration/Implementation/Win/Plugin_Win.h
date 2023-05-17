
#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#if WD_ENABLED(WD_PLATFORM_WINDOWS)

#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>
#  include <Foundation/Configuration/Plugin.h>
#  include <Foundation/IO/OSFile.h>
#  include <Foundation/Logging/Log.h>
#  include <Foundation/Strings/StringBuilder.h>

typedef HMODULE wdPluginModule;

void wdPlugin::GetPluginPaths(const char* szPluginName, wdStringBuilder& ref_sOriginalFile, wdStringBuilder& ref_sCopiedFile, wdUInt8 uiFileCopyNumber)
{
  auto sPluginName = wdStringView(szPluginName);

  ref_sOriginalFile = wdOSFile::GetApplicationDirectory();
  ref_sOriginalFile.AppendPath(sPluginName);
  ref_sOriginalFile.Append(".dll");

  ref_sCopiedFile = wdOSFile::GetApplicationDirectory();
  ref_sCopiedFile.AppendPath(sPluginName);

  if (!wdOSFile::ExistsFile(ref_sOriginalFile))
  {
    ref_sOriginalFile = wdOSFile::GetCurrentWorkingDirectory();
    ref_sOriginalFile.AppendPath(sPluginName);
    ref_sOriginalFile.Append(".dll");

    ref_sCopiedFile = wdOSFile::GetCurrentWorkingDirectory();
    ref_sCopiedFile.AppendPath(sPluginName);
  }

  if (uiFileCopyNumber > 0)
    ref_sCopiedFile.AppendFormat("{0}", uiFileCopyNumber);

  ref_sCopiedFile.Append(".loaded");
}

wdResult UnloadPluginModule(wdPluginModule& ref_pModule, const char* szPluginFile)
{
  // reset last error code
  SetLastError(ERROR_SUCCESS);

  if (FreeLibrary(ref_pModule) == FALSE)
  {
    wdLog::Error("Could not unload plugin '{0}'. Error-Code {1}", szPluginFile, wdArgErrorCode(GetLastError()));
    return WD_FAILURE;
  }

  ref_pModule = nullptr;
  return WD_SUCCESS;
}

wdResult LoadPluginModule(const char* szFileToLoad, wdPluginModule& ref_pModule, const char* szPluginFile)
{
  // reset last error code
  SetLastError(ERROR_SUCCESS);

#  if WD_ENABLED(WD_PLATFORM_WINDOWS_UWP)
  wdStringBuilder relativePath = szFileToLoad;
  WD_SUCCEED_OR_RETURN(relativePath.MakeRelativeTo(wdOSFile::GetApplicationDirectory()));
  ref_pModule = LoadPackagedLibrary(wdStringWChar(relativePath).GetData(), 0);
#  else
  ref_pModule = LoadLibraryW(wdStringWChar(szFileToLoad).GetData());
#  endif

  if (ref_pModule == nullptr)
  {
    const DWORD err = GetLastError();
    wdLog::Error("Could not load plugin '{0}'. Error-Code {1}", szPluginFile, wdArgErrorCode(err));

    if (err == 126)
    {
      wdLog::Error("Please Note: This means that the plugin exists, but a DLL dependency of the plugin is missing. You probably need to copy 3rd "
                   "party DLLs next to the plugin.");
    }

    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

#else
#  error "This file should not have been included."
#endif
