#pragma once

#include <Foundation/Reflection/Reflection.h>
#include <Foundation/Time/Time.h>

struct nsSIUnitOfTime
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
/// The value is represented by an nsInt64 and allows storing time stamps from roughly
/// -291030 BC to 293970 AC.
/// Use this class to efficiently store a timestamp that is valid across platforms.
class NS_FOUNDATION_DLL nsTimestamp
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
  static const nsTimestamp CurrentTimestamp(); // [tested]

  NS_DECLARE_POD_TYPE();

  // *** Constructors ***
public:
  /// \brief Creates an invalidated timestamp.
  nsTimestamp(); // [tested]

  /// \brief Returns an invalid timestamp
  [[nodiscard]] static nsTimestamp MakeInvalid() { return nsTimestamp(); }

  /// \brief Returns a timestamp initialized from 'iTimeValue' in 'unitOfTime' since Unix epoch.
  [[nodiscard]] static nsTimestamp MakeFromInt(nsInt64 iTimeValue, nsSIUnitOfTime::Enum unitOfTime);

  // *** Public Functions ***
public:
  /// \brief Returns whether the timestamp is valid.
  bool IsValid() const; // [tested]

  /// \brief Returns the number of 'unitOfTime' since Unix epoch.
  nsInt64 GetInt64(nsSIUnitOfTime::Enum unitOfTime) const; // [tested]

  /// \brief Returns whether this timestamp is considered equal to 'rhs' in the given mode.
  ///
  /// Use CompareMode::FileTime when working with file time stamps across platforms.
  /// It will use the lowest resolution supported by all platforms to make sure the
  /// timestamp of a file is considered equal regardless on which platform it was retrieved.
  bool Compare(const nsTimestamp& rhs, CompareMode::Enum mode) const; // [tested]

  // *** Operators ***
public:
  /// \brief Adds the time value of "timeSpan" to this data value.
  void operator+=(const nsTime& timeSpan); // [tested]

  /// \brief Subtracts the time value of "timeSpan" from this date value.
  void operator-=(const nsTime& timeSpan); // [tested]

  /// \brief Returns the time span between this timestamp and "other".
  const nsTime operator-(const nsTimestamp& other) const; // [tested]

  /// \brief Returns a timestamp that is "timeSpan" further into the future from this timestamp.
  const nsTimestamp operator+(const nsTime& timeSpan) const; // [tested]

  /// \brief Returns a timestamp that is "timeSpan" further into the past from this timestamp.
  const nsTimestamp operator-(const nsTime& timeSpan) const; // [tested]


private:
  static constexpr const nsInt64 NS_INVALID_TIME_STAMP = nsMath::MinValue<nsInt64>();

  NS_ALLOW_PRIVATE_PROPERTIES(nsTimestamp);
  /// \brief The date is stored as microseconds since Unix epoch.
  nsInt64 m_iTimestamp = NS_INVALID_TIME_STAMP;
};

/// \brief Returns a timestamp that is "timeSpan" further into the future from "timestamp".
const nsTimestamp operator+(nsTime& ref_timeSpan, const nsTimestamp& timestamp);

NS_DECLARE_REFLECTABLE_TYPE(NS_FOUNDATION_DLL, nsTimestamp);

/// \brief The nsDateTime class can be used to convert nsTimestamp into a human readable form.
///
/// Note: As nsTimestamp is microseconds since Unix epoch, the values in this class will always be
/// in UTC.
class NS_FOUNDATION_DLL nsDateTime
{
public:
  /// \brief Creates an empty date time instance with an invalid date.
  ///
  /// Day, Month and Year will be invalid and must be set.
  nsDateTime(); // [tested]
  ~nsDateTime();

  /// \brief Checks whether all values are within valid ranges.
  bool IsValid() const;

  /// \brief Returns a date time that is all zero.
  [[nodiscard]] static nsDateTime MakeZero() { return nsDateTime(); }

  /// \brief Sets this instance to the given timestamp.
  ///
  /// This calls SetFromTimestamp() internally and asserts that the conversion succeeded.
  /// Use SetFromTimestamp() directly, if you need to be able to react to invalid data.
  [[nodiscard]] static nsDateTime MakeFromTimestamp(nsTimestamp timestamp);

  /// \brief Converts this instance' values into a nsTimestamp.
  ///
  /// The conversion is done via the OS and can fail for values that are outside the supported range.
  /// In this case, the returned value will be invalid. Anything after 1970 and before the
  /// not so distant future should be safe.
  [[nodiscard]] const nsTimestamp GetTimestamp() const; // [tested]

  /// \brief Sets this instance to the given timestamp.
  ///
  /// The conversion is done via the OS and will fail for invalid dates and values outside the supported range,
  /// in which case NS_FAILURE will be returned.
  /// Anything after 1970 and before the not so distant future should be safe.
  nsResult SetFromTimestamp(nsTimestamp timestamp);

  // *** Accessors ***
public:
  /// \brief Returns the currently set year.
  nsUInt32 GetYear() const; // [tested]

  /// \brief Sets the year to the given value.
  void SetYear(nsInt16 iYear); // [tested]

  /// \brief Returns the currently set month.
  nsUInt8 GetMonth() const; // [tested]

  /// \brief Sets the month to the given value. Asserts that the value is in the valid range [1, 12].
  void SetMonth(nsUInt8 uiMonth); // [tested]

  /// \brief Returns the currently set day.
  nsUInt8 GetDay() const; // [tested]

  /// \brief Sets the day to the given value. Asserts that the value is in the valid range [1, 31].
  void SetDay(nsUInt8 uiDay); // [tested]

  /// \brief Returns the currently set day of week.
  nsUInt8 GetDayOfWeek() const;

  /// \brief Sets the day of week to the given value. Asserts that the value is in the valid range [0, 6].
  void SetDayOfWeek(nsUInt8 uiDayOfWeek);

  /// \brief Returns the currently set hour.
  nsUInt8 GetHour() const; // [tested]

  /// \brief Sets the hour to the given value. Asserts that the value is in the valid range [0, 23].
  void SetHour(nsUInt8 uiHour); // [tested]

  /// \brief Returns the currently set minute.
  nsUInt8 GetMinute() const; // [tested]

  /// \brief Sets the minute to the given value. Asserts that the value is in the valid range [0, 59].
  void SetMinute(nsUInt8 uiMinute); // [tested]

  /// \brief Returns the currently set second.
  nsUInt8 GetSecond() const; // [tested]

  /// \brief Sets the second to the given value. Asserts that the value is in the valid range [0, 59].
  void SetSecond(nsUInt8 uiSecond); // [tested]

  /// \brief Returns the currently set microseconds.
  nsUInt32 GetMicroseconds() const; // [tested]

  /// \brief Sets the microseconds to the given value. Asserts that the value is in the valid range [0, 999999].
  void SetMicroseconds(nsUInt32 uiMicroSeconds); // [tested]

private:
  /// \brief The fraction of a second in microseconds of this date [0, 999999].
  nsUInt32 m_uiMicroseconds = 0;
  /// \brief The year of this date [-32k, +32k].
  nsInt16 m_iYear = 0;
  /// \brief The month of this date [1, 12].
  nsUInt8 m_uiMonth = 0;
  /// \brief The day of this date [1, 31].
  nsUInt8 m_uiDay = 0;
  /// \brief The day of week of this date [0, 6].
  nsUInt8 m_uiDayOfWeek = 0;
  /// \brief The hour of this date [0, 23].
  nsUInt8 m_uiHour = 0;
  /// \brief The number of minutes of this date [0, 59].
  nsUInt8 m_uiMinute = 0;
  /// \brief The number of seconds of this date [0, 59].
  nsUInt8 m_uiSecond = 0;
};

NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsDateTime& arg);

struct nsArgDateTime
{
  enum FormattingFlags
  {
    ShowDate = NS_BIT(0),
    TextualDate = ShowDate | NS_BIT(1),
    ShowWeekday = NS_BIT(2),
    ShowTime = NS_BIT(3),
    ShowSeconds = ShowTime | NS_BIT(4),
    ShowMilliseconds = ShowSeconds | NS_BIT(5),
    ShowTimeZone = NS_BIT(6),

    Default = ShowDate | ShowSeconds,
    DefaultTextual = TextualDate | ShowSeconds,
  };

  /// \brief Initialized a formatting object for an nsDateTime instance.
  /// \param dateTime The nsDateTime instance to format.
  /// \param bUseNames Indicates whether to use names for days of week and months (true)
  ///        or a purely numerical representation (false).
  /// \param bShowTimeZoneIndicator Whether to indicate the timnsone of the nsDateTime object.
  inline explicit nsArgDateTime(const nsDateTime& dateTime, nsUInt32 uiFormattingFlags = Default)
    : m_Value(dateTime)
    , m_uiFormattingFlags(uiFormattingFlags)
  {
  }

  nsDateTime m_Value;
  nsUInt32 m_uiFormattingFlags;
};

NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgDateTime& arg);

#include <Foundation/Time/Implementation/Timestamp_inl.h>
