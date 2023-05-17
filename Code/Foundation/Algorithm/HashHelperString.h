
#pragma once

#include <Foundation/Basics.h>

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/StringUtils.h>

/// \brief Hash helper to be used as a template argument to wdHashTable / wdHashSet for case insensitive string keys.
struct WD_FOUNDATION_DLL wdHashHelperString_NoCase
{
  inline static wdUInt32 Hash(wdStringView sValue); // [tested]

  WD_ALWAYS_INLINE static bool Equal(wdStringView lhs, wdStringView rhs); // [tested]
};

#include <Foundation/Algorithm/Implementation/HashHelperString_inl.h>
