#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Threading/ThreadUtils.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Time/Timestamp.h>

NS_CREATE_SIMPLE_TEST(Time, Timestamp)
{
  const nsInt64 iFirstContactUnixTimeInSeconds = 2942956800LL;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Constructors / Valid Check")
  {
    nsTimestamp invalidTimestamp;
    NS_TEST_BOOL(!invalidTimestamp.IsValid());

    nsTimestamp validTimestamp = nsTimestamp::MakeFromInt(0, nsSIUnitOfTime::Second);
    NS_TEST_BOOL(validTimestamp.IsValid());
    validTimestamp = nsTimestamp::MakeInvalid();
    NS_TEST_BOOL(!validTimestamp.IsValid());

    nsTimestamp currentTimestamp = nsTimestamp::CurrentTimestamp();
    // Kind of hard to hit a moving target, let's just test if it is in a probable range.
    NS_TEST_BOOL(currentTimestamp.IsValid());
    NS_TEST_BOOL_MSG(currentTimestamp.GetInt64(nsSIUnitOfTime::Second) > 1384597970LL, "The current time is before this test was written!");
    NS_TEST_BOOL_MSG(currentTimestamp.GetInt64(nsSIUnitOfTime::Second) < 32531209845LL,
      "This current time is after the year 3000! If this is actually the case, please fix this test.");

    // Sleep for 10 milliseconds
    nsThreadUtils::Sleep(nsTime::MakeFromMilliseconds(10));
    NS_TEST_BOOL_MSG(currentTimestamp.GetInt64(nsSIUnitOfTime::Microsecond) < nsTimestamp::CurrentTimestamp().GetInt64(nsSIUnitOfTime::Microsecond),
      "Sleeping for 10 ms should cause the timestamp to change!");
    NS_TEST_BOOL_MSG(!currentTimestamp.Compare(nsTimestamp::CurrentTimestamp(), nsTimestamp::CompareMode::Identical),
      "Sleeping for 10 ms should cause the timestamp to change!");

    // a valid timestamp should always be 'newer' than an invalid one
    NS_TEST_BOOL(currentTimestamp.Compare(nsTimestamp::MakeInvalid(), nsTimestamp::CompareMode::Newer) == true);
    // an invalid timestamp should not be 'newer' than any valid one
    NS_TEST_BOOL(nsTimestamp::MakeInvalid().Compare(currentTimestamp, nsTimestamp::CompareMode::Newer) == false);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Public Accessors")
  {
    const nsTimestamp epoch = nsTimestamp::MakeFromInt(0, nsSIUnitOfTime::Second);
    const nsTimestamp firstContact = nsTimestamp::MakeFromInt(iFirstContactUnixTimeInSeconds, nsSIUnitOfTime::Second);
    NS_TEST_BOOL(epoch.IsValid());
    NS_TEST_BOOL(firstContact.IsValid());

    // GetInt64 / SetInt64
    nsTimestamp firstContactTest = nsTimestamp::MakeFromInt(iFirstContactUnixTimeInSeconds, nsSIUnitOfTime::Second);
    NS_TEST_INT(firstContactTest.GetInt64(nsSIUnitOfTime::Second), iFirstContactUnixTimeInSeconds);
    NS_TEST_INT(firstContactTest.GetInt64(nsSIUnitOfTime::Millisecond), iFirstContactUnixTimeInSeconds * 1000LL);
    NS_TEST_INT(firstContactTest.GetInt64(nsSIUnitOfTime::Microsecond), iFirstContactUnixTimeInSeconds * 1000000LL);
    NS_TEST_INT(firstContactTest.GetInt64(nsSIUnitOfTime::Nanosecond), iFirstContactUnixTimeInSeconds * 1000000000LL);

    firstContactTest = nsTimestamp::MakeFromInt(firstContactTest.GetInt64(nsSIUnitOfTime::Second), nsSIUnitOfTime::Second);
    NS_TEST_BOOL(firstContactTest.Compare(firstContact, nsTimestamp::CompareMode::Identical));
    firstContactTest = nsTimestamp::MakeFromInt(firstContactTest.GetInt64(nsSIUnitOfTime::Millisecond), nsSIUnitOfTime::Millisecond);
    NS_TEST_BOOL(firstContactTest.Compare(firstContact, nsTimestamp::CompareMode::Identical));
    firstContactTest = nsTimestamp::MakeFromInt(firstContactTest.GetInt64(nsSIUnitOfTime::Microsecond), nsSIUnitOfTime::Microsecond);
    NS_TEST_BOOL(firstContactTest.Compare(firstContact, nsTimestamp::CompareMode::Identical));
    firstContactTest = nsTimestamp::MakeFromInt(firstContactTest.GetInt64(nsSIUnitOfTime::Nanosecond), nsSIUnitOfTime::Nanosecond);
    NS_TEST_BOOL(firstContactTest.Compare(firstContact, nsTimestamp::CompareMode::Identical));

    // IsEqual
    const nsTimestamp firstContactPlusAFewMicroseconds = nsTimestamp::MakeFromInt(firstContact.GetInt64(nsSIUnitOfTime::Microsecond) + 42, nsSIUnitOfTime::Microsecond);
    NS_TEST_BOOL(firstContact.Compare(firstContactPlusAFewMicroseconds, nsTimestamp::CompareMode::FileTimeEqual));
    NS_TEST_BOOL(!firstContact.Compare(firstContactPlusAFewMicroseconds, nsTimestamp::CompareMode::Identical));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Operators")
  {
    const nsTimestamp firstContact = nsTimestamp::MakeFromInt(iFirstContactUnixTimeInSeconds, nsSIUnitOfTime::Second);

    // Time span arithmetics
    const nsTime timeSpan1000s = nsTime::MakeFromSeconds(1000);
    NS_TEST_BOOL(timeSpan1000s.GetMicroseconds() == 1000000000LL);

    // operator +
    const nsTimestamp firstContactPlus1000s = firstContact + timeSpan1000s;
    nsInt64 iSpanDiff = firstContactPlus1000s.GetInt64(nsSIUnitOfTime::Microsecond) - firstContact.GetInt64(nsSIUnitOfTime::Microsecond);
    NS_TEST_BOOL(iSpanDiff == 1000000000LL);
    // You can only subtract points in time
    NS_TEST_BOOL(firstContactPlus1000s - firstContact == timeSpan1000s);

    const nsTimestamp T1000sPlusFirstContact = timeSpan1000s + firstContact;
    iSpanDiff = T1000sPlusFirstContact.GetInt64(nsSIUnitOfTime::Microsecond) - firstContact.GetInt64(nsSIUnitOfTime::Microsecond);
    NS_TEST_BOOL(iSpanDiff == 1000000000LL);
    // You can only subtract points in time
    NS_TEST_BOOL(T1000sPlusFirstContact - firstContact == timeSpan1000s);

    // operator -
    const nsTimestamp firstContactMinus1000s = firstContact - timeSpan1000s;
    iSpanDiff = firstContactMinus1000s.GetInt64(nsSIUnitOfTime::Microsecond) - firstContact.GetInt64(nsSIUnitOfTime::Microsecond);
    NS_TEST_BOOL(iSpanDiff == -1000000000LL);
    // You can only subtract points in time
    NS_TEST_BOOL(firstContact - firstContactMinus1000s == timeSpan1000s);


    // operator += / -=
    nsTimestamp testTimestamp = firstContact;
    testTimestamp += timeSpan1000s;
    NS_TEST_BOOL(testTimestamp.Compare(firstContactPlus1000s, nsTimestamp::CompareMode::Identical));
    testTimestamp -= timeSpan1000s;
    NS_TEST_BOOL(testTimestamp.Compare(firstContact, nsTimestamp::CompareMode::Identical));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsDateTime conversion")
  {
    // Constructor
    nsDateTime invalidDateTime;
    NS_TEST_BOOL(!invalidDateTime.GetTimestamp().IsValid());

    const nsTimestamp firstContact = nsTimestamp::MakeFromInt(iFirstContactUnixTimeInSeconds, nsSIUnitOfTime::Second);
    nsDateTime firstContactDataTime = nsDateTime::MakeFromTimestamp(firstContact);

    // Getter
    NS_TEST_INT(firstContactDataTime.GetYear(), 2063);
    NS_TEST_INT(firstContactDataTime.GetMonth(), 4);
    NS_TEST_INT(firstContactDataTime.GetDay(), 5);
    NS_TEST_BOOL(firstContactDataTime.GetDayOfWeek() == 4 ||
                 firstContactDataTime.GetDayOfWeek() == 255); // not supported on all platforms, should output 255 then
    NS_TEST_INT(firstContactDataTime.GetHour(), 0);
    NS_TEST_INT(firstContactDataTime.GetMinute(), 0);
    NS_TEST_INT(firstContactDataTime.GetSecond(), 0);
    NS_TEST_INT(firstContactDataTime.GetMicroseconds(), 0);

    // SetTimestamp / GetTimestamp
    nsTimestamp currentTimestamp = nsTimestamp::CurrentTimestamp();
    nsDateTime currentDateTime;
    currentDateTime.SetFromTimestamp(currentTimestamp).AssertSuccess();
    nsTimestamp currentTimestamp2 = currentDateTime.GetTimestamp();
    // OS date time functions should be accurate within one second.
    nsInt64 iDiff = nsMath::Abs(currentTimestamp.GetInt64(nsSIUnitOfTime::Microsecond) - currentTimestamp2.GetInt64(nsSIUnitOfTime::Microsecond));
    NS_TEST_BOOL(iDiff <= 1000000);

    // Setter
    nsDateTime oneSmallStep;
    oneSmallStep.SetYear(1969);
    oneSmallStep.SetMonth(7);
    oneSmallStep.SetDay(21);
    oneSmallStep.SetDayOfWeek(1);
    oneSmallStep.SetHour(2);
    oneSmallStep.SetMinute(56);
    oneSmallStep.SetSecond(0);
    oneSmallStep.SetMicroseconds(0);

    nsTimestamp oneSmallStepTimestamp = oneSmallStep.GetTimestamp();
    NS_TEST_BOOL(oneSmallStepTimestamp.IsValid());
    NS_TEST_INT(oneSmallStepTimestamp.GetInt64(nsSIUnitOfTime::Second), -14159040LL);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsDateTime formatting")
  {
    nsDateTime dateTime;

    dateTime.SetYear(2019);
    dateTime.SetMonth(8);
    dateTime.SetDay(16);
    dateTime.SetDayOfWeek(5);
    dateTime.SetHour(13);
    dateTime.SetMinute(40);
    dateTime.SetSecond(30);
    dateTime.SetMicroseconds(345678);

    char szTimestampFormatted[256] = "";

    // no names, no UTC, no milliseconds
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::Default));
    NS_TEST_STRING("2019-08-16 - 13:40:30", szTimestampFormatted);
    // no names, no UTC, with milliseconds
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::Default | nsArgDateTime::ShowMilliseconds));
    NS_TEST_STRING("2019-08-16 - 13:40:30.345", szTimestampFormatted);
    // no names, with UTC, no milliseconds
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::Default | nsArgDateTime::ShowTimeZone));
    NS_TEST_STRING("2019-08-16 - 13:40:30 (UTC)", szTimestampFormatted);
    // no names, with UTC, with milliseconds
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::ShowDate | nsArgDateTime::ShowMilliseconds | nsArgDateTime::ShowTimeZone));
    NS_TEST_STRING("2019-08-16 - 13:40:30.345 (UTC)", szTimestampFormatted);
    // with names, no UTC, no milliseconds
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::DefaultTextual | nsArgDateTime::ShowWeekday));
    NS_TEST_STRING("2019 Aug 16 (Fri) - 13:40:30", szTimestampFormatted);
    // no names, no UTC, with milliseconds
    BuildString(szTimestampFormatted, 256,
      nsArgDateTime(dateTime, nsArgDateTime::DefaultTextual | nsArgDateTime::ShowWeekday | nsArgDateTime::ShowMilliseconds));
    NS_TEST_STRING("2019 Aug 16 (Fri) - 13:40:30.345", szTimestampFormatted);
    // no names, with UTC, no milliseconds
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::DefaultTextual | nsArgDateTime::ShowWeekday | nsArgDateTime::ShowTimeZone));
    NS_TEST_STRING("2019 Aug 16 (Fri) - 13:40:30 (UTC)", szTimestampFormatted);
    // no names, with UTC, with milliseconds
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::DefaultTextual | nsArgDateTime::ShowWeekday | nsArgDateTime::ShowMilliseconds | nsArgDateTime::ShowTimeZone));
    NS_TEST_STRING("2019 Aug 16 (Fri) - 13:40:30.345 (UTC)", szTimestampFormatted);

    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::ShowDate));
    NS_TEST_STRING("2019-08-16", szTimestampFormatted);
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::TextualDate));
    NS_TEST_STRING("2019 Aug 16", szTimestampFormatted);
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::ShowTime));
    NS_TEST_STRING("13:40", szTimestampFormatted);
    BuildString(szTimestampFormatted, 256, nsArgDateTime(dateTime, nsArgDateTime::ShowWeekday | nsArgDateTime::ShowMilliseconds));
    NS_TEST_STRING("(Fri) - 13:40:30.345", szTimestampFormatted);
  }
}
