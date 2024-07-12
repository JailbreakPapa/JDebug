#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Reflection/Implementation/StaticRTTI.h>
#include <Foundation/Types/TypeTraits.h>

#define NS_DECLARE_VARIANCE_HASH_HELPER(TYPE)                        \
  template <>                                                        \
  struct nsHashHelper<TYPE>                                          \
  {                                                                  \
    NS_ALWAYS_INLINE static nsUInt32 Hash(const TYPE& value)         \
    {                                                                \
      return nsHashingUtils::xxHash32(&value, sizeof(TYPE));         \
    }                                                                \
    NS_ALWAYS_INLINE static bool Equal(const TYPE& a, const TYPE& b) \
    {                                                                \
      return a == b;                                                 \
    }                                                                \
  };

struct NS_FOUNDATION_DLL nsVarianceTypeBase
{
  NS_DECLARE_POD_TYPE();

  float m_fVariance = 0;
};

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsVarianceTypeBase);

struct NS_FOUNDATION_DLL nsVarianceTypeFloat : public nsVarianceTypeBase
{
  NS_DECLARE_POD_TYPE();
  bool operator==(const nsVarianceTypeFloat& rhs) const
  {
    return m_fVariance == rhs.m_fVariance && m_Value == rhs.m_Value;
  }
  bool operator!=(const nsVarianceTypeFloat& rhs) const
  {
    return !(*this == rhs);
  }
  float m_Value = 0;
};

NS_DECLARE_VARIANCE_HASH_HELPER(nsVarianceTypeFloat);
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsVarianceTypeFloat);
NS_DECLARE_CUSTOM_VARIANT_TYPE(nsVarianceTypeFloat);

struct NS_FOUNDATION_DLL nsVarianceTypeTime : public nsVarianceTypeBase
{
  NS_DECLARE_POD_TYPE();
  bool operator==(const nsVarianceTypeTime& rhs) const
  {
    return m_fVariance == rhs.m_fVariance && m_Value == rhs.m_Value;
  }
  bool operator!=(const nsVarianceTypeTime& rhs) const
  {
    return !(*this == rhs);
  }
  nsTime m_Value;
};

NS_DECLARE_VARIANCE_HASH_HELPER(nsVarianceTypeTime);
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsVarianceTypeTime);
NS_DECLARE_CUSTOM_VARIANT_TYPE(nsVarianceTypeTime);

struct NS_FOUNDATION_DLL nsVarianceTypeAngle : public nsVarianceTypeBase
{
  NS_DECLARE_POD_TYPE();
  bool operator==(const nsVarianceTypeAngle& rhs) const
  {
    return m_fVariance == rhs.m_fVariance && m_Value == rhs.m_Value;
  }
  bool operator!=(const nsVarianceTypeAngle& rhs) const
  {
    return !(*this == rhs);
  }
  nsAngle m_Value;
};

NS_DECLARE_VARIANCE_HASH_HELPER(nsVarianceTypeAngle);
NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsVarianceTypeAngle);
NS_DECLARE_CUSTOM_VARIANT_TYPE(nsVarianceTypeAngle);
