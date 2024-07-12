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
/// If you are using nsFixedPoint to get guaranteed deterministic behavior, you should minimize the usage of
/// nsFixedPoint <-> float conversions. You can set nsFixedPoint variables from float constants, but you should
/// never put data into nsFixedPoint variables that was computed using floating point arithmetic (even if the
/// computations are simple and look harmless). Instead do all those computations with nsFixedPoint variables.
template <nsUInt8 DecimalBits>
class nsFixedPoint
{
public:
  /// \brief Default constructor does not do any initialization.
  NS_ALWAYS_INLINE nsFixedPoint() = default; // [tested]

                                             /// \brief Construct from an integer.
  /* implicit */ nsFixedPoint(nsInt32 iIntVal) { *this = iIntVal; } // [tested]

                                                                    /// \brief Construct from a float.
  /* implicit */ nsFixedPoint(float fVal) { *this = fVal; } // [tested]

                                                            /// \brief Construct from a double.
  /* implicit */ nsFixedPoint(double fVal) { *this = fVal; } // [tested]

  /// \brief Assignment from an integer.
  const nsFixedPoint<DecimalBits>& operator=(nsInt32 iVal); // [tested]

  /// \brief Assignment from a float.
  const nsFixedPoint<DecimalBits>& operator=(float fVal); // [tested]

  /// \brief Assignment from a double.
  const nsFixedPoint<DecimalBits>& operator=(double fVal); // [tested]

  /// \brief Implicit conversion to int (the fractional part is dropped).
  nsInt32 ToInt() const; // [tested]

  /// \brief Implicit conversion to float.
  float ToFloat() const; // [tested]

  /// \brief Implicit conversion to double.
  double ToDouble() const; // [tested]

  /// \brief 'Equality' comparison.
  bool operator==(const nsFixedPoint<DecimalBits>& rhs) const { return m_iValue == rhs.m_iValue; } // [tested]

  /// \brief 'Inequality' comparison.
  bool operator!=(const nsFixedPoint<DecimalBits>& rhs) const { return m_iValue != rhs.m_iValue; } // [tested]

  /// \brief 'Less than' comparison.
  bool operator<(const nsFixedPoint<DecimalBits>& rhs) const { return m_iValue < rhs.m_iValue; } // [tested]

  /// \brief 'Greater than' comparison.
  bool operator>(const nsFixedPoint<DecimalBits>& rhs) const { return m_iValue > rhs.m_iValue; } // [tested]

  /// \brief 'Less than or equal' comparison.
  bool operator<=(const nsFixedPoint<DecimalBits>& rhs) const { return m_iValue <= rhs.m_iValue; } // [tested]

  /// \brief 'Greater than or equal' comparison.
  bool operator>=(const nsFixedPoint<DecimalBits>& rhs) const { return m_iValue >= rhs.m_iValue; } // [tested]


  const nsFixedPoint<DecimalBits> operator-() const { return nsFixedPoint<DecimalBits>(-m_iValue, true); }

  /// \brief += operator
  void operator+=(const nsFixedPoint<DecimalBits>& rhs) { m_iValue += rhs.m_iValue; } // [tested]

  /// \brief -= operator
  void operator-=(const nsFixedPoint<DecimalBits>& rhs) { m_iValue -= rhs.m_iValue; } // [tested]

  /// \brief *= operator
  void operator*=(const nsFixedPoint<DecimalBits>& rhs); // [tested]

  /// \brief /= operator
  void operator/=(const nsFixedPoint<DecimalBits>& rhs); // [tested]

  /// \brief *= operator with integers (more efficient)
  void operator*=(nsInt32 rhs) { m_iValue *= rhs; } // [tested]

  /// \brief /= operator with integers (more efficient)
  void operator/=(nsInt32 rhs) { m_iValue /= rhs; } // [tested]

  /// \brief Returns the underlying integer value. Mostly useful for serialization (or tests).
  nsInt32 GetRawValue() const { return m_iValue; }

  /// \brief Sets the underlying integer value. Mostly useful for serialization (or tests).
  void SetRawValue(nsInt32 iVal) { m_iValue = iVal; }

private:
  nsInt32 m_iValue;
};

template <nsUInt8 DecimalBits>
float ToFloat(nsFixedPoint<DecimalBits> f)
{
  return f.ToFloat();
}

// Additional operators:
// nsFixedPoint operator+ (nsFixedPoint, nsFixedPoint); // [tested]
// nsFixedPoint operator- (nsFixedPoint, nsFixedPoint); // [tested]
// nsFixedPoint operator* (nsFixedPoint, nsFixedPoint); // [tested]
// nsFixedPoint operator/ (nsFixedPoint, nsFixedPoint); // [tested]
// nsFixedPoint operator* (int, nsFixedPoint); // [tested]
// nsFixedPoint operator* (nsFixedPoint, int); // [tested]
// nsFixedPoint operator/ (nsFixedPoint, int); // [tested]

#include <Foundation/Math/Implementation/FixedPoint_inl.h>
