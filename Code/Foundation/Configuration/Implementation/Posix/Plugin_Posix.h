#include <Foundation/FoundationInternal.h>
NS_FOUNDATION_INTERNAL_HEADER

#include <dlfcn.h>

#include <Foundation/Configuration/Plugin.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/System/Process.h>

using nsPluginModule = void*;

bool nsPlugin::PlatformNeedsPluginCopy()
{
  return false;
}

void nsPlugin::GetPluginPaths(nsStringView sPluginName, nsStringBuilder& sOriginalFile, nsStringBuilder& sCopiedFile, nsUInt8 uiFileCopyNumber)
{
  sOriginalFile = nsOSFile::GetApplicationDirectory();
  sOriginalFile.AppendPath(sPluginName);
  sOriginalFile.Append(".so");

  sCopiedFile = nsOSFile::GetApplicationDirectory();
  sCopiedFile.AppendPath(sPluginName);

  if (uiFileCopyNumber > 0)
    sCopiedFile.AppendFormat("{0}", uiFileCopyNumber);

  sCopiedFile.Append(".loaded");
}

nsResult UnloadPluginModule(nsPluginModule& Module, nsStringView sPluginFile)
{
  if (dlclose(Module) != 0)
  {
    nsStringBuilder tmp;
    nsLog::Error("Could not unload plugin '{0}'. Error {1}", sPluginFile.GetData(tmp), static_cast<const char*>(dlerror()));
    return NS_FAILURE;
  }

  return NS_SUCCESS;
}

nsResult LoadPluginModule(nsStringView sFileToLoad, nsPluginModule& Module, nsStringView sPluginFile)
{
  nsStringBuilder tmp;
  Module = dlopen(sFileToLoad.GetData(tmp), RTLD_NOW | RTLD_GLOBAL);
  if (Module == nullptr)
  {
    nsLog::Error("Could not load plugin '{0}'. Error {1}.\nSet the environment variable LD_DEBUG=all to get more information.", sPluginFile.GetData(tmp), static_cast<const char*>(dlerror()));
    return NS_FAILURE;
  }
  return NS_SUCCESS;
}
