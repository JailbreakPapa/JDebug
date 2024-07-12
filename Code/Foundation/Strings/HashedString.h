#pragma once

#include <Foundation/Algorithm/HashingUtils.h>
#include <Foundation/Containers/Map.h>
#include <Foundation/Strings/String.h>
#include <Foundation/Threading/AtomicInteger.h>

class nsTempHashedString;

/// \brief This class is optimized to take nearly no memory (sizeof(void*)) and to allow very fast checks whether two strings are identical.
///
/// Internally only a reference to the string data is stored. The data itself is stored in a central location, where no duplicates are
/// possible. Thus two identical strings will result in identical nsHashedString objects, which makes equality comparisons very easy
/// (it's a pointer comparison).\n
/// Copying nsHashedString objects around and assigning between them is very fast as well.\n
/// \n
/// Assigning from some other string type is rather slow though, as it requires thread synchronization.\n
/// You can also get access to the actual string data via GetString().\n
/// \n
/// You should use nsHashedString whenever the size of the encapsulating object is important and when changes to the string itself
/// are rare, but checks for equality might be frequent (e.g. in a system where objects are identified via their name).\n
/// At runtime when you need to compare nsHashedString objects with some temporary string object, used nsTempHashedString,
/// as it will only use the string's hash value for comparison, but will not store the actual string anywhere.
class NS_FOUNDATION_DLL nsHashedString
{
public:
  struct HashedData
  {
#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
    nsAtomicInteger32 m_iRefCount;
#endif
    nsString m_sString;
  };

  // Do NOT use a hash-table! The map does not relocate memory when it resizes, which is a vital aspect for the hashed strings to work.
  using StringStorage = nsMap<nsUInt64, HashedData, nsCompareHelper<nsUInt64>, nsStaticsAllocatorWrapper>;
  using HashedType = StringStorage::Iterator;

#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  /// \brief This will remove all hashed strings from the central storage, that are not referenced anymore.
  ///
  /// All hashed string values are stored in a central location and nsHashedString just references them. Those strings are then
  /// reference counted. Once some string is not referenced anymore, its ref count reaches zero, but it will not be removed from
  /// the storage, as it might be reused later again.
  /// This function will clean up all unused strings. It should typically not be necessary to call this function at all, unless lots of
  /// strings get stored in nsHashedString that are not really used throughout the applications life time.
  ///
  /// Returns the number of unused strings that were removed.
  static nsUInt32 ClearUnusedStrings();
#endif

  NS_DECLARE_MEM_RELOCATABLE_TYPE();

  /// \brief Initializes this string to the empty string.
  nsHashedString(); // [tested]

  /// \brief Copies the given nsHashedString.
  nsHashedString(const nsHashedString& rhs); // [tested]

  /// \brief Moves the given nsHashedString.
  nsHashedString(nsHashedString&& rhs); // [tested]

#if NS_ENABLED(NS_HASHED_STRING_REF_COUNTING)
  /// \brief Releases the reference to the internal data. Does NOT deallocate any data, even if this held the last reference to some string.
  ~nsHashedString();
#endif

  /// \brief Copies the given nsHashedString.
  void operator=(const nsHashedString& rhs); // [tested]

  /// \brief Moves the given nsHashedString.
  void operator=(nsHashedString&& rhs); // [tested]

  /// \brief Assigning a new string from a string constant is a slow operation, but the hash computation can happen at compile time.
  ///
  /// If you need to create an object to compare nsHashedString objects against, prefer to use nsTempHashedString. It will only compute
  /// the strings hash value, but does not require any thread synchronization.
  template <size_t N>
  void Assign(const char (&string)[N]); // [tested]

  template <size_t N>
  void Assign(char (&string)[N]) = delete;

  /// \brief Assigning a new string from a non-hashed string is a very slow operation, this should be used rarely.
  ///
  /// If you need to create an object to compare nsHashedString objects against, prefer to use nsTempHashedString. It will only compute
  /// the strings hash value, but does not require any thread synchronization.
  void Assign(nsStringView sString); // [tested]

  /// \brief Comparing whether two hashed strings are identical is just a pointer comparison. This operation is what nsHashedString is
  /// optimized for.
  ///
  /// \note Comparing between nsHashedString objects is always error-free, so even if two string had the same hash value, although they are
  /// different, this comparison function will not report they are the same.
  bool operator==(const nsHashedString& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsHashedString&);

  /// \brief Compares this string object to an nsTempHashedString object. This should be used whenever some object needs to be found
  /// and the string to compare against is not yet an nsHashedString object.
  bool operator==(const nsTempHashedString& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsTempHashedString&);

  /// \brief This operator allows sorting objects by hash value, not by alphabetical order.
  bool operator<(const nsHashedString& rhs) const; // [tested]

  /// \brief This operator allows sorting objects by hash value, not by alphabetical order.
  bool operator<(const nsTempHashedString& rhs) const; // [tested]

  /// \brief Gives access to the actual string data, so you can do all the typical (read-only) string operations on it.
  const nsString& GetString() const; // [tested]

  /// \brief Gives access to the actual string data, so you can do all the typical (read-only) string operations on it.
  const char* GetData() const;

  /// \brief Returns the hash of the stored string.
  nsUInt64 GetHash() const; // [tested]

  /// \brief Returns whether the string is empty.
  bool IsEmpty() const;

  /// \brief Resets the string to the empty string.
  void Clear();

  /// \brief Returns a string view to this string's data.
  NS_ALWAYS_INLINE operator nsStringView() const { return GetString().GetView(); }

  /// \brief Returns a string view to this string's data.
  NS_ALWAYS_INLINE nsStringView GetView() const { return GetString().GetView(); }

  /// \brief Returns a pointer to the internal Utf8 string.
  NS_ALWAYS_INLINE operator const char*() const { return GetData(); }

private:
  static void InitHashedString();
  static HashedType AddHashedString(nsStringView sString, nsUInt64 uiHash);

  HashedType m_Data;
};

/// \brief Helper function to create an nsHashedString. This can be used to initialize static hashed string variables.
template <size_t N>
nsHashedString nsMakeHashedString(const char (&string)[N]);


/// \brief A class to use together with nsHashedString for quick comparisons with temporary strings that need not be stored further.
///
/// Whenever you have objects that use nsHashedString members and you need to compare against them with some temporary string,
/// prefer to use nsTempHashedString instead of nsHashedString, as the latter requires thread synchronization to actually set up the
/// object.
class NS_FOUNDATION_DLL nsTempHashedString
{
  friend class nsHashedString;

public:
  nsTempHashedString(); // [tested]

  /// \brief Creates an nsTempHashedString object from the given string constant. The hash can be computed at compile time.
  template <size_t N>
  nsTempHashedString(const char (&string)[N]); // [tested]

  template <size_t N>
  nsTempHashedString(char (&string)[N]) = delete;

  /// \brief Creates an nsTempHashedString object from the given string. Computes the hash of the given string during runtime, which might
  /// be slow.
  explicit nsTempHashedString(nsStringView sString); // [tested]

  /// \brief Copies the hash from rhs.
  nsTempHashedString(const nsTempHashedString& rhs); // [tested]

  /// \brief Copies the hash from the nsHashedString.
  nsTempHashedString(const nsHashedString& rhs); // [tested]

  explicit nsTempHashedString(nsUInt32 uiHash) = delete;

  /// \brief Copies the hash from the 64 bit integer.
  explicit nsTempHashedString(nsUInt64 uiHash);

  /// \brief The hash of the given string can be computed at compile time.
  template <size_t N>
  void operator=(const char (&string)[N]); // [tested]

  /// \brief Computes and stores the hash of the given string during runtime, which might be slow.
  void operator=(nsStringView sString); // [tested]

  /// \brief Copies the hash from rhs.
  void operator=(const nsTempHashedString& rhs); // [tested]

  /// \brief Copies the hash from the nsHashedString.
  void operator=(const nsHashedString& rhs); // [tested]

  /// \brief Compares the two objects by their hash value. Might report incorrect equality, if two strings have the same hash value.
  bool operator==(const nsTempHashedString& rhs) const; // [tested]
  NS_ADD_DEFAULT_OPERATOR_NOTEQUAL(const nsTempHashedString&);

  /// \brief This operator allows soring objects by hash value, not by alphabetical order.
  bool operator<(const nsTempHashedString& rhs) const; // [tested]

  /// \brief Checks whether the nsTempHashedString represents the empty string.
  bool IsEmpty() const; // [tested]

  /// \brief Resets the string to the empty string.
  void Clear(); // [tested]

  /// \brief Returns the hash of the stored string.
  nsUInt64 GetHash() const; // [tested]

private:
  nsUInt64 m_uiHash;
};

// For nsFormatString
NS_FOUNDATION_DLL nsStringView BuildString(char* szTmp, nsUInt32 uiLength, const nsHashedString& sArg);

#include <Foundation/Strings/Implementation/HashedString_inl.h>
