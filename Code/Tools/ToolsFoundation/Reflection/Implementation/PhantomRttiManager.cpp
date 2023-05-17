#include <ToolsFoundation/ToolsFoundationPCH.h>

#include <Foundation/Configuration/Startup.h>
#include <Foundation/Memory/Allocator.h>
#include <Foundation/Profiling/Profiling.h>
#include <Foundation/Reflection/Reflection.h>
#include <ToolsFoundation/Reflection/PhantomRtti.h>
#include <ToolsFoundation/Reflection/PhantomRttiManager.h>
#include <ToolsFoundation/Reflection/ToolsReflectionUtils.h>

wdCopyOnBroadcastEvent<const wdPhantomRttiManagerEvent&> wdPhantomRttiManager::s_Events;

wdHashTable<const char*, wdPhantomRTTI*> wdPhantomRttiManager::s_NameToPhantom;

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(ToolsFoundation, ReflectedTypeManager)

  BEGIN_SUBSYSTEM_DEPENDENCIES
  "Foundation"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    wdPhantomRttiManager::Startup();
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    wdPhantomRttiManager::Shutdown();
  }

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

////////////////////////////////////////////////////////////////////////
// wdPhantomRttiManager public functions
////////////////////////////////////////////////////////////////////////

const wdRTTI* wdPhantomRttiManager::RegisterType(wdReflectedTypeDescriptor& ref_desc)
{
  WD_PROFILE_SCOPE("RegisterType");
  wdRTTI* pType = wdRTTI::FindTypeByName(ref_desc.m_sTypeName);
  wdPhantomRTTI* pPhantom = nullptr;
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
    pPhantom = WD_DEFAULT_NEW(wdPhantomRTTI, ref_desc.m_sTypeName.GetData(), wdRTTI::FindTypeByName(ref_desc.m_sParentTypeName), 0,
      ref_desc.m_uiTypeVersion, wdVariantType::Invalid, ref_desc.m_Flags, ref_desc.m_sPluginName.GetData());

    pPhantom->SetProperties(ref_desc.m_Properties);
    pPhantom->SetAttributes(ref_desc.m_Attributes);
    pPhantom->SetFunctions(ref_desc.m_Functions);
    pPhantom->SetupParentHierarchy();

    s_NameToPhantom[pPhantom->GetTypeName()] = pPhantom;

    wdPhantomRttiManagerEvent msg;
    msg.m_pChangedType = pPhantom;
    msg.m_Type = wdPhantomRttiManagerEvent::Type::TypeAdded;
    s_Events.Broadcast(msg, 1); /// \todo Had to increase the recursion depth to allow registering phantom types that are based on actual
                                /// types coming from the engine process
  }
  else
  {
    pPhantom->UpdateType(ref_desc);

    wdPhantomRttiManagerEvent msg;
    msg.m_pChangedType = pPhantom;
    msg.m_Type = wdPhantomRttiManagerEvent::Type::TypeChanged;
    s_Events.Broadcast(msg, 1);
  }

  return pPhantom;
}

bool wdPhantomRttiManager::UnregisterType(const wdRTTI* pRtti)
{
  wdPhantomRTTI* pPhantom = nullptr;
  s_NameToPhantom.TryGetValue(pRtti->GetTypeName(), pPhantom);

  if (pPhantom == nullptr)
    return false;

  {
    wdPhantomRttiManagerEvent msg;
    msg.m_pChangedType = pPhantom;
    msg.m_Type = wdPhantomRttiManagerEvent::Type::TypeRemoved;
    s_Events.Broadcast(msg);
  }

  s_NameToPhantom.Remove(pPhantom->GetTypeName());

  WD_DEFAULT_DELETE(pPhantom);
  return true;
}

////////////////////////////////////////////////////////////////////////
// wdPhantomRttiManager private functions
////////////////////////////////////////////////////////////////////////

void wdPhantomRttiManager::PluginEventHandler(const wdPluginEvent& e)
{
  if (e.m_EventType == wdPluginEvent::Type::BeforeUnloading)
  {
    while (!s_NameToPhantom.IsEmpty())
    {
      UnregisterType(s_NameToPhantom.GetIterator().Value());
    }

    WD_ASSERT_DEV(s_NameToPhantom.IsEmpty(), "wdPhantomRttiManager::Shutdown: Removal of types failed!");
  }
}

void wdPhantomRttiManager::Startup()
{
  wdPlugin::Events().AddEventHandler(&wdPhantomRttiManager::PluginEventHandler);
}


void wdPhantomRttiManager::Shutdown()
{
  wdPlugin::Events().RemoveEventHandler(&wdPhantomRttiManager::PluginEventHandler);

  while (!s_NameToPhantom.IsEmpty())
  {
    UnregisterType(s_NameToPhantom.GetIterator().Value());
  }

  WD_ASSERT_DEV(s_NameToPhantom.IsEmpty(), "wdPhantomRttiManager::Shutdown: Removal of types failed!");
}
