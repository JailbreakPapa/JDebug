#pragma once

#include <Foundation/Strings/Implementation/StringIterator.h>

#include <type_traits>

/// Base class which marks a class as containing string data
struct wdThisIsAString
{
};

class wdStringBuilder;

/// \brief wdStringView represent a read-only sub-string of a larger string, as it can store a dedicated string end position.
/// It derives from wdStringBase and thus provides a large set of functions for search and comparisons.
///
/// Attention: wdStringView does not store string data itself. It only stores pointers into memory. For example,
/// when you get an wdStringView to an wdStringBuilder, the wdStringView instance will point to the exact same memory,
/// enabling you to iterate over it (read-only).
/// That means that an wdStringView is only valid as long as its source data is not modified. Once you make any kind
/// of modification to the source data, you should not continue using the wdStringView to that data anymore,
/// as it might now point into invalid memory.
struct WD_FOUNDATION_DLL wdStringView : public wdThisIsAString
{
public:
  WD_DECLARE_POD_TYPE();

  using iterator = wdStringIterator;
  using const_iterator = wdStringIterator;
  using reverse_iterator = wdStringReverseIterator;
  using const_reverse_iterator = wdStringReverseIterator;

  /// \brief Default constructor creates an invalid view.
  constexpr wdStringView();

  /// \brief Creates a string view starting at the given position, ending at the next '\0' terminator.
  wdStringView(char* pStart);

  /// \brief Creates a string view starting at the given position, ending at the next '\0' terminator.
  template <typename T>                                                                                           // T is always const char*
  constexpr wdStringView(T pStart, typename std::enable_if<std::is_same<T, const char*>::value, int>::type* = 0); // [tested]

  /// \brief Creates a string view from any class / struct which is implicitly convertible to const char *
  template <typename T>
  WD_ALWAYS_INLINE wdStringView(const T&& str, typename std::enable_if<std::is_same<T, const char*>::value == false && std::is_convertible<T, const char*>::value, int>::type* = 0); // [tested]

  /// \brief Creates a string view for the range from pStart to pEnd.
  wdStringView(const char* pStart, const char* pEnd); // [tested]

  /// \brief Creates a string view for the range from pStart to pStart + uiLength.
  constexpr wdStringView(const char* pStart, wdUInt32 uiLength);

  /// \brief Construct a string view from a string literal.
  template <size_t N>
  wdStringView(const char (&str)[N]);

  /// \brief Construct a string view from a fixed size buffer
  template <size_t N>
  wdStringView(char (&str)[N]);

  /// \brief Advances the start to the next character, unless the end of the range was reached.
  void operator++(); // [tested]

  /// \brief Advances the start forwards by d characters. Does not move it beyond the range's end.
  void operator+=(wdUInt32 d); // [tested]

  /// \brief Returns the first pointed to character in Utf32 encoding.
  wdUInt32 GetCharacter() const; // [tested]

  /// \brief Returns true, if the current string pointed to is non empty.
  bool IsValid() const; // [tested]

  /// \brief Returns the data as a zero-terminated string.
  ///
  /// The string will be copied to \a tempStorage and the pointer to that is returned.
  /// If you really need the raw pointer to the wdStringView memory or are absolutely certain that the view points
  /// to a zero-terminated string, you can use GetStartPointer()
  const char* GetData(wdStringBuilder& ref_sTempStorage) const; // [tested]

  /// \brief Returns the number of bytes from the start position up to its end.
  ///
  /// \note Note that the element count (bytes) may be larger than the number of characters in that string, due to Utf8 encoding.
  wdUInt32 GetElementCount() const { return (wdUInt32)(m_pEnd - m_pStart); } // [tested]

  /// \brief Allows to set the start position to a different value.
  ///
  /// Must be between the current start and end range.
  void SetStartPosition(const char* szCurPos); // [tested]

  /// \brief Returns the start of the view range.
  /// \note Be careful to not use this and assume the view will be zero-terminated. Use GetData(wdStringBuilder&) instead to be safe.
  const char* GetStartPointer() const { return m_pStart; } // [tested]

  /// \brief Returns the end of the view range. This will point to the byte AFTER the last character.
  ///
  /// That means it might point to the '\0' terminator, UNLESS the view only represents a sub-string of a larger string.
  /// Accessing the value at 'GetEnd' has therefore no real use.
  const char* GetEndPointer() const { return m_pEnd; } // [tested]

  /// Returns whether the string is an empty string.
  bool IsEmpty() const; // [tested]

  /// \brief Compares this string view with the other string view for equality.
  bool IsEqual(wdStringView sOther) const;

  /// \brief Compares this string view with the other string view for equality.
  bool IsEqual_NoCase(wdStringView sOther) const;

  /// Compares up to a given number of characters of this string with the other string for equality. Case insensitive.
  bool IsEqualN(wdStringView sOther, wdUInt32 uiCharsToCompare) const; // [tested]

  /// Compares up to a given number of characters of this string with the other string for equality. Case insensitive.
  bool IsEqualN_NoCase(wdStringView sOther, wdUInt32 uiCharsToCompare) const; // [tested]

  /// Compares this string with the other one. Returns 0 for equality, -1 if this string is 'smaller', 1 otherwise.
  wdInt32 Compare(wdStringView sOther) const; // [tested]

  /// Compares up to a given number of characters of this string with the other one. Returns 0 for equality, -1 if this string is 'smaller',
  /// 1 otherwise.
  wdInt32 CompareN(wdStringView sOther, wdUInt32 uiCharsToCompare) const; // [tested]

  /// Compares this string with the other one. Returns 0 for equality, -1 if this string is 'smaller', 1 otherwise. Case insensitive.
  wdInt32 Compare_NoCase(wdStringView sOther) const; // [tested]

  /// Compares up to a given number of characters of this string with the other one. Returns 0 for equality, -1 if this string is 'smaller',
  /// 1 otherwise. Case insensitive.
  wdInt32 CompareN_NoCase(wdStringView sOther, wdUInt32 uiCharsToCompare) const; // [tested]

  /// Returns true, if this string starts with the given string.
  bool StartsWith(wdStringView sStartsWith) const; // [tested]

  /// Returns true, if this string starts with the given string. Case insensitive.
  bool StartsWith_NoCase(wdStringView sStartsWith) const; // [tested]

  /// Returns true, if this string ends with the given string.
  bool EndsWith(wdStringView sEndsWith) const; // [tested]

  /// Returns true, if this string ends with the given string. Case insensitive.
  bool EndsWith_NoCase(wdStringView sEndsWith) const; // [tested]

  /// \brief Computes the pointer to the n-th character in the string. This is a linear search from the start.
  const char* ComputeCharacterPosition(wdUInt32 uiCharacterIndex) const;

  /// Returns a pointer to the first occurrence of szStringToFind, or nullptr if none was found.
  /// To find the next occurrence, use an wdStringView which points to the next position and call FindSubString again.
  const char* FindSubString(wdStringView sStringToFind, const char* szStartSearchAt = nullptr) const; // [tested]

  /// Returns a pointer to the first occurrence of szStringToFind, or nullptr if none was found. Case insensitive.
  /// To find the next occurrence, use an wdStringView which points to the next position and call FindSubString again.
  const char* FindSubString_NoCase(wdStringView sStringToFind, const char* szStartSearchAt = nullptr) const; // [tested]

  /// Returns a pointer to the last occurrence of szStringToFind, or nullptr if none was found.
  /// szStartSearchAt allows to start searching at the end of the string (if it is nullptr) or at an earlier position.
  const char* FindLastSubString(wdStringView sStringToFind, const char* szStartSearchAt = nullptr) const; // [tested]

  /// Returns a pointer to the last occurrence of szStringToFind, or nullptr if none was found. Case insensitive.
  /// szStartSearchAt allows to start searching at the end of the string (if it is nullptr) or at an earlier position.
  const char* FindLastSubString_NoCase(wdStringView sStringToFind, const char* szStartSearchAt = nullptr) const; // [tested]

  /// Searches for the word szSearchFor. If IsDelimiterCB returns true for both characters in front and back of the word, the position is
  /// returned. Otherwise nullptr.
  const char* FindWholeWord(const char* szSearchFor, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt = nullptr) const; // [tested]

  /// Searches for the word szSearchFor. If IsDelimiterCB returns true for both characters in front and back of the word, the position is
  /// returned. Otherwise nullptr. Ignores case.
  const char* FindWholeWord_NoCase(const char* szSearchFor, wdStringUtils::WD_CHARACTER_FILTER isDelimiterCB, const char* szStartSearchAt = nullptr) const; // [tested]


  /// \brief Shrinks the view range by uiShrinkCharsFront characters at the front and by uiShrinkCharsBack characters at the back.
  ///
  /// Thus reduces the range of the view to a smaller sub-string.
  /// The current position is clamped to the new start of the range.
  /// The new end position is clamped to the new start of the range.
  /// If more characters are removed from the range, than it actually contains, the view range will become 'empty'
  /// and its state will be set to invalid, however no error or assert will be triggered.
  void Shrink(wdUInt32 uiShrinkCharsFront, wdUInt32 uiShrinkCharsBack); // [tested]

  /// \brief Returns a sub-string that is shrunk at the start and front by the given amount of characters (not bytes!).
  wdStringView GetShrunk(wdUInt32 uiShrinkCharsFront, wdUInt32 uiShrinkCharsBack = 0) const;

  /// \brief Identical to 'Shrink(1, 0)' in functionality, but slightly more efficient.
  void ChopAwayFirstCharacterUtf8(); // [tested]

  /// \brief Similar to ChopAwayFirstCharacterUtf8(), but assumes that the first character is ASCII and thus exactly one byte in length.
  /// Asserts that this is the case.
  /// More efficient than ChopAwayFirstCharacterUtf8(), if it is known that the first character is ASCII.
  void ChopAwayFirstCharacterAscii(); // [tested]

  /// \brief Removes all characters from the start and end that appear in the given strings by adjusting the begin and end of the view.
  void Trim(const char* szTrimChars); // [tested]

  /// \brief Removes all characters from the start and/or end that appear in the given strings by adjusting the begin and end of the view.
  void Trim(const char* szTrimCharsStart, const char* szTrimCharsEnd); // [tested]

  /// \brief If the string starts with one of the given words (case insensitive), it is removed and the function returns true.
  bool TrimWordStart(wdStringView sWord1, wdStringView sWord2 = {}, wdStringView sWord3 = {}, wdStringView sWord4 = {}, wdStringView sWord5 = {}); // [tested]

  /// \brief If the string ends with one of the given words (case insensitive), it is removed and the function returns true.
  bool TrimWordEnd(wdStringView sWord1, wdStringView sWord2 = {}, wdStringView sWord3 = {}, wdStringView sWord4 = {}, wdStringView sWord5 = {}); // [tested]

  /// \brief Fills the given container with wdStringView's which represent each found substring.
  /// If bReturnEmptyStrings is true, even empty strings between separators are returned.
  /// Output must be a container that stores wdStringView's and provides the functions 'Clear' and 'Append'.
  /// szSeparator1 to szSeparator6 are strings which act as separators and indicate where to split the string.
  /// This string itself will not be modified.
  template <typename Container>
  void Split(bool bReturnEmptyStrings, Container& ref_output, const char* szSeparator1, const char* szSeparator2 = nullptr, const char* szSeparator3 = nullptr, const char* szSeparator4 = nullptr, const char* szSeparator5 = nullptr, const char* szSeparator6 = nullptr) const; // [tested]

  /// \brief Returns an iterator to this string, which points to the very first character.
  ///
  /// Note that this iterator will only be valid as long as this string lives.
  /// Once the original string is destroyed, all iterators to them will point into invalid memory.
  iterator GetIteratorFront() const;

  /// \brief Returns an iterator to this string, which points to the very last character (NOT the end).
  ///
  /// Note that this iterator will only be valid as long as this string lives.
  /// Once the original string is destroyed, all iterators to them will point into invalid memory.
  reverse_iterator GetIteratorBack() const;

  // ******* Path Functions ********

  /// \brief Checks whether the given path has any file extension
  bool HasAnyExtension() const; // [tested]

  /// \brief Checks whether the given path ends with the given extension. szExtension should start with a '.' for performance reasons, but
  /// it will work without a '.' too.
  bool HasExtension(wdStringView sExtension) const; // [tested]

  /// \brief Returns the file extension of the given path. Will be empty, if the path does not end with a proper extension.
  wdStringView GetFileExtension() const; // [tested]

  /// \brief Returns the file name of a path, excluding the path and extension.
  ///
  /// If the path already ends with a path separator, the result will be empty.
  wdStringView GetFileName() const; // [tested]

  /// \brief Returns the substring that represents the file name including the file extension.
  ///
  /// Returns an empty string, if sPath already ends in a path separator, or is empty itself.
  wdStringView GetFileNameAndExtension() const; // [tested]

  /// \brief Returns the directory of the given file, which is the substring up to the last path separator.
  ///
  /// If the path already ends in a path separator, and thus points to a folder, instead of a file, the unchanged path is returned.
  /// "path/to/file" -> "path/to/"
  /// "path/to/folder/" -> "path/to/folder/"
  /// "filename" -> ""
  /// "/file_at_root_level" -> "/"
  wdStringView GetFileDirectory() const; // [tested]

  /// \brief Returns true, if the given path represents an absolute path on the current OS.
  bool IsAbsolutePath() const; // [tested]

  /// \brief Returns true, if the given path represents a relative path on the current OS.
  bool IsRelativePath() const; // [tested]

  /// \brief Returns true, if the given path represents a 'rooted' path. See wdFileSystem for details.
  bool IsRootedPath() const; // [tested]

  /// \brief Extracts the root name from a rooted path
  ///
  /// ":MyRoot" -> "MyRoot"
  /// ":MyRoot\folder" -> "MyRoot"
  /// ":\MyRoot\folder" -> "MyRoot"
  /// ":/MyRoot\folder" -> "MyRoot"
  /// Returns an empty string, if the path is not rooted.
  wdStringView GetRootedPathRootName() const; // [tested]

private:
  const char* m_pStart = nullptr;
  const char* m_pEnd = nullptr;
};

/// \brief String literal suffix to create a wdStringView.
///
/// Example:
/// "Hello World"
constexpr wdStringView operator"" _wdsv(const char* pString, size_t uiLen);

WD_ALWAYS_INLINE typename wdStringView::iterator begin(wdStringView sContainer)
{
  return typename wdStringView::iterator(sContainer.GetStartPointer(), sContainer.GetEndPointer(), sContainer.GetStartPointer());
}

WD_ALWAYS_INLINE typename wdStringView::const_iterator cbegin(wdStringView sContainer)
{
  return typename wdStringView::const_iterator(sContainer.GetStartPointer(), sContainer.GetEndPointer(), sContainer.GetStartPointer());
}

WD_ALWAYS_INLINE typename wdStringView::iterator end(wdStringView sContainer)
{
  return typename wdStringView::iterator(sContainer.GetStartPointer(), sContainer.GetEndPointer(), sContainer.GetEndPointer());
}

WD_ALWAYS_INLINE typename wdStringView::const_iterator cend(wdStringView sContainer)
{
  return typename wdStringView::const_iterator(sContainer.GetStartPointer(), sContainer.GetEndPointer(), sContainer.GetEndPointer());
}


WD_ALWAYS_INLINE typename wdStringView::reverse_iterator rbegin(wdStringView sContainer)
{
  return typename wdStringView::reverse_iterator(sContainer.GetStartPointer(), sContainer.GetEndPointer(), sContainer.GetEndPointer());
}

WD_ALWAYS_INLINE typename wdStringView::const_reverse_iterator crbegin(wdStringView sContainer)
{
  return typename wdStringView::const_reverse_iterator(sContainer.GetStartPointer(), sContainer.GetEndPointer(), sContainer.GetEndPointer());
}

WD_ALWAYS_INLINE typename wdStringView::reverse_iterator rend(wdStringView sContainer)
{
  return typename wdStringView::reverse_iterator(sContainer.GetStartPointer(), sContainer.GetEndPointer(), nullptr);
}

WD_ALWAYS_INLINE typename wdStringView::const_reverse_iterator crend(wdStringView sContainer)
{
  return typename wdStringView::const_reverse_iterator(sContainer.GetStartPointer(), sContainer.GetEndPointer(), nullptr);
}

#include <Foundation/Strings/Implementation/StringView_inl.h>
