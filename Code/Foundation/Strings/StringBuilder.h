#pragma once

#include <Foundation/ThirdParty/utf8/utf8.h>

#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Memory/MemoryUtils.h>
#include <Foundation/Strings/FormatString.h>
#include <Foundation/Strings/Implementation/StringBase.h>
#include <Foundation/Strings/PathUtils.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Strings/StringView.h>

template <wdUInt16 Size>
struct wdHybridStringBase;

template <wdUInt16 Size, typename AllocatorWrapper>
struct wdHybridString;

class wdStreamReader;
class wdFormatString;

/// \brief wdStringBuilder is a class that is meant for creating and modifying strings.
///
/// It is not meant to store strings for a longer duration.
/// Each wdStringBuilder uses an wdHybridArray to allocate a large buffer on the stack, such that string manipulations
/// are possible without memory allocations, unless the string is too large.
/// No sharing of data happens between wdStringBuilder instances, as it is expected that they will be modified anyway.
/// Instead all data is always copied, therefore instances should not be passed by copy.
/// All string data is stored Utf8 encoded, just as all other string classes, too.
/// That makes it difficult to modify individual characters. Instead you should prefer high-level functions
/// such as 'ReplaceSubString'. If individual characters must be modified, it might make more sense to create
/// a second wdStringBuilder, and iterate over the first while rebuilding the desired result in the second.
/// Once a string is built and should only be stored for read access, it should be stored in an wdString instance.
class WD_FOUNDATION_DLL wdStringBuilder : public wdStringBase<wdStringBuilder>
{
public:
  /// \brief Initializes the string to be empty. No data is allocated, but the wdStringBuilder ALWAYS creates an array on the stack.
  wdStringBuilder(wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Copies the given string into this one.
  wdStringBuilder(const wdStringBuilder& rhs); // [tested]

  /// \brief Moves the given string into this one.
  wdStringBuilder(wdStringBuilder&& rhs) noexcept;

  /// \brief Copies the given string into this one.
  template <wdUInt16 Size>
  wdStringBuilder(const wdHybridStringBase<Size>& rhs)
    : m_uiCharacterCount(rhs.m_uiCharacterCount)
    , m_Data(rhs.m_Data)
  {
  }

  /// \brief Copies the given string into this one.
  template <wdUInt16 Size, typename A>
  wdStringBuilder(const wdHybridString<Size, A>& rhs)
    : m_uiCharacterCount(rhs.m_uiCharacterCount)
    , m_Data(rhs.m_Data)
  {
  }


  /// \brief Moves the given string into this one.
  template <wdUInt16 Size>
  wdStringBuilder(wdHybridStringBase<Size>&& rhs)
    : m_uiCharacterCount(rhs.m_uiCharacterCount)
    , m_Data(std::move(rhs.m_Data))
  {
  }

  /// \brief Moves the given string into this one.
  template <wdUInt16 Size, typename A>
  wdStringBuilder(wdHybridString<Size, A>&& rhs)
    : m_uiCharacterCount(rhs.m_uiCharacterCount)
    , m_Data(std::move(rhs.m_Data))
  {
  }

  /// \brief Constructor that appends all the given strings.
  wdStringBuilder(wdStringView sData1, wdStringView sData2, wdStringView sData3 = {}, wdStringView sData4 = {},
    wdStringView sData5 = {}, wdStringView sData6 = {}); // [tested]

  /// \brief Copies the given Utf8 string into this one.
  /* implicit */ wdStringBuilder(const char* szUTF8, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Copies the given wchar_t string into this one.
  /* implicit */ wdStringBuilder(const wchar_t* pWChar, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Copies the given substring into this one. The wdStringView might actually be a substring of this very string.
  /* implicit */ wdStringBuilder(wdStringView rhs, wdAllocatorBase* pAllocator = wdFoundation::GetDefaultAllocator()); // [tested]

  /// \brief Copies the given string into this one.
  void operator=(const wdStringBuilder& rhs); // [tested]

  /// \brief Moves the given string into this one.
  void operator=(wdStringBuilder&& rhs) noexcept;

  /// \brief Copies the given Utf8 string into this one.
  void operator=(const char* szUTF8); // [tested]

  /// \brief Copies the given wchar_t string into this one.
  void operator=(const wchar_t* pWChar); // [tested]

  /// \brief Copies the given substring into this one. The wdStringView might actually be a substring of this very string.
  void operator=(wdStringView rhs); // [tested]

  /// \brief Copies the given string into this one.
  template <wdUInt16 Size>
  void operator=(const wdHybridStringBase<Size>& rhs)
  {
    m_uiCharacterCount = rhs.m_uiCharacterCount;
    m_Data = rhs.m_Data;
  }

  /// \brief Copies the given string into this one.
  template <wdUInt16 Size, typename A>
  void operator=(const wdHybridString<Size, A>& rhs)
  {
    m_uiCharacterCount = rhs.m_uiCharacterCount;
    m_Data = rhs.m_Data;
  }

  /// \brief Moves the given string into this one.
  template <wdUInt16 Size>
  void operator=(wdHybridStringBase<Size>&& rhs)
  {
    m_uiCharacterCount = rhs.m_uiCharacterCount;
    m_Data = std::move(rhs.m_Data);
  }

  /// \brief Moves the given string into this one.
  template <wdUInt16 Size, typename A>
  void operator=(wdHybridString<Size, A>&& rhs) noexcept
  {
    m_uiCharacterCount = rhs.m_uiCharacterCount;
    m_Data = std::move(rhs.m_Data);
  }

  /// \brief Returns the allocator that is used by this object.
  wdAllocatorBase* GetAllocator() const;

  /// \brief Resets this string to be empty. Does not deallocate any previously allocated data, as it might be reused later again.
  void Clear(); // [tested]

  /// \brief Returns a char pointer to the internal Utf8 data.
  const char* GetData() const; // [tested]

  /// \brief Returns the number of bytes that this string takes up.
  wdUInt32 GetElementCount() const; // [tested]

  /// \brief Returns the number of characters of which this string consists. Might be less than GetElementCount, if it contains Utf8
  /// multi-byte characters.
  wdUInt32 GetCharacterCount() const; // [tested]

  /// \brief Returns whether this string only contains ASCII characters, which means that GetElementCount() == GetCharacterCount()
  bool IsPureASCII() const; // [tested]

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
  void ChangeCharacter(iterator& ref_it, wdUInt32 uiCharacter); // [tested]

  /// \brief Sets the string by concatenating all given strings.
  void Set(wdStringView sData1, wdStringView sData2 = {}, wdStringView sData3 = {}, wdStringView sData4 = {}, wdStringView sData5 = {}, wdStringView sData6 = {});

  /// \brief Copies the string starting at \a pStart up to \a pEnd (exclusive).
  void SetSubString_FromTo(const char* pStart, const char* pEnd);

  /// \brief Copies the string starting at \a pStart with a length of \a uiElementCount bytes.
  void SetSubString_ElementCount(const char* pStart, wdUInt32 uiElementCount);

  /// \brief Copies the string starting at \a pStart with a length of \a uiCharacterCount characters.
  void SetSubString_CharacterCount(const char* pStart, wdUInt32 uiCharacterCount);

  /// \brief Appends a single Utf32 character.
  void Append(wdUInt32 uiChar); // [tested]

  /// \brief Appends all the given strings at the back of this string in one operation.
  void Append(const wchar_t* pData1, const wchar_t* pData2 = nullptr, const wchar_t* pData3 = nullptr, const wchar_t* pData4 = nullptr, const wchar_t* pData5 = nullptr, const wchar_t* pData6 = nullptr); // [tested]

  /// \brief Appends all the given strings at the back of this string in one operation.
  void Append(wdStringView sData1, wdStringView sData2 = {}, wdStringView sData3 = {}, wdStringView sData4 = {}, wdStringView sData5 = {}, wdStringView sData6 = {}); // [tested]

  /// \brief Prepends a single Utf32 character.
  void Prepend(wdUInt32 uiChar); // [tested]

  /// \brief Prepends all the given strings to the front of this string in one operation.
  void Prepend(const wchar_t* pData1, const wchar_t* pData2 = nullptr, const wchar_t* pData3 = nullptr, const wchar_t* pData4 = nullptr,
    const wchar_t* pData5 = nullptr, const wchar_t* pData6 = nullptr); // [tested]

  /// \brief Prepends all the given strings to the front of this string in one operation.
  void Prepend(wdStringView sData1, wdStringView sData2 = {}, wdStringView sData3 = {}, wdStringView sData4 = {},
    wdStringView sData5 = {}, wdStringView sData6 = {}); // [tested]

  /// \brief Sets this string to the formatted string, uses printf-style formatting.
  void Printf(const char* szUtf8Format, ...); // [tested]

  /// \brief Sets this string to the formatted string, uses printf-style formatting.
  void PrintfArgs(const char* szUtf8Format, va_list szArgs); // [tested]

  /// \brief Replaces this with a formatted string. Uses '{}' formatting placeholders, see wdFormatString for details.
  void Format(const wdFormatString& string);

  /// \brief Replaces this with a formatted string. Uses '{}' formatting placeholders, see wdFormatString for details.
  template <typename... ARGS>
  void Format(const char* szFormat, ARGS&&... args)
  {
    Format(wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Appends a formatted string. Uses '{}' formatting placeholders, see wdFormatString for details.
  void AppendFormat(const wdFormatString& string);

  /// \brief Appends a formatted string. Uses '{}' formatting placeholders, see wdFormatString for details.
  template <typename... ARGS>
  void AppendFormat(const char* szFormat, ARGS&&... args)
  {
    AppendFormat(wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Prepends a formatted string. Uses '{}' formatting placeholders, see wdFormatString for details.
  void PrependFormat(const wdFormatString& string);

  /// \brief Prepends a formatted string. Uses '{}' formatting placeholders, see wdFormatString for details.
  template <typename... ARGS>
  void PrependFormat(const char* szFormat, ARGS&&... args)
  {
    PrependFormat(wdFormatStringImpl<ARGS...>(szFormat, std::forward<ARGS>(args)...));
  }

  /// \brief Removes the first n and last m characters from this string.
  ///
  /// This function will never reallocate data.
  /// Removing characters at the back is very cheap.
  /// Removing characters at the front needs to move data around, so can be quite costly.
  void Shrink(wdUInt32 uiShrinkCharsFront, wdUInt32 uiShrinkCharsBack); // [tested]

  /// \brief Reserves uiNumElements bytes.
  void Reserve(wdUInt32 uiNumElements); // [tested]


  /// \brief Replaces the string that starts at szStartPos and ends at szEndPos with the string szReplaceWith.
  void ReplaceSubString(const char* szStartPos, const char* szEndPos, wdStringView sReplaceWith); // [tested]

  /// \brief A wrapper around ReplaceSubString. Will insert the given string at szInsertAtPos.
  void Insert(const char* szInsertAtPos, wdStringView sTextToInsert); // [tested]

  /// \brief A wrapper around ReplaceSubString. Will remove the substring which starts at szRemoveFromPos and ends at szRemoveToPos.
  void Remove(const char* szRemoveFromPos, const char* szRemoveToPos); // [tested]

  /// \brief Replaces the first occurrence of szSearchFor by szReplacement. Optionally starts searching at szStartSearchAt (or the
  /// beginning).
  ///
  /// Returns the first position where szSearchFor was found, or nullptr if nothing was found (and replaced).
  const char* ReplaceFirst(wdStringView sSearchFor, wdStringView sReplacement, const char* szStartSearchAt = nullptr); // [tested]

  /// \brief Case-insensitive version of ReplaceFirst.
  const char* ReplaceFirst_NoCase(wdStringView sSearchFor, wdStringView sReplacement, const char* szStartSearchAt = nullptr); // [tested]

  /// \brief Replaces the last occurrence of szSearchFor by szReplacement. Optionally starts searching at szStartSearchAt (or the end).
  ///
  /// Returns the last position where szSearchFor was found, or nullptr if nothing was found (and replaced).
  const char* ReplaceLast(wdStringView sSearchFor, wdStringView sReplacement, const char* szStartSearchAt = nullptr); // [tested]

  /// \brief Case-insensitive version of ReplaceLast.
  const char* ReplaceLast_NoCase(wdStringView sSearchFor, wdStringView sReplacement, const char* szStartSearchAt = nullptr); // [tested]

  /// \brief Replaces all occurrences of szSearchFor by szReplacement. Returns the number of replacements.
  wdUInt32 ReplaceAll(wdStringView sSearchFor, wdStringView sReplacement); // [tested]

  /// \brief Case-insensitive version of ReplaceAll.
  wdUInt32 ReplaceAll_NoCase(wdStringView sSearchFor, wdStringView sReplacement); // [tested]

  /// \brief Replaces the first occurrence of szSearchFor by szReplaceWith, if szSearchFor was found to be a 'whole word', as indicated by
  /// the delimiter function IsDelimiterCB.
  ///
  /// Returns the start position of where the word was replaced or nullptr if nothing got replaced.
  const char* ReplaceWholeWord(const char* szSearchFor, wdStringView sReplaceWith, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB); // [tested]

  /// \brief Case-insensitive version of ReplaceWholeWord.
  ///
  /// Returns the start position of where the word was replaced or nullptr if nothing got replaced.
  const char* ReplaceWholeWord_NoCase(const char* szSearchFor, wdStringView sReplaceWith, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB); // [tested]

  /// \brief Replaces all occurrences of szSearchFor by szReplaceWith, if szSearchFor was found to be a 'whole word', as indicated by the
  /// delimiter function IsDelimiterCB.
  ///
  /// Returns how many words got replaced.
  wdUInt32 ReplaceWholeWordAll(const char* szSearchFor, wdStringView sReplaceWith, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB); // [tested]

  /// \brief Case-insensitive version of ReplaceWholeWordAll.
  ///
  /// Returns how many words got replaced.
  wdUInt32 ReplaceWholeWordAll_NoCase(const char* szSearchFor, wdStringView sReplaceWith, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB); // [tested]

  /// \brief Replaces the current string with the content from the stream. Reads the stream to its end.
  void ReadAll(wdStreamReader& inout_stream);

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
  void PathParentDirectory(wdUInt32 uiLevelsUp = 1); // [tested]

  /// \brief Appends several path pieces. Makes sure they are always properly separated by a slash.
  ///
  /// Will call 'MakeCleanPath' internally, so the representation of the path might change.
  void AppendPath(wdStringView sPath1, wdStringView sPath2 = {}, wdStringView sPath3 = {}, wdStringView sPath4 = {}); // [tested]

  /// \brief Similar to Append() but the very first argument is a separator that is only appended (once) if the existing string is not empty and does
  /// not already end with the separator.
  ///
  /// This is useful when one wants to append entries that require a separator like a comma in between items. E.g. calling
  /// AppendWithSeparator(", ", "a", "b");
  /// AppendWithSeparator(", ", "c", "d");
  /// results in the string "ab, cd"
  void AppendWithSeparator(wdStringView sSeparator, wdStringView sText1, wdStringView sText2 = wdStringView(), wdStringView sText3 = wdStringView(), wdStringView sText4 = wdStringView(), wdStringView sText5 = wdStringView(), wdStringView sText6 = wdStringView());

  /// \brief Changes the file name part of the path, keeps the extension intact (if there is any).
  void ChangeFileName(wdStringView sNewFileName); // [tested]

  /// \brief Changes the file name and the extension part of the path.
  void ChangeFileNameAndExtension(wdStringView sNewFileNameWithExtension); // [tested]

  /// \brief Only changes the file extension of the path. If there is no extension yet, one is appended (including a dot).
  ///
  /// sNewExtension may or may not start with a dot.
  /// If sNewExtension is empty, the file extension is removed, but the dot remains.
  /// E.g. "file.txt" -> "file."
  /// If the full extension should be removed, including the dot, use RemoveFileExtension() instead.
  void ChangeFileExtension(wdStringView sNewExtension); // [tested]

  /// \brief If any extension exists, it is removed, including the dot before it.
  void RemoveFileExtension(); // [tested]

  /// \brief Converts this path into a relative path to the path with the awesome variable name 'szAbsolutePathToMakeThisRelativeTo'
  ///
  /// If the method succeeds the StringBuilder's contents are modified in place.
  wdResult MakeRelativeTo(wdStringView sAbsolutePathToMakeThisRelativeTo); // [tested]

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
  wdUInt64 GetHeapMemoryUsage() const { return m_Data.GetHeapMemoryUsage(); }

  /// \brief Removes all characters from the start and end that appear in the given strings.
  void Trim(const char* szTrimChars); // [tested]

  /// \brief Removes all characters from the start and/or end that appear in the given strings.
  void Trim(const char* szTrimCharsStart, const char* szTrimCharsEnd); // [tested]

  /// \brief If the string starts with one of the given words (case insensitive), it is removed and the function returns true.
  bool TrimWordStart(const char* szWord1, const char* szWord2 = nullptr, const char* szWord3 = nullptr, const char* szWord4 = nullptr, const char* szWord5 = nullptr); // [tested]

  /// \brief If the string ends with one of the given words (case insensitive), it is removed and the function returns true.
  bool TrimWordEnd(const char* szWord1, const char* szWord2 = nullptr, const char* szWord3 = nullptr, const char* szWord4 = nullptr, const char* szWord5 = nullptr); // [tested]

private:
  /// \brief Will remove all double path separators (slashes and backslashes) in a path, except if the path starts with two (back-)slashes,
  /// those are kept, as they might indicate a UNC path.
  void RemoveDoubleSlashesInPath(); // [tested]

  void ChangeCharacterNonASCII(iterator& it, wdUInt32 uiCharacter);
  void AppendTerminator();

  // needed for better copy construction
  template <wdUInt16 T>
  friend struct wdHybridStringBase;

  friend wdStreamReader;

  wdUInt32 m_uiCharacterCount;
  wdHybridArray<char, 128> m_Data;
};

#include <Foundation/Strings/Implementation/StringBuilder_inl.h>
