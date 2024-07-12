#include <Foundation/FoundationPCH.h>

#include <Foundation/Configuration/Singleton.h>

nsMap<size_t, nsSingletonRegistry::SingletonEntry> nsSingletonRegistry::s_Singletons;

const nsMap<size_t, nsSingletonRegistry::SingletonEntry>& nsSingletonRegistry::GetAllRegisteredSingletons()
{
  return s_Singletons;
}
