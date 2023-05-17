
#pragma once

#include <Foundation/Basics.h>

/// \brief This class provides implementations of different hashing algorithms.
class WD_FOUNDATION_DLL wdHashingUtils
{
public:
  /// \brief Calculates the CRC32 checksum of the given key.
  static wdUInt32 CRC32Hash(const void* pKey, size_t uiSizeInBytes); // [tested]

  /// \brief Calculates the 32bit murmur hash of the given key.
  static wdUInt32 MurmurHash32(const void* pKey, size_t uiSizeInByte, wdUInt32 uiSeed = 0); // [tested]

  /// \brief Calculates the 64bit murmur hash of the given key.
  static wdUInt64 MurmurHash64(const void* pKey, size_t uiSizeInByte, wdUInt64 uiSeed = 0); // [tested]

  /// \brief Calculates the 32bit murmur hash of a string constant at compile time. Encoding does not matter here.
  template <size_t N>
  constexpr static wdUInt32 MurmurHash32String(const char (&str)[N], wdUInt32 uiSeed = 0); // [tested]

  /// \brief Calculates the 32bit murmur hash of a string pointer during runtime. Encoding does not matter here.
  ///
  /// We cannot pass a string pointer directly since a string constant would be treated as pointer as well.
  static wdUInt32 MurmurHash32String(wdStringView sStr, wdUInt32 uiSeed = 0); // [tested]

  /// \brief Calculates the 32bit xxHash of the given key.
  static wdUInt32 xxHash32(const void* pKey, size_t uiSizeInByte, wdUInt32 uiSeed = 0); // [tested]

  /// \brief Calculates the 64bit xxHash of the given key.
  static wdUInt64 xxHash64(const void* pKey, size_t uiSizeInByte, wdUInt64 uiSeed = 0); // [tested]

  /// \brief Calculates the 32bit xxHash of the given string literal at compile time.
  template <size_t N>
  constexpr static wdUInt32 xxHash32String(const char (&str)[N], wdUInt32 uiSeed = 0); // [tested]

  /// \brief Calculates the 64bit xxHash of the given string literal at compile time.
  template <size_t N>
  constexpr static wdUInt64 xxHash64String(const char (&str)[N], wdUInt64 uiSeed = 0); // [tested]

  /// \brief Calculates the 32bit xxHash of a string pointer during runtime.
  ///
  /// We cannot pass a string pointer directly since a string constant would be treated as pointer as well.
  static wdUInt32 xxHash32String(wdStringView sStr, wdUInt32 uiSeed = 0); // [tested]

  /// \brief Calculates the 64bit xxHash of a string pointer during runtime.
  ///
  /// We cannot pass a string pointer directly since a string constant would be treated as pointer as well.
  static wdUInt64 xxHash64String(wdStringView sStr, wdUInt64 uiSeed = 0); // [tested]

  /// \brief Calculates the hash of the given string literal at compile time.
  template <size_t N>
  constexpr static wdUInt64 StringHash(const char (&str)[N], wdUInt64 uiSeed = 0); // [tested]

  /// \brief Calculates the hash of a string pointer at runtime.
  ///
  /// We cannot pass a string pointer directly since a string constant would be treated as pointer as well.
  static wdUInt64 StringHash(wdStringView sStr, wdUInt64 uiSeed = 0); // [tested]

  /// \brief Truncates a 64 bit string hash to 32 bit.
  ///
  /// This is necessary when a 64 bit string hash is used in a hash table (which only uses 32 bit indices).
  constexpr static wdUInt32 StringHashTo32(wdUInt64 uiHash);

  /// \brief Combines two 32 bit hash values into one.
  constexpr static wdUInt32 CombineHashValues32(wdUInt32 ui0, wdUInt32 ui1);
};

/// \brief Helper struct to calculate the Hash of different types.
///
/// This struct can be used to provide a custom hash function for wdHashTable. The default implementation uses the xxHash function.
template <typename T>
struct wdHashHelper
{
  template <typename U>
  static wdUInt32 Hash(const U& value);

  template <typename U>
  static bool Equal(const T& a, const U& b);
};

#include <Foundation/Algorithm/Implementation/HashingMurmur_inl.h>
#include <Foundation/Algorithm/Implementation/HashingUtils_inl.h>
#include <Foundation/Algorithm/Implementation/HashingXxHash_inl.h>
