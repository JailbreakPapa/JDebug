#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Math/Angle.h>
#include <Foundation/Math/Constants.h>
#include <Foundation/Math/Declarations.h>


/// \brief This namespace provides common math-functionality as functions.
///
/// It is a namespace, instead of a static class, because that allows it to be extended
/// at other locations, which is especially useful when adding custom types.
namespace nsMath
{
  /// \brief Returns whether the given value is NaN under this type.
  template <typename Type>
  constexpr static bool IsNaN(Type value)
  {
    return false;
  }

  /// \brief Returns whether the given value represents a finite value (i.e. not +/- Infinity and not NaN)
  template <typename Type>
  constexpr static bool IsFinite(Type value)
  {
    return true;
  }

  /// ***** Trigonometric Functions *****

  /// \brief Takes an angle, returns its sine
  [[nodiscard]] float Sin(nsAngle a); // [tested]

  /// \brief Takes an angle, returns its cosine
  [[nodiscard]] float Cos(nsAngle a); // [tested]

  /// \brief Takes an angle, returns its tangent
  [[nodiscard]] float Tan(nsAngle a); // [tested]

  /// \brief Returns the arcus sinus of f
  [[nodiscard]] nsAngle ASin(float f); // [tested]

  /// \brief Returns the arcus cosinus of f
  [[nodiscard]] nsAngle ACos(float f); // [tested]

  /// \brief Returns the arcus tangent of f
  [[nodiscard]] nsAngle ATan(float f); // [tested]

  /// \brief Returns the atan2 of x and y
  [[nodiscard]] nsAngle ATan2(float y, float x); // [tested]

  /// \brief Returns e^f
  [[nodiscard]] float Exp(float f); // [tested]

  /// \brief Returns the logarithmus naturalis of f
  [[nodiscard]] float Ln(float f); // [tested]

  /// \brief Returns log (f), to the base 2
  [[nodiscard]] float Log2(float f); // [tested]

  /// \brief Returns the integral logarithm to the base 2, that comes closest to the given integer.
  [[nodiscard]] nsUInt32 Log2i(nsUInt32 uiVal); // [tested]

  /// \brief Returns log (f), to the base 10
  [[nodiscard]] float Log10(float f); // [tested]

  /// \brief Returns log (f), to the base fBase
  [[nodiscard]] float Log(float fBase, float f); // [tested]

  /// \brief Returns 2^f
  [[nodiscard]] float Pow2(float f); // [tested]

  /// \brief Returns base^exp
  [[nodiscard]] float Pow(float fBase, float fExp); // [tested]

  /// \brief Returns 2^f
  [[nodiscard]] constexpr nsInt32 Pow2(nsInt32 i); // [tested]

  /// \brief Returns base^exp
  [[nodiscard]] nsInt32 Pow(nsInt32 iBase, nsInt32 iExp); // [tested]

  /// \brief Returns f * f
  template <typename T>
  [[nodiscard]] constexpr T Square(T f); // [tested]

  /// \brief Returns the square root of f
  [[nodiscard]] float Sqrt(float f); // [tested]

  /// \brief Returns the square root of f
  [[nodiscard]] double Sqrt(double f); // [tested]

  /// \brief Returns the n-th root of f.
  [[nodiscard]] float Root(float f, float fNthRoot); // [tested]

  /// \brief Returns the sign of f (i.e: -1, 1 or 0)
  template <typename T>
  [[nodiscard]] constexpr T Sign(T f); // [tested]

  /// \brief Returns the absolute value of f
  template <typename T>
  [[nodiscard]] constexpr T Abs(T f); // [tested]

  /// \brief Returns the smaller value, f1 or f2
  template <typename T>
  [[nodiscard]] constexpr T Min(T f1, T f2); // [tested]

  /// \brief Returns the smaller value, f1 or f2 or ...
  template <typename T, typename... ARGS>
  [[nodiscard]] constexpr T Min(T f1, T f2, ARGS... f); // [tested]

  /// \brief Returns the greater value, f1 or f2
  template <typename T>
  [[nodiscard]] constexpr T Max(T f1, T f2); // [tested]

  /// \brief Returns the smaller value, f1 or f2 or ...
  template <typename T, typename... ARGS>
  [[nodiscard]] constexpr T Min(T f1, T f2, ARGS... f); // [tested]

  /// \brief Clamps "value" to the range [min; max]. Returns "value", if it is inside the range already
  template <typename T>
  [[nodiscard]] constexpr T Clamp(T value, T min_val, T max_val); // [tested]

  /// \brief Clamps "value" to the range [0; 1]. Returns "value", if it is inside the range already
  template <typename T>
  [[nodiscard]] constexpr T Saturate(T value); // [tested]

  /// \brief Returns the next smaller integer, closest to f. Also the SMALLER value, if f is negative.
  [[nodiscard]] float Floor(float f); // [tested]

  /// \brief Returns the next higher integer, closest to f. Also the HIGHER value, if f is negative.
  [[nodiscard]] float Ceil(float f); // [tested]

  /// \brief Returns a multiple of fMultiple that is smaller than f.
  [[nodiscard]] float RoundDown(float f, float fMultiple); // [tested]

  /// \brief Returns a multiple of fMultiple that is smaller than f.
  [[nodiscard]] double RoundDown(double f, double fMultiple); // [tested]

  /// \brief Returns a multiple of fMultiple that is larger than f.
  [[nodiscard]] float RoundUp(float f, float fMultiple); // [tested]

  /// \brief Returns a multiple of fMultiple that is larger than f.
  [[nodiscard]] double RoundUp(double f, double fMultiple); // [tested]

  /// \brief Returns the integer-part of f (removes the fraction).
  template <typename Type>
  [[nodiscard]] Type Trunc(Type f); // [tested]

  /// \brief Casts the float to an integer, removes the fractional part
  ///
  /// \sa Trunc, Round, Floor, Ceil
  [[nodiscard]] constexpr nsInt32 FloatToInt(float value);

  // There is a compiler bug in VS 2019 targeting 32-bit that causes an internal compiler error when casting double to long long.
  // FloatToInt(double) is not available on these version of the MSVC compiler.
#if NS_DISABLED(NS_PLATFORM_ARCH_X86) || (_MSC_VER <= 1916)
  /// \brief Casts the float to an integer, removes the fractional part
  ///
  /// \sa Trunc, Round, Floor, Ceil
  [[nodiscard]] constexpr nsInt64 FloatToInt(double value);
#endif

  /// \brief Rounds f to the next integer.
  ///
  /// If f is positive 0.5 is rounded UP (i.e. to 1), if f is negative, -0.5 is rounded DOWN (i.e. to -1).
  [[nodiscard]] float Round(float f); // [tested]

  /// \brief Rounds f to the next integer.
  ///
  /// If f is positive 0.5 is rounded UP (i.e. to 1), if f is negative, -0.5 is rounded DOWN (i.e. to -1).
  [[nodiscard]] double Round(double f); // [tested]

  /// \brief Rounds f to the closest value of multiple.
  [[nodiscard]] float RoundToMultiple(float f, float fMultiple);

  /// \brief Rounds f to the closest value of multiple.
  [[nodiscard]] double RoundToMultiple(double f, double fMultiple);

  /// \brief Returns the fraction-part of f.
  template <typename Type>
  [[nodiscard]] Type Fraction(Type f); // [tested]

  /// \brief Returns "value mod div" for floats. This also works with negative numbers, both for value and for div.
  [[nodiscard]] float Mod(float value, float fDiv); // [tested]

  /// \brief Returns "value mod div" for doubles. This also works with negative numbers, both for value and for div.
  [[nodiscard]] double Mod(double f, double fDiv); // [tested]

  /// \brief Returns 1 / f
  template <typename Type>
  [[nodiscard]] constexpr Type Invert(Type f); // [tested]

  /// \brief Returns a multiple of the given multiple that is larger than or equal to value.
  [[nodiscard]] constexpr nsInt32 RoundUp(nsInt32 value, nsUInt16 uiMultiple); // [tested]

  /// \brief Returns a multiple of the given multiple that is smaller than or equal to value.
  [[nodiscard]] constexpr nsInt32 RoundDown(nsInt32 value, nsUInt16 uiMultiple); // [tested]

  /// \brief Returns a multiple of the given multiple that is greater than or equal to value.
  [[nodiscard]] constexpr nsUInt32 RoundUp(nsUInt32 value, nsUInt16 uiMultiple); // [tested]

  /// \brief Returns a multiple of the given multiple that is smaller than or equal to value.
  [[nodiscard]] constexpr nsUInt32 RoundDown(nsUInt32 value, nsUInt16 uiMultiple); // [tested]

  /// \brief Returns true, if i is an odd number
  [[nodiscard]] constexpr bool IsOdd(nsInt32 i); // [tested]

  /// \brief Returns true, if i is an even number
  [[nodiscard]] constexpr bool IsEven(nsInt32 i); // [tested]

  /// \brief Returns the index of the least significant bit set
  ///
  /// Asserts that bitmask is not 0.
  [[nodiscard]] nsUInt32 FirstBitLow(nsUInt32 uiBitmask); // [tested]

  /// \brief Returns the index of the least significant bit set
  ///
  /// Asserts that bitmask is not 0.
  [[nodiscard]] nsUInt32 FirstBitLow(nsUInt64 uiBitmask); // [tested]

  /// \brief Returns the index of the most significant bit set
  ///
  /// Asserts that bitmask is not 0.
  [[nodiscard]] nsUInt32 FirstBitHigh(nsUInt32 uiBitmask); // [tested]

  /// \brief Returns the index of the most significant bit set
  ///
  /// Asserts that bitmask is not 0.
  [[nodiscard]] nsUInt32 FirstBitHigh(nsUInt64 uiBitmask); // [tested]

  /// Returns the number of zeros at the end (least significant part) of a bitmask.
  ///
  /// E.g.
  /// 0b0111 -> 0
  /// 0b0110 -> 1
  /// 0b0100 -> 2
  /// Returns 32 when the input is 0
  [[nodiscard]] nsUInt32 CountTrailingZeros(nsUInt32 uiBitmask); // [tested]

  /// \brief 64 bit overload for CountTrailingZeros()
  [[nodiscard]] nsUInt32 CountTrailingZeros(nsUInt64 uiBitmask); // [tested]

  /// Returns the number of zeros at the start (most significant part) of a bitmask.
  ///
  /// E.g.
  /// 0b0111 -> 29
  /// 0b0011 -> 30
  /// 0b0001 -> 31
  /// 0b0000 -> 32
  /// Returns 32 when the input is 0
  [[nodiscard]] nsUInt32 CountLeadingZeros(nsUInt32 uiBitmask); // [tested]

  /// \brief Returns the number of bits set
  [[nodiscard]] nsUInt32 CountBits(nsUInt32 value);

  /// \brief Returns the number of bits set
  [[nodiscard]] nsUInt32 CountBits(nsUInt64 value);

  /// \brief Creates a bitmask in which the low N bits are set. For example for N=5, this would be '0000 ... 0001 1111'
  ///
  /// For N >= 32 all bits will be set.
  template <typename Type>
  [[nodiscard]] Type Bitmask_LowN(nsUInt32 uiNumBitsToSet);

  /// \brief Creates a bitmask in which the high N bits are set. For example for N=5, this would be '1111 1000 ... 0000'
  ///
  /// For N >= 32 all bits will be set.
  template <typename Type>
  [[nodiscard]] Type Bitmask_HighN(nsUInt32 uiNumBitsToSet);

  /// \brief Swaps the values in the two variables f1 and f2
  template <typename T>
  void Swap(T& ref_f1, T& ref_f2); // [tested]

  /// \brief Returns the linear interpolation of f1 and f2. factor is a value between 0 and 1.
  template <typename T>
  [[nodiscard]] T Lerp(T f1, T f2, float fFactor); // [tested]

  /// \brief Returns the linear interpolation of f1 and f2. factor is a value between 0 and 1.
  template <typename T>
  [[nodiscard]] T Lerp(T f1, T f2, double fFactor); // [tested]

  /// \brief Returns the interpolation factor such that Lerp(fMin, fMax, factor) == fValue.
  template <typename T>
  [[nodiscard]] constexpr float Unlerp(T fMin, T fMax, T fValue); // [tested]

  /// \brief Returns 0, if value < edge, and 1, if value >= edge.
  template <typename T>
  [[nodiscard]] constexpr T Step(T value, T edge); // [tested]

  /// \brief Returns 0, if value is <= edge1, 1 if value >= edge2 and the hermite interpolation in between
  template <typename Type>
  [[nodiscard]] Type SmoothStep(Type value, Type edge1, Type edge2); // [tested]

  /// \brief Returns 0, if value is <= edge1, 1 if value >= edge2 and the second order hermite interpolation in between
  template <typename Type>
  [[nodiscard]] Type SmootherStep(Type value, Type edge1, Type edge2); // [tested]

  /// \brief Returns true, if there exists some x with base^x == value
  [[nodiscard]] NS_FOUNDATION_DLL bool IsPowerOf(nsInt32 value, nsInt32 iBase); // [tested]

  /// \brief Returns true, if there exists some x with 2^x == value
  [[nodiscard]] constexpr bool IsPowerOf2(nsInt32 value); // [tested]

  /// \brief Returns true, if there exists some x with 2^x == value
  [[nodiscard]] constexpr bool IsPowerOf2(nsUInt32 value); // [tested]

  /// \brief Returns true, if there exists some x with 2^x == value
  [[nodiscard]] constexpr bool IsPowerOf2(nsUInt64 value); // [tested]

  /// \brief Returns the next power-of-two that is <= value
  [[nodiscard]] NS_FOUNDATION_DLL nsUInt32 PowerOfTwo_Floor(nsUInt32 value); // [tested]

  /// \brief Returns the next power-of-two that is <= value
  [[nodiscard]] NS_FOUNDATION_DLL nsUInt64 PowerOfTwo_Floor(nsUInt64 value); // [tested]

  /// \brief Returns the next power-of-two that is >= value
  [[nodiscard]] NS_FOUNDATION_DLL nsUInt32 PowerOfTwo_Ceil(nsUInt32 value); // [tested]

  /// \brief Returns the next power-of-two that is >= value
  [[nodiscard]] NS_FOUNDATION_DLL nsUInt64 PowerOfTwo_Ceil(nsUInt64 value); // [tested]

  /// \brief Returns the greatest common divisor.
  [[nodiscard]] NS_FOUNDATION_DLL nsUInt32 GreatestCommonDivisor(nsUInt32 a, nsUInt32 b); // [tested]

  /// \brief Checks, whether fValue is in the range [fDesired - fMaxImprecision; fDesired + fMaxImprecision].
  template <typename Type>
  [[nodiscard]] constexpr bool IsEqual(Type lhs, Type rhs, Type fEpsilon);

  /// \brief Checks whether the value of the first parameter lies between the value of the second and third.
  template <typename T>
  [[nodiscard]] constexpr bool IsInRange(T value, T minVal, T maxVal); // [tested]

  /// \brief Checks whether the given number is close to zero.
  template <typename Type>
  [[nodiscard]] bool IsZero(Type f, Type fEpsilon); // [tested]

  /// \brief Converts a color value from float [0;1] range to unsigned byte [0;255] range, with proper rounding
  [[nodiscard]] nsUInt8 ColorFloatToByte(float value); // [tested]

  /// \brief Converts a color value from float [0;1] range to unsigned short [0;65535] range, with proper rounding
  [[nodiscard]] nsUInt16 ColorFloatToShort(float value); // [tested]

  /// \brief Converts a color value from float [-1;1] range to signed byte [-127;127] range, with proper rounding
  [[nodiscard]] nsInt8 ColorFloatToSignedByte(float value); // [tested]

  /// \brief Converts a color value from float [-1;1] range to signed short [-32767;32767] range, with proper rounding
  [[nodiscard]] nsInt16 ColorFloatToSignedShort(float value); // [tested]

  /// \brief Converts a color value from unsigned byte [0;255] range to float [0;1] range, with proper rounding
  [[nodiscard]] constexpr float ColorByteToFloat(nsUInt8 value); // [tested]

  /// \brief Converts a color value from unsigned short [0;65535] range to float [0;1] range, with proper rounding
  [[nodiscard]] constexpr float ColorShortToFloat(nsUInt16 value); // [tested]

  /// \brief Converts a color value from signed byte [-128;127] range to float [-1;1] range, with proper rounding
  [[nodiscard]] constexpr float ColorSignedByteToFloat(nsInt8 value); // [tested]

  /// \brief Converts a color value from signed short [-32768;32767] range to float [0;1] range, with proper rounding
  [[nodiscard]] constexpr float ColorSignedShortToFloat(nsInt16 value); // [tested]

  /// \brief Evaluates the cubic spline defined by four control points at time \a t and returns the interpolated result.
  /// Can be used with T as float, vec2, vec3 or vec4
  template <typename T, typename T2>
  [[nodiscard]] T EvaluateBnsierCurve(T2 t, const T& startPoint, const T& controlPoint1, const T& controlPoint2, const T& endPoint);

  /// \brief out_Result = \a a * \a b. If an overflow happens, NS_FAILURE is returned.
  NS_FOUNDATION_DLL nsResult TryMultiply32(nsUInt32& out_uiResult, nsUInt32 a, nsUInt32 b, nsUInt32 c = 1, nsUInt32 d = 1); // [tested]

  /// \brief returns \a a * \a b. If an overflow happens, the program is terminated.
  [[nodiscard]] NS_FOUNDATION_DLL nsUInt32 SafeMultiply32(nsUInt32 a, nsUInt32 b, nsUInt32 c = 1, nsUInt32 d = 1);

  /// \brief out_Result = \a a * \a b. If an overflow happens, NS_FAILURE is returned.
  NS_FOUNDATION_DLL nsResult TryMultiply64(nsUInt64& out_uiResult, nsUInt64 a, nsUInt64 b, nsUInt64 c = 1, nsUInt64 d = 1); // [tested]

  /// \brief returns \a a * \a b. If an overflow happens, the program is terminated.
  [[nodiscard]] NS_FOUNDATION_DLL nsUInt64 SafeMultiply64(nsUInt64 a, nsUInt64 b, nsUInt64 c = 1, nsUInt64 d = 1);

  /// \brief Checks whether the given 64bit value actually fits into size_t, If it doesn't NS_FAILURE is returned.
  nsResult TryConvertToSizeT(size_t& out_uiResult, nsUInt64 uiValue); // [tested]

  /// \brief Checks whether the given 64bit value actually fits into size_t, If it doesn't the program is terminated.
  [[nodiscard]] NS_FOUNDATION_DLL size_t SafeConvertToSizeT(nsUInt64 uiValue);

  /// \brief If 'value' is not-a-number (NaN) 'fallback' is returned, otherwise 'value' is passed through unmodified.
  [[nodiscard]] NS_FOUNDATION_DLL float ReplaceNaN(float fValue, float fFallback); // [tested]

  /// \brief If 'value' is not-a-number (NaN) 'fallback' is returned, otherwise 'value' is passed through unmodified.
  [[nodiscard]] NS_FOUNDATION_DLL double ReplaceNaN(double fValue, double fFallback); // [tested]

} // namespace nsMath

#include <Foundation/Math/Implementation/MathDouble_inl.h>
#include <Foundation/Math/Implementation/MathFixedPoint_inl.h>
#include <Foundation/Math/Implementation/MathFloat_inl.h>
#include <Foundation/Math/Implementation/MathInt32_inl.h>
#include <Foundation/Math/Implementation/Math_inl.h>
