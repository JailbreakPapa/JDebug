#pragma once

#include <Foundation/ThirdParty/utf8/utf8.h>

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Memory/MemoryUtils.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/Implementation/StringBase.h>
#include <Foundation/Strings/PathUtils.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Strings/StringView.h>

template <nsUInt16 Size>
struct nsHybridStringBase;

template <nsUInt16 Size, typename AllocatorWrapper>
struct nsHybridString;

class nsStreamReader;
class nsFormatString;

/// \brief nsStringBuilder is a class that is meant for creating and modifying strings.
///
/// It is not meant to store strings for a longer duration.
/// Each nsStringBuilder uses an nsHybridArray to allocate a large buffer on the stack, such that string manipulations
/// are possible without memory allocations, unless the string is too large.
/// No sharing of data happens between nsStringBuilder instances, as it is expected that they will be modified anyway.
/// Instead all data is always copied, therefore instances should not be passed by copy.
/// All string data is stored Utf8 encoded, just as all other string classes, too.
/// That makes it difficult to modify individual characters. Instead you should prefer high-level functions
/// such as 'ReplaceSubString'. If individual characters must be modified, it might make more sense to create
/// a second nsStringBuilder, and iterate over the first while rebuilding the desired result in the second.
/// Once a string is built and should only be stored for read access, it should be stored in an nsString instance.
class NS_FOUNDATION_DLL nsStringBuilder : public nsStringBase<nsStringBuilder>
{
public:
  /// \brief Initializes the string to be empty. No data is allocated, but the nsStringBuilder ALWAYS creates an array on the stack.
  nsStringBuilder(nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Copies the given string into this one.
  nsStringBuilder(const nsStringBuilder& rhs); // [tested]

  /// \brief Moves the given string into this one.
  nsStringBuilder(nsStringBuilder&& rhs) noexcept;

  /// \brief Copies the given string into this one.
  template <nsUInt16 Size>
  nsStringBuilder(const nsHybridStringBase<Size>& rhs)
    : m_Data(rhs.m_Data)
  {
  }

  /// \brief Copies the given string into this one.
  template <nsUInt16 Size, typename A>
  nsStringBuilder(const nsHybridString<Size, A>& rhs)
    : m_Data(rhs.m_Data)
  {
  }


  /// \brief Moves the given string into this one.
  template <nsUInt16 Size>
  nsStringBuilder(nsHybridStringBase<Size>&& rhs)
    : m_Data(std::move(rhs.m_Data))
  {
  }

  /// \brief Moves the given string into this one.
  template <nsUInt16 Size, typename A>
  nsStringBuilder(nsHybridString<Size, A>&& rhs)
    : m_Data(std::move(rhs.m_Data))
  {
  }

  /// \brief Constructor that appends all the given strings.
  nsStringBuilder(nsStringView sData1, nsStringView sData2, nsStringView sData3 = {}, nsStringView sData4 = {},
    nsStringView sData5 = {}, nsStringView sData6 = {}); // [tested]

                                                         /// \brief Copies the given Utf8 string into this one.
  /* implicit */ nsStringBuilder(const char* szUTF8, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

                                                                                                                     /// \brief Copies the given wchar_t string into this one.
  /* implicit */ nsStringBuilder(const wchar_t* pWChar, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

                                                                                                                        /// \brief Copies the given substring into this one. The nsStringView might actually be a substring of this very string.
  /* implicit */ nsStringBuilder(nsStringView rhs, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Copies the given string into this one.
  void operator=(const nsStringBuilder& rhs); // [tested]

  /// \brief Moves the given string into this one.
  void operator=(nsStringBuilder&& rhs) noexcept;

  /// \brief Copies the given Utf8 string into this one.
  void operator=(const char* szUTF8); // [tested]

  /// \brief Copies the given wchar_t string into this one.
  void operator=(const wchar_t* pWChar); // [tested]

  /// \brief Copies the given substring into this one. The nsStringView might actually be a substring of this very string.
  void operator=(nsStringView rhs); // [tested]

  /// \brief Copies the given string into this one.
  template <nsUInt16 Size>
  void operator=(const nsHybridStringBase<Size>& rhs)
  {
    m_Data = rhs.m_Data;
  }

  /// \brief Copies the given string into this one.
  template <nsUInt16 Size, typename A>
  void operator=(const nsHybridString<Size, A>& rhs)
  {
    m_Data = rhs.m_Data;
  }

  /// \brief Moves the given string into this one.
  template <nsUInt16 Size>
  void operator=(nsHybridStringBase<Size>&& rhs)
  {
    m_Data = std::move(rhs.m_Data);
  }

  /// \brief Moves the given string into this one.
  template <nsUInt16 Size, typename A>
  void operator=(nsHybridString<Size, A>&& rhs) noexcept
  {
    m_Data = std::move(rhs.m_Data);
  }

  /// \brief Returns the allocator that is used by this object.
  nsAllocator* GetAllocator() const;

  /// \brief Resets this string to be empty. Does not deallocate any previously allocated data, as it might be reused later again.
  void Clear(); // [tested]

  /// \brief Returns a char pointer to the internal Utf8 data.
  const char* GetData() const; // [tested]

  /// \brief Returns the number of bytes that this string takes up.
  nsUInt32 GetElementCount() const; // [tested]

  /// \brief Returns the number of characters of which this string consists. Might be less than GetElementCount, if it contains Utf8
  /// multi-byte characters.
  ///
  /// \note This is a slow operation, as it has to run through the entire string to count the Unicode characters.
  /// Only call this once and use the result as long as the string doesn't change. Don't call this in a loop.
  nsUInt32 GetCharacterCount() const; // [tested]

  /// \brief Converts all characters to upper case. Might move the string data around, so all iterators to the data will be invalid
  /// afterwards.
  void ToUpper(); // [tested]

  /// \brief Converts all characters to lower case. Might move the string data around, so all iterators to the data will be invalid
  /// afterwards.
  void ToLower(); // [tested]

  /// \brief Changes the single character in this string, to which the iterator currently points.
  ///
  /// The string might need to be moved around, if its encoding size changes, however the given iterator will be adjusted
  /// so that it will always stay valid.
  /// \note
  /// This can be a very costly operation (unless this string is pure ASCII).
  /// It is only provided for the few rare cases where it is more convenient and performance is not of concern.
  /// If possible, do not use this function, at all.
  void ChangeCharacter(iterator& ref_it, nsUInt32 uiCharacter); // [tested]

  /// \brief Sets the string to the given string.
  void Set(nsStringView sData1); // [tested]
  /// \brief Sets the string by concatenating all given strings.
  void Set(nsStringView sData1, nsStringView sData2); // [tested]
  /// \brief Sets the string by concatenating all given strings.
  void Set(nsStringView sData1, nsStringView sData2, nsStringView sData3); // [tested]
  /// \brief Sets the string by concatenating all given strings.
  void Set(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4); // [tested]
  /// \brief Sets the string by concatenating all given strings.
  void Set(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4, nsStringView sData5, nsStringView sData6 = {}); // [tested]

  /// \brief Sets several path pieces. Makes sure they are always properly separated by a slash.
  void SetPath(nsStringView sData1, nsStringView sData2, nsStringView sData3 = {}, nsStringView sData4 = {});

  /// \brief Copies the string starting at \a pStart up to \a pEnd (exclusive).
  void SetSubString_FromTo(const char* pStart, const char* pEnd);

  /// \brief Copies the string starting at \a pStart with a length of \a uiElementCount bytes.
  void SetSubString_ElementCount(const char* pStart, nsUInt32 uiElementCount);

  /// \brief Copies the string starting at \a pStart with a length of \a uiCharacterCount characters.
  void SetSubString_CharacterCount(const char* pStart, nsUInt32 uiCharacterCount);

  /// \brief Appends a single Utf32 character.
  void Append(nsUInt32 uiChar); // [tested]

  /// \brief Appends all the given strings at the back of this string in one operation.
  void Append(const wchar_t* pData1, const wchar_t* pData2 = nullptr, const wchar_t* pData3 = nullptr, const wchar_t* pData4 = nullptr, const wchar_t* pData5 = nullptr, const wchar_t* pData6 = nullptr); // [tested]

  /// \brief Appends all the given strings to the back of this string in one operation.
  void Append(nsStringView sData1); // [tested]
  /// \brief Appends all the given strings to the back of this string in one operation.
  void Append(nsStringView sData1, nsStringView sData2); // [tested]
  /// \brief Appends all the given strings to the back of this string in one operation.
  void Append(nsStringView sData1, nsStringView sData2, nsStringView sData3); // [tested]
  /// \brief Appends all the given strings to the back of this string in one operation.
  void Append(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4); // [tested]
  /// \brief Appends all the given strings to the back of this string in one operation.
  void Append(nsStringView sData1, nsStringView sData2, nsStringView sData3, nsStringView sData4, nsStringView sData5, nsStringView sData6 = {}); // [tested]

  /// \brief Prepends a single Utf32 character.
  void Prepend(nsUInt32 uiChar); // [tested]

  /// \brief Prepends all the given strings to the front of this string in one operation.
  void Prepend(const wchar_t* pData1, const wchar_t* pData2 = nullptr, const wchar_t* pData3 = nullptr, const wchar_t* pData4 = nullptr,
    const wchar_t* pData5 = nullptr, const wchar_t* pData6 = nullptr); // [tested]

  /// \brief Prepends all the given strings to the front of this string in one operation.
  void Prepend(nsStringView sData1, nsStringView sData2 = {}, nsStringView sData3 = {}, nsStringView sData4 = {},
    nsStringView sData5 = {}, nsStringView sData6 = {}); // [tested]

  /// \brief Sets this string to the formatted string, uses printf-style formatting.
  void SetPrintf(const char* szUtf8Format, ...); // [tested]

  /// \brief Sets this string to the formatted string, uses printf-style formatting.
  void SetPrintfArgs(const char* szUtf8Format, va_list szArgs); // [tested]

  /// \brief Replaces this with a formatted string. Uses '{}' formatting placeholders, see nsFormatString for details.
  void SetFormat(const nsFormatString& string);

  /// \brief Replaces this with a formatted string. Uses '{}' formatting placeholders, see nsFormatString for details.
  template <typename... ARGS>
  void SetFormat(const char* szFormat, ARGS&&... args)
  {
    SetFormat(nsFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Appends a formatted string. Uses '{}' formatting placeholders, see nsFormatString for details.
  void AppendFormat(const nsFormatString& string);

  /// \brief Appends a formatted string. Uses '{}' formatting placeholders, see nsFormatString for details.
  template <typename... ARGS>
  void AppendFormat(const char* szFormat, ARGS&&... args)
  {
    AppendFormat(nsFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Prepends a formatted string. Uses '{}' formatting placeholders, see nsFormatString for details.
  void PrependFormat(const nsFormatString& string);

  /// \brief Prepends a formatted string. Uses '{}' formatting placeholders, see nsFormatString for details.
  template <typename... ARGS>
  void PrependFormat(const char* szFormat, ARGS&&... args)
  {
    PrependFormat(nsFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Removes the first n and last m characters from this string.
  ///
  /// This function will never reallocate data.
  /// Removing characters at the back is very cheap.
  /// Removing characters at the front needs to move data around, so can be quite costly.
  void Shrink(nsUInt32 uiShrinkCharsFront, nsUInt32 uiShrinkCharsBack); // [tested]

  /// \brief Reserves uiNumElements bytes.
  void Reserve(nsUInt32 uiNumElements); // [tested]


  /// \brief Replaces the string that starts at szStartPos and ends at szEndPos with the string szReplaceWith.
  void ReplaceSubString(const char* szStartPos, const char* szEndPos, nsStringView sReplaceWith); // [tested]

  /// \brief A wrapper around ReplaceSubString. Will insert the given string at szInsertAtPos.
  void Insert(const char* szInsertAtPos, nsStringView sTextToInsert); // [tested]

  /// \brief A wrapper around ReplaceSubString. Will remove the substring which starts at szRemoveFromPos and ends at szRemoveToPos.
  void Remove(const char* szRemoveFromPos, const char* szRemoveToPos); // [tested]

  /// \brief Replaces the first occurrence of szSearchFor by szReplacement. Optionally starts searching at szStartSearchAt (or the
  /// beginning).
  ///
  /// Returns the first position where szSearchFor was found, or nullptr if nothing was found (and replaced).
  const char* ReplaceFirst(nsStringView sSearchFor, nsStringView sReplacement, const char* szStartSearchAt = nullptr); // [tested]

  /// \brief Case-insensitive version of ReplaceFirst.
  const char* ReplaceFirst_NoCase(nsStringView sSearchFor, nsStringView sReplacement, const char* szStartSearchAt = nullptr); // [tested]

  /// \brief Replaces the last occurrence of szSearchFor by szReplacement. Optionally starts searching at szStartSearchAt (or the end).
  ///
  /// Returns the last position where szSearchFor was found, or nullptr if nothing was found (and replaced).
  const char* ReplaceLast(nsStringView sSearchFor, nsStringView sReplacement, const char* szStartSearchAt = nullptr); // [tested]

  /// \brief Case-insensitive version of ReplaceLast.
  const char* ReplaceLast_NoCase(nsStringView sSearchFor, nsStringView sReplacement, const char* szStartSearchAt = nullptr); // [tested]

  /// \brief Replaces all occurrences of szSearchFor by szReplacement. Returns the number of replacements.
  nsUInt32 ReplaceAll(nsStringView sSearchFor, nsStringView sReplacement); // [tested]

  /// \brief Case-insensitive version of ReplaceAll.
  nsUInt32 ReplaceAll_NoCase(nsStringView sSearchFor, nsStringView sReplacement); // [tested]

  /// \brief Replaces the first occurrence of szSearchFor by szReplaceWith, if szSearchFor was found to be a 'whole word', as indicated by
  /// the delimiter function IsDelimiterCB.
  ///
  /// Returns the start position of where the word was replaced or nullptr if nothing got replaced.
  const char* ReplaceWholeWord(const char* szSearchFor, nsStringView sReplaceWith, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB); // [tested]

  /// \brief Case-insensitive version of ReplaceWholeWord.
  ///
  /// Returns the start position of where the word was replaced or nullptr if nothing got replaced.
  const char* ReplaceWholeWord_NoCase(const char* szSearchFor, nsStringView sReplaceWith, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB); // [tested]

  /// \brief Replaces all occurrences of szSearchFor by szReplaceWith, if szSearchFor was found to be a 'whole word', as indicated by the
  /// delimiter function IsDelimiterCB.
  ///
  /// Returns how many words got replaced.
  nsUInt32 ReplaceWholeWordAll(const char* szSearchFor, nsStringView sReplaceWith, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB); // [tested]

  /// \brief Case-insensitive version of ReplaceWholeWordAll.
  ///
  /// Returns how many words got replaced.
  nsUInt32 ReplaceWholeWordAll_NoCase(const char* szSearchFor, nsStringView sReplaceWith, nsStringUtils::NS_CHARACTER_FILTER isDelimiterCB); // [tested]

  /// \brief Replaces the current string with the content from the stream. Reads the stream to its end.
  void ReadAll(nsStreamReader& inout_stream);

  // ******* Path Functions ********

  /// \brief Removes "../" where possible, replaces all path separators with /, removes double slashes.
  ///
  /// All paths use slashes on all platforms. If you need to convert a path to the OS specific representation, use
  /// 'MakePathSeparatorsNative' 'MakeCleanPath' will in rare circumstances grow the string by one character. That means it is quite safe to
  /// assume that it will not waste time on memory allocations. If it is repeatedly called on the same string, it has a minor overhead for
  /// computing the same string over and over, but no memory allocations will be done (everything is in-place).
  ///
  /// Removes all double path separators (slashes and backslashes) in a path, except if the path starts with two (back-)slashes, those are
  /// kept, as they might indicate a UNC path.
  void MakeCleanPath(); // [tested]

  /// \brief Modifies this string to point to the parent directory.
  ///
  /// 'uiLevelsUp' can be used to go several folders upwards. It has to be at least one.
  /// If there are no more folders to go up, "../" is appended as much as needed.
  void PathParentDirectory(nsUInt32 uiLevelsUp = 1); // [tested]

  /// \brief Appends several path pieces. Makes sure they are always properly separated by a slash.
  void AppendPath(nsStringView sPath1, nsStringView sPath2 = {}, nsStringView sPath3 = {}, nsStringView sPath4 = {}); // [tested]

  /// \brief Similar to Append() but the very first argument is a separator that is only appended (once) if the existing string is not empty and does
  /// not already end with the separator.
  ///
  /// This is useful when one wants to append entries that require a separator like a comma in between items. E.g. calling
  /// AppendWithSeparator(", ", "a", "b");
  /// AppendWithSeparator(", ", "c", "d");
  /// results in the string "ab, cd"
  void AppendWithSeparator(nsStringView sSeparator, nsStringView sText1, nsStringView sText2 = nsStringView(), nsStringView sText3 = nsStringView(), nsStringView sText4 = nsStringView(), nsStringView sText5 = nsStringView(), nsStringView sText6 = nsStringView());

  /// \brief Changes the file name part of the path, keeps the extension intact (if there is any).
  void ChangeFileName(nsStringView sNewFileName); // [tested]

  /// \brief Changes the file name and the extension part of the path.
  void ChangeFileNameAndExtension(nsStringView sNewFileNameWithExtension); // [tested]

  /// \brief Only changes the file extension of the path. If there is no extension yet, one is appended (including a dot).
  ///
  /// sNewExtension may or may not start with a dot.
  /// If sNewExtension is empty, the file extension is removed, but the dot remains.
  /// E.g. "file.txt" -> "file."
  /// If you also want to remove the dot, use RemoveFileExtension() instead.
  ///
  /// If bFullExtension is false, a file named "file.a.b.c" will replace only "c".
  /// If bFullExtension is true, a file named "file.a.b.c" will replace all of "a.b.c".
  void ChangeFileExtension(nsStringView sNewExtension, bool bFullExtension = false); // [tested]

  /// \brief If any extension exists, it is removed, including the dot before it.
  ///
  /// If bFullExtension is false, a file named "file.a.b.c" will end up as "file.a.b"
  /// If bFullExtension is true, a file named "file.a.b.c" will end up as "file"
  void RemoveFileExtension(bool bFullExtension = false); // [tested]

  /// \brief Converts this path into a relative path to the path with the awesome variable name 'szAbsolutePathToMakeThisRelativeTo'
  ///
  /// If the method succeeds the StringBuilder's contents are modified in place.
  nsResult MakeRelativeTo(nsStringView sAbsolutePathToMakeThisRelativeTo); // [tested]

  /// \brief Cleans this path up and replaces all path separators by the OS specific separator.
  ///
  /// This can be used, if you want to present paths in the OS specific form to the user in the UI.
  /// In all other cases the internal representation uses slashes, no matter on which operating system.
  void MakePathSeparatorsNative(); // [tested]

  /// \brief Checks whether this path is a sub-path of the given path.
  ///
  /// This function will call 'MakeCleanPath' to be able to compare both paths, thus it might modify the data of this instance.
  bool IsPathBelowFolder(const char* szPathToFolder); // [tested]

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  nsUInt64 GetHeapMemoryUsage() const { return m_Data.GetHeapMemoryUsage(); }

  /// \brief Removes all characters from the start and end that appear in the given strings.
  ///
  /// The default string removes all standard whitespace characters.
  void Trim(const char* szTrimChars = " \f\n\r\t\v"); // [tested]

  /// \brief Removes all characters from the start and/or end that appear in the given strings.
  void Trim(const char* szTrimCharsStart, const char* szTrimCharsEnd); // [tested]

  /// \brief Removes all characters from the start that appear in the given strings.
  ///
  /// The default string removes all standard whitespace characters.
  void TrimLeft(const char* szTrimChars = " \f\n\r\t\v");

  /// \brief Removes all characters from the end that appear in the given strings.
  ///
  /// The default string removes all standard whitespace characters.
  void TrimRight(const char* szTrimChars = " \f\n\r\t\v");

  /// \brief If the string starts with the given word (case insensitive), it is removed and the function returns true.
  bool TrimWordStart(nsStringView sWord); // [tested]

  /// \brief If the string ends with the given word (case insensitive), it is removed and the function returns true.
  bool TrimWordEnd(nsStringView sWord); // [tested]

#if NS_ENABLED(NS_INTEROP_STL_STRINGS)
  /// \brief Copies the given substring into this one. The nsStringView might actually be a substring of this very string.
  /* implicit */ nsStringBuilder(const std::string_view& rhs, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator());

  /// \brief Copies the given substring into this one. The nsStringView might actually be a substring of this very string.
  /* implicit */ nsStringBuilder(const std::string& rhs, nsAllocator* pAllocator = nsFoundation::GetDefaultAllocator());

  /// \brief Copies the given substring into this one. The nsStringView might actually be a substring of this very string.
  void operator=(const std::string_view& rhs);

  /// \brief Copies the given substring into this one. The nsStringView might actually be a substring of this very string.
  void operator=(const std::string& rhs);
#endif

private:
  /// \brief Will remove all double path separators (slashes and backslashes) in a path, except if the path starts with two (back-)slashes,
  /// those are kept, as they might indicate a UNC path.
  void RemoveDoubleSlashesInPath(); // [tested]

  void ChangeCharacterNonASCII(iterator& it, nsUInt32 uiCharacter);
  void AppendTerminator();

  // needed for better copy construction
  template <nsUInt16 T>
  friend struct nsHybridStringBase;

  friend nsStreamReader;

  nsHybridArray<char, 128> m_Data;
};

#include <Foundation/Strings/Implementation/StringBuilder_inl.h>
