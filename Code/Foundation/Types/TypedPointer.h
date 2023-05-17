#pragma once

#include <Foundation/Types/TypeTraits.h>

class wdRTTI;

/// \brief A typed raw pointer.
///
/// Common use case is the storage of object pointers inside an wdVariant.
/// Has the same lifetime concerns that any other raw pointer.
/// \sa wdVariant
struct wdTypedPointer
{
  WD_DECLARE_POD_TYPE();
  void* m_pObject = nullptr;
  const wdRTTI* m_pType = nullptr;

  wdTypedPointer() = default;
  wdTypedPointer(void* pObject, const wdRTTI* pType)
    : m_pObject(pObject)
    , m_pType(pType)
  {
  }

  bool operator==(const wdTypedPointer& rhs) const
  {
    return m_pObject == rhs.m_pObject;
  }
  bool operator!=(const wdTypedPointer& rhs) const
  {
    return m_pObject != rhs.m_pObject;
  }
};
