#pragma once

#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Foundation/Types/Variant.h>

class nsDocumentObject;

struct NS_GUIFOUNDATION_DLL nsPropertySelection
{
  const nsDocumentObject* m_pObject;
  nsVariant m_Index;

  bool operator==(const nsPropertySelection& rhs) const { return m_pObject == rhs.m_pObject && m_Index == rhs.m_Index; }

  bool operator<(const nsPropertySelection& rhs) const
  {
    // Qt6 requires the less than operator but never calls it, so we use this dummy for now.
    NS_ASSERT_NOT_IMPLEMENTED;
    return false;
  }
};

struct NS_GUIFOUNDATION_DLL nsPropertyClipboard
{
  nsString m_Type;
  nsVariant m_Value;
};
NS_DECLARE_REFLECTABLE_TYPE(NS_GUIFOUNDATION_DLL, nsPropertyClipboard)
