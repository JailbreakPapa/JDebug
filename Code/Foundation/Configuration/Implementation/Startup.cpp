#include <Foundation/FoundationPCH.h>

#include <Foundation/Communication/GlobalEvent.h>
#include <Foundation/Configuration/Startup.h>
#include <Foundation/Containers/Set.h>
#include <Foundation/Logging/Log.h>
#include <Foundation/Threading/ThreadUtils.h>

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsSubSystem);

bool nsStartup::s_bPrintAllSubSystems = true;
nsStartupStage::Enum nsStartup::s_CurrentState = nsStartupStage::None;
nsDynamicArray<const char*> nsStartup::s_ApplicationTags;


void nsStartup::AddApplicationTag(const char* szTag)
{
  s_ApplicationTags.PushBack(szTag);
}

bool nsStartup::HasApplicationTag(const char* szTag)
{
  for (nsUInt32 i = 0; i < s_ApplicationTags.GetCount(); ++i)
  {
    if (nsStringUtils::IsEqual_NoCase(s_ApplicationTags[i], szTag))
      return true;
  }

  return false;
}

void nsStartup::PrintAllSubsystems()
{
  NS_LOG_BLOCK("Available Subsystems");

  nsSubSystem* pSub = nsSubSystem::GetFirstInstance();

  while (pSub)
  {
    nsLog::Debug("Subsystem: '{0}::{1}'", pSub->GetGroupName(), pSub->GetSubSystemName());

    if (pSub->GetDependency(0) == nullptr)
      nsLog::Debug("  <no dependencies>");
    else
    {
      for (nsInt32 i = 0; pSub->GetDependency(i) != nullptr; ++i)
        nsLog::Debug("  depends on '{0}'", pSub->GetDependency(i));
    }

    nsLog::Debug("");

    pSub = pSub->GetNextInstance();
  }
}

void nsStartup::AssignSubSystemPlugin(nsStringView sPluginName)
{
  // iterates over all existing subsystems and finds those that have no plugin name yet
  // assigns the given name to them

  nsSubSystem* pSub = nsSubSystem::GetFirstInstance();

  while (pSub)
  {
    if (pSub->m_sPluginName.IsEmpty())
    {
      pSub->m_sPluginName = sPluginName;
    }

    pSub = pSub->GetNextInstance();
  }
}

void nsStartup::PluginEventHandler(const nsPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case nsPluginEvent::BeforeLoading:
    {
      AssignSubSystemPlugin("Static");
    }
    break;

    case nsPluginEvent::AfterLoadingBeforeInit:
    {
      AssignSubSystemPlugin(EventData.m_sPluginBinary);
    }
    break;

    case nsPluginEvent::StartupShutdown:
    {
      nsStartup::UnloadPluginSubSystems(EventData.m_sPluginBinary);
    }
    break;

    case nsPluginEvent::AfterPluginChanges:
    {
      nsStartup::ReinitToCurrentState();
    }
    break;

    default:
      break;
  }
}

static bool IsGroupName(nsStringView sName)
{
  nsSubSystem* pSub = nsSubSystem::GetFirstInstance();

  bool bGroup = false;
  bool bSubSystem = false;

  while (pSub)
  {
    if (pSub->GetGroupName() == sName)
      bGroup = true;

    if (pSub->GetSubSystemName() == sName)
      bSubSystem = true;

    pSub = pSub->GetNextInstance();
  }

  NS_ASSERT_ALWAYS(!bGroup || !bSubSystem, "There cannot be a SubSystem AND a Group called '{0}'.", sName);

  return bGroup;
}

static nsStringView GetGroupSubSystems(nsStringView sGroup, nsInt32 iSubSystem)
{
  nsSubSystem* pSub = nsSubSystem::GetFirstInstance();

  while (pSub)
  {
    if (pSub->GetGroupName() == sGroup)
    {
      if (iSubSystem == 0)
        return pSub->GetSubSystemName();

      --iSubSystem;
    }

    pSub = pSub->GetNextInstance();
  }

  return nullptr;
}

void nsStartup::ComputeOrder(nsDeque<nsSubSystem*>& Order)
{
  Order.Clear();
  nsSet<nsString> sSystemsInited;

  bool bCouldInitAny = true;

  while (bCouldInitAny)
  {
    bCouldInitAny = false;

    nsSubSystem* pSub = nsSubSystem::GetFirstInstance();

    while (pSub)
    {
      if (!sSystemsInited.Find(pSub->GetSubSystemName()).IsValid())
      {
        bool bAllDependsFulfilled = true;
        nsInt32 iDep = 0;

        while (pSub->GetDependency(iDep) != nullptr)
        {
          if (IsGroupName(pSub->GetDependency(iDep)))
          {
            nsInt32 iSubSystemIndex = 0;
            nsStringView sNextSubSystem = GetGroupSubSystems(pSub->GetDependency(iDep), iSubSystemIndex);
            while (sNextSubSystem.IsValid())
            {
              if (!sSystemsInited.Find(sNextSubSystem).IsValid())
              {
                bAllDependsFulfilled = false;
                break;
              }

              ++iSubSystemIndex;
              sNextSubSystem = GetGroupSubSystems(pSub->GetDependency(iDep), iSubSystemIndex);
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

void nsStartup::Startup(nsStartupStage::Enum stage)
{
  if (stage == nsStartupStage::BaseSystems)
  {
    nsFoundation::Initialize();
  }

  const char* szStartup[] = {"Startup Base", "Startup Core", "Startup Engine"};

  if (stage == nsStartupStage::CoreSystems)
  {
    Startup(nsStartupStage::BaseSystems);

    nsGlobalEvent::Broadcast(NS_GLOBALEVENT_STARTUP_CORESYSTEMS_BEGIN);

    if (s_bPrintAllSubSystems)
    {
      s_bPrintAllSubSystems = false;
      PrintAllSubsystems();
    }
  }

  if (stage == nsStartupStage::HighLevelSystems)
  {
    Startup(nsStartupStage::CoreSystems);

    nsGlobalEvent::Broadcast(NS_GLOBALEVENT_STARTUP_HIGHLEVELSYSTEMS_BEGIN);
  }

  NS_LOG_BLOCK(szStartup[stage]);

  nsDeque<nsSubSystem*> Order;
  ComputeOrder(Order);

  for (nsUInt32 i = 0; i < Order.GetCount(); ++i)
  {
    if (!Order[i]->m_bStartupDone[stage])
    {
      Order[i]->m_bStartupDone[stage] = true;

      switch (stage)
      {
        case nsStartupStage::BaseSystems:
          nsLog::Debug("Executing 'Base' startup for sub-system '{1}::{0}'", Order[i]->GetSubSystemName(), Order[i]->GetGroupName());
          Order[i]->OnBaseSystemsStartup();
          break;
        case nsStartupStage::CoreSystems:
          nsLog::Debug("Executing 'Core' startup for sub-system '{1}::{0}'", Order[i]->GetSubSystemName(), Order[i]->GetGroupName());
          Order[i]->OnCoreSystemsStartup();
          break;
        case nsStartupStage::HighLevelSystems:
          nsLog::Debug("Executing 'Engine' startup for sub-system '{1}::{0}'", Order[i]->GetSubSystemName(), Order[i]->GetGroupName());
          Order[i]->OnHighLevelSystemsStartup();
          break;

        default:
          break;
      }
    }
  }

  // now everything should be started
  {
    NS_LOG_BLOCK("Failed SubSystems");

    nsSet<nsString> sSystemsFound;

    nsSubSystem* pSub = nsSubSystem::GetFirstInstance();

    while (pSub)
    {
      sSystemsFound.Insert(pSub->GetSubSystemName());
      pSub = pSub->GetNextInstance();
    }

    pSub = nsSubSystem::GetFirstInstance();

    while (pSub)
    {
      if (!pSub->m_bStartupDone[stage])
      {
        nsInt32 iDep = 0;

        while (pSub->GetDependency(iDep) != nullptr)
        {
          if (!sSystemsFound.Find(pSub->GetDependency(iDep)).IsValid())
          {
            nsLog::Error("SubSystem '{0}::{1}' could not be started because dependency '{2}' is unknown.", pSub->GetGroupName(),
              pSub->GetSubSystemName(), pSub->GetDependency(iDep));
          }
          else
          {
            nsLog::Error("SubSystem '{0}::{1}' could not be started because dependency '{2}' has not been initialized.", pSub->GetGroupName(),
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
    case nsStartupStage::BaseSystems:
      break;
    case nsStartupStage::CoreSystems:
      nsGlobalEvent::Broadcast(NS_GLOBALEVENT_STARTUP_CORESYSTEMS_END);
      break;
    case nsStartupStage::HighLevelSystems:
      nsGlobalEvent::Broadcast(NS_GLOBALEVENT_STARTUP_HIGHLEVELSYSTEMS_END);
      break;

    default:
      break;
  }

  if (s_CurrentState == nsStartupStage::None)
  {
    nsPlugin::Events().AddEventHandler(PluginEventHandler);
  }

  s_CurrentState = stage;
}

void nsStartup::Shutdown(nsStartupStage::Enum stage)
{
  // without that we cannot function, so make sure it is up and running
  nsFoundation::Initialize();

  {
    const char* szStartup[] = {"Shutdown Base", "Shutdown Core", "Shutdown Engine"};

    if (stage == nsStartupStage::BaseSystems)
    {
      Shutdown(nsStartupStage::CoreSystems);
    }

    if (stage == nsStartupStage::CoreSystems)
    {
      Shutdown(nsStartupStage::HighLevelSystems);
      s_bPrintAllSubSystems = true;

      nsGlobalEvent::Broadcast(NS_GLOBALEVENT_SHUTDOWN_CORESYSTEMS_BEGIN);
    }

    if (stage == nsStartupStage::HighLevelSystems)
    {
      nsGlobalEvent::Broadcast(NS_GLOBALEVENT_SHUTDOWN_HIGHLEVELSYSTEMS_BEGIN);
    }

    NS_LOG_BLOCK(szStartup[stage]);

    nsDeque<nsSubSystem*> Order;
    ComputeOrder(Order);

    for (nsInt32 i = (nsInt32)Order.GetCount() - 1; i >= 0; --i)
    {
      if (Order[i]->m_bStartupDone[stage])
      {
        switch (stage)
        {
          case nsStartupStage::CoreSystems:
            nsLog::Debug("Executing 'Core' shutdown of sub-system '{0}::{1}'", Order[i]->GetGroupName(), Order[i]->GetSubSystemName());
            Order[i]->OnCoreSystemsShutdown();
            break;

          case nsStartupStage::HighLevelSystems:
            nsLog::Debug("Executing 'Engine' shutdown of sub-system '{0}::{1}'", Order[i]->GetGroupName(), Order[i]->GetSubSystemName());
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
    case nsStartupStage::CoreSystems:
      nsGlobalEvent::Broadcast(NS_GLOBALEVENT_SHUTDOWN_CORESYSTEMS_END);
      break;

    case nsStartupStage::HighLevelSystems:
      nsGlobalEvent::Broadcast(NS_GLOBALEVENT_SHUTDOWN_HIGHLEVELSYSTEMS_END);
      break;

    default:
      break;
  }

  if (s_CurrentState != nsStartupStage::None)
  {
    s_CurrentState = (nsStartupStage::Enum)(((nsInt32)stage) - 1);

    if (s_CurrentState == nsStartupStage::None)
    {
      nsPlugin::Events().RemoveEventHandler(PluginEventHandler);
    }
  }
}

bool nsStartup::HasDependencyOnPlugin(nsSubSystem* pSubSystem, nsStringView sModule)
{
  if (pSubSystem->m_sPluginName == sModule)
    return true;

  for (nsUInt32 i = 0; pSubSystem->GetDependency(i) != nullptr; ++i)
  {
    nsSubSystem* pSub = nsSubSystem::GetFirstInstance();
    while (pSub)
    {
      if (pSub->GetSubSystemName() == pSubSystem->GetDependency(i))
      {
        if (HasDependencyOnPlugin(pSub, sModule))
          return true;

        break;
      }

      pSub = pSub->GetNextInstance();
    }
  }

  return false;
}

void nsStartup::UnloadPluginSubSystems(nsStringView sPluginName)
{
  NS_LOG_BLOCK("Unloading Plugin SubSystems", sPluginName);
  nsLog::Dev("Plugin to unload: '{0}'", sPluginName);

  nsGlobalEvent::Broadcast(NS_GLOBALEVENT_UNLOAD_PLUGIN_BEGIN, nsVariant(sPluginName));

  nsDeque<nsSubSystem*> Order;
  ComputeOrder(Order);

  for (nsInt32 i = (nsInt32)Order.GetCount() - 1; i >= 0; --i)
  {
    if (Order[i]->m_bStartupDone[nsStartupStage::HighLevelSystems] && HasDependencyOnPlugin(Order[i], sPluginName))
    {
      nsLog::Info("Engine shutdown of SubSystem '{0}::{1}', because it depends on Plugin '{2}'.", Order[i]->GetGroupName(), Order[i]->GetSubSystemName(), sPluginName);
      Order[i]->OnHighLevelSystemsShutdown();
      Order[i]->m_bStartupDone[nsStartupStage::HighLevelSystems] = false;
    }
  }

  for (nsInt32 i = (nsInt32)Order.GetCount() - 1; i >= 0; --i)
  {
    if (Order[i]->m_bStartupDone[nsStartupStage::CoreSystems] && HasDependencyOnPlugin(Order[i], sPluginName))
    {
      nsLog::Info("Core shutdown of SubSystem '{0}::{1}', because it depends on Plugin '{2}'.", Order[i]->GetGroupName(), Order[i]->GetSubSystemName(), sPluginName);
      Order[i]->OnCoreSystemsShutdown();
      Order[i]->m_bStartupDone[nsStartupStage::CoreSystems] = false;
    }
  }


  nsGlobalEvent::Broadcast(NS_GLOBALEVENT_UNLOAD_PLUGIN_END, nsVariant(sPluginName));
}

void nsStartup::ReinitToCurrentState()
{
  if (s_CurrentState != nsStartupStage::None)
    Startup(s_CurrentState);
}
