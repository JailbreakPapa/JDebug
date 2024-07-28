#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Strings/String.h>

class NS_CORE_DLL nsWorldModuleConfig
{
public:
  nsResult Save();
  void Load();
  void Apply();

  void AddInterfaceImplementation(nsStringView sInterfaceName, nsStringView sImplementationName);
  void RemoveInterfaceImplementation(nsStringView sInterfaceName);

  struct InterfaceImpl
  {
    nsString m_sInterfaceName;
    nsString m_sImplementationName;

    bool operator<(const InterfaceImpl& rhs) const { return m_sInterfaceName < rhs.m_sInterfaceName; }
  };

  nsHybridArray<InterfaceImpl, 8> m_InterfaceImpls;
};
