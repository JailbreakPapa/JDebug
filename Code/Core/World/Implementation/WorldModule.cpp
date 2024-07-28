#include <Core/CorePCH.h>

#include <Core/World/World.h>

// clang-format off
NS_BEGIN_DYNAMIC_REFLECTED_TYPE(nsWorldModule, 1, nsRTTINoAllocator)
NS_END_DYNAMIC_REFLECTED_TYPE;
// clang-format on

nsWorldModule::nsWorldModule(nsWorld* pWorld)
  : m_pWorld(pWorld)
{
}

nsWorldModule::~nsWorldModule() = default;

nsUInt32 nsWorldModule::GetWorldIndex() const
{
  return GetWorld()->GetIndex();
}

// protected methods

void nsWorldModule::RegisterUpdateFunction(const UpdateFunctionDesc& desc)
{
  m_pWorld->RegisterUpdateFunction(desc);
}

void nsWorldModule::DeregisterUpdateFunction(const UpdateFunctionDesc& desc)
{
  m_pWorld->DeregisterUpdateFunction(desc);
}

nsAllocator* nsWorldModule::GetAllocator()
{
  return m_pWorld->GetAllocator();
}

nsInternal::WorldLargeBlockAllocator* nsWorldModule::GetBlockAllocator()
{
  return m_pWorld->GetBlockAllocator();
}

bool nsWorldModule::GetWorldSimulationEnabled() const
{
  return m_pWorld->GetWorldSimulationEnabled();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Core, WorldModuleFactory)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Reflection"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsPlugin::Events().AddEventHandler(nsWorldModuleFactory::PluginEventHandler);
    nsWorldModuleFactory::GetInstance()->FillBaseTypeIds();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsPlugin::Events().RemoveEventHandler(nsWorldModuleFactory::PluginEventHandler);
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

static nsWorldModuleTypeId s_uiNextTypeId = 0;
static nsDynamicArray<nsWorldModuleTypeId> s_freeTypeIds;
static constexpr nsWorldModuleTypeId s_InvalidWorldModuleTypeId = nsWorldModuleTypeId(-1);

nsWorldModuleFactory::nsWorldModuleFactory() = default;

// static
nsWorldModuleFactory* nsWorldModuleFactory::GetInstance()
{
  static nsWorldModuleFactory* pInstance = new nsWorldModuleFactory();
  return pInstance;
}

nsWorldModuleTypeId nsWorldModuleFactory::GetTypeId(const nsRTTI* pRtti)
{
  nsWorldModuleTypeId uiTypeId = s_InvalidWorldModuleTypeId;
  m_TypeToId.TryGetValue(pRtti, uiTypeId);
  return uiTypeId;
}

nsWorldModule* nsWorldModuleFactory::CreateWorldModule(nsWorldModuleTypeId typeId, nsWorld* pWorld)
{
  if (typeId < m_CreatorFuncs.GetCount())
  {
    CreatorFunc func = m_CreatorFuncs[typeId].m_Func;
    return (*func)(pWorld->GetAllocator(), pWorld);
  }

  return nullptr;
}

void nsWorldModuleFactory::RegisterInterfaceImplementation(nsStringView sInterfaceName, nsStringView sImplementationName)
{
  m_InterfaceImplementations.Insert(sInterfaceName, sImplementationName);

  nsStringBuilder sTemp = sInterfaceName;
  const nsRTTI* pInterfaceRtti = nsRTTI::FindTypeByName(sTemp);

  sTemp = sImplementationName;
  const nsRTTI* pImplementationRtti = nsRTTI::FindTypeByName(sTemp);

  if (pInterfaceRtti != nullptr && pImplementationRtti != nullptr)
  {
    m_TypeToId[pInterfaceRtti] = m_TypeToId[pImplementationRtti];
    return;
  }

  // Clear existing mapping if it maps to the wrong type
  nsUInt16 uiTypeId;
  if (pInterfaceRtti != nullptr && m_TypeToId.TryGetValue(pInterfaceRtti, uiTypeId))
  {
    if (m_CreatorFuncs[uiTypeId].m_pRtti->GetTypeName() != sImplementationName)
    {
      NS_ASSERT_DEV(pImplementationRtti == nullptr, "Implementation error");
      m_TypeToId.Remove(pInterfaceRtti);
    }
  }
}
nsWorldModuleTypeId nsWorldModuleFactory::RegisterWorldModule(const nsRTTI* pRtti, CreatorFunc creatorFunc)
{
  NS_ASSERT_DEV(pRtti != nsGetStaticRTTI<nsWorldModule>(), "Trying to register a world module that is not reflected!");
  NS_ASSERT_DEV(
    m_TypeToId.GetCount() < nsWorld::GetMaxNumWorldModules(), "Max number of world modules reached: {}", nsWorld::GetMaxNumWorldModules());

  nsWorldModuleTypeId uiTypeId = s_InvalidWorldModuleTypeId;
  if (m_TypeToId.TryGetValue(pRtti, uiTypeId))
  {
    return uiTypeId;
  }

  if (s_freeTypeIds.IsEmpty())
  {
    NS_ASSERT_DEV(s_uiNextTypeId < NS_MAX_WORLD_MODULE_TYPES - 1, "World module id overflow!");

    uiTypeId = s_uiNextTypeId++;
  }
  else
  {
    uiTypeId = s_freeTypeIds.PeekBack();
    s_freeTypeIds.PopBack();
  }

  m_TypeToId.Insert(pRtti, uiTypeId);

  m_CreatorFuncs.EnsureCount(uiTypeId + 1);

  auto& creatorFuncContext = m_CreatorFuncs[uiTypeId];
  creatorFuncContext.m_Func = creatorFunc;
  creatorFuncContext.m_pRtti = pRtti;

  return uiTypeId;
}

// static
void nsWorldModuleFactory::PluginEventHandler(const nsPluginEvent& EventData)
{
  if (EventData.m_EventType == nsPluginEvent::AfterLoadingBeforeInit)
  {
    nsWorldModuleFactory::GetInstance()->FillBaseTypeIds();
  }

  if (EventData.m_EventType == nsPluginEvent::AfterUnloading)
  {
    nsWorldModuleFactory::GetInstance()->ClearUnloadedTypeToIDs();
  }
}

namespace
{
  struct NewEntry
  {
    NS_DECLARE_POD_TYPE();

    const nsRTTI* m_pRtti;
    nsWorldModuleTypeId m_uiTypeId;
  };
} // namespace

void nsWorldModuleFactory::AdjustBaseTypeId(const nsRTTI* pParentRtti, const nsRTTI* pRtti, nsUInt16 uiParentTypeId)
{
  nsDynamicArray<nsPlugin::PluginInfo> infos;
  nsPlugin::GetAllPluginInfos(infos);

  auto HasManualDependency = [&](nsStringView sPluginName) -> bool
  {
    for (const auto& p : infos)
    {
      if (p.m_sName == sPluginName)
      {
        return !p.m_LoadFlags.IsSet(nsPluginLoadFlags::CustomDependency);
      }
    }

    return false;
  };

  nsStringView szPlugin1 = m_CreatorFuncs[uiParentTypeId].m_pRtti->GetPluginName();
  nsStringView szPlugin2 = pRtti->GetPluginName();

  const bool bPrio1 = HasManualDependency(szPlugin1);
  const bool bPrio2 = HasManualDependency(szPlugin2);

  if (bPrio1 && !bPrio2)
  {
    // keep the previous one
    return;
  }

  if (!bPrio1 && bPrio2)
  {
    // take the new one
    m_TypeToId[pParentRtti] = m_TypeToId[pRtti];
    return;
  }

  nsLog::Error("Interface '{}' is already implemented by '{}'. Specify which implementation should be used via RegisterInterfaceImplementation() or WorldModules.ddl config file.", pParentRtti->GetTypeName(), m_CreatorFuncs[uiParentTypeId].m_pRtti->GetTypeName());
}

void nsWorldModuleFactory::FillBaseTypeIds()
{
  // m_TypeToId contains RTTI types for nsWorldModules and nsComponents
  // m_TypeToId[nsComponent] maps to TypeID for its respective nsComponentManager
  // m_TypeToId[nsWorldModule] maps to TypeID for itself OR in case of an interface to the derived type that implements the interface
  // after types are registered we only have a mapping for m_TypeToId[nsWorldModule(impl)] and now we want to add
  // the mapping for m_TypeToId[nsWorldModule(interface)], such that querying the TypeID for the interface works as well
  // and yields the implementation

  nsHybridArray<NewEntry, 64, nsStaticsAllocatorWrapper> newEntries;
  const nsRTTI* pModuleRtti = nsGetStaticRTTI<nsWorldModule>(); // base type where we want to stop iterating upwards

  // explicit mappings
  for (auto it = m_InterfaceImplementations.GetIterator(); it.IsValid(); ++it)
  {
    const nsRTTI* pInterfaceRtti = nsRTTI::FindTypeByName(it.Key());
    const nsRTTI* pImplementationRtti = nsRTTI::FindTypeByName(it.Value());

    if (pInterfaceRtti != nullptr && pImplementationRtti != nullptr)
    {
      m_TypeToId[pInterfaceRtti] = m_TypeToId[pImplementationRtti];
    }
  }

  // automatic mappings
  for (auto it = m_TypeToId.GetIterator(); it.IsValid(); ++it)
  {
    const nsRTTI* pRtti = it.Key();

    // ignore components, we only want to fill out mappings for the base types of world modules
    if (!pRtti->IsDerivedFrom<nsWorldModule>())
      continue;

    const nsWorldModuleTypeId uiTypeId = it.Value();

    for (const nsRTTI* pParentRtti = pRtti->GetParentType(); pParentRtti != pModuleRtti; pParentRtti = pParentRtti->GetParentType())
    {
      // we are only interested in parent types that are pure interfaces
      if (!pParentRtti->GetTypeFlags().IsSet(nsTypeFlags::Abstract))
        continue;

      // skip if we have an explicit mapping for this interface, they are already handled above
      if (m_InterfaceImplementations.GetValue(pParentRtti->GetTypeName()) != nullptr)
        continue;


      if (nsUInt16* pParentTypeId = m_TypeToId.GetValue(pParentRtti))
      {
        if (*pParentTypeId != uiTypeId)
        {
          AdjustBaseTypeId(pParentRtti, pRtti, *pParentTypeId);
        }
      }
      else
      {
        auto& newEntry = newEntries.ExpandAndGetRef();
        newEntry.m_pRtti = pParentRtti;
        newEntry.m_uiTypeId = uiTypeId;
      }
    }
  }

  // delayed insertion to not interfere with the iteration above
  for (auto& newEntry : newEntries)
  {
    m_TypeToId.Insert(newEntry.m_pRtti, newEntry.m_uiTypeId);
  }
}

void nsWorldModuleFactory::ClearUnloadedTypeToIDs()
{
  nsSet<const nsRTTI*> allRttis;
  nsRTTI::ForEachType([&](const nsRTTI* pRtti)
    { allRttis.Insert(pRtti); });

  nsSet<nsWorldModuleTypeId> mappedIdsToRemove;

  for (auto it = m_TypeToId.GetIterator(); it.IsValid();)
  {
    const nsRTTI* pRtti = it.Key();
    const nsWorldModuleTypeId uiTypeId = it.Value();

    if (!allRttis.Contains(pRtti))
    {
      // type got removed, clear it from the map
      it = m_TypeToId.Remove(it);

      // and record that all other types that map to the same typeId also must be removed
      mappedIdsToRemove.Insert(uiTypeId);
    }
    else
    {
      ++it;
    }
  }

  // now remove all mappings that map to an invalid typeId
  // this can be more than one, since we can map multiple (interface) types to the same implementation
  for (auto it = m_TypeToId.GetIterator(); it.IsValid();)
  {
    const nsWorldModuleTypeId uiTypeId = it.Value();

    if (mappedIdsToRemove.Contains(uiTypeId))
    {
      it = m_TypeToId.Remove(it);
    }
    else
    {
      ++it;
    }
  }

  // Finally, adding all invalid typeIds to the free list for reusing later
  for (nsWorldModuleTypeId removedId : mappedIdsToRemove)
  {
    s_freeTypeIds.PushBack(removedId);
  }
}

NS_STATICLINK_FILE(Core, Core_World_Implementation_WorldModule);
