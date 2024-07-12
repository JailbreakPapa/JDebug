#pragma once

#include <Foundation/Basics.h>

#include <Foundation/Utilities/EnumerableClass.h>

class NS_FOUNDATION_DLL nsPlatformDesc : public nsEnumerable<nsPlatformDesc>
{
  NS_DECLARE_ENUMERABLE_CLASS(nsPlatformDesc);

public:
  nsPlatformDesc(const char* szName)
  {
    m_szName = szName;
  }

  const char* GetName() const
  {
    return m_szName;
  }

  static const nsPlatformDesc& GetThisPlatformDesc()
  {
    return *s_pThisPlatform;
  }

private:
  static const nsPlatformDesc* s_pThisPlatform;

  const char* m_szName;
};
