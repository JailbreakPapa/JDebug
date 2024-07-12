#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/StringView.h>

class nsStringBuilder;
class nsVariant;
class nsAngle;
class nsRational;
struct nsTime;

struct nsArgI
{
  inline explicit nsArgI(nsInt64 value, nsUInt8 uiWidth = 1, bool bPadWithZeros = false, nsUInt8 uiBase = 10)
    : m_Value(value)
    , m_uiWidth(uiWidth)
    , m_bPadWithZeros(bPadWithZeros)
    , m_uiBase(uiBase)
  {
  }

  nsInt64 m_Value;
  nsUInt8 m_uiWidth;
  bool m_bPadWithZeros;
  nsUInt8 m_uiBase;
};

struct nsArgU
{
  inline explicit nsArgU(nsUInt64 value, nsUInt8 uiWidth = 1, bool bPadWithZeros = false, nsUInt8 uiBase = 10, bool bUpperCase = false)
    : m_Value(value)
    , m_uiWidth(uiWidth)
    , m_bPadWithZeros(bPadWithZeros)
    , m_bUpperCase(bUpperCase)
    , m_uiBase(uiBase)
  {
  }

  nsUInt64 m_Value;
  nsUInt8 m_uiWidth;
  bool m_bPadWithZeros;
  bool m_bUpperCase;
  nsUInt8 m_uiBase;
};

struct nsArgF
{
  inline explicit nsArgF(double value, nsInt8 iPrecision = -1, bool bScientific = false, nsUInt8 uiWidth = 1, bool bPadWithZeros = false)
    : m_Value(value)
    , m_uiWidth(uiWidth)
    , m_bPadWithZeros(bPadWithZeros)
    , m_bScientific(bScientific)
    , m_iPrecision(iPrecision)
  {
  }

  double m_Value;
  nsUInt8 m_uiWidth;
  bool m_bPadWithZeros;
  bool m_bScientific;
  nsInt8 m_iPrecision;
};

struct nsArgC
{
  inline explicit nsArgC(char value)
    : m_Value(value)
  {
  }

  char m_Value;
};

struct nsArgP
{
  inline explicit nsArgP(const void* value)
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
struct nsArgHumanReadable
{
  inline nsArgHumanReadable(const double value, const nsUInt64 uiBase, const char* const* const pSuffixes, nsUInt32 uiSuffixCount)
    : m_Value(value)
    , m_Base(uiBase)
    , m_Suffixes(pSuffixes)
    , m_SuffixCount(uiSuffixCount)
  {
  }

  inline nsArgHumanReadable(const nsInt64 value, const nsUInt64 uiBase, const char* const* const pSuffixes, nsUInt32 uiSuffixCount)
    : nsArgHumanReadable(static_cast<double>(value), uiBase, pSuffixes, uiSuffixCount)
  {
  }

  inline explicit nsArgHumanReadable(const double value)
    : nsArgHumanReadable(value, 1000u, m_DefaultSuffixes, NS_ARRAY_SIZE(m_DefaultSuffixes))
  {
  }

  inline explicit nsArgHumanReadable(const nsInt64 value)
    : nsArgHumanReadable(static_cast<double>(value), 1000u, m_DefaultSuffixes, NS_ARRAY_SIZE(m_DefaultSuffixes))
  {
  }

  const double m_Value;
  const nsUInt64 m_Base;
  const char* const* const m_Suffixes;
  const char* const m_DefaultSuffixes[6] = {"", "K", "M", "G", "T", "P"};
  const nsUInt32 m_SuffixCount;
};

struct nsArgFileSize : public nsArgHumanReadable
{
  inline explicit nsArgFileSize(const nsUInt64 value)
    : nsArgHumanReadable(static_cast<double>(value), 1024u, m_ByteSuffixes, NS_ARRAY_SIZE(m_ByteSuffixes))
  {
  }

  const char* const m_ByteSuffixes[6] = {"B", "KB", "MB", "GB", "TB", "PB"};
};

#if NS_ENABLED(NS_PLATFORM_WINDOWS)
/// \brief Converts a windows HRESULT into an error code and a human-readable error message.
/// Pass in `GetLastError()` function or an HRESULT from another error source. Be careful when printing multiple values, a function could clear `GetLastError` as a side-effect so it is best to store it in a temp variable before printing a complex error message.
/// \sa https://learn.microsoft.com/en-gb/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
struct nsArgErrorCode
{
  inline explicit nsArgErrorCode(nsUInt32 uiErrorCode)
    : m_ErrorCode(uiErrorCode)
  {
  }

  nsUInt32 m_ErrorCode;
};
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgErrorCode& arg);

#endif

#if NS_ENABLED(NS_PLATFORM_LINUX) || NS_ENABLED(NS_PLATFORM_ANDROID)
/// \brief Many Linux APIs will fill out error on failure. This converts the error into an error code and a human-readable error message.
/// Pass in the linux `errno` symbol. Be careful when printing multiple values, a function could clear `errno` as a side-effect so it is best to store it in a temp variable before printing a complex error message.
/// You may have to include #include <errno.h> use this.
/// \sa https://man7.org/linux/man-pages/man3/errno.3.html
struct nsArgErrno
{
  inline explicit nsArgErrno(nsInt32 iErrno)
    : m_iErrno(iErrno)
  {
  }

  nsInt32 m_iErrno;
};
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgErrno& arg);
#endif

/// \brief Wraps a string that may contain sensitive information, such as user file paths.
///
/// The application can specify a function to scramble this type of information. By default no such function is set.
/// A general purpose function is provided with 'BuildString_SensitiveUserData_Hash()'
///
/// \param sSensitiveInfo The information that may need to be scrambled.
/// \param szContext A custom string to identify the 'context', ie. what type of sensitive data is being scrambled.
///        This may be passed through unmodified, or can guide the scrambling function to choose how to output the sensitive data.
struct nsArgSensitive
{
  inline explicit nsArgSensitive(const nsStringView& sSensitiveInfo, const char* szContext = nullptr)
    : m_sSensitiveInfo(sSensitiveInfo)
    , m_szContext(szContext)
  {
  }

  const nsStringView m_sSensitiveInfo;
  const char* m_szContext;

  using BuildStringCallback = nsStringView (*)(char*, nsUInt32, const nsArgSensitive&);
  NS_FOUNDATION_DLL static BuildStringCallback s_BuildStringCB;

  /// \brief Set s_BuildStringCB to this function to enable scrambling of sensitive data.
  NS_FOUNDATION_DLL static nsStringView BuildString_SensitiveUserData_Hash(char* szTmp, nsUInt32 uiLength, const nsArgSensitive& arg);
};

NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgI& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsInt64 iArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsInt32 iArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgU& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsUInt64 uiArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsUInt32 uiArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgF& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, double fArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, bool bArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const char* szArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const wchar_t* pArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsStringBuilder& sArg);
NS_FOUNDATION_DLL const nsStringView& BuildString(char* szTmp, nsUInt32 uiLength, const nsStringView& sArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgC& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgP& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, nsResult arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsVariant& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsAngle& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsRational& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgHumanReadable& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsTime& arg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsArgSensitive& arg);


#if NS_ENABLED(NS_COMPILER_GCC) || NS_ENABLED(NS_COMPILER_CLANG)

// on these platforms "long int" is a different type from "long long int"

NS_ALWAYS_INLINE nsStringView BuildString(char* szTmp, nsUInt32 uiLength, long int iArg)
{
  return BuildString(szTmp, uiLength, static_cast<nsInt64>(iArg));
}

NS_ALWAYS_INLINE nsStringView BuildString(char* szTmp, nsUInt32 uiLength, unsigned long int uiArg)
{
  return BuildString(szTmp, uiLength, static_cast<nsUInt64>(uiArg));
}

#endif
