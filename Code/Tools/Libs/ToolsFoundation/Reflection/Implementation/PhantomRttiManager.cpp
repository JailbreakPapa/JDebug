#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Memory/AllocatorWithPolicy.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundation/Reflection/PhantomRtti.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>

nsCopyOnBroadcastEvent<const nsPhantomRttiManagerEvent&> nsPhantomRttiManager::s_Events;

nsHashTable<nsStringView, nsPhantomRTTI*> nsPhantomRttiManager::s_NameToPhantom;

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, ReflectedTypeManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    nsPhantomRttiManager::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsPhantomRttiManager::Shutdown();
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

////////////////////////////////////////////////////////////////////////
// nsPhantomRttiManager public functions
////////////////////////////////////////////////////////////////////////

const nsRTTI* nsPhantomRttiManager::RegisterType(nsReflectedTypeDescriptor& ref_desc)
{
  NS_PROFILE_SCOPE("RegisterType");
  const nsRTTI* pType = nsRTTI::FindTypeByName(ref_desc.m_sTypeName);
  nsPhantomRTTI* pPhantom = nullptr;
  s_NameToPhantom.TryGetValue(ref_desc.m_sTypeName, pPhantom);

  // concrete type !
  if (pPhantom == nullptr && pType != nullptr)
  {
    return pType;
  }

  if (pPhantom != nullptr && pPhantom->IsEqualToDescriptor(ref_desc))
    return pPhantom;

  if (pPhantom == nullptr)
  {
    pPhantom = NS_DEFAULT_NEW(nsPhantomRTTI, ref_desc.m_sTypeName.GetData(), nsRTTI::FindTypeByName(ref_desc.m_sParentTypeName), 0,
      ref_desc.m_uiTypeVersion, nsVariantType::Invalid, ref_desc.m_Flags, ref_desc.m_sPluginName.GetData());

    pPhantom->SetProperties(ref_desc.m_Properties);
    pPhantom->SetAttributes(ref_desc.m_Attributes);
    pPhantom->SetFunctions(ref_desc.m_Functions);
    pPhantom->SetupParentHierarchy();

    s_NameToPhantom[pPhantom->GetTypeName()] = pPhantom;

    nsPhantomRttiManagerEvent msg;
    msg.m_pChangedType = pPhantom;
    msg.m_Type = nsPhantomRttiManagerEvent::Type::TypeAdded;
    s_Events.Broadcast(msg, 1); /// \todo Had to increase the recursion depth to allow registering phantom types that are based on actual
                                /// types coming from the engine process
  }
  else
  {
    pPhantom->UpdateType(ref_desc);

    nsPhantomRttiManagerEvent msg;
    msg.m_pChangedType = pPhantom;
    msg.m_Type = nsPhantomRttiManagerEvent::Type::TypeChanged;
    s_Events.Broadcast(msg, 1);
  }

  return pPhantom;
}

bool nsPhantomRttiManager::UnregisterType(const nsRTTI* pRtti)
{
  nsPhantomRTTI* pPhantom = nullptr;
  s_NameToPhantom.TryGetValue(pRtti->GetTypeName(), pPhantom);

  if (pPhantom == nullptr)
    return false;

  {
    nsPhantomRttiManagerEvent msg;
    msg.m_pChangedType = pPhantom;
    msg.m_Type = nsPhantomRttiManagerEvent::Type::TypeRemoved;
    s_Events.Broadcast(msg);
  }

  s_NameToPhantom.Remove(pPhantom->GetTypeName());

  NS_DEFAULT_DELETE(pPhantom);
  return true;
}

////////////////////////////////////////////////////////////////////////
// nsPhantomRttiManager private functions
////////////////////////////////////////////////////////////////////////

void nsPhantomRttiManager::PluginEventHandler(const nsPluginEvent& e)
{
  if (e.m_EventType == nsPluginEvent::Type::BeforeUnloading)
  {
    while (!s_NameToPhantom.IsEmpty())
    {
      UnregisterType(s_NameToPhantom.GetIterator().Value());
    }

    NS_ASSERT_DEV(s_NameToPhantom.IsEmpty(), "nsPhantomRttiManager::Shutdown: Removal of types failed!");
  }
}

void nsPhantomRttiManager::Startup()
{
  nsPlugin::Events().AddEventHandler(&nsPhantomRttiManager::PluginEventHandler);
}


void nsPhantomRttiManager::Shutdown()
{
  nsPlugin::Events().RemoveEventHandler(&nsPhantomRttiManager::PluginEventHandler);

  while (!s_NameToPhantom.IsEmpty())
  {
    UnregisterType(s_NameToPhantom.GetIterator().Value());
  }

  NS_ASSERT_DEV(s_NameToPhantom.IsEmpty(), "nsPhantomRttiManager::Shutdown: Removal of types failed!");
}
