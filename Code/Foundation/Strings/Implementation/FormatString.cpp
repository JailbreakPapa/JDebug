#include <Foundation/FoundationPCH.h>

#include <Foundation/Math/Rational.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/HashedString.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Strings/StringBuilder.h>
#include <Foundation/Types/Variant.h>

nsFormatString::nsFormatString(const nsStringBuilder& s)
{
  m_sString = s.GetView();
}

const char* nsFormatString::GetTextCStr(nsStringBuilder& out_sString) const
{
  out_sString = m_sString;
  return out_sString.GetData();
}

nsStringView nsFormatString::BuildFormattedText(nsStringBuilder& ref_sStorage, nsStringView* pArgs, nsUInt32 uiNumArgs) const
{
  nsStringView sString = m_sString;

  nsUInt32 uiLastParam = -1;

  ref_sStorage.Clear();
  while (!sString.IsEmpty())
  {
    if (sString.StartsWith("%"))
    {
      if (sString.TrimWordStart("%%"))
      {
        ref_sStorage.Append("%"_nssv);
      }
      else
      {
        NS_ASSERT_DEBUG(false, "Single percentage signs are not allowed in nsFormatString. Did you forgot to migrate a printf-style "
                               "string? Use double percentage signs for the actual character.");
      }
    }
    else if (sString.GetElementCount() >= 3 && *sString.GetStartPointer() == '{' && *(sString.GetStartPointer() + 1) >= '0' && *(sString.GetStartPointer() + 1) <= '9' && *(sString.GetStartPointer() + 2) == '}')
    {
      uiLastParam = *(sString.GetStartPointer() + 1) - '0';
      NS_ASSERT_DEV(uiLastParam < uiNumArgs, "Too many placeholders in format string");

      if (uiLastParam < uiNumArgs)
      {
        ref_sStorage.Append(pArgs[uiLastParam]);
      }

      sString.ChopAwayFirstCharacterAscii();
      sString.ChopAwayFirstCharacterAscii();
      sString.ChopAwayFirstCharacterAscii();
    }
    else if (sString.TrimWordStart("{}"))
    {
      ++uiLastParam;
      NS_ASSERT_DEV(uiLastParam < uiNumArgs, "Too many placeholders in format string");

      if (uiLastParam < uiNumArgs)
      {
        ref_sStorage.Append(pArgs[uiLastParam]);
      }
    }
    else
    {
      const nsUInt32 character = sString.GetCharacter();
      ref_sStorage.Append(character);
      sString.ChopAwayFirstCharacterUtf8();
    }
  }

  return ref_sStorage.GetView();
}

//////////////////////////////////////////////////////////////////////////

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgI& arg)
{
  nsUInt32 writepos = 0;
  nsStringUtils::OutputFormattedInt(szTmp, uiLength, writepos, arg.m_Value, arg.m_uiWidth, arg.m_bPadWithZeros, arg.m_uiBase);
  szTmp[writepos] = '\0';
  return nsStringView(szTmp, szTmp + writepos);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsInt64 iArg)
{
  nsUInt32 writepos = 0;
  nsStringUtils::OutputFormattedInt(szTmp, uiLength, writepos, iArg, 1, false, 10);
  szTmp[writepos] = '\0';
  return nsStringView(szTmp, szTmp + writepos);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsInt32 iArg)
{
  return BuildString(szTmp, uiLength, (nsInt64)iArg);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgU& arg)
{
  nsUInt32 writepos = 0;
  nsStringUtils::OutputFormattedUInt(szTmp, uiLength, writepos, arg.m_Value, arg.m_uiWidth, arg.m_bPadWithZeros, arg.m_uiBase, arg.m_bUpperCase);
  szTmp[writepos] = '\0';
  return nsStringView(szTmp, szTmp + writepos);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsUInt64 uiArg)
{
  nsUInt32 writepos = 0;
  nsStringUtils::OutputFormattedUInt(szTmp, uiLength, writepos, uiArg, 1, false, 10, false);
  szTmp[writepos] = '\0';
  return nsStringView(szTmp, szTmp + writepos);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsUInt32 uiArg)
{
  return BuildString(szTmp, uiLength, (nsUInt64)uiArg);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgF& arg)
{
  nsUInt32 writepos = 0;
  nsStringUtils::OutputFormattedFloat(szTmp, uiLength, writepos, arg.m_Value, arg.m_uiWidth, arg.m_bPadWithZeros, arg.m_iPrecision, arg.m_bScientific);
  szTmp[writepos] = '\0';
  return nsStringView(szTmp, szTmp + writepos);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, double fArg)
{
  nsUInt32 writepos = 0;
  nsStringUtils::OutputFormattedFloat(szTmp, uiLength, writepos, fArg, 1, false, -1, false);
  szTmp[writepos] = '\0';
  return nsStringView(szTmp, szTmp + writepos);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, bool bArg)
{
  if (bArg)
    return "true";

  return "false";
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const char* szArg)
{
  return szArg;
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const wchar_t* pArg)
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
      const nsUInt32 uiUtf32 = nsUnicodeUtils::DecodeWCharToUtf32(pArg);

      // encode utf32 to wchar_t
      nsUnicodeUtils::EncodeUtf32ToUtf8(uiUtf32, szTmp);
    }
  }

  // Append terminator. As the extra byte for trailing zero is accounted for in uiLength, this is safe.
  *szTmp = '\0';

  return start;
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsString& sArg)
{
  return nsStringView(sArg.GetData(), sArg.GetData() + sArg.GetElementCount());
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsHashedString& sArg)
{
  return nsStringView(sArg.GetData(), sArg.GetData() + sArg.GetString().GetElementCount());
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsStringBuilder& sArg)
{
  return nsStringView(sArg.GetData(), sArg.GetData() + sArg.GetElementCount());
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsUntrackedString& sArg)
{
  return nsStringView(sArg.GetData(), sArg.GetData() + sArg.GetElementCount());
}

const nsStringView& BuildString(char* szTmp, nsUInt32 uiLength, const nsStringView& sArg)
{
  return sArg;
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgC& arg)
{
  szTmp[0] = arg.m_Value;
  szTmp[1] = '\0';

  return nsStringView(&szTmp[0], &szTmp[1]);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgP& arg)
{
  nsStringUtils::snprintf(szTmp, uiLength, "%p", arg.m_Value);
  return nsStringView(szTmp);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsResult arg)
{
  if (arg.Failed())
    return "<failed>";
  else
    return "<succeeded>";
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsVariant& arg)
{
  nsString sString = arg.ConvertTo<nsString>();
  nsStringUtils::snprintf(szTmp, uiLength, "%s", sString.GetData());
  return nsStringView(szTmp);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsAngle& arg)
{
  nsUInt32 writepos = 0;
  nsStringUtils::OutputFormattedFloat(szTmp, uiLength - 2, writepos, arg.GetDegree(), 1, false, 1, false);

  // Utf-8 representation of the degree sign
  szTmp[writepos + 0] = /*(char)0xC2;*/ -62;
  szTmp[writepos + 1] = /*(char)0xB0;*/ -80;
  szTmp[writepos + 2] = '\0';

  return nsStringView(szTmp, szTmp + writepos + 2);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsRational& arg)
{
  nsUInt32 writepos = 0;

  if (arg.IsIntegral())
  {
    nsStringUtils::OutputFormattedInt(szTmp, uiLength, writepos, arg.GetIntegralResult(), 1, false, 10);

    return nsStringView(szTmp, szTmp + writepos);
  }
  else
  {
    nsStringUtils::snprintf(szTmp, uiLength, "%i/%i", arg.GetNumerator(), arg.GetDenominator());

    return nsStringView(szTmp);
  }
}

nsStringView BuildString(char* pTmp, nsUInt32 uiLength, const nsTime& arg)
{
  nsUInt32 writepos = 0;

  const double fAbsSec = nsMath::Abs(arg.GetSeconds());

  if (fAbsSec < 0.000001)
  {
    nsStringUtils::OutputFormattedFloat(pTmp, uiLength - 5, writepos, arg.GetNanoseconds(), 1, false, 1, false, true);
    // szTmp[writepos++] = ' ';
    pTmp[writepos++] = 'n';
    pTmp[writepos++] = 's';
  }
  else if (fAbsSec < 0.001)
  {
    nsStringUtils::OutputFormattedFloat(pTmp, uiLength - 5, writepos, arg.GetMicroseconds(), 1, false, 1, false, true);

    // szTmp[writepos++] = ' ';
    // Utf-8 representation of the microsecond (us) sign
    pTmp[writepos++] = /*(char)0xC2;*/ -62;
    pTmp[writepos++] = /*(char)0xB5;*/ -75;
    pTmp[writepos++] = 's';
  }
  else if (fAbsSec < 1.0)
  {
    nsStringUtils::OutputFormattedFloat(pTmp, uiLength - 5, writepos, arg.GetMilliseconds(), 1, false, 1, false, true);

    // tmp[writepos++] = ' ';
    pTmp[writepos++] = 'm';
    pTmp[writepos++] = 's';
  }
  else if (fAbsSec < 60.0)
  {
    nsStringUtils::OutputFormattedFloat(pTmp, uiLength - 5, writepos, arg.GetSeconds(), 1, false, 1, false, true);

    // szTmp[writepos++] = ' ';
    pTmp[writepos++] = 's';
    pTmp[writepos++] = 'e';
    pTmp[writepos++] = 'c';
  }
  else if (fAbsSec < 60.0 * 60.0)
  {
    double tRem = fAbsSec;

    nsInt32 iMin = static_cast<nsInt32>(nsMath::Trunc(tRem / 60.0));
    tRem -= iMin * 60;
    iMin *= nsMath::Sign(static_cast<nsInt32>(arg.GetSeconds()));

    const nsInt32 iSec = static_cast<nsInt32>(nsMath::Trunc(tRem));

    writepos = nsStringUtils::snprintf(pTmp, uiLength, "%imin %isec", iMin, iSec);
  }
  else
  {
    double tRem = fAbsSec;

    nsInt32 iHrs = static_cast<nsInt32>(nsMath::Trunc(tRem / (60.0 * 60.0)));
    tRem -= iHrs * 60 * 60;
    iHrs *= nsMath::Sign(static_cast<nsInt32>(arg.GetSeconds()));

    const nsInt32 iMin = static_cast<nsInt32>(nsMath::Trunc(tRem / 60.0));
    tRem -= iMin * 60;

    const nsInt32 iSec = static_cast<nsInt32>(nsMath::Trunc(tRem));

    writepos = nsStringUtils::snprintf(pTmp, uiLength, "%ih %imin %isec", iHrs, iMin, iSec);
  }

  pTmp[writepos] = '\0';
  return nsStringView(pTmp, pTmp + writepos);
}

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgHumanReadable& arg)
{
  nsUInt32 suffixIndex = 0;
  nsUInt64 divider = 1;
  double absValue = nsMath::Abs(arg.m_Value);
  while (absValue / divider >= arg.m_Base && suffixIndex < arg.m_SuffixCount - 1)
  {
    divider *= arg.m_Base;
    ++suffixIndex;
  }

  nsUInt32 writepos = 0;
  if (divider == 1 && nsMath::Fraction(arg.m_Value) == 0.0)
  {
    nsStringUtils::OutputFormattedInt(szTmp, uiLength, writepos, static_cast<nsInt64>(arg.m_Value), 1, false, 10);
  }
  else
  {
    nsStringUtils::OutputFormattedFloat(szTmp, uiLength, writepos, arg.m_Value / divider, 1, false, 2, false);
  }
  nsStringUtils::Copy(szTmp + writepos, uiLength - writepos, arg.m_Suffixes[suffixIndex]);

  return nsStringView(szTmp);
}

nsArgSensitive::BuildStringCallback nsArgSensitive::s_BuildStringCB = nullptr;

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgSensitive& arg)
{
  if (nsArgSensitive::s_BuildStringCB)
  {
    return nsArgSensitive::s_BuildStringCB(szTmp, uiLength, arg);
  }

  return arg.m_sSensitiveInfo;
}

nsStringView nsArgSensitive::BuildString_SensitiveUserData_Hash(char* szTmp, nsUInt32 uiLength, const nsArgSensitive& arg)
{
  const nsUInt32 len = arg.m_sSensitiveInfo.GetElementCount();

  if (len == 0)
    return nsStringView();

  if (!nsStringUtils::IsNullOrEmpty(arg.m_szContext))
  {
    nsStringUtils::snprintf(
      szTmp, uiLength, "sud:%s#%08x($%u)", arg.m_szContext, nsHashingUtils::xxHash32(arg.m_sSensitiveInfo.GetStartPointer(), len), len);
  }
  else
  {
    nsStringUtils::snprintf(szTmp, uiLength, "sud:#%08x($%u)", nsHashingUtils::xxHash32(arg.m_sSensitiveInfo.GetStartPointer(), len), len);
  }

  return szTmp;
}

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
#  include <Foundation/Basics/Platform/Win/IncludeWindows.h>

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgErrorCode& arg)
{
  LPVOID lpMsgBuf = nullptr;
  if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, arg.m_ErrorCode,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&lpMsgBuf, 0, nullptr) == 0)
  {
    DWORD err = GetLastError();
    nsStringUtils::snprintf(szTmp, uiLength, "%i (FormatMessageW failed with error code %i)", arg.m_ErrorCode, err);
    return nsStringView(szTmp);
  }

  LPWSTR pCRLF = wcschr((LPWSTR)lpMsgBuf, L'\r');
  if (pCRLF != nullptr)
  {
    // remove the \r\n that FormatMessageW always appends
    *pCRLF = L'\0';
  }

  // we need a bigger boat
  static thread_local char FullMessage[256];

  nsStringUtils::snprintf(FullMessage, NS_ARRAY_SIZE(FullMessage), "%i (\"%s\")", arg.m_ErrorCode, nsStringUtf8((LPWSTR)lpMsgBuf).GetData());
  LocalFree(lpMsgBuf);
  return nsStringView(FullMessage);
}
#endif

#if NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
#  include <string.h>

nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgErrno& arg)
{
  const char* szErrorMsg = std::strerror(arg.m_iErrno);
  nsStringUtils::snprintf(szTmp, uiLength, "%i (\"%s\")", arg.m_iErrno, szErrorMsg);
  return nsStringView(szTmp);
}
#endif
