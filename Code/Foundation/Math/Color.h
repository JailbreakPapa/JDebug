#pragma once

#include <Foundation/Math/Math.h>
#include <Foundation/Math/Vec4.h>

/// \brief nsColor represents an RGBA color in linear color space. Values are stored as float, allowing HDR values and full precision color
/// modifications.
///
/// nsColor is the central class to handle colors throughout the engine. With floating point precision it can handle any value, including HDR colors.
/// Since it is stored in linear space, doing color transformations (e.g. adding colors or multiplying them) work as expected.
///
/// When you need to pass colors to the GPU you have multiple options.
///   * If you can spare the bandwidth, you should prefer to use floating point formats, e.g. the same as nsColor on the CPU.
///   * If you need higher precision and HDR values, you can use nsColorLinear16f as a storage format with only half the memory footprint.
///   * If you need to preserve memory and LDR values are sufficient, you should use nsColorGammaUB. This format uses 8 Bit per pixel
///     but stores colors in Gamma space, resulting in higher precision in the range that the human eye can distinguish better.
///     However, when you store a color in Gamma space, you need to make sure to convert it back to linear space before doing ANY computations
///     with it. E.g. your shader needs to convert the color.
///   * You can also use 8 Bit per pixel with a linear color space by using nsColorLinearUB, however this may give very noticeable precision loss.
///
/// When working with color in your code, be aware to always use the correct class to handle color conversions properly.
/// E.g. when you hardcode a color in source code, you might go to a Paint program, pick a nice color and then type that value into the
/// source code. Note that ALL colors that you see on screen are implicitly in sRGB / Gamma space. That means you should do the following cast:\n
///
/// \code
///   nsColor linear = nsColorGammaUB(100, 149, 237);
/// \endcode
///
/// This will automatically convert the color from Gamma to linear space. From there on all mathematical operations are possible.
///
/// The inverse has to be done when you want to present the value of a color in a UI:
///
/// \code
///   nsColorGammaUB gamma = nsColor(0.39f, 0.58f, 0.93f);
/// \endcode
///
/// Now the integer values in \a gamma can be used to e.g. populate a color picker and the color displayed on screen will show up the same, as
/// in a gamma correct 3D rendering.
///
///
///
/// The predefined colors can be seen at http://www.w3schools.com/colors/colors_names.asp
class NS_FOUNDATION_DLL nsColor
{
public:
  NS_DECLARE_POD_TYPE();

  // *** Predefined Colors ***
public:
  static const nsColor AliceBlue;            ///< #F0F8FF
  static const nsColor AntiqueWhite;         ///< #FAEBD7
  static const nsColor Aqua;                 ///< #00FFFF
  static const nsColor Aquamarine;           ///< #7FFFD4
  static const nsColor Azure;                ///< #F0FFFF
  static const nsColor Beige;                ///< #F5F5DC
  static const nsColor Bisque;               ///< #FFE4C4
  static const nsColor Black;                ///< #000000
  static const nsColor BlanchedAlmond;       ///< #FFEBCD
  static const nsColor Blue;                 ///< #0000FF
  static const nsColor BlueViolet;           ///< #8A2BE2
  static const nsColor Brown;                ///< #A52A2A
  static const nsColor BurlyWood;            ///< #DEB887
  static const nsColor CadetBlue;            ///< #5F9EA0
  static const nsColor Chartreuse;           ///< #7FFF00
  static const nsColor Chocolate;            ///< #D2691E
  static const nsColor Coral;                ///< #FF7F50
  static const nsColor CornflowerBlue;       ///< #6495ED  The original!
  static const nsColor Cornsilk;             ///< #FFF8DC
  static const nsColor Crimson;              ///< #DC143C
  static const nsColor Cyan;                 ///< #00FFFF
  static const nsColor DarkBlue;             ///< #00008B
  static const nsColor DarkCyan;             ///< #008B8B
  static const nsColor DarkGoldenRod;        ///< #B8860B
  static const nsColor DarkGray;             ///< #A9A9A9
  static const nsColor DarkGrey;             ///< #A9A9A9
  static const nsColor DarkGreen;            ///< #006400
  static const nsColor DarkKhaki;            ///< #BDB76B
  static const nsColor DarkMagenta;          ///< #8B008B
  static const nsColor DarkOliveGreen;       ///< #556B2F
  static const nsColor DarkOrange;           ///< #FF8C00
  static const nsColor DarkOrchid;           ///< #9932CC
  static const nsColor DarkRed;              ///< #8B0000
  static const nsColor DarkSalmon;           ///< #E9967A
  static const nsColor DarkSeaGreen;         ///< #8FBC8F
  static const nsColor DarkSlateBlue;        ///< #483D8B
  static const nsColor DarkSlateGray;        ///< #2F4F4F
  static const nsColor DarkSlateGrey;        ///< #2F4F4F
  static const nsColor DarkTurquoise;        ///< #00CED1
  static const nsColor DarkViolet;           ///< #9400D3
  static const nsColor DeepPink;             ///< #FF1493
  static const nsColor DeepSkyBlue;          ///< #00BFFF
  static const nsColor DimGray;              ///< #696969
  static const nsColor DimGrey;              ///< #696969
  static const nsColor DodgerBlue;           ///< #1E90FF
  static const nsColor FireBrick;            ///< #B22222
  static const nsColor FloralWhite;          ///< #FFFAF0
  static const nsColor ForestGreen;          ///< #228B22
  static const nsColor Fuchsia;              ///< #FF00FF
  static const nsColor Gainsboro;            ///< #DCDCDC
  static const nsColor GhostWhite;           ///< #F8F8FF
  static const nsColor Gold;                 ///< #FFD700
  static const nsColor GoldenRod;            ///< #DAA520
  static const nsColor Gray;                 ///< #808080
  static const nsColor Grey;                 ///< #808080
  static const nsColor Green;                ///< #008000
  static const nsColor GreenYellow;          ///< #ADFF2F
  static const nsColor HoneyDew;             ///< #F0FFF0
  static const nsColor HotPink;              ///< #FF69B4
  static const nsColor IndianRed;            ///< #CD5C5C
  static const nsColor Indigo;               ///< #4B0082
  static const nsColor Ivory;                ///< #FFFFF0
  static const nsColor Khaki;                ///< #F0E68C
  static const nsColor Lavender;             ///< #E6E6FA
  static const nsColor LavenderBlush;        ///< #FFF0F5
  static const nsColor LawnGreen;            ///< #7CFC00
  static const nsColor LemonChiffon;         ///< #FFFACD
  static const nsColor LightBlue;            ///< #ADD8E6
  static const nsColor LightCoral;           ///< #F08080
  static const nsColor LightCyan;            ///< #E0FFFF
  static const nsColor LightGoldenRodYellow; ///< #FAFAD2
  static const nsColor LightGray;            ///< #D3D3D3
  static const nsColor LightGrey;            ///< #D3D3D3
  static const nsColor LightGreen;           ///< #90EE90
  static const nsColor LightPink;            ///< #FFB6C1
  static const nsColor LightSalmon;          ///< #FFA07A
  static const nsColor LightSeaGreen;        ///< #20B2AA
  static const nsColor LightSkyBlue;         ///< #87CEFA
  static const nsColor LightSlateGray;       ///< #778899
  static const nsColor LightSlateGrey;       ///< #778899
  static const nsColor LightSteelBlue;       ///< #B0C4DE
  static const nsColor LightYellow;          ///< #FFFFE0
  static const nsColor Lime;                 ///< #00FF00
  static const nsColor LimeGreen;            ///< #32CD32
  static const nsColor Linen;                ///< #FAF0E6
  static const nsColor Magenta;              ///< #FF00FF
  static const nsColor Maroon;               ///< #800000
  static const nsColor MediumAquaMarine;     ///< #66CDAA
  static const nsColor MediumBlue;           ///< #0000CD
  static const nsColor MediumOrchid;         ///< #BA55D3
  static const nsColor MediumPurple;         ///< #9370DB
  static const nsColor MediumSeaGreen;       ///< #3CB371
  static const nsColor MediumSlateBlue;      ///< #7B68EE
  static const nsColor MediumSpringGreen;    ///< #00FA9A
  static const nsColor MediumTurquoise;      ///< #48D1CC
  static const nsColor MediumVioletRed;      ///< #C71585
  static const nsColor MidnightBlue;         ///< #191970
  static const nsColor MintCream;            ///< #F5FFFA
  static const nsColor MistyRose;            ///< #FFE4E1
  static const nsColor Moccasin;             ///< #FFE4B5
  static const nsColor NavajoWhite;          ///< #FFDEAD
  static const nsColor Navy;                 ///< #000080
  static const nsColor OldLace;              ///< #FDF5E6
  static const nsColor Olive;                ///< #808000
  static const nsColor OliveDrab;            ///< #6B8E23
  static const nsColor Orange;               ///< #FFA500
  static const nsColor OrangeRed;            ///< #FF4500
  static const nsColor Orchid;               ///< #DA70D6
  static const nsColor PaleGoldenRod;        ///< #EEE8AA
  static const nsColor PaleGreen;            ///< #98FB98
  static const nsColor PaleTurquoise;        ///< #AFEEEE
  static const nsColor PaleVioletRed;        ///< #DB7093
  static const nsColor PapayaWhip;           ///< #FFEFD5
  static const nsColor PeachPuff;            ///< #FFDAB9
  static const nsColor Peru;                 ///< #CD853F
  static const nsColor Pink;                 ///< #FFC0CB
  static const nsColor Plum;                 ///< #DDA0DD
  static const nsColor PowderBlue;           ///< #B0E0E6
  static const nsColor Purple;               ///< #800080
  static const nsColor RebeccaPurple;        ///< #663399
  static const nsColor Red;                  ///< #FF0000
  static const nsColor RosyBrown;            ///< #BC8F8F
  static const nsColor RoyalBlue;            ///< #4169E1
  static const nsColor SaddleBrown;          ///< #8B4513
  static const nsColor Salmon;               ///< #FA8072
  static const nsColor SandyBrown;           ///< #F4A460
  static const nsColor SeaGreen;             ///< #2E8B57
  static const nsColor SeaShell;             ///< #FFF5EE
  static const nsColor Sienna;               ///< #A0522D
  static const nsColor Silver;               ///< #C0C0C0
  static const nsColor SkyBlue;              ///< #87CEEB
  static const nsColor SlateBlue;            ///< #6A5ACD
  static const nsColor SlateGray;            ///< #708090
  static const nsColor SlateGrey;            ///< #708090
  static const nsColor Snow;                 ///< #FFFAFA
  static const nsColor SpringGreen;          ///< #00FF7F
  static const nsColor SteelBlue;            ///< #4682B4
  static const nsColor Tan;                  ///< #D2B48C
  static const nsColor Teal;                 ///< #008080
  static const nsColor Thistle;              ///< #D8BFD8
  static const nsColor Tomato;               ///< #FF6347
  static const nsColor Turquoise;            ///< #40E0D0
  static const nsColor Violet;               ///< #EE82EE
  static const nsColor Wheat;                ///< #F5DEB3
  static const nsColor White;                ///< #FFFFFF
  static const nsColor WhiteSmoke;           ///< #F5F5F5
  static const nsColor Yellow;               ///< #FFFF00
  static const nsColor YellowGreen;          ///< #9ACD32

  // *** Data ***
public:
  float r;
  float g;
  float b;
  float a;

  // *** Static Functions ***
public:
  /// \brief Returns a color with all four RGBA components set to Not-A-Number (NaN).
  [[nodiscard]] static nsColor MakeNaN();

  /// \brief Returns a color with all four RGBA components set to zero. This is different to nsColor::Black, which has alpha still set to 1.0.
  [[nodiscard]] static nsColor MakeZero();

  /// \brief Returns a color with the given r, g, b, a values. The values must be given in a linear color space.
  [[nodiscard]] static nsColor MakeRGBA(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha = 1.0f);

  // *** Constructors ***
public:
  /// \brief default-constructed color is uninitialized (for speed)
  nsColor(); // [tested]

  /// \brief Initializes the color with r, g, b, a. The color values must be given in a linear color space.
  ///
  /// To initialize the color from a Gamma color space, e.g. when using a color value that was determined with a color picker,
  /// use the constructor that takes a nsColorGammaUB object for initialization.
  constexpr nsColor(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha = 1.0f); // [tested]

  /// \brief Initializes this color from a nsColorLinearUB object.
  ///
  /// Prefer to either use linear colors with floating point precision, or to use nsColorGammaUB for 8 bit per pixel colors in gamma space.
  nsColor(const nsColorLinearUB& cc); // [tested]

  /// \brief Initializes this color from a nsColorGammaUB object.
  ///
  /// This should be the preferred method when hard-coding colors in source code.
  nsColor(const nsColorGammaUB& cc); // [tested]

#if NS_ENABLED(NS_MATH_CHECK_FOR_NAN)
  void AssertNotNaN() const
  {
    NS_ASSERT_ALWAYS(!IsNaN(), "This object contains NaN values. This can happen when you forgot to initialize it before using it. Please check that "
                               "all code-paths properly initialize this object.");
  }
#endif

  /// \brief Sets the RGB components, ignores alpha.
  void SetRGB(float fLinearRed, float fLinearGreen, float fLinearBlue); // [tested]

  /// \brief Sets all four RGBA components.
  void SetRGBA(float fLinearRed, float fLinearGreen, float fLinearBlue, float fLinearAlpha = 1.0f); // [tested]

  // *** Conversion Operators/Functions ***
public:
  /// \brief Returns a color created from the kelvin temperature. https://wikipedia.org/wiki/Color_temperature
  /// Originally inspired from https://tannerhelland.com/2012/09/18/convert-temperature-rgb-algorithm-code.html
  /// But with heavy modification to better fit the mapping shown out in https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf
  /// Physically accurate clipping points are 6580K for Red and 6560K for G and B. but approximated to 6570k for all to give a better mapping.
  static nsColor MakeFromKelvin(nsUInt32 uiKelvin);

  /// \brief Sets this color from a HSV (hue, saturation, value) format.
  ///
  /// \a hue is in range [0; 360], \a sat and \a val are in range [0; 1]
  [[nodiscard]] static nsColor MakeHSV(float fHue, float fSat, float fVal); // [tested]

  /// \brief Converts the color part to HSV format.
  ///
  /// \a hue is in range [0; 360], \a sat and \a val are in range [0; 1]
  void GetHSV(float& out_fHue, float& out_fSat, float& out_fValue) const; // [tested]

  /// \brief Conversion to const float*
  const float* GetData() const { return &r; }

  /// \brief Conversion to float*
  float* GetData() { return &r; }

  /// \brief Returns the 4 color values packed in an nsVec4
  const nsVec4 GetAsVec4() const;

  /// \brief Helper function to convert a float color value from gamma space to linear color space.
  static float GammaToLinear(float fGamma); // [tested]
  /// \brief Helper function to convert a float color value from linear space to gamma color space.
  static float LinearToGamma(float fGamma); // [tested]

  /// \brief Helper function to convert a float RGB color value from gamma space to linear color space.
  static nsVec3 GammaToLinear(const nsVec3& vGamma); // [tested]
  /// \brief Helper function to convert a float RGB color value from linear space to gamma color space.
  static nsVec3 LinearToGamma(const nsVec3& vGamma); // [tested]

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
  /// \see nsColor IsNormalized
  nsColor GetInvertedColor() const; // [tested]

  /// \brief Calculates the complementary color for this color (hue shifted by 180 degrees). The complementary color will have the same alpha.
  nsColor GetComplementaryColor() const; // [tested]

  /// \brief Multiplies the given factor into red, green and blue, but not alpha.
  void ScaleRGB(float fFactor);

  /// \brief Multiplies the given factor into red, green, blue and also alpha.
  void ScaleRGBA(float fFactor);

  /// \brief Returns 1 for an LDR color (all ´RGB components < 1). Otherwise the value of the largest component. Ignores alpha.
  float ComputeHdrMultiplier() const;

  /// \brief Returns the base-2 logarithm of ComputeHdrMultiplier().
  /// 0 for LDR colors, +1, +2, etc. for HDR colors.
  float ComputeHdrExposureValue() const;

  /// \brief Raises 2 to the power \a ev and multiplies RGB with that factor.
  void ApplyHdrExposureValue(float fEv);

  /// \brief If this is an HDR color, the largest component value is used to normalize RGB to LDR range. Alpha is unaffected.
  void NormalizeToLdrRange();

  /// \brief Returns a darker color by converting the color to HSV, dividing the *value* by fFactor and converting it back.
  nsColor GetDarker(float fFactor = 2.0f) const;

  // *** Numeric properties ***
public:
  /// \brief Returns true, if any of \a r, \a g, \a b or \a a is NaN.
  bool IsNaN() const; // [tested]

  /// \brief Checks that all components are finite numbers.
  bool IsValid() const; // [tested]

  // *** Operators ***
public:
  /// \brief Converts the color from nsColorLinearUB to linear float values.
  void operator=(const nsColorLinearUB& cc); // [tested]

  /// \brief Converts the color from nsColorGammaUB to linear float values. Gamma is correctly converted to linear space.
  void operator=(const nsColorGammaUB& cc); // [tested]

  /// \brief Adds \a rhs component-wise to this color.
  void operator+=(const nsColor& rhs); // [tested]

  /// \brief Subtracts \a rhs component-wise from this vector.
  void operator-=(const nsColor& rhs); // [tested]

  /// \brief Multiplies \a rhs component-wise with this color.
  void operator*=(const nsColor& rhs); // [tested]

  /// \brief Multiplies all components of this color with f.
  void operator*=(float f); // [tested]

  /// \brief Divides all components of this color by f.
  void operator/=(float f); // [tested]

  /// \brief Transforms the RGB components by the matrix. Alpha has no influence on the computation and will stay unmodified. The fourth row of the
  /// matrix is ignored.
  ///
  /// This operation can be used to do basic color correction.
  void operator*=(const nsMat4& rhs); // [tested]

  /// \brief Equality Check (bitwise). Only compares RGB, ignores Alpha.
  bool IsIdenticalRGB(const nsColor& rhs) const; // [tested]

  /// \brief Equality Check (bitwise). Compares all four components.
  bool IsIdenticalRGBA(const nsColor& rhs) const; // [tested]

  /// \brief Equality Check with epsilon. Only compares RGB, ignores Alpha.
  bool IsEqualRGB(const nsColor& rhs, float fEpsilon) const; // [tested]

  /// \brief Equality Check with epsilon. Compares all four components.
  bool IsEqualRGBA(const nsColor& rhs, float fEpsilon) const; // [tested]

  /// \brief Returns the current color but with changes the alpha value to the given value.
  nsColor WithAlpha(float fAlpha) const;

  /// \brief Packs the 4 color values as uint8 into a single uint32 with A in the least significant bits and R in the most significant ones.
  [[nodiscard]] nsUInt32 ToRGBA8() const;

  /// \brief Packs the 4 color values as uint8 into a single uint32 with R in the least significant bits and A in the most significant ones.
  [[nodiscard]] nsUInt32 ToABGR8() const;
};

// *** Operators ***

/// \brief Component-wise addition.
const nsColor operator+(const nsColor& c1, const nsColor& c2); // [tested]

/// \brief Component-wise subtraction.
const nsColor operator-(const nsColor& c1, const nsColor& c2); // [tested]

/// \brief Component-wise multiplication.
const nsColor operator*(const nsColor& c1, const nsColor& c2); // [tested]

/// \brief Returns a scaled color.
const nsColor operator*(float f, const nsColor& c); // [tested]

/// \brief Returns a scaled color. Will scale all components.
const nsColor operator*(const nsColor& c, float f); // [tested]

/// \brief Returns a scaled color. Will scale all components.
const nsColor operator/(const nsColor& c, float f); // [tested]

/// \brief Transforms the RGB components by the matrix. Alpha has no influence on the computation and will stay unmodified. The fourth row of the
/// matrix is ignored.
///
/// This operation can be used to do basic color correction.
const nsColor operator*(const nsMat4& lhs, const nsColor& rhs); // [tested]

/// \brief Returns true, if both colors are identical in all components.
bool operator==(const nsColor& c1, const nsColor& c2); // [tested]

/// \brief Returns true, if both colors are not identical in all components.
bool operator!=(const nsColor& c1, const nsColor& c2); // [tested]

/// \brief Strict weak ordering. Useful for sorting colors into a map.
bool operator<(const nsColor& c1, const nsColor& c2); // [tested]

NS_CHECK_AT_COMPILETIME(sizeof(nsColor) == 16);

#include <Foundation/Math/Implementation/Color_inl.h>
