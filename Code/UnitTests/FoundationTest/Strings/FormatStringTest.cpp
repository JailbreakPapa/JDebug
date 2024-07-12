#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/IO/FileSystem/FileWriter.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Time/Stopwatch.h>
#include <Foundation/Time/Time.h>
#include <Foundation/Time/Timestamp.h>

#include <Foundation/Types/ScopeExit.h>
#include <stdarg.h>

void TestFormat(const nsFormatString& str, const char* szExpected)
{
  nsStringBuilder sb;
  nsStringView szText = str.GetText(sb);

  NS_TEST_STRING(szText, szExpected);
}

void TestFormatWChar(const nsFormatString& str, const wchar_t* pExpected)
{
  nsStringBuilder sb;
  nsStringView szText = str.GetText(sb);

  NS_TEST_WSTRING(nsStringWChar(szText), pExpected);
}

void CompareSnprintf(nsStringBuilder& ref_sLog, const nsFormatString& str, const char* szFormat, ...)
{
  va_list args;
  va_start(args, szFormat);

  char Temp1[256];
  char Temp2[256];

  // reusing args list crashes on GCC / Clang
  nsStringUtils::vsnprintf(Temp1, 256, szFormat, args);
  vsnprintf(Temp2, 256, szFormat, args);
  NS_TEST_STRING(Temp1, Temp2);

  nsTime t1, t2, t3;
  nsStopwatch sw;
  {
    sw.StopAndReset();

    for (nsUInt32 i = 0; i < 10000; ++i)
    {
      nsStringUtils::vsnprintf(Temp1, 256, szFormat, args);
    }

    t1 = sw.Checkpoint();
  }

  {
    sw.StopAndReset();

    for (nsUInt32 i = 0; i < 10000; ++i)
    {
      vsnprintf(Temp2, 256, szFormat, args);
    }

    t2 = sw.Checkpoint();
  }

  {
    nsStringBuilder sb;

    sw.StopAndReset();
    for (nsUInt32 i = 0; i < 10000; ++i)
    {
      nsStringView sText = str.GetText(sb);
    }

    t3 = sw.Checkpoint();
  }

  ref_sLog.AppendFormat("ns: {0} msec, std: {1} msec, nsFmt: {2} msec : {3} -> {4}\n", nsArgF(t1.GetMilliseconds(), 2), nsArgF(t2.GetMilliseconds(), 2),
    nsArgF(t3.GetMilliseconds(), 2), szFormat, Temp1);

  va_end(args);
}

NS_CREATE_SIMPLE_TEST(Strings, FormatString)
{
  nsStringBuilder perfLog;

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Basics")
  {
    const char* tmp = "stringviewstuff";

    const char* sz = "sz";
    nsString string = "string";
    nsStringBuilder sb = "builder";
    nsStringView sv(tmp + 6, tmp + 10);

    TestFormat(nsFmt("{0}, {1}, {2}, {3}", nsInt8(-1), nsInt16(-2), nsInt32(-3), nsInt64(-4)), "-1, -2, -3, -4");
    TestFormat(nsFmt("{0}, {1}, {2}, {3}", nsUInt8(1), nsUInt16(2), nsUInt32(3), nsUInt64(4)), "1, 2, 3, 4");

    TestFormat(nsFmt("{0}, {1}", nsArgHumanReadable(0ll), nsArgHumanReadable(1ll)), "0, 1");
    TestFormat(nsFmt("{0}, {1}", nsArgHumanReadable(-0ll), nsArgHumanReadable(-1ll)), "0, -1");
    TestFormat(nsFmt("{0}, {1}", nsArgHumanReadable(999ll), nsArgHumanReadable(1000ll)), "999, 1.00K");
    TestFormat(nsFmt("{0}, {1}", nsArgHumanReadable(-999ll), nsArgHumanReadable(-1000ll)), "-999, -1.00K");
    // 999.999 gets rounded up for precision 2, so result is 1000.00K not 999.99K
    TestFormat(nsFmt("{0}, {1}", nsArgHumanReadable(999'999ll), nsArgHumanReadable(1'000'000ll)), "1000.00K, 1.00M");
    TestFormat(nsFmt("{0}, {1}", nsArgHumanReadable(-999'999ll), nsArgHumanReadable(-1'000'000ll)), "-1000.00K, -1.00M");

    TestFormat(nsFmt("{0}, {1}", nsArgFileSize(0u), nsArgFileSize(1u)), "0B, 1B");
    TestFormat(nsFmt("{0}, {1}", nsArgFileSize(1023u), nsArgFileSize(1024u)), "1023B, 1.00KB");
    // 1023.999 gets rounded up for precision 2, so result is 1024.00KB not 1023.99KB
    TestFormat(nsFmt("{0}, {1}", nsArgFileSize(1024u * 1024u - 1u), nsArgFileSize(1024u * 1024u)), "1024.00KB, 1.00MB");

    const char* const suffixes[] = {" Foo", " Bar", " Foobar"};
    const nsUInt32 suffixCount = NS_ARRAY_SIZE(suffixes);
    TestFormat(nsFmt("{0}", nsArgHumanReadable(0ll, 25u, suffixes, suffixCount)), "0 Foo");
    TestFormat(nsFmt("{0}", nsArgHumanReadable(25ll, 25u, suffixes, suffixCount)), "1.00 Bar");
    TestFormat(nsFmt("{0}", nsArgHumanReadable(25ll * 25ll * 2ll, 25u, suffixes, suffixCount)), "2.00 Foobar");

    TestFormat(nsFmt("{0}", nsArgHumanReadable(-0ll, 25u, suffixes, suffixCount)), "0 Foo");
    TestFormat(nsFmt("{0}", nsArgHumanReadable(-25ll, 25u, suffixes, suffixCount)), "-1.00 Bar");
    TestFormat(nsFmt("{0}", nsArgHumanReadable(-25ll * 25ll * 2ll, 25u, suffixes, suffixCount)), "-2.00 Foobar");

    TestFormat(nsFmt("'{0}, {1}'", "inl", sz), "'inl, sz'");
    TestFormat(nsFmt("'{0}'", string), "'string'");
    TestFormat(nsFmt("'{0}'", sb), "'builder'");
    TestFormat(nsFmt("'{0}'", sv), "'view'");

    TestFormat(nsFmt("{3}, {1}, {0}, {2}", nsArgF(23.12345f, 1), nsArgI(42), 17, 12.34f), "12.34, 42, 23.1, 17");

    const wchar_t* wsz = L"wsz";
    TestFormatWChar(nsFmt("'{0}, {1}'", "inl", wsz), L"'inl, wsz'");
    TestFormatWChar(nsFmt("'{0}, {1}'", L"inl", wsz), L"'inl, wsz'");
    // Temp buffer limit is 63 byte (64 including trailing zero). Each character in UTF-8 can potentially use 4 byte.
    // All input characters are 1 byte, so the 60th character is the last with 4 bytes left in the buffer.
    // Thus we end up with truncation after 60 characters.
    const wchar_t* wszTooLong = L"123456789.123456789.123456789.123456789.123456789.123456789.WAAAAAAAAAAAAAAH";
    const wchar_t* wszTooLongExpected = L"123456789.123456789.123456789.123456789.123456789.123456789.";
    const wchar_t* wszTooLongExpected2 =
      L"'123456789.123456789.123456789.123456789.123456789.123456789., 123456789.123456789.123456789.123456789.123456789.123456789.'";
    TestFormatWChar(nsFmt("{0}", wszTooLong), wszTooLongExpected);
    TestFormatWChar(nsFmt("'{0}, {1}'", wszTooLong, wszTooLong), wszTooLongExpected2);
  }

  NS_TEST_BLOCK(nsTestBlock::DisabledNoWarning, "Compare Performance")
  {
    CompareSnprintf(perfLog, nsFmt("Hello {0}, i = {1}, f = {2}", "World", 42, nsArgF(3.141f, 2)), "Hello %s, i = %i, f = %.2f", "World", 42, 3.141f);
    CompareSnprintf(perfLog, nsFmt("No formatting at all"), "No formatting at all");
    CompareSnprintf(perfLog, nsFmt("{0}, {1}, {2}, {3}, {4}", "AAAAAA", "BBBBBBB", "CCCCCC", "DDDDDDDDDDDDD", "EE"), "%s, %s, %s, %s, %s", "AAAAAA",
      "BBBBBBB", "CCCCCC", "DDDDDDDDDDDDD", "EE");
    CompareSnprintf(perfLog, nsFmt("{0}", 23), "%i", 23);
    CompareSnprintf(perfLog, nsFmt("{0}", 23.123456789), "%f", 23.123456789);
    CompareSnprintf(perfLog, nsFmt("{0}", nsArgF(23.123456789, 2)), "%.2f", 23.123456789);
    CompareSnprintf(perfLog, nsFmt("{0}", nsArgI(123456789, 20, true)), "%020i", 123456789);
    CompareSnprintf(perfLog, nsFmt("{0}", nsArgI(123456789, 20, true, 16)), "%020X", 123456789);
    CompareSnprintf(perfLog, nsFmt("{0}", nsArgU(1234567890987ll, 30, false, 16)), "%30llx", 1234567890987ll);
    CompareSnprintf(perfLog, nsFmt("{0}", nsArgU(1234567890987ll, 30, false, 16, true)), "%30llX", 1234567890987ll);
    CompareSnprintf(perfLog, nsFmt("{0}, {1}, {2}, {3}, {4}", 0, 1, 2, 3, 4), "%i, %i, %i, %i, %i", 0, 1, 2, 3, 4);
    CompareSnprintf(perfLog, nsFmt("{0}, {1}, {2}, {3}, {4}", 0.1, 1.1, 2.1, 3.1, 4.1), "%.1f, %.1f, %.1f, %.1f, %.1f", 0.1, 1.1, 2.1, 3.1, 4.1);
    CompareSnprintf(perfLog, nsFmt("{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9),
      "%i, %i, %i, %i, %i, %i, %i, %i, %i, %i", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9);
    CompareSnprintf(perfLog, nsFmt("{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}", 0.1, 1.1, 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1, 9.1),
      "%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f", 0.1, 1.1, 2.1, 3.1, 4.1, 5.1, 6.1, 7.1, 8.1, 9.1);
    CompareSnprintf(perfLog, nsFmt("{0}", nsArgC('z')), "%c", 'z');

    CompareSnprintf(perfLog, nsFmt("{}, {}, {}, {}, {}, {}, {}, {}, {}, {}", 0, 1, 2, 3, 4, 5, 6, 7, 8, 9), "%i, %i, %i, %i, %i, %i, %i, %i, %i, %i",
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9);

    // FILE* file = fopen("D:\\snprintf_perf.txt", "wb");
    // if (file)
    //{
    //  fwrite(perfLog.GetData(), 1, perfLog.GetElementCount(), file);
    //  fclose(file);
    //}
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Auto Increment")
  {
    TestFormat(nsFmt("{}{}{}{}", nsInt8(1), nsInt16(2), nsInt32(3), nsInt64(4)), "1234");
    TestFormat(nsFmt("{3}{2}{1}{0}", nsInt8(1), nsInt16(2), nsInt32(3), nsInt64(4)), "4321");

    TestFormat(nsFmt("{}, {}, {}, {}", nsInt8(-1), nsInt16(-2), nsInt32(-3), nsInt64(-4)), "-1, -2, -3, -4");
    TestFormat(nsFmt("{}, {}, {}, {}", nsUInt8(1), nsUInt16(2), nsUInt32(3), nsUInt64(4)), "1, 2, 3, 4");

    TestFormat(nsFmt("{0}, {}, {}, {}", nsUInt8(1), nsUInt16(2), nsUInt32(3), nsUInt64(4)), "1, 2, 3, 4");

    TestFormat(nsFmt("{1}, {}, {}, {}", nsUInt8(1), nsUInt16(2), nsUInt32(3), nsUInt64(4), nsUInt64(5)), "2, 3, 4, 5");

    TestFormat(nsFmt("{2}, {}, {1}, {}", nsUInt8(1), nsUInt16(2), nsUInt32(3), nsUInt64(4), nsUInt64(5)), "3, 4, 2, 3");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsTime")
  {
    TestFormat(nsFmt("{}", nsTime()), "0ns");
    TestFormat(nsFmt("{}", nsTime::MakeFromNanoseconds(999)), "999ns");
    TestFormat(nsFmt("{}", nsTime::MakeFromNanoseconds(999.1)), "999.1ns");
    TestFormat(nsFmt("{}", nsTime::MakeFromMicroseconds(999)), (const char*)u8"999\u00B5s");     // Utf-8 encoding for the microsecond sign
    TestFormat(nsFmt("{}", nsTime::MakeFromMicroseconds(999.2)), (const char*)u8"999.2\u00B5s"); // Utf-8 encoding for the microsecond sign
    TestFormat(nsFmt("{}", nsTime::MakeFromMilliseconds(-999)), "-999ms");
    TestFormat(nsFmt("{}", nsTime::MakeFromMilliseconds(-999.3)), "-999.3ms");
    TestFormat(nsFmt("{}", nsTime::MakeFromSeconds(59)), "59sec");
    TestFormat(nsFmt("{}", nsTime::MakeFromSeconds(-59.9)), "-59.9sec");
    TestFormat(nsFmt("{}", nsTime::MakeFromSeconds(75)), "1min 15sec");
    TestFormat(nsFmt("{}", nsTime::MakeFromSeconds(-75.4)), "-1min 15sec");
    TestFormat(nsFmt("{}", nsTime::MakeFromMinutes(59)), "59min 0sec");
    TestFormat(nsFmt("{}", nsTime::MakeFromMinutes(-1)), "-1min 0sec");
    TestFormat(nsFmt("{}", nsTime::MakeFromMinutes(90)), "1h 30min 0sec");
    TestFormat(nsFmt("{}", nsTime::MakeFromMinutes(-90.5)), "-1h 30min 30sec");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "nsDateTime")
  {
    {
      nsDateTime dt;
      dt.SetYear(2019);
      dt.SetMonth(6);
      dt.SetDay(12);
      dt.SetHour(13);
      dt.SetMinute(26);
      dt.SetSecond(51);
      dt.SetMicroseconds(7000);

      TestFormat(nsFmt("{}", dt), "2019-06-12_13-26-51-007");
    }

    {
      nsDateTime dt;
      dt.SetYear(0);
      dt.SetMonth(1);
      dt.SetDay(1);
      dt.SetHour(0);
      dt.SetMinute(0);
      dt.SetSecond(0);
      dt.SetMicroseconds(0);

      TestFormat(nsFmt("{}", dt), "0000-01-01_00-00-00-000");
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "Sensitive Info")
  {
    auto prev = nsArgSensitive::s_BuildStringCB;
    NS_SCOPE_EXIT(nsArgSensitive::s_BuildStringCB = prev);

    nsArgSensitive::s_BuildStringCB = nsArgSensitive::BuildString_SensitiveUserData_Hash;

    nsStringBuilder fmt;

    fmt.SetFormat("Password: {}", nsArgSensitive("hunter2", "pwd"));
    NS_TEST_STRING(fmt, "Password: sud:pwd#96d66ce6($7)");

    fmt.SetFormat("Password: {}", nsArgSensitive("hunter2"));
    NS_TEST_STRING(fmt, "Password: sud:#96d66ce6($7)");
  }
}
