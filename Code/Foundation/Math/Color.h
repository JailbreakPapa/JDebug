#pragma once

#include <Foundation/Math/Math.h>
#include <Foundation/Math/Vec4.h>

/// \brief wdColor represents and RGBA color in linear color space. Values are stored as float, allowing HDR values and full precision color
/// modifications.
///
/// wdColor is the central class to handle colors throughout the engine. With floating point precision it can handle any value, including HDR colors.
/// Since it is stored in linear space, doing color transformations (e.g. adding colors or multiplying them) work as expected.
///
/// When you need to pass colors to the GPU you have multiple options.
///   * If you can spare the bandwidth, you should prefer to use floating point formats, e.g. the same as wdColor on the CPU.
///   * If you need higher precision and HDR values, you can use wdColorLinear16f as a storage format with only half the memory footprint.
///   * If you need to use preserve memory and LDR values are sufficient, you should use wdColorGammaUB. This format uses 8 Bit per pixel
///     but stores colors in Gamma space, resulting in higher precision in the range that the human eye can distinguish better.
///     However, when you store a color in Gamma space, you need to make sure to convert it back to linear space before doing ANY computations
///     with it. E.g. your shader needs to convert the color.
///   * You can also use 8 Bit per pixel with a linear color space by using wdColorLinearUB, however this may give very noticeable precision loss.
///
/// When working with color in your code, be aware to always use the correct class to handle color conversions properly.
/// E.g. when you hardcode a color in source code, you might go to a Paint program, pick a nice color and then type that value into the
/// source code. Note that ALL colors that you see on screen are implicitly in sRGB / Gamma space. That means you should do the following cast:\n
///
/// \code
///   wdColor linear = wdColorGammaUB(100, 149, 237);
/// \endcode
///
/// This will automatically convert the color from Gamma to linear space. From there on all mathematical operations are possible.
///
/// The inverse has to be done when you want to present the value of a color in a UI:
///
/// \code
///   wdColorGammaUB gamma = wdColor(0.39f, 0.58f, 0.93f);
/// \endcode
///
/// Now the integer values in \a gamma can be used to e.g. populate a color picker and the color displayed on screen will show up the same, as
/// in a gamma correct 3D rendering.
///
///
///
/// The predefined colors can be seen at http://www.w3schools.com/colors/colors_names.asp
class WD_FOUNDATION_DLL wdColor
{
public:
  WD_DECLARE_POD_TYPE();

  // *** Predefined Colors ***
public:
  static const wdColor AliceBlue;            ///< #F0F8FF
  static const wdColor AntiqueWhite;         ///< #FAEBD7
  static const wdColor Aqua;                 ///< #00FFFF
  static const wdColor Aquamarine;           ///< #7FFFD4
  static const wdColor Azure;                ///< #F0FFFF
  static const wdColor Beige;                ///< #F5F5DC
  static const wdColor Bisque;               ///< #FFE4C4
  static const wdColor Black;                ///< #000000
  static const wdColor BlanchedAlmond;       ///< #FFEBCD
  static const wdColor Blue;                 ///< #0000FF
  static const wdColor BlueViolet;           ///< #8A2BE2
  static const wdColor Brown;                ///< #A52A2A
  static const wdColor BurlyWood;            ///< #DEB887
  static const wdColor CadetBlue;            ///< #5F9EA0
  static const wdColor Chartreuse;           ///< #7FFF00
  static const wdColor Chocolate;            ///< #D2691E
  static const wdColor Coral;                ///< #FF7F50
  static const wdColor CornflowerBlue;       ///< #6495ED  The original!
  static const wdColor Cornsilk;             ///< #FFF8DC
  static const wdColor Crimson;              ///< #DC143C
  static const wdColor Cyan;                 ///< #00FFFF
  static const wdColor DarkBlue;             ///< #00008B
  static const wdColor DarkCyan;             ///< #008B8B
  static const wdColor DarkGoldenRod;        ///< #B8860B
  static const wdColor DarkGray;             ///< #A9A9A9
  static const wdColor DarkGrey;             ///< #A9A9A9
  static const wdColor DarkGreen;            ///< #006400
  static const wdColor DarkKhaki;            ///< #BDB76B
  static const wdColor DarkMagenta;          ///< #8B008B
  static const wdColor DarkOliveGreen;       ///< #556B2F
  static const wdColor DarkOrange;           ///< #FF8C00
  static const wdColor DarkOrchid;           ///< #9932CC
  static const wdColor DarkRed;              ///< #8B0000
  static const wdColor DarkSalmon;           ///< #E9967A
  static const wdColor DarkSeaGreen;         ///< #8FBC8F
  static const wdColor DarkSlateBlue;        ///< #483D8B
  static const wdColor DarkSlateGray;        ///< #2F4F4F
  static const wdColor DarkSlateGrey;        ///< #2F4F4F
  static const wdColor DarkTurquoise;        ///< #00CED1
  static const wdColor DarkViolet;           ///< #9400D3
  static const wdColor DeepPink;             ///< #FF1493
  static const wdColor DeepSkyBlue;          ///< #00BFFF
  static const wdColor DimGray;              ///< #696969
  static const wdColor DimGrey;              ///< #696969
  static const wdColor DodgerBlue;           ///< #1E90FF
  static const wdColor FireBrick;            ///< #B22222
  static const wdColor FloralWhite;          ///< #FFFAF0
  static const wdColor ForestGreen;          ///< #228B22
  static const wdColor Fuchsia;              ///< #FF00FF
  static const wdColor Gainsboro;            ///< #DCDCDC
  static const wdColor GhostWhite;           ///< #F8F8FF
  static const wdColor Gold;                 ///< #FFD700
  static const wdColor GoldenRod;            ///< #DAA520
  static const wdColor Gray;                 ///< #808080
  static const wdColor Grey;                 ///< #808080
  static const wdColor Green;                ///< #008000
  static const wdColor GreenYellow;          ///< #ADFF2F
  static const wdColor HoneyDew;             ///< #F0FFF0
  static const wdColor HotPink;              ///< #FF69B4
  static const wdColor IndianRed;            ///< #CD5C5C
  static const wdColor Indigo;               ///< #4B0082
  static const wdColor Ivory;                ///< #FFFFF0
  static const wdColor Khaki;                ///< #F0E68C
  static const wdColor Lavender;             ///< #E6E6FA
  static const wdColor LavenderBlush;        ///< #FFF0F5
  static const wdColor LawnGreen;            ///< #7CFC00
  static const wdColor LemonChiffon;         ///< #FFFACD
  static const wdColor LightBlue;            ///< #ADD8E6
  static const wdColor LightCoral;           ///< #F08080
  static const wdColor LightCyan;            ///< #E0FFFF
  static const wdColor LightGoldenRodYellow; ///< #FAFAD2
  static const wdColor LightGray;            ///< #D3D3D3
  static const wdColor LightGrey;            ///< #D3D3D3
  static const wdColor LightGreen;           ///< #90EE90
  static const wdColor LightPink;            ///< #FFB6C1
  static const wdColor LightSalmon;          ///< #FFA07A
  static const wdColor LightSeaGreen;        ///< #20B2AA
  static const wdColor LightSkyBlue;         ///< #87CEFA
  static const wdColor LightSlateGray;       ///< #778899
  static const wdColor LightSlateGrey;       ///< #778899
  static const wdColor LightSteelBlue;       ///< #B0C4DE
  static const wdColor LightYellow;          ///< #FFFFE0
  static const wdColor Lime;                 ///< #00FF00
  static const wdColor LimeGreen;            ///< #32CD32
  static const wdColor Linen;                ///< #FAF0E6
  static const wdColor Magenta;              ///< #FF00FF
  static const wdColor Maroon;               ///< #800000
  static const wdColor MediumAquaMarine;     ///< #66CDAA
  static const wdColor MediumBlue;           ///< #0000CD
  static const wdColor MediumOrchid;         ///< #BA55D3
  static const wdColor MediumPurple;         ///< #9370DB
  static const wdColor MediumSeaGreen;       ///< #3CB371
  static const wdColor MediumSlateBlue;      ///< #7B68EE
  static const wdColor MediumSpringGreen;    ///< #00FA9A
  static const wdColor MediumTurquoise;      ///< #48D1CC
  static const wdColor MediumVioletRed;      ///< #C71585
  static const wdColor MidnightBlue;         ///< #191970
  static const wdColor MintCream;            ///< #F5FFFA
  static const wdColor MistyRose;            ///< #FFE4E1
  static const wdColor Moccasin;             ///< #FFE4B5
  static const wdColor NavajoWhite;          ///< #FFDEAD
  static const wdColor Navy;                 ///< #000080
  static const wdColor OldLace;              ///< #FDF5E6
  static const wdColor Olive;                ///< #808000
  static const wdColor OliveDrab;            ///< #6B8E23
  static const wdColor Orange;               ///< #FFA500
  static const wdColor OrangeRed;            ///< #FF4500
  static const wdColor Orchid;               ///< #DA70D6
  static const wdColor PaleGoldenRod;        ///< #EEE8AA
  static const wdColor PaleGreen;            ///< #98FB98
  static const wdColor PaleTurquoise;        ///< #AFEEEE
  static const wdColor PaleVioletRed;        ///< #DB7093
  static const wdColor PapayaWhip;           ///< #FFEFD5
  static const wdColor PeachPuff;            ///< #FFDAB9
  static const wdColor Peru;                 ///< #CD853F
  static const wdColor Pink;                 ///< #FFC0CB
  static const wdColor Plum;                 ///< #DDA0DD
  static const wdColor PowderBlue;           ///< #B0E0E6
  static const wdColor Purple;               ///< #800080
  static const wdColor RebeccaPurple;        ///< #663399
  static const wdColor Red;                  ///< #FF0000
  static const wdColor RosyBrown;            ///< #BC8F8F
  static const wdColor RoyalBlue;            ///< #4169E1
  static const wdColor SaddleBrown;          ///< #8B4513
  static const wdColor Salmon;               ///< #FA8072
  static const wdColor SandyBrown;           ///< #F4A460
  static const wdColor SeaGreen;             ///< #2E8B57
  static const wdColor SeaShell;             ///< #FFF5EE
  static const wdColor Sienna;               ///< #A0522D
  static const wdColor Silver;               ///< #C0C0C0
  static const wdColor SkyBlue;              ///< #87CEEB
  static const wdColor SlateBlue;            ///< #6A5ACD
  static const wdColor SlateGray;            ///< #708090
  static const wdColor SlateGrey;            ///< #708090
  static const wdColor Snow;                 ///< #FFFAFA
  static const wdColor SpringGreen;          ///< #00FF7F
  static const wdColor SteelBlue;            ///< #4682B4
  static const wdColor Tan;                  ///< #D2B48C
  static const wdColor Teal;                 ///< #008080
  static const wdColor Thistle;              ///< #D8BFD8
  static const wdColor Tomato;               ///< #FF6347
  static const wdColor Turquoise;            ///< #40E0D0
  static const wdColor Violet;               ///< #EE82EE
  static const wdColor Wheat;                ///< #F5DEB3
  static const wdColor White;                ///< #FFFFFF
  static const wdColor WhiteSmoke;           ///< #F5F5F5
  static const wdColor Yellow;               ///< #FFFF00
  static const wdColor YellowGreen;          ///< #9ACD32

  // *** Data ***
public:
  float r;
  float g;
  float b;
  float a;

  // *** Static Functions ***
public:
  /// \brief Returns a color with all four RGBA components set to zero. This is different to wdColor::Black, which has alpha still set to 1.0.
  static wdColor ZeroColor();

  // *** Constructors ***
public:
  /// \brief default-constructed color is uninitialized (for speed)
  wdColor(); // [tested]

  /// \brief Initializes the color with r, g, b, a. The color values must be given in a linear color space.
  ///
  /// To initialize the color from a Gamma color space, e.g. when using a color value that was determined with a color picker,
  /// use the constructor that takes a wdColorGammaUB object for initialization.
  constexpr wdColor(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha = 1.0f); // [tested]

  /// \brief Initializes this color from a wdColorLinearUB object.
  ///
  /// Prefer to either use linear colors with floating point precision, or to use wdColorGammaUB for 8 bit per pixel colors in gamma space.
  wdColor(const wdColorLinearUB& cc); // [tested]

  /// \brief Initializes this color from a wdColorGammaUB object.
  ///
  /// This should be the preferred method when hardcoding colors in source code.
  wdColor(const wdColorGammaUB& cc); // [tested]

#if WD_ENABLED(WD_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    WD_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please check that "
                               "all code-paths properly initialize this object.");
  }
#endif

  /// \brief Sets the RGB components, ignores alpha.
  void SetRGB(float fLinearRed, float fLinearGreen, float fLinearBlue); // [tested]

  /// \brief Sets all four RGBA components.
  void SetRGBA(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha = 1.0f); // [tested]

  /// \brief Sets all four RGBA components to zero.
  void SetZero();

  // *** Conversion Operators/Functions ***
public:
  /// \brief Sets this color from a HSV (hue, saturation, value) format.
  ///
  /// \a hue is in range [0; 360], \a sat and \a val are in range [0; 1]
  void SetHSV(float fHue, float fSat, float fVal); // [tested]

  /// \brief Converts the color part to HSV format.
  ///
  /// \a hue is in range [0; 360], \a sat and \a val are in range [0; 1]
  void GetHSV(float& ref_fHue, float& ref_fSat, float& ref_fVal) const; // [tested]

  /// \brief Conversion to const float*
  const float* GetData() const { return &r; }

  /// \brief Conversion to float*
  float* GetData() { return &r; }

  /// \brief Returns the 4 color values packed in an wdVec4
  const wdVec4 GetAsVec4() const;

  /// \brief Helper function to convert a float color value from gamma space to linear color space.
  static float GammaToLinear(float fGamma); // [tested]
  /// \brief Helper function to convert a float color value from linear space to gamma color space.
  static float LinearToGamma(float fGamma); // [tested]

  /// \brief Helper function to convert a float RGB color value from gamma space to linear color space.
  static wdVec3 GammaToLinear(const wdVec3& vGamma); // [tested]
  /// \brief Helper function to convert a float RGB color value from linear space to gamma color space.
  static wdVec3 LinearToGamma(const wdVec3& vGamma); // [tested]

  // *** Color specific functions ***
public:
  /// \brief Returns if the color is in the Range [0; 1] on all 4 channels.
  bool IsNormalized() const; // [tested]

  /// \brief Calculates the average of the RGB channels.
  float CalcAverageRGB() const;

  /// \brief Computes saturation.
  float GetSaturation() const; // [tested]

  /// \brief Computes the perceived luminance. Assumes linear color space (http://en.wikipedia.org/wiki/Luminance_%28relative%29).
  float GetLuminance() const; /// [tested]

  /// \brief Performs a simple (1.0 - color) inversion on all four channels.
  ///
  /// Using this function on non-normalized colors will lead to negative results.
  /// \see wdColor IsNormalized
  wdColor GetInvertedColor() const; // [tested]

  /// \brief Calculates the complementary color for this color (hue shifted by 180 degrees). The complementary color will have the same alpha.
  wdColor GetComplementaryColor() const; // [tested]

  /// \brief Multiplies the given factor into red, green and blue, but not alpha.
  void ScaleRGB(float fFactor);

  /// \brief Returns 1 for an LDR color (all ´RGB components < 1). Otherwise the value of the largest component. Ignores alpha.
  float ComputeHdrMultiplier() const;

  /// \brief Returns the base-2 logarithm of ComputeHdrMultiplier().
  /// 0 for LDR colors, +1, +2, etc. for HDR colors.
  float ComputeHdrExposureValue() const;

  /// \brief Raises 2 to the power \a ev and multiplies RGB with that factor.
  void ApplyHdrExposureValue(float fEv);

  /// \brief If this is an HDR color, the largest component value is used to normalize RGB to LDR range. Alpha is unaffected.
  void NormalizeToLdrRange();

  // *** Numeric properties ***
public:
  /// \brief Returns true, if any of \a r, \a g, \a b or \a a is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Checks that all components are finite numbers.
  bool IsValid() const; // [tested]

  // *** Operators ***
public:
  /// \brief Converts the color from wdColorLinearUB to linear float values.
  void operator=(const wdColorLinearUB& cc); // [tested]

  /// \brief Converts the color from wdColorGammaUB to linear float values. Gamma is correctly converted to linear space.
  void operator=(const wdColorGammaUB& cc); // [tested]

  /// \brief Adds \a rhs component-wise to this color.
  void operator+=(const wdColor& rhs); // [tested]

  /// \brief Subtracts \a rhs component-wise from this vector.
  void operator-=(const wdColor& rhs); // [tested]

  /// \brief Multiplies \a rhs component-wise with this color.
  void operator*=(const wdColor& rhs); // [tested]

  /// \brief Multiplies all components of this color with f.
  void operator*=(float f); // [tested]

  /// \brief Divides all components of this color by f.
  void operator/=(float f); // [tested]

  /// \brief Transforms the RGB components by the matrix. Alpha has no influence on the computation and will stay unmodified. The fourth row of the
  /// matrix is ignored.
  ///
  /// This operation can be used to do basic color correction.
  void operator*=(const wdMat4& rhs); // [tested]

  /// \brief Equality Check (bitwise). Only compares RGB, ignores Alpha.
  bool IsIdenticalRGB(const wdColor& rhs) const; // [tested]

  /// \brief Equality Check (bitwise). Compares all four components.
  bool IsIdenticalRGBA(const wdColor& rhs) const; // [tested]

  /// \brief Equality Check with epsilon. Only compares RGB, ignores Alpha.
  bool IsEqualRGB(const wdColor& rhs, float fEpsilon) const; // [tested]

  /// \brief Equality Check with epsilon. Compares all four components.
  bool IsEqualRGBA(const wdColor& rhs, float fEpsilon) const; // [tested]

  /// \brief Returns the current color but with changes the alpha value to the given value.
  wdColor WithAlpha(float fAlpha) const;
};

// *** Operators ***

/// \brief Component-wise addition.
const wdColor operator+(const wdColor& c1, const wdColor& c2); // [tested]

/// \brief Component-wise subtraction.
const wdColor operator-(const wdColor& c1, const wdColor& c2); // [tested]

/// \brief Component-wise multiplication.
const wdColor operator*(const wdColor& c1, const wdColor& c2); // [tested]

/// \brief Returns a scaled color.
const wdColor operator*(float f, const wdColor& c); // [tested]

/// \brief Returns a scaled color. Will scale all components.
const wdColor operator*(const wdColor& c, float f); // [tested]

/// \brief Returns a scaled color. Will scale all components.
const wdColor operator/(const wdColor& c, float f); // [tested]

/// \brief Transforms the RGB components by the matrix. Alpha has no influence on the computation and will stay unmodified. The fourth row of the
/// matrix is ignored.
///
/// This operation can be used to do basic color correction.
const wdColor operator*(const wdMat4& lhs, const wdColor& rhs); // [tested]

/// \brief Returns true, if both colors are identical in all components.
bool operator==(const wdColor& c1, const wdColor& c2); // [tested]

/// \brief Returns true, if both colors are not identical in all components.
bool operator!=(const wdColor& c1, const wdColor& c2); // [tested]

/// \brief Strict weak ordering. Useful for sorting colors into a map.
bool operator<(const wdColor& c1, const wdColor& c2); // [tested]

WD_CHECK_AT_COMPILETIME(sizeof(wdColor) == 16);

#include <Foundation/Math/Implementation/Color_inl.h>
