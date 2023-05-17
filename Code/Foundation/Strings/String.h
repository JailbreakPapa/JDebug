#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/Implementation/StringBase.h>
#include <Foundation/Strings/StringConversion.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Strings/StringView.h>

class wdStringBuilder;
class wdStreamReader;

/// \brief A string class for storing and passing around strings.
///
/// This class only allows read-access to its data. It does not allow modifications.
/// To build / modify strings, use the wdStringBuilder class.
/// wdHybridString has an internal array to store short strings without any memory allocations, it will dynamically
/// allocate additional memory, if that cache is insufficient. Thus a hybrid string will always take up a certain amount
/// of memory, which might be of concern when it is used as a member variable, in such cases you might want to use an
/// wdHybridString with a very small internal array (1 would basically make it into a completely dynamic string).
/// On the other hand, creating wdHybridString instances on the stack and working locally with them, is quite fast.
/// Prefer to use the typedef'd string types \a wdString, \a wdDynamicString, \a wdString32 etc.
/// Most strings in an application are rather short, typically shorter than 20 characters.
/// Use \a wdString, which is a typedef'd wdHybridString to use a cache size that is sufficient for more than 90%
/// of all use cases.
template <wdUInt16 Size>
struct wdHybridStringBase : public wdStringBase<wdHybridStringBase<Size>>
{
protected:
  /// \brief Creates an empty string.
  wdHybridStringBase(wdAllocatorBase* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  wdHybridStringBase(const wdHybridStringBase& rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Moves the data from \a rhs.
  wdHybridStringBase(wdHybridStringBase&& rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  wdHybridStringBase(const char* rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  wdHybridStringBase(const wchar_t* rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  wdHybridStringBase(const wdStringView& rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  wdHybridStringBase(const wdStringBuilder& rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Moves the data from \a rhs.
  wdHybridStringBase(wdStringBuilder&& rhs, wdAllocatorBase* pAllocator); // [tested]

  /// \brief Destructor.
  ~wdHybridStringBase(); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const wdHybridStringBase& rhs); // [tested]

  /// \brief Moves the data from \a rhs.
  void operator=(wdHybridStringBase&& rhs); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const char* rhs); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const wchar_t* rhs); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const wdStringView& rhs); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const wdStringBuilder& rhs); // [tested]

  /// \brief Moves the data from \a rhs.
  void operator=(wdStringBuilder&& rhs); // [tested]

public:

  /// \brief Resets this string to an empty string.
  ///
  /// This will not deallocate any previously allocated data, but reuse that memory.
  void Clear(); // [tested]

  /// \brief Returns a pointer to the internal Utf8 string.
  const char* GetData() const; // [tested]

  /// \brief Returns the amount of bytes that this string takes (excluding the '\0' terminator).
  wdUInt32 GetElementCount() const; // [tested]

  /// \brief Returns the number of characters in this string.
  wdUInt32 GetCharacterCount() const; // [tested]

  /// \brief Returns a view to a sub-string of this string, starting at character uiFirstCharacter, up until uiFirstCharacter +
  /// uiNumCharacters.
  ///
  /// Note that this view will only be valid as long as this wdHybridString lives.
  /// Once the original string is destroyed, all views to them will point into invalid memory.
  wdStringView GetSubString(wdUInt32 uiFirstCharacter, wdUInt32 uiNumCharacters) const; // [tested]

  /// \brief Returns a view to the sub-string containing the first uiNumCharacters characters of this string.
  ///
  /// Note that this view will only be valid as long as this wdHybridString lives.
  /// Once the original string is destroyed, all views to them will point into invalid memory.
  wdStringView GetFirst(wdUInt32 uiNumCharacters) const; // [tested]

  /// \brief Returns a view to the sub-string containing the last uiNumCharacters characters of this string.
  ///
  /// Note that this view will only be valid as long as this wdHybridString lives.
  /// Once the original string is destroyed, all views to them will point into invalid memory.
  wdStringView GetLast(wdUInt32 uiNumCharacters) const; // [tested]

  /// \brief Replaces the current string with the content from the stream. Reads the stream to its end.
  void ReadAll(wdStreamReader& inout_stream);

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  wdUInt64 GetHeapMemoryUsage() const { return m_Data.GetHeapMemoryUsage(); }

private:
  friend class wdStringBuilder;

  wdHybridArray<char, Size> m_Data;
  wdUInt32 m_uiCharacterCount = 0;
};


/// \brief \see wdHybridStringBase
template <wdUInt16 Size, typename AllocatorWrapper = wdDefaultAllocatorWrapper>
struct wdHybridString : public wdHybridStringBase<Size>
{
public:
  WD_DECLARE_MEM_RELOCATABLE_TYPE();

  wdHybridString();
  wdHybridString(wdAllocatorBase* pAllocator);

  wdHybridString(const wdHybridString<Size, AllocatorWrapper>& other);
  wdHybridString(const wdHybridStringBase<Size>& other);
  wdHybridString(const char* rhs);
  wdHybridString(const wchar_t* rhs);
  wdHybridString(const wdStringView& rhs);
  wdHybridString(const wdStringBuilder& rhs);
  wdHybridString(wdStringBuilder&& rhs);

  wdHybridString(wdHybridString<Size, AllocatorWrapper>&& other);
  wdHybridString(wdHybridStringBase<Size>&& other);


  void operator=(const wdHybridString<Size, AllocatorWrapper>& rhs);
  void operator=(const wdHybridStringBase<Size>& rhs);
  void operator=(const char* szString);
  void operator=(const wchar_t* pString);
  void operator=(const wdStringView& rhs);
  void operator=(const wdStringBuilder& rhs);
  void operator=(wdStringBuilder&& rhs);

  void operator=(wdHybridString<Size, AllocatorWrapper>&& rhs);
  void operator=(wdHybridStringBase<Size>&& rhs);
};

using wdDynamicString = wdHybridString<1>;
/// \brief String that uses the static allocator to prevent leak reports in RTTI attributes.
using wdUntrackedString = wdHybridString<32, wdStaticAllocatorWrapper>;
using wdString = wdHybridString<32>;
using wdString16 = wdHybridString<16>;
using wdString24 = wdHybridString<24>;
using wdString32 = wdHybridString<32>;
using wdString48 = wdHybridString<48>;
using wdString64 = wdHybridString<64>;
using wdString128 = wdHybridString<128>;
using wdString256 = wdHybridString<256>;

WD_CHECK_AT_COMPILETIME_MSG(wdGetTypeClass<wdString>::value == 2, "string is not memory relocatable");

template <wdUInt16 Size>
struct wdCompareHelper<wdHybridString<Size>>
{
  WD_ALWAYS_INLINE bool Less(wdStringView lhs, wdStringView rhs) const
  {
    return lhs.Compare(rhs) < 0;
  }

  WD_ALWAYS_INLINE bool Equal(wdStringView lhs, wdStringView rhs) const
  {
    return lhs.IsEqual(rhs);
  }
};

struct wdCompareString_NoCase
{
  WD_ALWAYS_INLINE bool Less(wdStringView lhs, wdStringView rhs) const
  {
    return lhs.Compare_NoCase(rhs) < 0;
  }

  WD_ALWAYS_INLINE bool Equal(wdStringView lhs, wdStringView rhs) const
  {
    return lhs.IsEqual_NoCase(rhs);
  }
};

struct CompareConstChar
{
  /// \brief Returns true if a is less than b
  WD_ALWAYS_INLINE bool Less(const char* a, const char* b) const { return wdStringUtils::Compare(a, b) < 0; }

  /// \brief Returns true if a is equal to b
  WD_ALWAYS_INLINE bool Equal(const char* a, const char* b) const { return wdStringUtils::IsEqual(a, b); }
};

// For wdFormatString
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdString& sArg);
WD_FOUNDATION_DLL wdStringView BuildString(char* szTmp, wdUInt32 uiLength, const wdUntrackedString& sArg);

#include <Foundation/Strings/Implementation/String_inl.h>
