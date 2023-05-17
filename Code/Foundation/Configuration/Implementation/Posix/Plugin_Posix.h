#include <Foundation/FoundationInternal.h>
WD_FOUNDATION_INTERNAL_HEADER

#include <dlfcn.h>

#include <Foundation/Configuration/Plugin.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>

typedef void* wdPluginModule;

void wdPlugin::GetPluginPaths(const char* szPluginName, wdStringBuilder& sOriginalFile, wdStringBuilder& sCopiedFile, wdUInt8 uiFileCopyNumber)
{
  sOriginalFile = wdOSFile::GetApplicationDirectory();
  sOriginalFile.AppendPath(szPluginName);
  sOriginalFile.Append(".so");

  sCopiedFile = wdOSFile::GetApplicationDirectory();
  sCopiedFile.AppendPath(szPluginName);

  if (uiFileCopyNumber > 0)
    sCopiedFile.AppendFormat("{0}", uiFileCopyNumber);

  sCopiedFile.Append(".loaded");
}

wdResult UnloadPluginModule(wdPluginModule& Module, const char* szPluginFile)
{
  if (dlclose(Module) != 0)
  {
    wdLog::Error("Could not unload plugin '{0}'. Error {1}", szPluginFile, static_cast<const char*>(dlerror()));
    return WD_FAILURE;
  }

  return WD_SUCCESS;
}

wdResult LoadPluginModule(const char* szFileToLoad, wdPluginModule& Module, const char* szPluginFile)
{
  Module = dlopen(szFileToLoad, RTLD_NOW | RTLD_GLOBAL);
  if (Module == nullptr)
  {
    wdLog::Error("Could not load plugin '{0}'. Error {1}.\nSet the environment variable LD_DEBUG=all to get more information.", szPluginFile, static_cast<const char*>(dlerror()));
    return WD_FAILURE;
  }
  return WD_SUCCESS;
}