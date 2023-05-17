#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Time/Time.h>

struct wdSIUnitOfTime
{
  enum Enum
  {
    Nanosecond,  ///< SI-unit of time (10^-9 second)
    Microsecond, ///< SI-unit of time (10^-6 second)
    Millisecond, ///< SI-unit of time (10^-3 second)
    Second,      ///< SI-unit of time (base unit)
  };
};

/// \brief The timestamp class encapsulates a date in time as microseconds since Unix epoch.
///
/// The value is represented by an wdInt64 and allows storing time stamps from roughly
/// -291030 BC to 293970 AC.
/// Use this class to efficiently store a timestamp that is valid across platforms.
class WD_FOUNDATION_DLL wdTimestamp
{
public:
  struct CompareMode
  {
    enum Enum
    {
      FileTimeEqual, ///< Uses a resolution that guarantees that a file's timestamp is considered equal on all platforms.
      Identical,     ///< Uses maximal stored resolution.
      Newer,         ///< Just compares values and returns true if the left-hand side is larger than the right hand side
    };
  };
  /// \brief  Returns the current timestamp. Returned value will always be valid.
  ///
  /// Depending on the platform the precision varies between seconds and nanoseconds.
  static const wdTimestamp CurrentTimestamp(); // [tested]

  WD_DECLARE_POD_TYPE();

  // *** Constructors ***
public:
  /// \brief Creates an invalidated timestamp.
  wdTimestamp(); // [tested]

  /// \brief Creates an new timestamp with the given time in the given unit of time since Unix epoch.
  wdTimestamp(wdInt64 iTimeValue, wdSIUnitOfTime::Enum unitOfTime); // [tested]

  // *** Public Functions ***
public:
  /// \brief Invalidates the timestamp.
  void Invalidate(); // [tested]

  /// \brief Returns whether the timestamp is valid.
  bool IsValid() const; // [tested]

  /// \brief Returns the number of 'unitOfTime' since Unix epoch.
  wdInt64 GetInt64(wdSIUnitOfTime::Enum unitOfTime) const; // [tested]

  /// \brief Sets the timestamp as 'iTimeValue' in 'unitOfTime' since Unix epoch.
  void SetInt64(wdInt64 iTimeValue, wdSIUnitOfTime::Enum unitOfTime); // [tested]

  /// \brief Returns whether this timestamp is considered equal to 'rhs' in the given mode.
  ///
  /// Use CompareMode::FileTime when working with file time stamps across platforms.
  /// It will use the lowest resolution supported by all platforms to make sure the
  /// timestamp of a file is considered equal regardless on which platform it was retrieved.
  bool Compare(const wdTimestamp& rhs, CompareMode::Enum mode) const; // [tested]

  // *** Operators ***
public:
  /// \brief Adds the time value of "timeSpan" to this data value.
  void operator+=(const wdTime& timeSpan); // [tested]

  /// \brief Subtracts the time value of "timeSpan" from this date value.
  void operator-=(const wdTime& timeSpan); // [tested]

  /// \brief Returns the time span between this timestamp and "other".
  const wdTime operator-(const wdTimestamp& other) const; // [tested]

  /// \brief Returns a timestamp that is "timeSpan" further into the future from this timestamp.
  const wdTimestamp operator+(const wdTime& timeSpan) const; // [tested]

  /// \brief Returns a timestamp that is "timeSpan" further into the past from this timestamp.
  const wdTimestamp operator-(const wdTime& timeSpan) const; // [tested]


private:
  WD_ALLOW_PRIVATE_PROPERTIES(wdTimestamp);
  /// \brief The date is stored as microseconds since Unix epoch.
  wdInt64 m_iTimestamp;
};

/// \brief Returns a timestamp that is "timeSpan" further into the future from "timestamp".
const wdTimestamp operator+(wdTime& ref_timeSpan, const wdTimestamp& timestamp);

WD_DECLARE_REFLECTABLE_TYPE(WD_FOUNDATION_DLL, wdTimestamp);

/// \brief The wdDateTime class can be used to convert wdTimestamp into a human readable form.
///
/// Note: As wdTimestamp is microseconds since Unix epoch, the values in this class will always be
/// in UTC.
class WD_FOUNDATION_DLL wdDateTime
{
public:
  /// \brief Creates an empty date time instance with an invalid date.
  ///
  /// Day, Month and Year will be invalid and must be set.
  wdDateTime(); // [tested]

  /// \brief Creates a date time instance from the given timestamp.
  wdDateTime(wdTimestamp timestamp); // [tested]

  /// \brief Converts this instance' values into a wdTimestamp.
  ///
  /// The conversion is done via the OS and can fail for values that are outside the supported range.
  /// In this case, the returned value will be invalid. Anything after 1970 and before the
  /// not so distant future should be safe.
  const wdTimestamp GetTimestamp() const; // [tested]

  /// \brief Sets this instance to the given timestamp.
  ///
  /// The conversion is done via the OS and will fail for invalid dates and values outside the supported range,
  /// in which case false will be returned.
  /// Anything after 1970 and before the not so distant future should be safe.
  bool SetTimestamp(wdTimestamp timestamp); // [tested]

  // *** Accessors ***
public:
  /// \brief Returns the currently set year.
  wdUInt32 GetYear() const; // [tested]

  /// \brief Sets the year to the given value.
  void SetYear(wdInt16 iYear); // [tested]

  /// \brief Returns the currently set month.
  wdUInt8 GetMonth() const; // [tested]

  /// \brief Sets the month to the given value, will be clamped to valid range [1, 12].
  void SetMonth(wdUInt8 uiMonth); // [tested]

  /// \brief Returns the currently set day.
  wdUInt8 GetDay() const; // [tested]

  /// \brief Sets the day to the given value, will be clamped to valid range [1, 31].
  void SetDay(wdUInt8 uiDay); // [tested]

  /// \brief Returns the currently set day of week.
  wdUInt8 GetDayOfWeek() const;

  /// \brief Sets the day of week to the given value, will be clamped to valid range [0, 6].
  void SetDayOfWeek(wdUInt8 uiDayOfWeek);

  /// \brief Returns the currently set hour.
  wdUInt8 GetHour() const; // [tested]

  /// \brief Sets the hour to the given value, will be clamped to valid range [0, 23].
  void SetHour(wdUInt8 uiHour); // [tested]

  /// \brief Returns the currently set minute.
  wdUInt8 GetMinute() const; // [tested]

  /// \brief Sets the minute to the given value, will be clamped to valid range [0, 59].
  void SetMinute(wdUInt8 uiMinute); // [tested]

  /// \brief Returns the currently set second.
  wdUInt8 GetSecond() const; // [tested]

  /// \brief Sets the second to the given value, will be clamped to valid range [0, 59].
  void SetSecond(wdUInt8 uiSecond); // [tested]

  /// \brief Returns the currently set microseconds.
  wdUInt32 GetMicroseconds() const; // [tested]

  /// \brief Sets the microseconds to the given value, will be clamped to valid range [0, 999999].
  void SetMicroseconds(wdUInt32 uiMicroSeconds); // [tested]

private:
  /// \brief The fraction of a second in microseconds of this date [0, 999999].
  wdUInt32 m_uiMicroseconds;
  /// \brief The year of this date [-32k, +32k].
  wdInt16 m_iYear;
  /// \brief The month of this date [1, 12].
  wdUInt8 m_uiMonth;
  /// \brief The day of this date [1, 31].
  wdUInt8 m_uiDay;
  /// \brief The day of week of this date [0, 6].
  wdUInt8 m_uiDayOfWeek;
  /// \brief The hour of this date [0, 23].
  wdUInt8 m_uiHour;
  /// \brief The number of minutes of this date [0, 59].
  wdUInt8 m_uiMinute;
  /// \brief The number of seconds of this date [0, 59].
  wdUInt8 m_uiSecond;
};

WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdDateTime& arg);

struct wdArgDateTime
{
  enum FormattingFlags
  {
    ShowDate = WD_BIT(0),
    TextualDate = ShowDate | WD_BIT(1),
    ShowWeekday = WD_BIT(2),
    ShowTime = WD_BIT(3),
    ShowSeconds = ShowTime | WD_BIT(4),
    ShowMilliseconds = ShowSeconds | WD_BIT(5),
    ShowTimeZone = WD_BIT(6),

    Default = ShowDate | ShowSeconds,
    DefaultTextual = TextualDate | ShowSeconds,
  };

  /// \brief Initialized a formatting object for an wdDateTime instance.
  /// \param dateTime The wdDateTime instance to format.
  /// \param bUseNames Indicates whether to use names for days of week and months (true)
  ///        or a purely numerical representation (false).
  /// \param bShowTimeZoneIndicator Whether to indicate the timwdone of the wdDateTime object.
  inline explicit wdArgDateTime(const wdDateTime& dateTime, wdUInt32 uiFormattingFlags = Default)
    : m_Value(dateTime)
    , m_uiFormattingFlags(uiFormattingFlags)
  {
  }

  wdDateTime m_Value;
  wdUInt32 m_uiFormattingFlags;
};

WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgDateTime& arg);

#include <Foundation/Time/Implementation/Timestamp_inl.h>
