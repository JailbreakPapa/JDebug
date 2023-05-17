#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Configuration/Implementation/Win/Plugin_Win.h>
#elif WD_ENABLED(WD_PLATFORM_OSX) || WD_ENABLED(WD_PLATFORM_LINUX)
#  include <Foundation/Configuration/Implementation/Posix/Plugin_Posix.h>
#elif WD_ENABLED(WD_PLATFORM_ANDROID)
#  include <Foundation/Configuration/Implementation/Android/Plugin_Android.h>
#else
#  error "Plugins not implemented on this Platform."
#endif

wdResult UnloadPluginModule(wdPluginModule& ref_pModule, const char* szPluginFile);
wdResult LoadPluginModule(const char* szFileToLoad, wdPluginModule& ref_pModule, const char* szPluginFile);

struct ModuleData
{
  wdPluginModule m_hModule = 0;
  wdUInt8 m_uiFileNumber = 0;
  bool m_bCalledOnLoad = false;
  wdHybridArray<wdPluginInitCallback, 2> m_OnLoadCB;
  wdHybridArray<wdPluginInitCallback, 2> m_OnUnloadCB;
  wdHybridArray<wdString, 2> m_sPluginDependencies;
  wdBitflags<wdPluginLoadFlags> m_LoadFlags;

  void Initialize();
  void Uninitialize();
};

static ModuleData g_StaticModule;
static ModuleData* g_pCurrentlyLoadingModule = nullptr;
static wdMap<wdString, ModuleData> g_LoadedModules;
static wdDynamicArray<wdString> s_PluginLoadOrder;
static wdUInt32 s_uiMaxParallelInstances = 32;
static wdInt32 s_iPluginChangeRecursionCounter = 0;

wdCopyOnBroadcastEvent<const wdPluginEvent&> s_PluginEvents;

void wdPlugin::SetMaxParallelInstances(wdUInt32 uiMaxParallelInstances)
{
  s_uiMaxParallelInstances = wdMath::Max(1u, uiMaxParallelInstances);
}

void wdPlugin::InitializeStaticallyLinkedPlugins()
{
  g_StaticModule.Initialize();
}

void wdPlugin::GetAllPluginInfos(wdDynamicArray<PluginInfo>& ref_infos)
{
  ref_infos.Clear();

  ref_infos.Reserve(g_LoadedModules.GetCount());

  for (auto mod : g_LoadedModules)
  {
    auto& pi = ref_infos.ExpandAndGetRef();
    pi.m_sName = mod.Key();
    pi.m_sDependencies = mod.Value().m_sPluginDependencies;
    pi.m_LoadFlags = mod.Value().m_LoadFlags;
  }
}

void ModuleData::Initialize()
{
  if (m_bCalledOnLoad)
    return;

  m_bCalledOnLoad = true;

  for (const auto& dep : m_sPluginDependencies)
  {
    // TODO: ignore ??
    wdPlugin::LoadPlugin(dep).IgnoreResult();
  }

  for (auto cb : m_OnLoadCB)
  {
    cb();
  }
}

void ModuleData::Uninitialize()
{
  if (!m_bCalledOnLoad)
    return;

  for (wdUInt32 i = m_OnUnloadCB.GetCount(); i > 0; --i)
  {
    m_OnUnloadCB[i - 1]();
  }

  m_bCalledOnLoad = false;
}

void wdPlugin::BeginPluginChanges()
{
  if (s_iPluginChangeRecursionCounter == 0)
  {
    wdPluginEvent e;
    e.m_EventType = wdPluginEvent::BeforePluginChanges;
    s_PluginEvents.Broadcast(e);
  }

  ++s_iPluginChangeRecursionCounter;
}

void wdPlugin::EndPluginChanges()
{
  --s_iPluginChangeRecursionCounter;

  if (s_iPluginChangeRecursionCounter == 0)
  {
    wdPluginEvent e;
    e.m_EventType = wdPluginEvent::AfterPluginChanges;
    s_PluginEvents.Broadcast(e);
  }
}

static wdResult UnloadPluginInternal(const char* szPluginFile)
{
  auto thisMod = g_LoadedModules.Find(szPluginFile);

  if (!thisMod.IsValid())
    return WD_SUCCESS;

  wdLog::Debug("Plugin to unload: \"{0}\"", szPluginFile);

  wdPlugin::BeginPluginChanges();
  WD_SCOPE_EXIT(wdPlugin::EndPluginChanges());

  // Broadcast event: Before unloading plugin
  {
    wdPluginEvent e;
    e.m_EventType = wdPluginEvent::BeforeUnloading;
    e.m_szPluginBinary = szPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  // Broadcast event: Startup Shutdown
  {
    wdPluginEvent e;
    e.m_EventType = wdPluginEvent::StartupShutdown;
    e.m_szPluginBinary = szPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  // Broadcast event: After Startup Shutdown
  {
    wdPluginEvent e;
    e.m_EventType = wdPluginEvent::AfterStartupShutdown;
    e.m_szPluginBinary = szPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  thisMod.Value().Uninitialize();

  // unload the plugin module
  if (UnloadPluginModule(thisMod.Value().m_hModule, szPluginFile) == WD_FAILURE)
  {
    wdLog::Error("Unloading plugin module '{}' failed.", szPluginFile);
    return WD_FAILURE;
  }

  // delete the plugin copy that we had loaded
  {
    wdStringBuilder sOriginalFile, sCopiedFile;
    wdPlugin::GetPluginPaths(szPluginFile, sOriginalFile, sCopiedFile, g_LoadedModules[szPluginFile].m_uiFileNumber);

    wdOSFile::DeleteFile(sCopiedFile).IgnoreResult();
  }

  // Broadcast event: After unloading plugin
  {
    wdPluginEvent e;
    e.m_EventType = wdPluginEvent::AfterUnloading;
    e.m_szPluginBinary = szPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  wdLog::Success("Plugin '{0}' is unloaded.", szPluginFile);
  g_LoadedModules.Remove(thisMod);

  return WD_SUCCESS;
}

static wdResult LoadPluginInternal(const char* szPluginFile, wdBitflags<wdPluginLoadFlags> flags)
{
  wdUInt8 uiFileNumber = 0;

  wdStringBuilder sOriginalFile, sCopiedFile;
  wdPlugin::GetPluginPaths(szPluginFile, sOriginalFile, sCopiedFile, uiFileNumber);

  if (!wdOSFile::ExistsFile(sOriginalFile))
  {
    wdLog::Error("The plugin '{0}' does not exist.", szPluginFile);
    return WD_FAILURE;
  }

  if (flags.IsSet(wdPluginLoadFlags::LoadCopy))
  {
    // create a copy of the original plugin file
    const wdUInt8 uiMaxParallelInstances = static_cast<wdUInt8>(s_uiMaxParallelInstances);
    for (uiFileNumber = 0; uiFileNumber < uiMaxParallelInstances; ++uiFileNumber)
    {
      wdPlugin::GetPluginPaths(szPluginFile, sOriginalFile, sCopiedFile, uiFileNumber);
      if (wdOSFile::CopyFile(sOriginalFile, sCopiedFile) == WD_SUCCESS)
        goto success;
    }

    wdLog::Error("Could not copy the plugin file '{0}' to '{1}' (and all previous file numbers). Plugin MaxParallelInstances is set to {2}.", sOriginalFile, sCopiedFile, s_uiMaxParallelInstances);

    g_LoadedModules.Remove(sCopiedFile);
    return WD_FAILURE;
  }
  else
  {
    sCopiedFile = sOriginalFile;
  }

success:

  auto& thisMod = g_LoadedModules[szPluginFile];
  thisMod.m_uiFileNumber = uiFileNumber;
  thisMod.m_LoadFlags = flags;

  wdPlugin::BeginPluginChanges();
  WD_SCOPE_EXIT(wdPlugin::EndPluginChanges());

  // Broadcast Event: Before loading plugin
  {
    wdPluginEvent e;
    e.m_EventType = wdPluginEvent::BeforeLoading;
    e.m_szPluginBinary = szPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  g_pCurrentlyLoadingModule = &thisMod;

  if (LoadPluginModule(sCopiedFile, g_pCurrentlyLoadingModule->m_hModule, szPluginFile) == WD_FAILURE)
  {
    // loaded, but failed
    g_pCurrentlyLoadingModule = nullptr;
    thisMod.m_hModule = 0;

    return WD_FAILURE;
  }

  g_pCurrentlyLoadingModule = nullptr;

  {
    // Broadcast Event: After loading plugin, before init
    {
      wdPluginEvent e;
      e.m_EventType = wdPluginEvent::AfterLoadingBeforeInit;
      e.m_szPluginBinary = szPluginFile;
      s_PluginEvents.Broadcast(e);
    }

    thisMod.Initialize();

    // Broadcast Event: After loading plugin
    {
      wdPluginEvent e;
      e.m_EventType = wdPluginEvent::AfterLoading;
      e.m_szPluginBinary = szPluginFile;
      s_PluginEvents.Broadcast(e);
    }
  }

  wdLog::Success("Plugin '{0}' is loaded.", szPluginFile);
  return WD_SUCCESS;
}

bool wdPlugin::ExistsPluginFile(const char* szPluginFile)
{
  wdStringBuilder sOriginalFile, sCopiedFile;
  GetPluginPaths(szPluginFile, sOriginalFile, sCopiedFile, 0);

  return wdOSFile::ExistsFile(sOriginalFile);
}

wdResult wdPlugin::LoadPlugin(const char* szPluginFile, wdBitflags<wdPluginLoadFlags> flags /*= wdPluginLoadFlags::Default*/)
{
  if (flags.IsSet(wdPluginLoadFlags::PluginIsOptional))
  {
    // early out without logging an error

    if (!ExistsPluginFile(szPluginFile))
      return WD_FAILURE;
  }

  WD_LOG_BLOCK("Loading Plugin", szPluginFile);

  if (g_LoadedModules.Find(szPluginFile).IsValid())
  {
    wdLog::Debug("Plugin '{0}' already loaded.", szPluginFile);
    return WD_SUCCESS;
  }

  // make sure this is done first
  InitializeStaticallyLinkedPlugins();

  wdLog::Debug("Plugin to load: \"{0}\"", szPluginFile);

  // make sure to use a static string pointer from now on, that stays where it is
  szPluginFile = g_LoadedModules.FindOrAdd(szPluginFile).Key();

  wdResult res = LoadPluginInternal(szPluginFile, flags);

  if (res.Succeeded())
  {
    s_PluginLoadOrder.PushBack(szPluginFile);
  }
  else
  {
    // If we failed to load the plugin, it shouldn't be in the loaded modules list
    g_LoadedModules.Remove(szPluginFile);
  }

  return res;
}

void wdPlugin::UnloadAllPlugins()
{
  BeginPluginChanges();
  WD_SCOPE_EXIT(EndPluginChanges());

  for (wdUInt32 i = s_PluginLoadOrder.GetCount(); i > 0; --i)
  {
    if (UnloadPluginInternal(s_PluginLoadOrder[i - 1]).Failed())
    {
      // not sure what to do
    }
  }

  WD_ASSERT_DEBUG(g_LoadedModules.IsEmpty(), "Not all plugins were unloaded somehow.");

  for (auto mod : g_LoadedModules)
  {
    mod.Value().Uninitialize();
  }

  // also shut down all plugin objects that are statically linked
  g_StaticModule.Uninitialize();

  s_PluginLoadOrder.Clear();
  g_LoadedModules.Clear();
}

const wdCopyOnBroadcastEvent<const wdPluginEvent&>& wdPlugin::Events()
{
  return s_PluginEvents;
}

wdPlugin::Init::Init(wdPluginInitCallback onLoadOrUnloadCB, bool bOnLoad)
{
  ModuleData* pMD = g_pCurrentlyLoadingModule ? g_pCurrentlyLoadingModule : &g_StaticModule;

  if (bOnLoad)
    pMD->m_OnLoadCB.PushBack(onLoadOrUnloadCB);
  else
    pMD->m_OnUnloadCB.PushBack(onLoadOrUnloadCB);
}

wdPlugin::Init::Init(const char* szAddPluginDependency)
{
  ModuleData* pMD = g_pCurrentlyLoadingModule ? g_pCurrentlyLoadingModule : &g_StaticModule;

  pMD->m_sPluginDependencies.PushBack(szAddPluginDependency);
}

WD_STATICLINK_FILE(Foundation, Foundation_Configuration_Implementation_Plugin);
