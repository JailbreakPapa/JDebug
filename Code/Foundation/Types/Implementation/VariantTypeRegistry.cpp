#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/Implementation/RTTI.h>
#include <Foundation/Types/VariantTypeRegistry.h>

WD_IMPLEMENT_SINGLETON(wdVariantTypeRegistry);

// clang-format off
WD_BEGIN_SUBSYSTEM_DECLARATION(Foundation, VariantTypeRegistry)

BEGIN_SUBSYSTEM_DEPENDENCIES
"Reflection"
END_SUBSYSTEM_DEPENDENCIES

ON_CORESYSTEMS_STARTUP
{
  WD_DEFAULT_NEW(wdVariantTypeRegistry);
}

ON_CORESYSTEMS_SHUTDOWN
{
  wdVariantTypeRegistry * pDummy = wdVariantTypeRegistry::GetSingleton();
  WD_DEFAULT_DELETE(pDummy);
}

WD_END_SUBSYSTEM_DECLARATION;
// clang-format on

wdVariantTypeRegistry::wdVariantTypeRegistry()
  : m_SingletonRegistrar(this)
{
  wdPlugin::Events().AddEventHandler(wdMakeDelegate(&wdVariantTypeRegistry::PluginEventHandler, this));

  UpdateTypes();
}

wdVariantTypeRegistry::~wdVariantTypeRegistry()
{
  wdPlugin::Events().RemoveEventHandler(wdMakeDelegate(&wdVariantTypeRegistry::PluginEventHandler, this));
}

const wdVariantTypeInfo* wdVariantTypeRegistry::FindVariantTypeInfo(const wdRTTI* pType) const
{
  const wdVariantTypeInfo* pTypeInfo = nullptr;
  m_TypeInfos.TryGetValue(pType, pTypeInfo);
  return pTypeInfo;
}

void wdVariantTypeRegistry::PluginEventHandler(const wdPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case wdPluginEvent::AfterLoadingBeforeInit:
    case wdPluginEvent::AfterUnloading:
      UpdateTypes();
      break;
    default:
      break;
  }
}

void wdVariantTypeRegistry::UpdateTypes()
{
  m_TypeInfos.Clear();
  wdVariantTypeInfo* pInstance = wdVariantTypeInfo::GetFirstInstance();

  while (pInstance)
  {
    WD_ASSERT_DEV(pInstance->GetType()->GetAllocator()->CanAllocate(), "Custom type '{0}' needs to be allocatable.", pInstance->GetType()->GetTypeName());

    m_TypeInfos.Insert(pInstance->GetType(), pInstance);
    pInstance = pInstance->GetNextInstance();
  }
}

//////////////////////////////////////////////////////////////////////////

WD_ENUMERABLE_CLASS_IMPLEMENTATION(wdVariantTypeInfo);

wdVariantTypeInfo::wdVariantTypeInfo()
{
}


WD_STATICLINK_FILE(Foundation, Foundation_Types_Implementation_VariantTypeRegistry);
