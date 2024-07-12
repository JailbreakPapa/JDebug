#include <Foundation/FoundationPCH.h>

#include <Foundation/Reflection/Implementation/RTTI.h>
#include <Foundation/Types/VariantTypeRegistry.h>

NS_IMPLEMENT_SINGLETON(nsVariantTypeRegistry);

// clang-format off
NS_BEGIN_SUBSYSTEM_DECLARATION(Foundation, VariantTypeRegistry)

  BEGIN_SUBSYSTEM_DEPENDENCIES
    "Reflection"
  END_SUBSYSTEM_DEPENDENCIES

  ON_CORESYSTEMS_STARTUP
  {
    NS_DEFAULT_NEW(nsVariantTypeRegistry);
  }

  ON_CORESYSTEMS_SHUTDOWN
  {
    nsVariantTypeRegistry * pDummy = nsVariantTypeRegistry::GetSingleton();
    NS_DEFAULT_DELETE(pDummy);
  }

NS_END_SUBSYSTEM_DECLARATION;
// clang-format on

nsVariantTypeRegistry::nsVariantTypeRegistry()
  : m_SingletonRegistrar(this)
{
  nsPlugin::Events().AddEventHandler(nsMakeDelegate(&nsVariantTypeRegistry::PluginEventHandler, this));

  UpdateTypes();
}

nsVariantTypeRegistry::~nsVariantTypeRegistry()
{
  nsPlugin::Events().RemoveEventHandler(nsMakeDelegate(&nsVariantTypeRegistry::PluginEventHandler, this));
}

const nsVariantTypeInfo* nsVariantTypeRegistry::FindVariantTypeInfo(const nsRTTI* pType) const
{
  const nsVariantTypeInfo* pTypeInfo = nullptr;
  m_TypeInfos.TryGetValue(pType, pTypeInfo);
  return pTypeInfo;
}

void nsVariantTypeRegistry::PluginEventHandler(const nsPluginEvent& EventData)
{
  switch (EventData.m_EventType)
  {
    case nsPluginEvent::AfterLoadingBeforeInit:
    case nsPluginEvent::AfterUnloading:
      UpdateTypes();
      break;
    default:
      break;
  }
}

void nsVariantTypeRegistry::UpdateTypes()
{
  m_TypeInfos.Clear();
  nsVariantTypeInfo* pInstance = nsVariantTypeInfo::GetFirstInstance();

  while (pInstance)
  {
    NS_ASSERT_DEV(pInstance->GetType()->GetAllocator()->CanAllocate(), "Custom type '{0}' needs to be allocatable.", pInstance->GetType()->GetTypeName());

    m_TypeInfos.Insert(pInstance->GetType(), pInstance);
    pInstance = pInstance->GetNextInstance();
  }
}

//////////////////////////////////////////////////////////////////////////

NS_ENUMERABLE_CLASS_IMPLEMENTATION(nsVariantTypeInfo);

nsVariantTypeInfo::nsVariantTypeInfo() = default;


NS_STATICLINK_FILE(Foundation, Foundation_Types_Implementation_VariantTypeRegistry);
