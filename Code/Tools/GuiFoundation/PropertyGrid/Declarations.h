#pragma once

#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Foundation/Types/Variant.h>

class wdDocumentObject;

struct WD_GUIFOUNDATION_DLL wdPropertySelection
{
  const wdDocumentObject* m_pObject;
  wdVariant m_Index;

  bool operator==(const wdPropertySelection& rhs) const { return m_pObject == rhs.m_pObject && m_Index == rhs.m_Index; }

  bool operator<(const wdPropertySelection& rhs) const
  {
    // Qt6 requires the less than operator but never calls it, so we use this dummy for now.
    WD_ASSERT_NOT_IMPLEMENTED;
    return false;
  }
};

struct WD_GUIFOUNDATION_DLL wdPropertyClipboard
{
  wdString m_Type;
  wdVariant m_Value;
};
WD_DECLARE_REFLECTABLE_TYPE(WD_GUIFOUNDATION_DLL, wdPropertyClipboard)
