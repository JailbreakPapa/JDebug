#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Containers/HybridArray.h>
#include <Foundation/Strings/Implementation/StringBase.h>
#include <Foundation/Strings/StringConversion.h>
#include <Foundation/Strings/StringUtils.h>
#include <Foundation/Strings/StringView.h>

class nsStringBuilder;
class nsStreamReader;

/// \brief A string class for storing and passing around strings.
///
/// This class only allows read-access to its data. It does not allow modifications.
/// To build / modify strings, use the nsStringBuilder class.
/// nsHybridString has an internal array to store short strings without any memory allocations, it will dynamically
/// allocate additional memory, if that cache is insufficient. Thus a hybrid string will always take up a certain amount
/// of memory, which might be of concern when it is used as a member variable, in such cases you might want to use an
/// nsHybridString with a very small internal array (1 would basically make it into a completely dynamic string).
/// On the other hand, creating nsHybridString instances on the stack and working locally with them, is quite fast.
/// Prefer to use the typedef'd string types \a nsString, \a nsDynamicString, \a nsString32 etc.
/// Most strings in an application are rather short, typically shorter than 20 characters.
/// Use \a nsString, which is a typedef'd nsHybridString to use a cache size that is sufficient for more than 90%
/// of all use cases.
template <nsUInt16 Size>
struct nsHybridStringBase : public nsStringBase<nsHybridStringBase<Size>>
{
protected:
  /// \brief Creates an empty string.
  nsHybridStringBase(nsAllocator* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  nsHybridStringBase(const nsHybridStringBase& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Moves the data from \a rhs.
  nsHybridStringBase(nsHybridStringBase&& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  nsHybridStringBase(const char* rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  nsHybridStringBase(const wchar_t* rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  nsHybridStringBase(const nsStringView& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Copies the data from \a rhs.
  nsHybridStringBase(const nsStringBuilder& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Moves the data from \a rhs.
  nsHybridStringBase(nsStringBuilder&& rhs, nsAllocator* pAllocator); // [tested]

  /// \brief Destructor.
  ~nsHybridStringBase(); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const nsHybridStringBase& rhs); // [tested]

  /// \brief Moves the data from \a rhs.
  void operator=(nsHybridStringBase&& rhs); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const char* rhs); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const wchar_t* rhs); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const nsStringView& rhs); // [tested]

  /// \brief Copies the data from \a rhs.
  void operator=(const nsStringBuilder& rhs); // [tested]

  /// \brief Moves the data from \a rhs.
  void operator=(nsStringBuilder&& rhs); // [tested]

#if NS_ENABLED(NS_INTEROP_STL_STRINGS)
  /// \brief Copies the data from \a rhs.
  nsHybridStringBase(const std::string_view& rhs, nsAllocator* pAllocator);

  /// \brief Copies the data from \a rhs.
  nsHybridStringBase(const std::string& rhs, nsAllocator* pAllocator);

  /// \brief Copies the data from \a rhs.
  void operator=(const std::string_view& rhs);

  /// \brief Copies the data from \a rhs.
  void operator=(const std::string& rhs);
#endif

public:
  /// \brief Resets this string to an empty string.
  ///
  /// This will not deallocate any previously allocated data, but reuse that memory.
  void Clear(); // [tested]

  /// \brief Returns a pointer to the internal Utf8 string.
  const char* GetData() const; // [tested]

  /// \brief Returns the amount of bytes that this string takes (excluding the '\0' terminator).
  nsUInt32 GetElementCount() const; // [tested]

  /// \brief Returns the number of characters in this string. Might be less than GetElementCount, if it contains Utf8
  /// multi-byte characters.
  ///
  /// \note This is a slow operation, as it has to run through the entire string to count the Unicode characters.
  /// Only call this once and use the result as long as the string doesn't change. Don't call this in a loop.
  nsUInt32 GetCharacterCount() const; // [tested]

  /// \brief Returns a view to a sub-string of this string, starting at character uiFirstCharacter, up until uiFirstCharacter +
  /// uiNumCharacters.
  ///
  /// Note that this view will only be valid as long as this nsHybridString lives.
  /// Once the original string is destroyed, all views to them will point into invalid memory.
  nsStringView GetSubString(nsUInt32 uiFirstCharacter, nsUInt32 uiNumCharacters) const; // [tested]

  /// \brief Returns a view to the sub-string containing the first uiNumCharacters characters of this string.
  ///
  /// Note that this view will only be valid as long as this nsHybridString lives.
  /// Once the original string is destroyed, all views to them will point into invalid memory.
  nsStringView GetFirst(nsUInt32 uiNumCharacters) const; // [tested]

  /// \brief Returns a view to the sub-string containing the last uiNumCharacters characters of this string.
  ///
  /// Note that this view will only be valid as long as this nsHybridString lives.
  /// Once the original string is destroyed, all views to them will point into invalid memory.
  nsStringView GetLast(nsUInt32 uiNumCharacters) const; // [tested]

  /// \brief Replaces the current string with the content from the stream. Reads the stream to its end.
  void ReadAll(nsStreamReader& inout_stream);

  /// \brief Returns the amount of bytes that are currently allocated on the heap.
  nsUInt64 GetHeapMemoryUsage() const { return m_Data.GetHeapMemoryUsage(); }

private:
  friend class nsStringBuilder;

  nsHybridArray<char, Size> m_Data;
};


/// \brief \see nsHybridStringBase
template <nsUInt16 Size, typename AllocatorWrapper = nsDefaultAllocatorWrapper>
struct nsHybridString : public nsHybridStringBase<Size>
{
public:
  nsHybridString();
  nsHybridString(nsAllocator* pAllocator);

  nsHybridString(const nsHybridString<Size, AllocatorWrapper>& other);
  nsHybridString(const nsHybridStringBase<Size>& other);
  nsHybridString(const char* rhs);
  nsHybridString(const wchar_t* rhs);
  nsHybridString(const nsStringView& rhs);
  nsHybridString(const nsStringBuilder& rhs);
  nsHybridString(nsStringBuilder&& rhs);
  nsHybridString(nsHybridString<Size, AllocatorWrapper>&& other);
  nsHybridString(nsHybridStringBase<Size>&& other);

  void operator=(const nsHybridString<Size, AllocatorWrapper>& rhs);
  void operator=(const nsHybridStringBase<Size>& rhs);
  void operator=(const char* szString);
  void operator=(const wchar_t* pString);
  void operator=(const nsStringView& rhs);
  void operator=(const nsStringBuilder& rhs);
  void operator=(nsStringBuilder&& rhs);
  void operator=(nsHybridString<Size, AllocatorWrapper>&& rhs);
  void operator=(nsHybridStringBase<Size>&& rhs);

#if NS_ENABLED(NS_INTEROP_STL_STRINGS)
  nsHybridString(const std::string_view& rhs);
  nsHybridString(const std::string& rhs);
  void operator=(const std::string_view& rhs);
  void operator=(const std::string& rhs);
#endif
};

/// \brief String that uses the static allocator to prevent leak reports in RTTI attributes.
using nsUntrackedString = nsHybridString<32, nsStaticsAllocatorWrapper>;

using nsDynamicString = nsHybridString<1>;
using nsString = nsHybridString<32>;
using nsString16 = nsHybridString<16>;
using nsString24 = nsHybridString<24>;
using nsString32 = nsHybridString<32>;
using nsString48 = nsHybridString<48>;
using nsString64 = nsHybridString<64>;
using nsString128 = nsHybridString<128>;
using nsString256 = nsHybridString<256>;

static_assert(nsGetTypeClass<nsString>::value == nsTypeIsClass::value);

template <nsUInt16 Size>
struct nsCompareHelper<nsHybridString<Size>>
{
  static NS_ALWAYS_INLINE bool Less(nsStringView lhs, nsStringView rhs)
  {
    return lhs.Compare(rhs) < 0;
  }

  static NS_ALWAYS_INLINE bool Equal(nsStringView lhs, nsStringView rhs)
  {
    return lhs.IsEqual(rhs);
  }
};

struct nsCompareString_NoCase
{
  static NS_ALWAYS_INLINE bool Less(nsStringView lhs, nsStringView rhs)
  {
    return lhs.Compare_NoCase(rhs) < 0;
  }

  static NS_ALWAYS_INLINE bool Equal(nsStringView lhs, nsStringView rhs)
  {
    return lhs.IsEqual_NoCase(rhs);
  }
};

struct CompareConstChar
{
  /// \brief Returns true if a is less than b
  static NS_ALWAYS_INLINE bool Less(const char* a, const char* b) { return nsStringUtils::Compare(a, b) < 0; }

  /// \brief Returns true if a is equal to b
  static NS_ALWAYS_INLINE bool Equal(const char* a, const char* b) { return nsStringUtils::IsEqual(a, b); }
};

// For nsFormatString
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsString& sArg);
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsUntrackedString& sArg);

#include <Foundation/Strings/Implementation/String_inl.h>
