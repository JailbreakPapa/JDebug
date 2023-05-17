#pragma once

#include <Foundation/Math/Color.h>
#include <Foundation/Math/Float16.h>

/// \brief A 16bit per channel float color storage format.
///
/// For any calculations or conversions use wdColor.
/// \see wdColor
class WD_FOUNDATION_DLL wdColorLinear16f
{
public:
  // Means that colors can be copied using memcpy instead of copy construction.
  WD_DECLARE_POD_TYPE();

  // *** Data ***
public:
  wdFloat16 r;
  wdFloat16 g;
  wdFloat16 b;
  wdFloat16 a;

  // *** Constructors ***
public:
  /// \brief default-constructed color is uninitialized (for speed)
  wdColorLinear16f(); // [tested]

  /// \brief Initializes the color with r, g, b, a
  wdColorLinear16f(wdFloat16 r, wdFloat16 g, wdFloat16 b, wdFloat16 a); // [tested]

  /// \brief Initializes the color with wdColor
  wdColorLinear16f(const wdColor& color); // [tested]

  // no copy-constructor and operator= since the default-generated ones will be faster

  // *** Functions ***
public:
  /// \brief Conversion to wdColor.
  wdColor ToLinearFloat() const; // [tested]

  /// \brief Conversion to const wdFloat16*.
  const wdFloat16* GetData() const { return &r; }

  /// \brief Conversion to wdFloat16* - use with care!
  wdFloat16* GetData() { return &r; }
};

#include <Foundation/Math/Implementation/Color16f_inl.h>
