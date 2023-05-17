#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Threading/ThreadUtils.h>

WD_ENUMERABLE_CLASS_IMPLEMENTATION(wdSubSystem);

bool wdStartup::s_bPrintAllSubSystems = true;
wdStartupStage::Enum wdStartup::s_CurrentState = wdStartupStage::None;
wdDynamicArray<const char*> wdStartup::s_ApplicationTags;


void wdStartup::AddApplicationTag(const char* szTag)
{
  s_ApplicationTags.PushBack(szTag);
}

bool wdStartup::HasApplicationTag(const char* szTag)
{
  for (wdUInt32 i = 0; i < s_ApplicationTags.GetCount(); ++i)
  {
    if (wdStringUtils::IsEqual_NoCase(s_ApplicationTags[i], szTag))
      return true;
  }

  return false;
}

void wdStartup::PrintAllSubsystems()
{
  WD_LOG_BLOCK("Available Subsystems");

  wdSubSystem* pSub = wdSubSystem::GetFirstInstance();

  while (pSub)
  {
    wdLog::Debug("Subsystem: '{0}::{1}'", pSub->GetGroupName(), pSub->GetSubSystemName());

    if (pSub->GetDependency(0) == nullptr)
      wdLog::Debug("  <no dependencies>");
    else
    {
      for (wdInt32 i = 0; pSub->GetDependency(i) != nullptr; ++i)
        wdLog::Debug("  depends on '{0}'", pSub->GetDependency(i));
    }

    wdLog::Debug("");

    pSub = pSub->GetNextInstance();
  }
}

void wdStartup::AssignSubSystemPlugin(const char* szPluginName)
{
  // iterates over all existing subsystems and finds those that have no plugin name yet
  // assigns the given name to them

  wdSubSystem* pSub = wdSubSystem::GetFirstInstance();

  while (pSub)
  {
    if (pSub->m_szPluginName == nullptr)
      pSub->m_szPluginName = szPluginName;

    pSub = pSub->GetNextInstance();
  }
}

void wdStartup::PluginEventHandler(const wdPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case wdPluginEvent::BeforeLoading:
    {
      AssignSubSystemPlugin("Static");
    }
    break;

    case wdPluginEvent::AfterLoadingBeforeInit:
    {
      AssignSubSystemPlugin(EventData.m_szPluginBinary);
    }
    break;

    case wdPluginEvent::StartupShutdown:
    {
      wdStartup::UnloadPluginSubSystems(EventData.m_szPluginBinary);
    }
    break;

    case wdPluginEvent::AfterPluginChanges:
    {
      wdStartup::ReinitToCurrentState();
    }
    break;

    default:
      break;
  }
}

static bool IsGroupName(const char* szName)
{
  wdSubSystem* pSub = wdSubSystem::GetFirstInstance();

  bool bGroup = false;
  bool bSubSystem = false;

  while (pSub)
  {
    if (wdStringUtils::IsEqual(pSub->GetGroupName(), szName))
      bGroup = true;

    if (wdStringUtils::IsEqual(pSub->GetSubSystemName(), szName))
      bSubSystem = true;

    pSub = pSub->GetNextInstance();
  }

  WD_ASSERT_ALWAYS(!bGroup || !bSubSystem, "There cannot be a SubSystem AND a Group called '{0}'.", szName);

  return bGroup;
}

static const char* GetGroupSubSystems(const char* szGroup, wdInt32 iSubSystem)
{
  wdSubSystem* pSub = wdSubSystem::GetFirstInstance();

  while (pSub)
  {
    if (wdStringUtils::IsEqual(pSub->GetGroupName(), szGroup))
    {
      if (iSubSystem == 0)
        return pSub->GetSubSystemName();

      --iSubSystem;
    }

    pSub = pSub->GetNextInstance();
  }

  return nullptr;
}

void wdStartup::ComputeOrder(wdDeque<wdSubSystem*>& Order)
{
  Order.Clear();
  wdSet<wdString> sSystemsInited;

  bool bCouldInitAny = true;

  while (bCouldInitAny)
  {
    bCouldInitAny = false;

    wdSubSystem* pSub = wdSubSystem::GetFirstInstance();

    while (pSub)
    {
      if (!sSystemsInited.Find(pSub->GetSubSystemName()).IsValid())
      {
        bool bAllDependsFulfilled = true;
        wdInt32 iDep = 0;

        while (pSub->GetDependency(iDep) != nullptr)
        {
          if (IsGroupName(pSub->GetDependency(iDep)))
          {
            wdInt32 iSubSystemIndex = 0;
            const char* szNextSubSystem = GetGroupSubSystems(pSub->GetDependency(iDep), iSubSystemIndex);
            while (szNextSubSystem)
            {
              if (!sSystemsInited.Find(szNextSubSystem).IsValid())
              {
                bAllDependsFulfilled = false;
                break;
              }

              ++iSubSystemIndex;
              szNextSubSystem = GetGroupSubSystems(pSub->GetDependency(iDep), iSubSystemIndex);
            }
          }
          else
          {
            if (!sSystemsInited.Find(pSub->GetDependency(iDep)).IsValid())
            {
              bAllDependsFulfilled = false;
              break;
            }
          }

          ++iDep;
        }

        if (bAllDependsFulfilled)
        {
          bCouldInitAny = true;
          Order.PushBack(pSub);
          sSystemsInited.Insert(pSub->GetSubSystemName());
        }
      }

      pSub = pSub->GetNextInstance();
    }
  }
}

void wdStartup::Startup(wdStartupStage::Enum stage)
{
  if (stage == wdStartupStage::BaseSystems)
  {
    wdFoundation::Initialize();
  }

  const char* szStartup[] = {"Startup Base", "Startup Core", "Startup Engine"};

  if (stage == wdStartupStage::CoreSystems)
  {
    Startup(wdStartupStage::BaseSystems);

    wdGlobalEvent::Broadcast(WD_GLOBALEVENT_STARTUP_CORESYSTEMS_BEGIN);

    if (s_bPrintAllSubSystems)
    {
      s_bPrintAllSubSystems = false;
      PrintAllSubsystems();
    }
  }

  if (stage == wdStartupStage::HighLevelSystems)
  {
    Startup(wdStartupStage::CoreSystems);

    wdGlobalEvent::Broadcast(WD_GLOBALEVENT_STARTUP_HIGHLEVELSYSTEMS_BEGIN);
  }

  WD_LOG_BLOCK(szStartup[stage]);

  wdDeque<wdSubSystem*> Order;
  ComputeOrder(Order);

  for (wdUInt32 i = 0; i < Order.GetCount(); ++i)
  {
    if (!Order[i]->m_bStartupDone[stage])
    {
      Order[i]->m_bStartupDone[stage] = true;

      switch (stage)
      {
        case wdStartupStage::BaseSystems:
          wdLog::Debug("Executing 'Base' startup for sub-system '{1}::{0}'", Order[i]->GetSubSystemName(), Order[i]->GetGroupName());
          Order[i]->OnBaseSystemsStartup();
          break;
        case wdStartupStage::CoreSystems:
          wdLog::Debug("Executing 'Core' startup for sub-system '{1}::{0}'", Order[i]->GetSubSystemName(), Order[i]->GetGroupName());
          Order[i]->OnCoreSystemsStartup();
          break;
        case wdStartupStage::HighLevelSystems:
          wdLog::Debug("Executing 'Engine' startup for sub-system '{1}::{0}'", Order[i]->GetSubSystemName(), Order[i]->GetGroupName());
          Order[i]->OnHighLevelSystemsStartup();
          break;

        default:
          break;
      }
    }
  }

  // now everything should be started
  {
    WD_LOG_BLOCK("Failed SubSystems");

    wdSet<wdString> sSystemsFound;

    wdSubSystem* pSub = wdSubSystem::GetFirstInstance();

    while (pSub)
    {
      sSystemsFound.Insert(pSub->GetSubSystemName());
      pSub = pSub->GetNextInstance();
    }

    pSub = wdSubSystem::GetFirstInstance();

    while (pSub)
    {
      if (!pSub->m_bStartupDone[stage])
      {
        wdInt32 iDep = 0;

        while (pSub->GetDependency(iDep) != nullptr)
        {
          if (!sSystemsFound.Find(pSub->GetDependency(iDep)).IsValid())
          {
            wdLog::Error("SubSystem '{0}::{1}' could not be started because dependency '{2}' is unknown.", pSub->GetGroupName(),
              pSub->GetSubSystemName(), pSub->GetDependency(iDep));
          }
          else
          {
            wdLog::Error("SubSystem '{0}::{1}' could not be started because dependency '{2}' has not been initialized.", pSub->GetGroupName(),
              pSub->GetSubSystemName(), pSub->GetDependency(iDep));
          }

          ++iDep;
        }
      }

      pSub = pSub->GetNextInstance();
    }
  }

  switch (stage)
  {
    case wdStartupStage::BaseSystems:
      break;
    case wdStartupStage::CoreSystems:
      wdGlobalEvent::Broadcast(WD_GLOBALEVENT_STARTUP_CORESYSTEMS_END);
      break;
    case wdStartupStage::HighLevelSystems:
      wdGlobalEvent::Broadcast(WD_GLOBALEVENT_STARTUP_HIGHLEVELSYSTEMS_END);
      break;

    default:
      break;
  }

  if (s_CurrentState == wdStartupStage::None)
  {
    wdPlugin::Events().AddEventHandler(PluginEventHandler);
  }

  s_CurrentState = stage;
}

void wdStartup::Shutdown(wdStartupStage::Enum stage)
{
  // without that we cannot function, so make sure it is up and running
  wdFoundation::Initialize();

  {
    const char* szStartup[] = {"Shutdown Base", "Shutdown Core", "Shutdown Engine"};

    if (stage == wdStartupStage::BaseSystems)
    {
      Shutdown(wdStartupStage::CoreSystems);
    }

    if (stage == wdStartupStage::CoreSystems)
    {
      Shutdown(wdStartupStage::HighLevelSystems);
      s_bPrintAllSubSystems = true;

      wdGlobalEvent::Broadcast(WD_GLOBALEVENT_SHUTDOWN_CORESYSTEMS_BEGIN);
    }

    if (stage == wdStartupStage::HighLevelSystems)
    {
      wdGlobalEvent::Broadcast(WD_GLOBALEVENT_SHUTDOWN_HIGHLEVELSYSTEMS_BEGIN);
    }

    WD_LOG_BLOCK(szStartup[stage]);

    wdDeque<wdSubSystem*> Order;
    ComputeOrder(Order);

    for (wdInt32 i = (wdInt32)Order.GetCount() - 1; i >= 0; --i)
    {
      if (Order[i]->m_bStartupDone[stage])
      {
        switch (stage)
        {
          case wdStartupStage::CoreSystems:
            wdLog::Debug("Executing 'Core' shutdown of sub-system '{0}::{1}'", Order[i]->GetGroupName(), Order[i]->GetSubSystemName());
            Order[i]->OnCoreSystemsShutdown();
            break;

          case wdStartupStage::HighLevelSystems:
            wdLog::Debug("Executing 'Engine' shutdown of sub-system '{0}::{1}'", Order[i]->GetGroupName(), Order[i]->GetSubSystemName());
            Order[i]->OnHighLevelSystemsShutdown();
            break;

          default:
            break;
        }

        Order[i]->m_bStartupDone[stage] = false;
      }
    }
  }

  switch (stage)
  {
    case wdStartupStage::CoreSystems:
      wdGlobalEvent::Broadcast(WD_GLOBALEVENT_SHUTDOWN_CORESYSTEMS_END);
      break;

    case wdStartupStage::HighLevelSystems:
      wdGlobalEvent::Broadcast(WD_GLOBALEVENT_SHUTDOWN_HIGHLEVELSYSTEMS_END);
      break;

    default:
      break;
  }

  if (s_CurrentState != wdStartupStage::None)
  {
    s_CurrentState = (wdStartupStage::Enum)(((wdInt32)stage) - 1);

    if (s_CurrentState == wdStartupStage::None)
    {
      wdPlugin::Events().RemoveEventHandler(PluginEventHandler);
    }
  }
}

bool wdStartup::HasDependencyOnPlugin(wdSubSystem* pSubSystem, const char* szModule)
{
  if (wdStringUtils::IsEqual(pSubSystem->m_szPluginName, szModule))
    return true;

  for (wdUInt32 i = 0; pSubSystem->GetDependency(i) != nullptr; ++i)
  {
    wdSubSystem* pSub = wdSubSystem::GetFirstInstance();
    while (pSub)
    {
      if (wdStringUtils::IsEqual(pSub->GetSubSystemName(), pSubSystem->GetDependency(i)))
      {
        if (HasDependencyOnPlugin(pSub, szModule))
          return true;

        break;
      }

      pSub = pSub->GetNextInstance();
    }
  }

  return false;
}

void wdStartup::UnloadPluginSubSystems(const char* szPluginName)
{
  WD_LOG_BLOCK("Unloading Plugin SubSystems", szPluginName);
  wdLog::Dev("Plugin to unload: '{0}'", szPluginName);

  wdGlobalEvent::Broadcast(WD_GLOBALEVENT_UNLOAD_PLUGIN_BEGIN, wdVariant(szPluginName));

  wdDeque<wdSubSystem*> Order;
  ComputeOrder(Order);

  for (wdInt32 i = (wdInt32)Order.GetCount() - 1; i >= 0; --i)
  {
    if (Order[i]->m_bStartupDone[wdStartupStage::HighLevelSystems] && HasDependencyOnPlugin(Order[i], szPluginName))
    {
      wdLog::Info("Engine shutdown of SubSystem '{0}::{1}', because it depends on Plugin '{2}'.", Order[i]->GetGroupName(),
        Order[i]->GetSubSystemName(), szPluginName);
      Order[i]->OnHighLevelSystemsShutdown();
      Order[i]->m_bStartupDone[wdStartupStage::HighLevelSystems] = false;
    }
  }

  for (wdInt32 i = (wdInt32)Order.GetCount() - 1; i >= 0; --i)
  {
    if (Order[i]->m_bStartupDone[wdStartupStage::CoreSystems] && HasDependencyOnPlugin(Order[i], szPluginName))
    {
      wdLog::Info("Core shutdown of SubSystem '{0}::{1}', because it depends on Plugin '{2}'.", Order[i]->GetGroupName(),
        Order[i]->GetSubSystemName(), szPluginName);
      Order[i]->OnCoreSystemsShutdown();
      Order[i]->m_bStartupDone[wdStartupStage::CoreSystems] = false;
    }
  }


  wdGlobalEvent::Broadcast(WD_GLOBALEVENT_UNLOAD_PLUGIN_END, wdVariant(szPluginName));
}

void wdStartup::ReinitToCurrentState()
{
  if (s_CurrentState != wdStartupStage::None)
    Startup(s_CurrentState);
}



WD_STATICLINK_FILE(Foundation, Foundation_Configuration_Implementation_Startup);
