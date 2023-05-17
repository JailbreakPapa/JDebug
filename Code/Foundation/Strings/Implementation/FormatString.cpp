#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Rational.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Variant.h>

wdFormatString::wdFormatString(const wdStringBuilder& s)
{
  m_szString = s.GetData();
}

void wdFormatString::SBAppendView(wdStringBuilder& sb, const wdStringView& sub)
{
  sb.Append(sub);
}

void wdFormatString::SBClear(wdStringBuilder& sb)
{
  sb.Clear();
}

void wdFormatString::SBAppendChar(wdStringBuilder& sb, wdUInt32 uiChar)
{
  sb.Append(uiChar);
}

const char* wdFormatString::SBReturn(wdStringBuilder& sb)
{
  return sb.GetData();
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgI& arg)
{
  wdUInt32 writepos = 0;
  wdStringUtils::OutputFormattedInt(szTmp, uiLength, writepos, arg.m_Value, arg.m_uiWidth, arg.m_bPadWithZeros, arg.m_uiBase);
  szTmp[writepos] = '\0';
  return wdStringView(szTmp, szTmp + writepos);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdInt64 iArg)
{
  wdUInt32 writepos = 0;
  wdStringUtils::OutputFormattedInt(szTmp, uiLength, writepos, iArg, 1, false, 10);
  szTmp[writepos] = '\0';
  return wdStringView(szTmp, szTmp + writepos);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdInt32 iArg)
{
  return BuildString(szTmp, uiLength, (wdInt64)iArg);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgU& arg)
{
  wdUInt32 writepos = 0;
  wdStringUtils::OutputFormattedUInt(szTmp, uiLength, writepos, arg.m_Value, arg.m_uiWidth, arg.m_bPadWithZeros, arg.m_uiBase, arg.m_bUpperCase);
  szTmp[writepos] = '\0';
  return wdStringView(szTmp, szTmp + writepos);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdUInt64 uiArg)
{
  wdUInt32 writepos = 0;
  wdStringUtils::OutputFormattedUInt(szTmp, uiLength, writepos, uiArg, 1, false, 10, false);
  szTmp[writepos] = '\0';
  return wdStringView(szTmp, szTmp + writepos);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdUInt32 uiArg)
{
  return BuildString(szTmp, uiLength, (wdUInt64)uiArg);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgF& arg)
{
  wdUInt32 writepos = 0;
  wdStringUtils::OutputFormattedFloat(szTmp, uiLength, writepos, arg.m_Value, arg.m_uiWidth, arg.m_bPadWithZeros, arg.m_iPrecision, arg.m_bScientific);
  szTmp[writepos] = '\0';
  return wdStringView(szTmp, szTmp + writepos);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, double fArg)
{
  wdUInt32 writepos = 0;
  wdStringUtils::OutputFormattedFloat(szTmp, uiLength, writepos, fArg, 1, false, -1, false);
  szTmp[writepos] = '\0';
  return wdStringView(szTmp, szTmp + writepos);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, bool bArg)
{
  if (bArg)
    return "true";

  return "false";
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const char* szArg)
{
  return szArg;
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wchar_t* pArg)
{
  const char* start = szTmp;
  if (pArg != nullptr)
  {
    // Code points in UTF-8 can be up to 4 byte, so the end pointer is 3 byte "earlier" than for
    // for a single byte character. One byte for trailing zero is already accounted for in uiLength.
    const char* tmpEnd = szTmp + uiLength - 3u;
    while (*pArg != '\0' && szTmp < tmpEnd)
    {
      // decode utf8 to utf32
      const wdUInt32 uiUtf32 = wdUnicodeUtils::DecodeWCharToUtf32(pArg);

      // encode utf32 to wchar_t
      wdUnicodeUtils::EncodeUtf32ToUtf8(uiUtf32, szTmp);
    }
  }

  // Append terminator. As the extra byte for trailing zero is accounted for in uiLength, this is safe.
  *szTmp = '\0';

  return start;
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdString& sArg)
{
  return wdStringView(sArg.GetData(), sArg.GetData() + sArg.GetElementCount());
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdHashedString& sArg)
{
  return wdStringView(sArg.GetData(), sArg.GetData() + sArg.GetString().GetElementCount());
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdStringBuilder& sArg)
{
  return wdStringView(sArg.GetData(), sArg.GetData() + sArg.GetElementCount());
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdUntrackedString& sArg)
{
  return wdStringView(sArg.GetData(), sArg.GetData() + sArg.GetElementCount());
}

const wdStringView& BuildString(char* szTmp, wdUInt32 uiLength, const wdStringView& sArg)
{
  return sArg;
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgC& arg)
{
  szTmp[0] = arg.m_Value;
  szTmp[1] = '\0';

  return wdStringView(&szTmp[0], &szTmp[1]);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgP& arg)
{
  wdStringUtils::snprintf(szTmp, uiLength, "%p", arg.m_Value);
  return wdStringView(szTmp);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdResult arg)
{
  if (arg.Failed())
    return "<failed>";
  else
    return "<succeeded>";
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdVariant& arg)
{
  wdString sString = arg.ConvertTo<wdString>();
  wdStringUtils::snprintf(szTmp, uiLength, "%s", sString.GetData());
  return wdStringView(szTmp);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdAngle& arg)
{
  wdUInt32 writepos = 0;
  wdStringUtils::OutputFormattedFloat(szTmp, uiLength - 2, writepos, arg.GetDegree(), 1, false, 1, false);

  // Utf-8 representation of the degree sign
  szTmp[writepos + 0] = (char)0xC2;
  szTmp[writepos + 1] = (char)0xB0;
  szTmp[writepos + 2] = '\0';

  return wdStringView(szTmp, szTmp + writepos + 2);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdRational& arg)
{
  wdUInt32 writepos = 0;

  if (arg.IsIntegral())
  {
    wdStringUtils::OutputFormattedInt(szTmp, uiLength, writepos, arg.GetIntegralResult(), 1, false, 10);

    return wdStringView(szTmp, szTmp + writepos);
  }
  else
  {
    wdStringUtils::snprintf(szTmp, uiLength, "%i/%i", arg.GetNumerator(), arg.GetDenominator());

    return wdStringView(szTmp);
  }
}

wdStringView BuildString(char* pTmp, wdUInt32 uiLength, const wdTime& arg)
{
  wdUInt32 writepos = 0;

  const double fAbsSec = wdMath::Abs(arg.GetSeconds());

  if (fAbsSec < 0.000001)
  {
    wdStringUtils::OutputFormattedFloat(pTmp, uiLength - 5, writepos, arg.GetNanoseconds(), 1, false, 1, false, true);
    // szTmp[writepos++] = ' ';
    pTmp[writepos++] = 'n';
    pTmp[writepos++] = 's';
  }
  else if (fAbsSec < 0.001)
  {
    wdStringUtils::OutputFormattedFloat(pTmp, uiLength - 5, writepos, arg.GetMicroseconds(), 1, false, 1, false, true);

    // szTmp[writepos++] = ' ';
    // Utf-8 representation of the microsecond (us) sign
    pTmp[writepos++] = (char)0xC2;
    pTmp[writepos++] = (char)0xB5;
    pTmp[writepos++] = 's';
  }
  else if (fAbsSec < 1.0)
  {
    wdStringUtils::OutputFormattedFloat(pTmp, uiLength - 5, writepos, arg.GetMilliseconds(), 1, false, 1, false, true);

    // tmp[writepos++] = ' ';
    pTmp[writepos++] = 'm';
    pTmp[writepos++] = 's';
  }
  else if (fAbsSec < 60.0)
  {
    wdStringUtils::OutputFormattedFloat(pTmp, uiLength - 5, writepos, arg.GetSeconds(), 1, false, 1, false, true);

    // szTmp[writepos++] = ' ';
    pTmp[writepos++] = 's';
    pTmp[writepos++] = 'e';
    pTmp[writepos++] = 'c';
  }
  else if (fAbsSec < 60.0 * 60.0)
  {
    double tRem = fAbsSec;

    wdInt32 iMin = static_cast<wdInt32>(wdMath::Trunc(tRem / 60.0));
    tRem -= iMin * 60;
    iMin *= wdMath::Sign(static_cast<wdInt32>(arg.GetSeconds()));

    const wdInt32 iSec = static_cast<wdInt32>(wdMath::Trunc(tRem));

    writepos = wdStringUtils::snprintf(pTmp, uiLength, "%imin %isec", iMin, iSec);
  }
  else
  {
    double tRem = fAbsSec;

    wdInt32 iHrs = static_cast<wdInt32>(wdMath::Trunc(tRem / (60.0 * 60.0)));
    tRem -= iHrs * 60 * 60;
    iHrs *= wdMath::Sign(static_cast<wdInt32>(arg.GetSeconds()));

    const wdInt32 iMin = static_cast<wdInt32>(wdMath::Trunc(tRem / 60.0));
    tRem -= iMin * 60;

    const wdInt32 iSec = static_cast<wdInt32>(wdMath::Trunc(tRem));

    writepos = wdStringUtils::snprintf(pTmp, uiLength, "%ih %imin %isec", iHrs, iMin, iSec);
  }

  pTmp[writepos] = '\0';
  return wdStringView(pTmp, pTmp + writepos);
}

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgHumanReadable& arg)
{
  wdUInt32 suffixIndex = 0;
  wdUInt64 divider = 1;
  double absValue = wdMath::Abs(arg.m_Value);
  while (absValue / divider >= arg.m_Base && suffixIndex < arg.m_SuffixCount - 1)
  {
    divider *= arg.m_Base;
    ++suffixIndex;
  }

  wdUInt32 writepos = 0;
  if (divider == 1 && wdMath::Fraction(arg.m_Value) == 0.0)
  {
    wdStringUtils::OutputFormattedInt(szTmp, uiLength, writepos, static_cast<wdInt64>(arg.m_Value), 1, false, 10);
  }
  else
  {
    wdStringUtils::OutputFormattedFloat(szTmp, uiLength, writepos, arg.m_Value / divider, 1, false, 2, false);
  }
  wdStringUtils::Copy(szTmp + writepos, uiLength - writepos, arg.m_Suffixes[suffixIndex]);

  return wdStringView(szTmp);
}

wdArgSensitive::BuildStringCallback wdArgSensitive::s_BuildStringCB = nullptr;

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgSensitive& arg)
{
  if (wdArgSensitive::s_BuildStringCB)
  {
    return wdArgSensitive::s_BuildStringCB(szTmp, uiLength, arg);
  }

  return arg.m_sSensitiveInfo;
}

wdStringView wdArgSensitive::BuildString_SensitiveUserData_Hash(char* szTmp, wdUInt32 uiLength, const wdArgSensitive& arg)
{
  const wdUInt32 len = arg.m_sSensitiveInfo.GetElementCount();

  if (len == 0)
    return wdStringView();

  if (!wdStringUtils::IsNullOrEmpty(arg.m_szContext))
  {
    wdStringUtils::snprintf(
      szTmp, uiLength, "sud:%s#%08x($%u)", arg.m_szContext, wdHashingUtils::xxHash32(arg.m_sSensitiveInfo.GetStartPointer(), len), len);
  }
  else
  {
    wdStringUtils::snprintf(szTmp, uiLength, "sud:#%08x($%u)", wdHashingUtils::xxHash32(arg.m_sSensitiveInfo.GetStartPointer(), len), len);
  }

  return szTmp;
}

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgErrorCode& arg)
{
  LPVOID lpMsgBuf = nullptr;
  if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, arg.m_ErrorCode,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&lpMsgBuf, 0, nullptr) == 0)
  {
    DWORD err = GetLastError();
    wdStringUtils::snprintf(szTmp, uiLength, "%i (FormatMessageW failed with error code %i)", arg.m_ErrorCode, err);
    return wdStringView(szTmp);
  }

  LPWSTR pCRLF = wcschr((LPWSTR)lpMsgBuf, L'\r');
  if (pCRLF != nullptr)
  {
    // remove the \r\n that FormatMessageW always appends
    *pCRLF = L'\0';
  }

  // we need a bigger boat
  static thread_local char FullMessage[256];

  wdStringUtils::snprintf(FullMessage, WD_ARRAY_SIZE(FullMessage), "%i (\"%s\")", arg.m_ErrorCode, wdStringUtf8((LPWSTR)lpMsgBuf).GetData());
  LocalFree(lpMsgBuf);
  return wdStringView(FullMessage);
}
#endif

WD_STATICLINK_FILE(Foundation, Foundation_Strings_Implementation_FormatString);
