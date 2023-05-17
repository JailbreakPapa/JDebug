#pragma once

#include <Foundation/Math/Color.h>
#include <Foundation/Math/Math.h>

/// \brief A 8bit per channel color storage format with undefined encoding. It is up to the user to reinterpret as a gamma or linear space
/// color.
///
/// \see wdColorLinearUB
/// \see wdColorGammaUB
class WD_FOUNDATION_DLL wdColorBaseUB
{
public:
  WD_DECLARE_POD_TYPE();

  wdUInt8 r;
  wdUInt8 g;
  wdUInt8 b;
  wdUInt8 a;

  /// \brief Default-constructed color is uninitialized (for speed)
  wdColorBaseUB() = default;

  /// \brief Initializes the color with r, g, b, a
  wdColorBaseUB(wdUInt8 r, wdUInt8 g, wdUInt8 b, wdUInt8 a = 255);

  /// \brief Conversion to const wdUInt8*.
  const wdUInt8* GetData() const { return &r; }

  /// \brief Conversion to wdUInt8*
  wdUInt8* GetData() { return &r; }
};

WD_CHECK_AT_COMPILETIME(sizeof(wdColorBaseUB) == 4);

/// \brief A 8bit per channel unsigned normalized (values interpreted as 0-1) color storage format that represents colors in linear space.
///
/// For any calculations or conversions use wdColor.
/// \see wdColor
class WD_FOUNDATION_DLL wdColorLinearUB : public wdColorBaseUB
{
public:
  WD_DECLARE_POD_TYPE();

  /// \brief Default-constructed color is uninitialized (for speed)
  wdColorLinearUB() = default; // [tested]

  /// \brief Initializes the color with r, g, b, a
  wdColorLinearUB(wdUInt8 r, wdUInt8 g, wdUInt8 b, wdUInt8 a = 255); // [tested]

  /// \brief Initializes the color with wdColor.
  /// Assumes that the given color is normalized.
  /// \see wdColor::IsNormalized
  wdColorLinearUB(const wdColor& color); // [tested]

  /// \brief Initializes the color with wdColor.
  void operator=(const wdColor& color); // [tested]

  /// \brief Converts this color to wdColor.
  wdColor ToLinearFloat() const; // [tested]
};

WD_CHECK_AT_COMPILETIME(sizeof(wdColorLinearUB) == 4);

/// \brief A 8bit per channel unsigned normalized (values interpreted as 0-1) color storage format that represents colors in gamma space.
///
/// For any calculations or conversions use wdColor.
/// \see wdColor
class WD_FOUNDATION_DLL wdColorGammaUB : public wdColorBaseUB
{
public:
  WD_DECLARE_POD_TYPE();

  /// \brief Default-constructed color is uninitialized (for speed)
  wdColorGammaUB() = default;

  /// \brief Copies the color values. RGB are assumed to be in Gamma space.
  wdColorGammaUB(wdUInt8 uiGammaRed, wdUInt8 uiGammaGreen, wdUInt8 uiGammaBlue, wdUInt8 uiLinearAlpha = 255); // [tested]

  /// \brief Initializes the color with wdColor. Converts the linear space color to gamma space.
  /// Assumes that the given color is normalized.
  /// \see wdColor::IsNormalized
  wdColorGammaUB(const wdColor& color); // [tested]

  /// \brief Initializes the color with wdColor. Converts the linear space color to gamma space.
  void operator=(const wdColor& color); // [tested]

  /// \brief Converts this color to wdColor.
  wdColor ToLinearFloat() const;
};

WD_CHECK_AT_COMPILETIME(sizeof(wdColorGammaUB) == 4);


#include <Foundation/Math/Implementation/Color8UNorm_inl.h>
