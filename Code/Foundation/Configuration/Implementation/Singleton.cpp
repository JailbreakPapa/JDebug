#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Singleton.h>

wdMap<size_t, wdSingletonRegistry::SingletonEntry> wdSingletonRegistry::s_Singletons;

const wdMap<size_t, wdSingletonRegistry::SingletonEntry>& wdSingletonRegistry::GetAllRegisteredSingletons()
{
  return s_Singletons;
}

WD_STATICLINK_FILE(Foundation, Foundation_Configuration_Implementation_Singleton);
