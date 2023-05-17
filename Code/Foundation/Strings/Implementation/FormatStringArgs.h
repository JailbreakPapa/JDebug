#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/StringView.h>

class wdStringBuilder;
class wdVariant;
class wdAngle;
class wdRational;
struct wdTime;

struct wdArgI
{
  inline explicit wdArgI(wdInt64 value, wdUInt8 uiWidth = 1, bool bPadWithZeros = false, wdUInt8 uiBase = 10)
    : m_Value(value)
    , m_uiWidth(uiWidth)
    , m_bPadWithZeros(bPadWithZeros)
    , m_uiBase(uiBase)
  {
  }

  wdInt64 m_Value;
  wdUInt8 m_uiWidth;
  bool m_bPadWithZeros;
  wdUInt8 m_uiBase;
};

struct wdArgU
{
  inline explicit wdArgU(wdUInt64 value, wdUInt8 uiWidth = 1, bool bPadWithZeros = false, wdUInt8 uiBase = 10, bool bUpperCase = false)
    : m_Value(value)
    , m_uiWidth(uiWidth)
    , m_bPadWithZeros(bPadWithZeros)
    , m_bUpperCase(bUpperCase)
    , m_uiBase(uiBase)
  {
  }

  wdUInt64 m_Value;
  wdUInt8 m_uiWidth;
  bool m_bPadWithZeros;
  bool m_bUpperCase;
  wdUInt8 m_uiBase;
};

struct wdArgF
{
  inline explicit wdArgF(double value, wdInt8 iPrecision = -1, bool bScientific = false, wdUInt8 uiWidth = 1, bool bPadWithZeros = false)
    : m_Value(value)
    , m_uiWidth(uiWidth)
    , m_bPadWithZeros(bPadWithZeros)
    , m_bScientific(bScientific)
    , m_iPrecision(iPrecision)
  {
  }

  double m_Value;
  wdUInt8 m_uiWidth;
  bool m_bPadWithZeros;
  bool m_bScientific;
  wdInt8 m_iPrecision;
};

struct wdArgC
{
  inline explicit wdArgC(char value)
    : m_Value(value)
  {
  }

  char m_Value;
};

struct wdArgP
{
  inline explicit wdArgP(const void* value)
    : m_Value(value)
  {
  }

  const void* m_Value;
};


/// \brief Formats a given number such that it will be in format [0, base){suffix} with suffix
/// representing a power of base. Resulting numbers are output with a precision of 2 fractional digits
/// and fractional digits are subject to rounding, so numbers at the upper boundary of [0, base)
/// may be rounded up to the next power of base.
///
/// E.g.: For the default case base is 1000 and suffixes are the SI unit suffixes (i.e. K for kilo, M for mega etc.)
///       Thus 0 remains 0, 1 remains 1, 1000 becomes 1.00K, and 2534000 becomes 2.53M. But 999.999 will
///       end up being displayed as 1000.00K for base 1000 due to rounding.
struct wdArgHumanReadable
{
  inline wdArgHumanReadable(const double value, const wdUInt64 uiBase, const char* const* const pSuffixes, wdUInt32 uiSuffixCount)
    : m_Value(value)
    , m_Base(uiBase)
    , m_Suffixes(pSuffixes)
    , m_SuffixCount(uiSuffixCount)
  {
  }

  inline wdArgHumanReadable(const wdInt64 value, const wdUInt64 uiBase, const char* const* const pSuffixes, wdUInt32 uiSuffixCount)
    : wdArgHumanReadable(static_cast<double>(value), uiBase, pSuffixes, uiSuffixCount)
  {
  }

  inline explicit wdArgHumanReadable(const double value)
    : wdArgHumanReadable(value, 1000u, m_DefaultSuffixes, WD_ARRAY_SIZE(m_DefaultSuffixes))
  {
  }

  inline explicit wdArgHumanReadable(const wdInt64 value)
    : wdArgHumanReadable(static_cast<double>(value), 1000u, m_DefaultSuffixes, WD_ARRAY_SIZE(m_DefaultSuffixes))
  {
  }

  const double m_Value;
  const wdUInt64 m_Base;
  const char* const* const m_Suffixes;
  const char* const m_DefaultSuffixes[6] = {"", "K", "M", "G", "T", "P"};
  const wdUInt32 m_SuffixCount;
};

struct wdArgFileSize : public wdArgHumanReadable
{
  inline explicit wdArgFileSize(const wdUInt64 value)
    : wdArgHumanReadable(static_cast<double>(value), 1024u, m_ByteSuffixes, WD_ARRAY_SIZE(m_ByteSuffixes))
  {
  }

  const char* const m_ByteSuffixes[6] = {"B", "KB", "MB", "GB", "TB", "PB"};
};

#if WD_ENABLED(WD_PLATFORM_WINDOWS)
struct wdArgErrorCode
{
  inline explicit wdArgErrorCode(wdUInt32 uiErrorCode)
    : m_ErrorCode(uiErrorCode)
  {
  }

  wdUInt32 m_ErrorCode;
};
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgErrorCode& arg);

#endif

/// \brief Wraps a string that may contain sensitive information, such as user file paths.
///
/// The application can specify a function to scramble this type of information. By default no such function is set.
/// A general purpose function is provided with 'BuildString_SensitiveUserData_Hash()'
///
/// \param sSensitiveInfo The information that may need to be scrambled.
/// \param szContext A custom string to identify the 'context', ie. what type of sensitive data is being scrambled.
///        This may be passed through unmodified, or can guide the scrambling function to choose how to output the sensitive data.
struct wdArgSensitive
{
  inline explicit wdArgSensitive(const wdStringView& sSensitiveInfo, const char* szContext = nullptr)
    : m_sSensitiveInfo(sSensitiveInfo)
    , m_szContext(szContext)
  {
  }

  const wdStringView m_sSensitiveInfo;
  const char* m_szContext;

  using BuildStringCallback = wdStringView (*)(char*, wdUInt32, const wdArgSensitive&);
  WD_FOUNDATION_DLL static BuildStringCallback s_BuildStringCB;

  /// \brief Set s_BuildStringCB to this function to enable scrambling of sensitive data.
  WD_FOUNDATION_DLL static wdStringView BuildString_SensitiveUserData_Hash(char* szTmp, wdUInt32 uiLength, const wdArgSensitive& arg);
};

WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgI& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdInt64 iArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdInt32 iArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgU& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdUInt64 uiArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdUInt32 uiArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgF& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, double fArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, bool bArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const char* szArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wchar_t* pArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdStringBuilder& sArg);
WD_FOUNDATION_DLL const wdStringView& BuildString(char* szTmp, wdUInt32 uiLength, const wdStringView& sArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgC& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgP& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, wdResult arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdVariant& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdAngle& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdRational& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgHumanReadable& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdTime& arg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdArgSensitive& arg);


#if WD_ENABLED(WD_COMPILER_GCC) || WD_ENABLED(WD_COMPILER_CLANG)

// on these platforms "long int" is a different type from "long long int"

WD_ALWAYS_INLINE wdStringView BuildString(char* szTmp, wdUInt32 uiLength, long int iArg)
{
  return BuildString(szTmp, uiLength, static_cast<wdInt64>(iArg));
}

WD_ALWAYS_INLINE wdStringView BuildString(char* szTmp, wdUInt32 uiLength, unsigned long int uiArg)
{
  return BuildString(szTmp, uiLength, static_cast<wdUInt64>(uiArg));
}

#endif
