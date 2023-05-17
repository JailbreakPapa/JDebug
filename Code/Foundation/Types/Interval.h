#pragma once

#include <Foundation/Math/Math.h>

/// \brief Represents an interval with a start and an end value.
template <class Type>
class wdInterval
{
public:
  /// \brief The default constructor initializes the two values to zero.
  constexpr wdInterval() = default;

  /// \brief Initializes both start and end to the same value.
  constexpr wdInterval(Type startAndEndValue);

  /// \brief Initializes start and end to the given values.
  ///
  /// Clamps the end value to not be lower than the start value.
  constexpr wdInterval(Type start, Type end);

  /// \brief Sets the start value. If necessary, the end value will adjusted to not be lower than the start value.
  void SetStartAdjustEnd(Type value);

  /// \brief Sets the end value. If necessary, the start value will adjusted to not be higher than the end value.
  void SetEndAdjustStart(Type value);

  /// \brief Adjusts the start and end value to be within the given range. Prefers to adjust the end value, over the start value.
  ///
  /// Enforces that the start and end value are not closer than the given minimum separation.
  void ClampToIntervalAdjustEnd(Type minValue, Type maxValue, Type minimumSeparation = Type());

  /// \brief Adjusts the start and end value to be within the given range. Prefers to adjust the start value, over the end value.
  ///
  /// Enforces that the start and end value are not closer than the given minimum separation.
  void ClampToIntervalAdjustStart(Type minValue, Type maxValue, Type minimumSeparation = Type());

  /// \brief Returns how much the start and and value are separated from each other.
  Type GetSeparation() const;

  bool operator==(const wdInterval<Type>& rhs) const;
  bool operator!=(const wdInterval<Type>& rhs) const;

  Type m_StartValue = Type();
  Type m_EndValue = Type();
};

using wdFloatInterval = wdInterval<float>;
using wdIntInterval = wdInterval<wdInt32>;

#include <Foundation/Types/Implementation/Interval_inl.h>
