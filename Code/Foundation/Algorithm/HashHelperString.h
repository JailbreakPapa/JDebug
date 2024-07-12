
#pragma once

#include <Foundation/Basics.h>

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/StringUtils.h>

/// \brief Hash helper to be used as a template argument to nsHashTable / nsHashSet for case insensitive string keys.
struct NS_FOUNDATION_DLL nsHashHelperString_NoCase
{
  inline static nsUInt32 Hash(nsStringView sValue);                       // [tested]

  NS_ALWAYS_INLINE static bool Equal(nsStringView lhs, nsStringView rhs); // [tested]
};

#include <Foundation/Algorithm/Implementation/HashHelperString_inl.h>
