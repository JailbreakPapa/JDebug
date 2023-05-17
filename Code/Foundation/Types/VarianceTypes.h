#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Foundation/Types/TypeTraits.h>

#define WD_DECLARE_VARIANCE_HASH_HELPER(TYPE)                        \
  template <>                                                        \
  struct wdHashHelper<TYPE>                                          \
  {                                                                  \
    WD_ALWAYS_INLINE static wdUInt32 Hash(const TYPE& value)         \
    {                                                                \
      return wdHashingUtils::xxHash32(&value, sizeof(TYPE));         \
    }                                                                \
    WD_ALWAYS_INLINE static bool Equal(const TYPE& a, const TYPE& b) \
    {                                                                \
      return a == b;                                                 \
    }                                                                \
  };

struct WD_FOUNDATION_DLL wdVarianceTypeBase
{
  WD_DECLARE_POD_TYPE();

  float m_fVariance = 0;
};

WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdVarianceTypeBase);

struct WD_FOUNDATION_DLL wdVarianceTypeFloat : public wdVarianceTypeBase
{
  WD_DECLARE_POD_TYPE();
  bool operator==(const wdVarianceTypeFloat& rhs) const
  {
    return m_fVariance == rhs.m_fVariance && m_Value == rhs.m_Value;
  }
  bool operator!=(const wdVarianceTypeFloat& rhs) const
  {
    return !(*this == rhs);
  }
  float m_Value = 0;
};

WD_DECLARE_VARIANCE_HASH_HELPER(wdVarianceTypeFloat);
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdVarianceTypeFloat);
WD_DECLARE_CUSTOM_VARIANT_TYPE(wdVarianceTypeFloat);

struct WD_FOUNDATION_DLL wdVarianceTypeTime : public wdVarianceTypeBase
{
  WD_DECLARE_POD_TYPE();
  bool operator==(const wdVarianceTypeTime& rhs) const
  {
    return m_fVariance == rhs.m_fVariance && m_Value == rhs.m_Value;
  }
  bool operator!=(const wdVarianceTypeTime& rhs) const
  {
    return !(*this == rhs);
  }
  wdTime m_Value;
};

WD_DECLARE_VARIANCE_HASH_HELPER(wdVarianceTypeTime);
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdVarianceTypeTime);
WD_DECLARE_CUSTOM_VARIANT_TYPE(wdVarianceTypeTime);

struct WD_FOUNDATION_DLL wdVarianceTypeAngle : public wdVarianceTypeBase
{
  WD_DECLARE_POD_TYPE();
  bool operator==(const wdVarianceTypeAngle& rhs) const
  {
    return m_fVariance == rhs.m_fVariance && m_Value == rhs.m_Value;
  }
  bool operator!=(const wdVarianceTypeAngle& rhs) const
  {
    return !(*this == rhs);
  }
  wdAngle m_Value;
};

WD_DECLARE_VARIANCE_HASH_HELPER(wdVarianceTypeAngle);
WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdVarianceTypeAngle);
WD_DECLARE_CUSTOM_VARIANT_TYPE(wdVarianceTypeAngle);
