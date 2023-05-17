#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Configuration/StaticSubSystem.h>

/// \brief The time class encapsulates a double value storing the time in seconds.
///
/// It offers convenient functions to get the time in other units.
/// wdTime is a high-precision time using the OS specific high-precision timing functions
/// and may thus be used for profiling as well as simulation code.
struct WD_FOUNDATION_DLL wdTime
{
public:
  /// \brief Gets the current time
  static wdTime Now(); // [tested]

  /// \brief Creates an instance of wdTime that was initialized from nanoseconds.
  WD_ALWAYS_INLINE constexpr static wdTime Nanoseconds(double fNanoseconds) { return wdTime(fNanoseconds * 0.000000001); }

  /// \brief Creates an instance of wdTime that was initialized from microseconds.
  WD_ALWAYS_INLINE constexpr static wdTime Microseconds(double fMicroseconds) { return wdTime(fMicroseconds * 0.000001); }

  /// \brief Creates an instance of wdTime that was initialized from milliseconds.
  WD_ALWAYS_INLINE constexpr static wdTime Milliseconds(double fMilliseconds) { return wdTime(fMilliseconds * 0.001); }

  /// \brief Creates an instance of wdTime that was initialized from seconds.
  WD_ALWAYS_INLINE constexpr static wdTime Seconds(double fSeconds) { return wdTime(fSeconds); }

  /// \brief Creates an instance of wdTime that was initialized from minutes.
  WD_ALWAYS_INLINE constexpr static wdTime Minutes(double fMinutes) { return wdTime(fMinutes * 60); }

  /// \brief Creates an instance of wdTime that was initialized from hours.
  WD_ALWAYS_INLINE constexpr static wdTime Hours(double fHours) { return wdTime(fHours * 60 * 60); }

  /// \brief Creates an instance of wdTime that was initialized with zero.
  WD_ALWAYS_INLINE constexpr static wdTime Zero() { return wdTime(0.0); }

  WD_DECLARE_POD_TYPE();

  /// \brief The default constructor sets the time to zero.
  WD_ALWAYS_INLINE constexpr wdTime() = default;

  /// \brief Sets the time value to zero.
  void SetZero();

  /// \brief Returns true if the stored time is exactly zero. That typically means the value was not changed from the default.
  WD_ALWAYS_INLINE constexpr bool IsZero() const { return m_fTime == 0.0; }

  /// \brief Checks for a negative time value.
  WD_ALWAYS_INLINE constexpr bool IsNegative() const { return m_fTime < 0.0; }

  /// \brief Checks for a positive time value. This does not include zero.
  WD_ALWAYS_INLINE constexpr bool IsPositive() const { return m_fTime > 0.0; }

  /// \brief Returns true if the stored time is zero or negative.
  WD_ALWAYS_INLINE constexpr bool IsZeroOrNegative() const { return m_fTime <= 0.0; }

  /// \brief Returns true if the stored time is zero or positive.
  WD_ALWAYS_INLINE constexpr bool IsZeroOrPositive() const { return m_fTime >= 0.0; }

  /// \brief Returns the time as a float value (in seconds).
  ///
  /// Useful for simulation time steps etc.
  /// Please note that it is not recommended to use the float value for long running
  /// time calculations since the precision can deteriorate quickly. (Only use for delta times is recommended)
  constexpr float AsFloatInSeconds() const;

  /// \brief Returns the nanoseconds value
  constexpr double GetNanoseconds() const;

  /// \brief Returns the microseconds value
  constexpr double GetMicroseconds() const;

  /// \brief Returns the milliseconds value
  constexpr double GetMilliseconds() const;

  /// \brief Returns the seconds value.
  constexpr double GetSeconds() const;

  /// \brief Returns the minutes value.
  constexpr double GetMinutes() const;

  /// \brief Returns the hours value.
  constexpr double GetHours() const;

  /// \brief Subtracts the time value of "other" from this instances value.
  constexpr void operator-=(const wdTime& other);

  /// \brief Adds the time value of "other" to this instances value.
  constexpr void operator+=(const wdTime& other);

  /// \brief Multiplies the time by the given factor
  constexpr void operator*=(double fFactor);

  /// \brief Divides the time by the given factor
  constexpr void operator/=(double fFactor);

  /// \brief Returns the difference: "this instance - other"
  constexpr wdTime operator-(const wdTime& other) const;

  /// \brief Returns the sum: "this instance + other"
  constexpr wdTime operator+(const wdTime& other) const;

  constexpr wdTime operator-() const;

  constexpr bool operator<(const wdTime& rhs) const { return m_fTime < rhs.m_fTime; }
  constexpr bool operator<=(const wdTime& rhs) const { return m_fTime <= rhs.m_fTime; }
  constexpr bool operator>(const wdTime& rhs) const { return m_fTime > rhs.m_fTime; }
  constexpr bool operator>=(const wdTime& rhs) const { return m_fTime >= rhs.m_fTime; }
  constexpr bool operator==(const wdTime& rhs) const { return m_fTime == rhs.m_fTime; }
  constexpr bool operator!=(const wdTime& rhs) const { return m_fTime != rhs.m_fTime; }

private:
  /// \brief For internal use only.
  constexpr explicit wdTime(double fTime);

  /// \brief The time is stored in seconds
  double m_fTime = 0.0;

private:
  WD_MAKE_SUBSYSTEM_STARTUP_FRIEND(Foundation, Time);

  static void Initialize();
};

constexpr wdTime operator*(wdTime t, double f);
constexpr wdTime operator*(double f, wdTime t);
constexpr wdTime operator*(wdTime f, wdTime t); // not physically correct, but useful (should result in seconds squared)

constexpr wdTime operator/(wdTime t, double f);
constexpr wdTime operator/(double f, wdTime t);
constexpr wdTime operator/(wdTime f, wdTime t); // not physically correct, but useful (should result in a value without a unit)


#include <Foundation/Time/Implementation/Time_inl.h>
