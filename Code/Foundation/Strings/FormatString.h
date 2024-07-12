#pragma once

#include <Foundation/Basics.h>
#include <Foundation/Strings/Implementation/FormatStringArgs.h>

class nsStringBuilder;
struct nsStringView;

/// \brief Implements formating of strings with placeholders and formatting options.
///
/// nsFormatString can be used anywhere where a string should be formatable when passing it into a function.
/// Good examples are nsStringBuilder::SetFormat() or nsLog::Info().
///
/// A function taking an nsFormatString can internally call nsFormatString::GetText() to retrieve he formatted result.
/// When calling such a function, one must wrap the parameter into 'nsFmt' to enable formatting options, example:
///   void MyFunc(const nsFormatString& text);
///   MyFunc(nsFmt("Cool Story {}", "Bro"));
///
/// To provide more convenience, one can add a template-function overload like this:
///   template <typename... ARGS>
///   void MyFunc(const char* szFormat, ARGS&&... args)
///   {
///     MyFunc(nsFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
///   }
///
/// This allows to call MyFunc() without the 'nsFmt' wrapper.
///
///
/// === Formatting ===
///
/// Placeholders for variables are specified using '{}'. These may use numbers from 0 to 9,
/// ie. {0}, {3}, {2}, etc. which allows to change the order or insert duplicates.
/// If no number is provided, each {} instance represents the next argument.
///
/// To specify special formatting, wrap the argument into an nsArgXY call:
///   nsArgC - for characters
///   nsArgI - for integer formatting
///   nsArgU - for unsigned integer formatting (e.g. HEX)
///   nsArgF - for floating point formatting
///   nsArgP - for pointer formatting
///   nsArgDateTime - for nsDateTime formatting options
///   nsArgErrorCode - for Windows error code formatting
///   nsArgHumanReadable - for shortening numbers with common abbreviations
///   nsArgFileSize - for representing file sizes
///
/// Example:
///   nsStringBuilder::SetFormat("HEX: {}", nsArgU(1337, 8 /*width*/, true /*pad with zeros*/, 16 /*base16*/, true/*upper case*/));
///
/// Arbitrary other types can support special formatting even without an nsArgXY call. E.g. nsTime and nsAngle do special formatting.
/// nsArgXY calls are only necessary if formatting options are needed for a specific formatting should be enforced (e.g. nsArgErrorCode
/// would otherwise just use uint32 formatting).
///
/// To implement custom formatting see the various free standing 'BuildString' functions.
class NS_FOUNDATION_DLL nsFormatString
{
  NS_DISALLOW_COPY_AND_ASSIGN(nsFormatString); // pass by reference, never pass by value

public:
  NS_ALWAYS_INLINE nsFormatString() = default;
  NS_ALWAYS_INLINE nsFormatString(const char* szString) { m_sString = szString; }
  NS_ALWAYS_INLINE nsFormatString(nsStringView sString) { m_sString = sString; }
  nsFormatString(const nsStringBuilder& s);
  virtual ~nsFormatString() = default;

  /// \brief Generates the formatted text. Make sure to only call this function once and only when the formatted string is really needed.
  ///
  /// Requires an nsStringBuilder as storage, ie. POTENTIALLY writes the formatted text into it.
  /// However, if no formatting is required, it may not touch the string builder at all and just return a string directly.
  ///
  /// \note Do not assume that the result is stored in \a sb. Always only use the return value. The string builder is only used
  /// when necessary.
  [[nodiscard]] virtual nsStringView GetText(nsStringBuilder&) const { return m_sString; }

  /// \brief Similar to GetText() but guaranteed to copy the string into the given string builder,
  /// and thus guaranteeing that the generated string is zero terminated.
  virtual const char* GetTextCStr(nsStringBuilder& out_sString) const;

  bool IsEmpty() const { return m_sString.IsEmpty(); }

  /// \brief Helper function to build the formatted text with the given arguments.
  ///
  /// \note We can't use nsArrayPtr here because of include order.
  nsStringView BuildFormattedText(nsStringBuilder& ref_sStorage, nsStringView* pArgs, nsUInt32 uiNumArgs) const;

protected:
  nsStringView m_sString;
};

#include <Foundation/Strings/Implementation/FormatStringImpl.h>

template <typename... ARGS>
NS_ALWAYS_INLINE nsFormatStringImpl<ARGS...> nsFmt(const char* szFormat, ARGS&&... args)
{
  return nsFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...);
}
