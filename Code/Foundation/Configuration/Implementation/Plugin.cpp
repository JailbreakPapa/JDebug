#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Plugin.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/IO/OSFile.h>
#include <Foundation/Logging/Log.h>

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Configuration/Implementation/Win/Plugin_Win.h>
#elif NS_ENABLED(NS_PLATFORM_OSX) || NS_ENABLED(NS_PLATFORM_LINUX)
#  include <Foundation/Configuration/Implementation/Posix/Plugin_Posix.h>
#elif NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <Foundation/Configuration/Implementation/Android/Plugin_Android.h>
#else
#  error "Plugins not implemented on this Platform."
#endif

nsResult UnloadPluginModule(nsPluginModule& ref_pModule, nsStringView sPluginFile);
nsResult LoadPluginModule(nsStringView sFileToLoad, nsPluginModule& ref_pModule, nsStringView sPluginFile);

nsDynamicArray<nsString>& GetStaticPlugins()
{
  static nsDynamicArray<nsString> s_StaticPlugins;
  return s_StaticPlugins;
}

struct ModuleData
{
  nsPluginModule m_hModule = 0;
  nsUInt8 m_uiFileNumber = 0;
  bool m_bCalledOnLoad = false;
  nsHybridArray<nsPluginInitCallback, 2> m_OnLoadCB;
  nsHybridArray<nsPluginInitCallback, 2> m_OnUnloadCB;
  nsHybridArray<nsString, 2> m_sPluginDependencies;
  nsBitflags<nsPluginLoadFlags> m_LoadFlags;

  void Initialize();
  void Uninitialize();
};

static ModuleData g_StaticModule;
static ModuleData* g_pCurrentlyLoadingModule = nullptr;
static nsMap<nsString, ModuleData> g_LoadedModules;
static nsDynamicArray<nsString> s_PluginLoadOrder;
static nsUInt32 s_uiMaxParallelInstances = 32;
static nsInt32 s_iPluginChangeRecursionCounter = 0;

nsCopyOnBroadcastEvent<const nsPluginEvent&> s_PluginEvents;

void nsPlugin::SetMaxParallelInstances(nsUInt32 uiMaxParallelInstances)
{
  s_uiMaxParallelInstances = nsMath::Max(1u, uiMaxParallelInstances);
}

void nsPlugin::InitializeStaticallyLinkedPlugins()
{
  if (!g_StaticModule.m_bCalledOnLoad)
  {
    // We need to trigger the nsPlugin events to make sure the sub-systems are initialized at least once.
    nsPlugin::BeginPluginChanges();
    NS_SCOPE_EXIT(nsPlugin::EndPluginChanges());
    g_StaticModule.Initialize();

#if NS_DISABLED(NS_COMPILE_ENGINE_AS_DLL)
    NS_LOG_BLOCK("Initialize Statically Linked Plugins");
    // Merely add dummy entries so plugins can be enumerated etc.
    for (nsStringView sPlugin : GetStaticPlugins())
    {
      g_LoadedModules.FindOrAdd(sPlugin);
      nsLog::Debug("Plugin '{0}' statically linked.", sPlugin);
    }
#endif
  }
}

void nsPlugin::GetAllPluginInfos(nsDynamicArray<PluginInfo>& ref_infos)
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
    nsPlugin::LoadPlugin(dep).IgnoreResult();
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

  for (nsUInt32 i = m_OnUnloadCB.GetCount(); i > 0; --i)
  {
    m_OnUnloadCB[i - 1]();
  }

  m_bCalledOnLoad = false;
}

void nsPlugin::BeginPluginChanges()
{
  if (s_iPluginChangeRecursionCounter == 0)
  {
    nsPluginEvent e;
    e.m_EventType = nsPluginEvent::BeforePluginChanges;
    s_PluginEvents.Broadcast(e);
  }

  ++s_iPluginChangeRecursionCounter;
}

void nsPlugin::EndPluginChanges()
{
  --s_iPluginChangeRecursionCounter;

  if (s_iPluginChangeRecursionCounter == 0)
  {
    nsPluginEvent e;
    e.m_EventType = nsPluginEvent::AfterPluginChanges;
    s_PluginEvents.Broadcast(e);
  }
}

static nsResult UnloadPluginInternal(nsStringView sPluginFile)
{
  auto thisMod = g_LoadedModules.Find(sPluginFile);

  if (!thisMod.IsValid())
    return NS_SUCCESS;

  nsLog::Debug("Plugin to unload: \"{0}\"", sPluginFile);

  nsPlugin::BeginPluginChanges();
  NS_SCOPE_EXIT(nsPlugin::EndPluginChanges());

  // Broadcast event: Before unloading plugin
  {
    nsPluginEvent e;
    e.m_EventType = nsPluginEvent::BeforeUnloading;
    e.m_sPluginBinary = sPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  // Broadcast event: Startup Shutdown
  {
    nsPluginEvent e;
    e.m_EventType = nsPluginEvent::StartupShutdown;
    e.m_sPluginBinary = sPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  // Broadcast event: After Startup Shutdown
  {
    nsPluginEvent e;
    e.m_EventType = nsPluginEvent::AfterStartupShutdown;
    e.m_sPluginBinary = sPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  thisMod.Value().Uninitialize();

  // unload the plugin module
  if (UnloadPluginModule(thisMod.Value().m_hModule, sPluginFile) == NS_FAILURE)
  {
    nsLog::Error("Unloading plugin module '{}' failed.", sPluginFile);
    return NS_FAILURE;
  }

  // delete the plugin copy that we had loaded
  if (nsPlugin::PlatformNeedsPluginCopy())
  {
    nsStringBuilder sOriginalFile, sCopiedFile;
    nsPlugin::GetPluginPaths(sPluginFile, sOriginalFile, sCopiedFile, g_LoadedModules[sPluginFile].m_uiFileNumber);

    nsOSFile::DeleteFile(sCopiedFile).IgnoreResult();
  }

  // Broadcast event: After unloading plugin
  {
    nsPluginEvent e;
    e.m_EventType = nsPluginEvent::AfterUnloading;
    e.m_sPluginBinary = sPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  nsLog::Success("Plugin '{0}' is unloaded.", sPluginFile);
  g_LoadedModules.Remove(thisMod);

  return NS_SUCCESS;
}

static nsResult LoadPluginInternal(nsStringView sPluginFile, nsBitflags<nsPluginLoadFlags> flags)
{
  nsUInt8 uiFileNumber = 0;

  nsStringBuilder sOriginalFile, sCopiedFile;
  nsPlugin::GetPluginPaths(sPluginFile, sOriginalFile, sCopiedFile, uiFileNumber);

  if (!nsOSFile::ExistsFile(sOriginalFile))
  {
    nsLog::Error("The plugin '{0}' does not exist.", sPluginFile);
    return NS_FAILURE;
  }

  if (nsPlugin::PlatformNeedsPluginCopy() && flags.IsSet(nsPluginLoadFlags::LoadCopy))
  {
    // create a copy of the original plugin file
    const nsUInt8 uiMaxParallelInstances = static_cast<nsUInt8>(s_uiMaxParallelInstances);
    for (uiFileNumber = 0; uiFileNumber < uiMaxParallelInstances; ++uiFileNumber)
    {
      nsPlugin::GetPluginPaths(sPluginFile, sOriginalFile, sCopiedFile, uiFileNumber);
      if (nsOSFile::CopyFile(sOriginalFile, sCopiedFile) == NS_SUCCESS)
        goto success;
    }

    nsLog::Error("Could not copy the plugin file '{0}' to '{1}' (and all previous file numbers). Plugin MaxParallelInstances is set to {2}.", sOriginalFile, sCopiedFile, s_uiMaxParallelInstances);

    g_LoadedModules.Remove(sCopiedFile);
    return NS_FAILURE;
  }
  else
  {
    sCopiedFile = sOriginalFile;
  }

success:

  auto& thisMod = g_LoadedModules[sPluginFile];
  thisMod.m_uiFileNumber = uiFileNumber;
  thisMod.m_LoadFlags = flags;

  nsPlugin::BeginPluginChanges();
  NS_SCOPE_EXIT(nsPlugin::EndPluginChanges());

  // Broadcast Event: Before loading plugin
  {
    nsPluginEvent e;
    e.m_EventType = nsPluginEvent::BeforeLoading;
    e.m_sPluginBinary = sPluginFile;
    s_PluginEvents.Broadcast(e);
  }

  g_pCurrentlyLoadingModule = &thisMod;

  if (LoadPluginModule(sCopiedFile, g_pCurrentlyLoadingModule->m_hModule, sPluginFile) == NS_FAILURE)
  {
    // loaded, but failed
    g_pCurrentlyLoadingModule = nullptr;
    thisMod.m_hModule = 0;

    return NS_FAILURE;
  }

  g_pCurrentlyLoadingModule = nullptr;

  {
    // Broadcast Event: After loading plugin, before init
    {
      nsPluginEvent e;
      e.m_EventType = nsPluginEvent::AfterLoadingBeforeInit;
      e.m_sPluginBinary = sPluginFile;
      s_PluginEvents.Broadcast(e);
    }

    thisMod.Initialize();

    // Broadcast Event: After loading plugin
    {
      nsPluginEvent e;
      e.m_EventType = nsPluginEvent::AfterLoading;
      e.m_sPluginBinary = sPluginFile;
      s_PluginEvents.Broadcast(e);
    }
  }

  nsLog::Success("Plugin '{0}' is loaded.", sPluginFile);
  return NS_SUCCESS;
}

bool nsPlugin::ExistsPluginFile(nsStringView sPluginFile)
{
  nsStringBuilder sOriginalFile, sCopiedFile;
  GetPluginPaths(sPluginFile, sOriginalFile, sCopiedFile, 0);

  return nsOSFile::ExistsFile(sOriginalFile);
}

nsResult nsPlugin::LoadPlugin(nsStringView sPluginFile, nsBitflags<nsPluginLoadFlags> flags /*= nsPluginLoadFlags::Default*/)
{
  NS_LOG_BLOCK("Loading Plugin", sPluginFile);

  // make sure this is done first
  InitializeStaticallyLinkedPlugins();

  if (g_LoadedModules.Find(sPluginFile).IsValid())
  {
    nsLog::Debug("Plugin '{0}' already loaded.", sPluginFile);
    return NS_SUCCESS;
  }

#if NS_DISABLED(NS_COMPILE_ENGINE_AS_DLL)
  // #TODO NS_COMPILE_ENGINE_AS_DLL and being able to load plugins are not necessarily the same thing.
  return NS_FAILURE;
#endif

  if (flags.IsSet(nsPluginLoadFlags::PluginIsOptional))
  {
    // early out without logging an error
    if (!ExistsPluginFile(sPluginFile))
      return NS_FAILURE;
  }

  nsLog::Debug("Plugin to load: \"{0}\"", sPluginFile);

  // make sure to use a static string pointer from now on, that stays where it is
  sPluginFile = g_LoadedModules.FindOrAdd(sPluginFile).Key();

  nsResult res = LoadPluginInternal(sPluginFile, flags);

  if (res.Succeeded())
  {
    s_PluginLoadOrder.PushBack(sPluginFile);
  }
  else
  {
    // If we failed to load the plugin, it shouldn't be in the loaded modules list
    g_LoadedModules.Remove(sPluginFile);
  }

  return res;
}

void nsPlugin::UnloadAllPlugins()
{
  BeginPluginChanges();
  NS_SCOPE_EXIT(EndPluginChanges());

  for (nsUInt32 i = s_PluginLoadOrder.GetCount(); i > 0; --i)
  {
    if (UnloadPluginInternal(s_PluginLoadOrder[i - 1]).Failed())
    {
      // not sure what to do
    }
  }

  NS_ASSERT_DEBUG(g_LoadedModules.IsEmpty(), "Not all plugins were unloaded somehow.");

  for (auto mod : g_LoadedModules)
  {
    mod.Value().Uninitialize();
  }

  // also shut down all plugin objects that are statically linked
  g_StaticModule.Uninitialize();

  s_PluginLoadOrder.Clear();
  g_LoadedModules.Clear();
}

const nsCopyOnBroadcastEvent<const nsPluginEvent&>& nsPlugin::Events()
{
  return s_PluginEvents;
}

nsPlugin::Init::Init(nsPluginInitCallback onLoadOrUnloadCB, bool bOnLoad)
{
  ModuleData* pMD = g_pCurrentlyLoadingModule ? g_pCurrentlyLoadingModule : &g_StaticModule;

  if (bOnLoad)
    pMD->m_OnLoadCB.PushBack(onLoadOrUnloadCB);
  else
    pMD->m_OnUnloadCB.PushBack(onLoadOrUnloadCB);
}

nsPlugin::Init::Init(const char* szAddPluginDependency)
{
  ModuleData* pMD = g_pCurrentlyLoadingModule ? g_pCurrentlyLoadingModule : &g_StaticModule;

  pMD->m_sPluginDependencies.PushBack(szAddPluginDependency);
}

#if NS_DISABLED(NS_COMPILE_ENGINE_AS_DLL)
nsPluginRegister::nsPluginRegister(const char* szAddPlugin)
{
  if (g_pCurrentlyLoadingModule == nullptr)
  {
    GetStaticPlugins().PushBack(szAddPlugin);
  }
}
#endif
