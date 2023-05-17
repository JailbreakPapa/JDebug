#pragma once

#include <Foundation/Math/Declarations.h>

/// \brief Implements fixed point arithmetic for fractional values.
///
/// Advantages over float and double are mostly that the computations are entirely integer-based and therefore
/// have a predictable (i.e. deterministic) result, independent from floating point settings, SSE support and
/// differences among CPUs.
/// Additionally fixed point arithmetic should be quite fast, compare to traditional floating point arithmetic
/// (not comparing it to SSE though).
/// With the template argument 'DecimalBits' you can specify how many bits are used for the fractional part.
/// I.e. a simple integer has zero DecimalBits. For a precision of about 1/1000 you need at least 10 DecimalBits
/// (1 << 10) == 1024.
/// Conversion between integer and fixed point is very fast (a shift), in contrast to float/int conversion.
///
/// If you are using wdFixedPoint to get guaranteed deterministic behavior, you should minimize the usage of
/// wdFixedPoint <-> float conversions. You can set wdFixedPoint variables from float constants, but you should
/// never put data into wdFixedPoint variables that was computed using floating point arithmetic (even if the
/// computations are simple and look harmless). Instead do all those computations with wdFixedPoint variables.
template <wdUInt8 DecimalBits>
class wdFixedPoint
{
public:
  /// \brief Default constructor does not do any initialization.
  WD_ALWAYS_INLINE wdFixedPoint() {} // [tested]

  /// \brief Construct from an integer.
  /* implicit */ wdFixedPoint(wdInt32 iIntVal) { *this = iIntVal; } // [tested]

  /// \brief Construct from a float.
  /* implicit */ wdFixedPoint(float fVal) { *this = fVal; } // [tested]

  /// \brief Construct from a double.
  /* implicit */ wdFixedPoint(double fVal) { *this = fVal; } // [tested]

  /// \brief Assignment from an integer.
  const wdFixedPoint<DecimalBits>& operator=(wdInt32 iVal); // [tested]

  /// \brief Assignment from a float.
  const wdFixedPoint<DecimalBits>& operator=(float fVal); // [tested]

  /// \brief Assignment from a double.
  const wdFixedPoint<DecimalBits>& operator=(double fVal); // [tested]

  /// \brief Implicit conversion to int (the fractional part is dropped).
  wdInt32 ToInt() const; // [tested]

  /// \brief Implicit conversion to float.
  float ToFloat() const; // [tested]

  /// \brief Implicit conversion to double.
  double ToDouble() const; // [tested]

  /// \brief 'Equality' comparison.
  bool operator==(const wdFixedPoint<DecimalBits>& rhs) const { return m_iValue == rhs.m_iValue; } // [tested]

  /// \brief 'Inequality' comparison.
  bool operator!=(const wdFixedPoint<DecimalBits>& rhs) const { return m_iValue != rhs.m_iValue; } // [tested]

  /// \brief 'Less than' comparison.
  bool operator<(const wdFixedPoint<DecimalBits>& rhs) const { return m_iValue < rhs.m_iValue; } // [tested]

  /// \brief 'Greater than' comparison.
  bool operator>(const wdFixedPoint<DecimalBits>& rhs) const { return m_iValue > rhs.m_iValue; } // [tested]

  /// \brief 'Less than or equal' comparison.
  bool operator<=(const wdFixedPoint<DecimalBits>& rhs) const { return m_iValue <= rhs.m_iValue; } // [tested]

  /// \brief 'Greater than or equal' comparison.
  bool operator>=(const wdFixedPoint<DecimalBits>& rhs) const { return m_iValue >= rhs.m_iValue; } // [tested]


  const wdFixedPoint<DecimalBits> operator-() const { return wdFixedPoint<DecimalBits>(-m_iValue, true); }

  /// \brief += operator
  void operator+=(const wdFixedPoint<DecimalBits>& rhs) { m_iValue += rhs.m_iValue; } // [tested]

  /// \brief -= operator
  void operator-=(const wdFixedPoint<DecimalBits>& rhs) { m_iValue -= rhs.m_iValue; } // [tested]

  /// \brief *= operator
  void operator*=(const wdFixedPoint<DecimalBits>& rhs); // [tested]

  /// \brief /= operator
  void operator/=(const wdFixedPoint<DecimalBits>& rhs); // [tested]

  /// \brief *= operator with integers (more efficient)
  void operator*=(wdInt32 rhs) { m_iValue *= rhs; } // [tested]

  /// \brief /= operator with integers (more efficient)
  void operator/=(wdInt32 rhs) { m_iValue /= rhs; } // [tested]

  /// \brief Returns the underlying integer value. Mostly useful for serialization (or tests).
  wdInt32 GetRawValue() const { return m_iValue; }

  /// \brief Sets the underlying integer value. Mostly useful for serialization (or tests).
  void SetRawValue(wdInt32 iVal) { m_iValue = iVal; }

private:
  wdInt32 m_iValue;
};

template <wdUInt8 DecimalBits>
float ToFloat(wdFixedPoint<DecimalBits> f)
{
  return f.ToFloat();
}

// Additional operators:
// wdFixedPoint operator+ (wdFixedPoint, wdFixedPoint); // [tested]
// wdFixedPoint operator- (wdFixedPoint, wdFixedPoint); // [tested]
// wdFixedPoint operator* (wdFixedPoint, wdFixedPoint); // [tested]
// wdFixedPoint operator/ (wdFixedPoint, wdFixedPoint); // [tested]
// wdFixedPoint operator* (int, wdFixedPoint); // [tested]
// wdFixedPoint operator* (wdFixedPoint, int); // [tested]
// wdFixedPoint operator/ (wdFixedPoint, int); // [tested]

#include <Foundation/Math/Implementation/FixedPoint_inl.h>
