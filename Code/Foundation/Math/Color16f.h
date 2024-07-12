#pragma once

#include <Foundation/Math/Color.h>
#include <Foundation/Math/Float16.h>

/// \brief A 16bit per channel float color storage format.
///
/// For any calculations or conversions use nsColor.
/// \see nsColor
class NS_FOUNDATION_DLL nsColorLinear16f
{
public:
  // Means that colors can be copied using memcpy instead of copy construction.
  NS_DECLARE_POD_TYPE();

  // *** Data ***
public:
  nsFloat16 r;
  nsFloat16 g;
  nsFloat16 b;
  nsFloat16 a;

  // *** Constructors ***
public:
  /// \brief default-constructed color is uninitialized (for speed)
  nsColorLinear16f(); // [tested]

  /// \brief Initializes the color with r, g, b, a
  nsColorLinear16f(nsFloat16 r, nsFloat16 g, nsFloat16 b, nsFloat16 a); // [tested]

  /// \brief Initializes the color with nsColor
  nsColorLinear16f(const nsColor& color); // [tested]

  // no copy-constructor and operator= since the default-generated ones will be faster

  // *** Functions ***
public:
  /// \brief Conversion to nsColor.
  nsColor ToLinearFloat() const; // [tested]

  /// \brief Conversion to const nsFloat16*.
  const nsFloat16* GetData() const { return &r; }

  /// \brief Conversion to nsFloat16* - use with care!
  nsFloat16* GetData() { return &r; }
};

#include <Foundation/Math/Implementation/Color16f_inl.h>
