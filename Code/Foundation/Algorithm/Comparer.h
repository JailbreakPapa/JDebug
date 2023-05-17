
#pragma once

/// \brief A comparer object is used in sorting algorithms to compare to objects of the same type.
template <typename T>
struct wdCompareHelper
{
  /// \brief Returns true if a is less than b
  WD_ALWAYS_INLINE bool Less(const T& a, const T& b) const
  {
    return a < b;
  }

  /// \brief Returns true if a is less than b
  template <typename U>
  WD_ALWAYS_INLINE bool Less(const T& a, const U& b) const
  {
    return a < b;
  }

  /// \brief Returns true if a is less than b
  template <typename U>
  WD_ALWAYS_INLINE bool Less(const U& a, const T& b) const
  {
    return a < b;
  }

  /// \brief Returns true if a is equal to b
  WD_ALWAYS_INLINE bool Equal(const T& a, const T& b) const
  {
    return a == b;
  }

  /// \brief Returns true if a is equal to b
  template <typename U>
  WD_ALWAYS_INLINE bool Equal(const T& a, const U& b) const
  {
    return a == b;
  }

  /// \brief Returns true if a is equal to b
  template <typename U>
  WD_ALWAYS_INLINE bool Equal(const U& a, const T& b) const
  {
    return a == b;
  }
};

// See <Foundation/Strings/String.h> for wdString specialization and case insensitive version.
