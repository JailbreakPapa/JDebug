#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/Implementation/FormatStringArgs.h>

class wdStringBuilder;
struct wdStringView;

/// \brief Implements formating of strings with placeholders and formatting options.
///
/// wdFormatString can be used anywhere where a string should be formatable when passing it into a function.
/// Good examples are wdStringBuilder::Format() or wdLog::Info().
///
/// A function taking an wdFormatString can internally call wdFormatString::GetText() to retrieve he formatted result.
/// When calling such a function, one must wrap the parameter into 'wdFmt' to enable formatting options, example:
///   void MyFunc(const wdFormatString& text);
///   MyFunc(wdFmt("Cool Story {}", "Bro"));
///
/// To provide more convenience, one can add a template-function overload like this:
///   template <typename... ARGS>
///   void MyFunc(const char* szFormat, ARGS&&... args)
///   {
///     MyFunc(wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
///   }
///
/// This allows to call MyFunc() without the 'wdFmt' wrapper.
///
///
/// === Formatting ===
///
/// Placeholders for variables are specified using '{}'. These may use numbers from 0 to 9,
/// ie. {0}, {3}, {2}, etc. which allows to change the order or insert duplicates.
/// If no number is provided, each {} instance represents the next argument.
///
/// To specify special formatting, wrap the argument into an wdArgXY call:
///   wdArgC - for characters
///   wdArgI - for integer formatting
///   wdArgU - for unsigned integer formatting (e.g. HEX)
///   wdArgF - for floating point formatting
///   wdArgP - for pointer formatting
///   wdArgDateTime - for wdDateTime formatting options
///   wdArgErrorCode - for Windows error code formatting
///   wdArgHumanReadable - for shortening numbers with common abbreviations
///   wdArgFileSize - for representing file sizes
///
/// Example:
///   wdStringBuilder::Format("HEX: {}", wdArgU(1337, 8 /*width*/, true /*pad with zeros*/, 16 /*base16*/, true/*upper case*/));
///
/// Arbitrary other types can support special formatting even without an wdArgXY call. E.g. wdTime and wdAngle do special formatting.
/// wdArgXY calls are only necessary if formatting options are needed for a specific formatting should be enforced (e.g. wdArgErrorCode
/// would otherwise just use uint32 formatting).
///
/// To implement custom formatting see the various free standing 'BuildString' functions.
class WD_FOUNDATION_DLL wdFormatString
{
  WD_DISALLOW_COPY_AND_ASSIGN(wdFormatString); // pass by reference, never pass by value

public:
  WD_ALWAYS_INLINE wdFormatString() { m_szString = nullptr; }
  WD_ALWAYS_INLINE wdFormatString(const char* szString) { m_szString = szString; }
  wdFormatString(const wdStringBuilder& s);
  virtual ~wdFormatString() = default;

  /// \brief Generates the formatted text. Make sure to only call this function once and only when the formatted string is really needed.
  ///
  /// Requires an wdStringBuilder as storage, ie. POTENTIALLY writes the formatted text into it.
  /// However, if no formatting is required, it may not touch the string builder at all and just return a string directly.
  ///
  /// \note Do not assume that the result is stored in \a sb. Always only use the return value. The string builder is only used
  /// when necessary.
  [[nodiscard]] virtual const char* GetText(wdStringBuilder&) const { return m_szString; }

  bool IsEmpty() const { return wdStringUtils::IsNullOrEmpty(m_szString); }

protected:
  // out of line function so that we don't need to include wdStringBuilder here, to break include dependency cycle
  static void SBAppendView(wdStringBuilder& sb, const wdStringView& sub);
  static void SBClear(wdStringBuilder& sb);
  static void SBAppendChar(wdStringBuilder& sb, wdUInt32 uiChar);
  static const char* SBReturn(wdStringBuilder& sb);

  const char* m_szString;
};

#include <Foundation/Strings/Implementation/FormatStringImpl.h>

template <typename... ARGS>
WD_ALWAYS_INLINE wdFormatStringImpl<ARGS...> wdFmt(const char* szFormat, ARGS&&... args)
{
  return wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...);
}
