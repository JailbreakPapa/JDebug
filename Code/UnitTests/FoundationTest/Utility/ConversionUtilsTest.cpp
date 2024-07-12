#include <FoundationTest/FoundationTestPCH.h>

#include <Foundation/Math/Random.h>
#include <Foundation/Utilities/ConversionUtils.h>

NS_CREATE_SIMPLE_TEST_GROUP(Utility);

NS_CREATE_SIMPLE_TEST(Utility, ConversionUtils)
{
  NS_TEST_BLOCK(nsTestBlock::Enabled, "StringToInt")
  {
    const char* szString = "1a";
    const char* szResultPos = nullptr;

    nsInt32 iRes = 42;
    szString = "01234";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 1234);
    NS_TEST_BOOL(szResultPos == szString + 5);

    iRes = 42;
    szString = "0";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 0);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "0000";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 0);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "-999999";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, -999999);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "-+999999";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, -999999);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "--999999";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 999999);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "++---+--+--999999";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, -999999);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "++--+--+--999999";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 999999);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "123+456";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 123);
    NS_TEST_BOOL(szResultPos == szString + 3);

    iRes = 42;
    szString = "123_456";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 123);
    NS_TEST_BOOL(szResultPos == szString + 3);

    iRes = 42;
    szString = "-123-456";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, -123);
    NS_TEST_BOOL(szResultPos == szString + 4);


    iRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToInt(nullptr, iRes) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToInt("", iRes) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToInt("a", iRes) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToInt("a15", iRes) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToInt("+", iRes) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToInt("-", iRes) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    szString = "1a";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 1);
    NS_TEST_BOOL(szResultPos == szString + 1);

    iRes = 42;
    szString = "0 23";
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 0);
    NS_TEST_BOOL(szResultPos == szString + 1);

    // overflow check

    iRes = 42;
    szString = "0002147483647"; // valid
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 2147483647);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "-2147483648"; // valid
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, (nsInt32)0x80000000);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "0002147483648"; // invalid
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    szString = "-2147483649"; // invalid
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    szString = "100'000"; // valid with c++ separator
    NS_TEST_BOOL(nsConversionUtils::StringToInt(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 100'000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StringToUInt")
  {
    const char* szString = "1a";
    const char* szResultPos = nullptr;

    nsUInt32 uiRes = 42;
    szString = "01234";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 1234);
    NS_TEST_BOOL(szResultPos == szString + 5);

    uiRes = 42;
    szString = "0";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 0);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    uiRes = 42;
    szString = "0000";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 0);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    uiRes = 42;
    szString = "-999999";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    uiRes = 42;
    szString = "-+999999";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    uiRes = 42;
    szString = "--999999";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 999999);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    uiRes = 42;
    szString = "++---+--+--999999";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    uiRes = 42;
    szString = "++--+--+--999999";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 999999);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    uiRes = 42;
    szString = "123+456";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 123);
    NS_TEST_BOOL(szResultPos == szString + 3);

    uiRes = 42;
    szString = "123_456";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 123);
    NS_TEST_BOOL(szResultPos == szString + 3);

    uiRes = 42;
    szString = "-123-456";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);
    NS_TEST_BOOL(szResultPos == szString + 4);


    uiRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(nullptr, uiRes) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);

    uiRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToUInt("", uiRes) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);

    uiRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToUInt("a", uiRes) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);

    uiRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToUInt("a15", uiRes) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);

    uiRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToUInt("+", uiRes) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);

    uiRes = 42;
    NS_TEST_BOOL(nsConversionUtils::StringToUInt("-", uiRes) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);

    uiRes = 42;
    szString = "1a";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 1);
    NS_TEST_BOOL(szResultPos == szString + 1);

    uiRes = 42;
    szString = "0 23";
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 0);
    NS_TEST_BOOL(szResultPos == szString + 1);

    // overflow check

    uiRes = 42;
    szString = "0004294967295"; // valid
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(uiRes, 4294967295u);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    uiRes = 42;
    szString = "0004294967296"; // invalid
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);

    uiRes = 42;
    szString = "-1"; // invalid
    NS_TEST_BOOL(nsConversionUtils::StringToUInt(szString, uiRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(uiRes, 42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StringToInt64")
  {
    // overflow check
    nsInt64 iRes = 42;
    const char* szString = "0002147483639"; // valid
    const char* szResultPos = nullptr;

    NS_TEST_BOOL(nsConversionUtils::StringToInt64(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 2147483639);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "0002147483640"; // also valid with 64bit
    NS_TEST_BOOL(nsConversionUtils::StringToInt64(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 2147483640);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "0009223372036854775807"; // last valid positive number
    NS_TEST_BOOL(nsConversionUtils::StringToInt64(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, 9223372036854775807);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "0009223372036854775808"; // invalid
    NS_TEST_BOOL(nsConversionUtils::StringToInt64(szString, iRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);

    iRes = 42;
    szString = "-9223372036854775808"; // last valid negative number
    NS_TEST_BOOL(nsConversionUtils::StringToInt64(szString, iRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_INT(iRes, (nsInt64)0x8000000000000000);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    iRes = 42;
    szString = "-9223372036854775809"; // invalid
    NS_TEST_BOOL(nsConversionUtils::StringToInt64(szString, iRes, &szResultPos) == NS_FAILURE);
    NS_TEST_INT(iRes, 42);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StringToFloat")
  {
    const char* szString = nullptr;
    const char* szResultPos = nullptr;

    double fRes = 42;
    szString = "23.45";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 23.45, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "-2345";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, -2345.0, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "-0";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 0.0, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "0_0000.0_00000_";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 0.0, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "_0_0000.0_00000_";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_FAILURE);

    fRes = 42;
    szString = ".123456789";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 0.123456789, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "+123E1";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 1230.0, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "  \r\t 123e0";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 123.0, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "\n123e6";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 123000000.0, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "\n1_2_3e+6";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 123000000.0, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = "  123E-6";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 0.000123, 0.00001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = " + - -+-123.45e-10";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, -0.000000012345, 0.0000001);
    NS_TEST_BOOL(szResultPos == szString + nsStringUtils::GetStringElementCount(szString));

    fRes = 42;
    szString = nullptr;
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_FAILURE);
    NS_TEST_DOUBLE(fRes, 42.0, 0.00001);

    fRes = 42;
    szString = "";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_FAILURE);
    NS_TEST_DOUBLE(fRes, 42.0, 0.00001);

    fRes = 42;
    szString = "-----";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_FAILURE);
    NS_TEST_DOUBLE(fRes, 42.0, 0.00001);

    fRes = 42;
    szString = " + - +++ - \r \n";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_FAILURE);
    NS_TEST_DOUBLE(fRes, 42.0, 0.00001);


    fRes = 42;
    szString = "65.345789xabc";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 65.345789, 0.000001);
    NS_TEST_BOOL(szResultPos == szString + 9);

    fRes = 42;
    szString = " \n \r \t + - 2314565.345789ff xabc";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, -2314565.345789, 0.000001);
    NS_TEST_BOOL(szResultPos == szString + 25);

    fRes = 42;
    szString = "100'000.0";
    NS_TEST_BOOL(nsConversionUtils::StringToFloat(szString, fRes, &szResultPos) == NS_SUCCESS);
    NS_TEST_DOUBLE(fRes, 100'000.0, 0.000001);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "StringToBool")
  {
    const char* szString = "";
    const char* szResultPos = nullptr;
    bool bRes = false;

    // true / false
    {
      bRes = false;
      szString = "true,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(bRes);
      NS_TEST_BOOL(*szResultPos == ',');

      bRes = true;
      szString = "FALSe,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(!bRes);
      NS_TEST_BOOL(*szResultPos == ',');
    }

    // on / off
    {
      bRes = false;
      szString = "\n on,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(bRes);
      NS_TEST_BOOL(*szResultPos == ',');

      bRes = true;
      szString = "\t\t \toFf,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(!bRes);
      NS_TEST_BOOL(*szResultPos == ',');
    }

    // 1 / 0
    {
      bRes = false;
      szString = "\r1,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(bRes);
      NS_TEST_BOOL(*szResultPos == ',');

      bRes = true;
      szString = "0,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(!bRes);
      NS_TEST_BOOL(*szResultPos == ',');
    }

    // yes / no
    {
      bRes = false;
      szString = "yes,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(bRes);
      NS_TEST_BOOL(*szResultPos == ',');

      bRes = true;
      szString = "NO,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(!bRes);
      NS_TEST_BOOL(*szResultPos == ',');
    }

    // enable / disable
    {
      bRes = false;
      szString = "enable,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(bRes);
      NS_TEST_BOOL(*szResultPos == ',');

      bRes = true;
      szString = "disABle,";
      szResultPos = nullptr;
      NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_SUCCESS);
      NS_TEST_BOOL(!bRes);
      NS_TEST_BOOL(*szResultPos == ',');
    }

    bRes = false;

    szString = "of,";
    szResultPos = nullptr;
    NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_FAILURE);
    NS_TEST_BOOL(szResultPos == nullptr);

    szString = "aon";
    szResultPos = nullptr;
    NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_FAILURE);
    NS_TEST_BOOL(szResultPos == nullptr);

    szString = "";
    szResultPos = nullptr;
    NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_FAILURE);
    NS_TEST_BOOL(szResultPos == nullptr);

    szString = nullptr;
    szResultPos = nullptr;
    NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_FAILURE);
    NS_TEST_BOOL(szResultPos == nullptr);

    szString = "tut";
    szResultPos = nullptr;
    NS_TEST_BOOL(nsConversionUtils::StringToBool(szString, bRes, &szResultPos) == NS_FAILURE);
    NS_TEST_BOOL(szResultPos == nullptr);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "HexCharacterToIntValue")
  {
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('0'), 0);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('1'), 1);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('2'), 2);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('3'), 3);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('4'), 4);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('5'), 5);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('6'), 6);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('7'), 7);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('8'), 8);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('9'), 9);

    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('a'), 10);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('b'), 11);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('c'), 12);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('d'), 13);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('e'), 14);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('f'), 15);

    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('A'), 10);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('B'), 11);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('C'), 12);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('D'), 13);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('E'), 14);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('F'), 15);

    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('g'), -1);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('h'), -1);
    NS_TEST_INT(nsConversionUtils::HexCharacterToIntValue('i'), -1);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ConvertHexStringToUInt32")
  {
    nsUInt32 res;

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("", res).Succeeded());
    NS_TEST_BOOL(res == 0);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("0x", res).Succeeded());
    NS_TEST_BOOL(res == 0);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("0", res).Succeeded());
    NS_TEST_BOOL(res == 0);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("0x0", res).Succeeded());
    NS_TEST_BOOL(res == 0);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("a", res).Succeeded());
    NS_TEST_BOOL(res == 10);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("0xb", res).Succeeded());
    NS_TEST_BOOL(res == 11);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("000c", res).Succeeded());
    NS_TEST_BOOL(res == 12);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("AA", res).Succeeded());
    NS_TEST_BOOL(res == 170);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("aAjbB", res).Failed());

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("aAbB", res).Succeeded());
    NS_TEST_BOOL(res == 43707);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("FFFFffff", res).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFFF);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("0000FFFFffff", res).Succeeded());
    NS_TEST_BOOL(res == 0xFFFF);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt32("100000000", res).Succeeded());
    NS_TEST_BOOL(res == 0x10000000);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ConvertHexStringToUInt64")
  {
    nsUInt64 res;

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("", res).Succeeded());
    NS_TEST_BOOL(res == 0);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("0x", res).Succeeded());
    NS_TEST_BOOL(res == 0);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("0", res).Succeeded());
    NS_TEST_BOOL(res == 0);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("0x0", res).Succeeded());
    NS_TEST_BOOL(res == 0);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("a", res).Succeeded());
    NS_TEST_BOOL(res == 10);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("0xb", res).Succeeded());
    NS_TEST_BOOL(res == 11);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("000c", res).Succeeded());
    NS_TEST_BOOL(res == 12);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("AA", res).Succeeded());
    NS_TEST_BOOL(res == 170);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("aAjbB", res).Failed());

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("aAbB", res).Succeeded());
    NS_TEST_BOOL(res == 43707);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("FFFFffff", res).Succeeded());
    NS_TEST_BOOL(res == 4294967295);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("0000FFFFffff", res).Succeeded());
    NS_TEST_BOOL(res == 4294967295);

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("0xfffffffffffffffy", res).Failed());

    NS_TEST_BOOL(nsConversionUtils::ConvertHexStringToUInt64("0xffffffffffffffffy", res).Succeeded());
    NS_TEST_BOOL(res == 0xFFFFFFFFFFFFFFFFllu);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ConvertBinaryToHex and ConvertHexStringToBinary")
  {
    nsDynamicArray<nsUInt8> binary;
    binary.SetCountUninitialized(1024);

    nsRandom r;
    r.InitializeFromCurrentTime();

    for (auto& val : binary)
    {
      val = static_cast<nsUInt8>(r.UIntInRange(256u));
    }

    nsStringBuilder sHex;
    nsConversionUtils::ConvertBinaryToHex(binary.GetData(), binary.GetCount(), [&sHex](const char* s)
      { sHex.Append(s); });

    nsDynamicArray<nsUInt8> binary2;
    binary2.SetCountUninitialized(1024);

    nsConversionUtils::ConvertHexToBinary(sHex, binary2.GetData(), binary2.GetCount());

    NS_TEST_BOOL(binary == binary2);
  }


  NS_TEST_BLOCK(nsTestBlock::Enabled, "ExtractFloatsFromString")
  {
    float v[16];

    const char* szText = "This 1 is 2.3 or 3.141 tests in 1.2 strings, maybe 4.5,6.78or9.101!";

    nsMemoryUtils::ZeroFill(v, 16);
    NS_TEST_INT(nsConversionUtils::ExtractFloatsFromString(szText, 0, v), 0);
    NS_TEST_FLOAT(v[0], 0.0f, 0.0f);

    nsMemoryUtils::ZeroFill(v, 16);
    NS_TEST_INT(nsConversionUtils::ExtractFloatsFromString(szText, 3, v), 3);
    NS_TEST_FLOAT(v[0], 1.0f, 0.0001f);
    NS_TEST_FLOAT(v[1], 2.3f, 0.0001f);
    NS_TEST_FLOAT(v[2], 3.141f, 0.0001f);
    NS_TEST_FLOAT(v[3], 0.0f, 0.0f);

    nsMemoryUtils::ZeroFill(v, 16);
    NS_TEST_INT(nsConversionUtils::ExtractFloatsFromString(szText, 6, v), 6);
    NS_TEST_FLOAT(v[0], 1.0f, 0.0001f);
    NS_TEST_FLOAT(v[1], 2.3f, 0.0001f);
    NS_TEST_FLOAT(v[2], 3.141f, 0.0001f);
    NS_TEST_FLOAT(v[3], 1.2f, 0.0001f);
    NS_TEST_FLOAT(v[4], 4.5f, 0.0001f);
    NS_TEST_FLOAT(v[5], 6.78f, 0.0001f);
    NS_TEST_FLOAT(v[6], 0.0f, 0.0f);

    nsMemoryUtils::ZeroFill(v, 16);
    NS_TEST_INT(nsConversionUtils::ExtractFloatsFromString(szText, 10, v), 7);
    NS_TEST_FLOAT(v[0], 1.0f, 0.0001f);
    NS_TEST_FLOAT(v[1], 2.3f, 0.0001f);
    NS_TEST_FLOAT(v[2], 3.141f, 0.0001f);
    NS_TEST_FLOAT(v[3], 1.2f, 0.0001f);
    NS_TEST_FLOAT(v[4], 4.5f, 0.0001f);
    NS_TEST_FLOAT(v[5], 6.78f, 0.0001f);
    NS_TEST_FLOAT(v[6], 9.101f, 0.0001f);
    NS_TEST_FLOAT(v[7], 0.0f, 0.0f);
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "ConvertStringToUuid and IsStringUuid")
  {
    nsUuid guid;
    nsStringBuilder sGuid;

    for (nsUInt32 i = 0; i < 100; ++i)
    {
      guid = nsUuid::MakeUuid();

      nsConversionUtils::ToString(guid, sGuid);

      NS_TEST_BOOL(nsConversionUtils::IsStringUuid(sGuid));

      nsUuid guid2 = nsConversionUtils::ConvertStringToUuid(sGuid);

      NS_TEST_BOOL(guid == guid2);
    }
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetColorName")
  {
    NS_TEST_STRING(nsString(nsConversionUtils::GetColorName(nsColorGammaUB(1, 2, 3))), "#010203");
    NS_TEST_STRING(nsString(nsConversionUtils::GetColorName(nsColorGammaUB(10, 20, 30, 40))), "#0A141E28");
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetColorByName")
  {
    NS_TEST_BOOL(nsConversionUtils::GetColorByName("#010203") == nsColorGammaUB(1, 2, 3));
    NS_TEST_BOOL(nsConversionUtils::GetColorByName("#0A141E28") == nsColorGammaUB(10, 20, 30, 40));

    NS_TEST_BOOL(nsConversionUtils::GetColorByName("#010203") == nsColorGammaUB(1, 2, 3));
    NS_TEST_BOOL(nsConversionUtils::GetColorByName("#0a141e28") == nsColorGammaUB(10, 20, 30, 40));
  }

  NS_TEST_BLOCK(nsTestBlock::Enabled, "GetColorByName and GetColorName")
  {
#define Check(name)                                                                  \
  {                                                                                  \
    bool valid = false;                                                              \
    const nsColor c = nsConversionUtils::GetColorByName(NS_STRINGIZE(name), &valid); \
    NS_TEST_BOOL(valid);                                                             \
    nsString sName = nsConversionUtils::GetColorName(c);                             \
    NS_TEST_STRING(sName, NS_STRINGIZE(name));                                       \
  }

#define Check2(name, otherName)                                                      \
  {                                                                                  \
    bool valid = false;                                                              \
    const nsColor c = nsConversionUtils::GetColorByName(NS_STRINGIZE(name), &valid); \
    NS_TEST_BOOL(valid);                                                             \
    nsString sName = nsConversionUtils::GetColorName(c);                             \
    NS_TEST_STRING(sName, NS_STRINGIZE(otherName));                                  \
  }

    Check(AliceBlue);
    Check(AntiqueWhite);
    Check(Aqua);
    Check(Aquamarine);
    Check(Azure);
    Check(Beige);
    Check(Bisque);
    Check(Black);
    Check(BlanchedAlmond);
    Check(Blue);
    Check(BlueViolet);
    Check(Brown);
    Check(BurlyWood);
    Check(CadetBlue);
    Check(Chartreuse);
    Check(Chocolate);
    Check(Coral);
    Check(CornflowerBlue); // The Original!
    Check(Cornsilk);
    Check(Crimson);
    Check2(Cyan, Aqua);
    Check(DarkBlue);
    Check(DarkCyan);
    Check(DarkGoldenRod);
    Check(DarkGray);
    Check2(DarkGrey, DarkGray);
    Check(DarkGreen);
    Check(DarkKhaki);
    Check(DarkMagenta);
    Check(DarkOliveGreen);
    Check(DarkOrange);
    Check(DarkOrchid);
    Check(DarkRed);
    Check(DarkSalmon);
    Check(DarkSeaGreen);
    Check(DarkSlateBlue);
    Check(DarkSlateGray);
    Check2(DarkSlateGrey, DarkSlateGray);
    Check(DarkTurquoise);
    Check(DarkViolet);
    Check(DeepPink);
    Check(DeepSkyBlue);
    Check(DimGray);
    Check2(DimGrey, DimGray);
    Check(DodgerBlue);
    Check(FireBrick);
    Check(FloralWhite);
    Check(ForestGreen);
    Check(Fuchsia);
    Check(Gainsboro);
    Check(GhostWhite);
    Check(Gold);
    Check(GoldenRod);
    Check(Gray);
    Check2(Grey, Gray);
    Check(Green);
    Check(GreenYellow);
    Check(HoneyDew);
    Check(HotPink);
    Check(IndianRed);
    Check(Indigo);
    Check(Ivory);
    Check(Khaki);
    Check(Lavender);
    Check(LavenderBlush);
    Check(LawnGreen);
    Check(LemonChiffon);
    Check(LightBlue);
    Check(LightCoral);
    Check(LightCyan);
    Check(LightGoldenRodYellow);
    Check(LightGray);
    Check2(LightGrey, LightGray);
    Check(LightGreen);
    Check(LightPink);
    Check(LightSalmon);
    Check(LightSeaGreen);
    Check(LightSkyBlue);
    Check(LightSlateGray);
    Check2(LightSlateGrey, LightSlateGray);
    Check(LightSteelBlue);
    Check(LightYellow);
    Check(Lime);
    Check(LimeGreen);
    Check(Linen);
    Check2(Magenta, Fuchsia);
    Check(Maroon);
    Check(MediumAquaMarine);
    Check(MediumBlue);
    Check(MediumOrchid);
    Check(MediumPurple);
    Check(MediumSeaGreen);
    Check(MediumSlateBlue);
    Check(MediumSpringGreen);
    Check(MediumTurquoise);
    Check(MediumVioletRed);
    Check(MidnightBlue);
    Check(MintCream);
    Check(MistyRose);
    Check(Moccasin);
    Check(NavajoWhite);
    Check(Navy);
    Check(OldLace);
    Check(Olive);
    Check(OliveDrab);
    Check(Orange);
    Check(OrangeRed);
    Check(Orchid);
    Check(PaleGoldenRod);
    Check(PaleGreen);
    Check(PaleTurquoise);
    Check(PaleVioletRed);
    Check(PapayaWhip);
    Check(PeachPuff);
    Check(Peru);
    Check(Pink);
    Check(Plum);
    Check(PowderBlue);
    Check(Purple);
    Check(RebeccaPurple);
    Check(Red);
    Check(RosyBrown);
    Check(RoyalBlue);
    Check(SaddleBrown);
    Check(Salmon);
    Check(SandyBrown);
    Check(SeaGreen);
    Check(SeaShell);
    Check(Sienna);
    Check(Silver);
    Check(SkyBlue);
    Check(SlateBlue);
    Check(SlateGray);
    Check2(SlateGrey, SlateGray);
    Check(Snow);
    Check(SpringGreen);
    Check(SteelBlue);
    Check(Tan);
    Check(Teal);
    Check(Thistle);
    Check(Tomato);
    Check(Turquoise);
    Check(Violet);
    Check(Wheat);
    Check(White);
    Check(WhiteSmoke);
    Check(Yellow);
    Check(YellowGreen);
  }
}
