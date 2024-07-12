#include <Foundation/Strings/Implementation/StringBase.h>

namespace nsInternal
{
  template <typename T, bool isString>
  struct HashHelperImpl
  {
    static nsUInt32 Hash(const T& value);
  };

  template <typename T>
  struct HashHelperImpl<T, true>
  {
    NS_ALWAYS_INLINE static nsUInt32 Hash(nsStringView sString)
    {
      return nsHashingUtils::StringHashTo32(nsHashingUtils::StringHash(sString));
    }
  };

  template <typename T, bool isString>
  NS_ALWAYS_INLINE nsUInt32 HashHelperImpl<T, isString>::Hash(const T& value)
  {
    NS_CHECK_AT_COMPILETIME_MSG(isString, "nsHashHelper is not implemented for the given type.");
    return 0;
  }
} // namespace nsInternal

template <typename T>
template <typename U>
NS_ALWAYS_INLINE nsUInt32 nsHashHelper<T>::Hash(const U& value)
{
  return nsInternal::HashHelperImpl<T, NS_IS_DERIVED_FROM_STATIC(nsThisIsAString, T)>::Hash(value);
}

template <typename T>
template <typename U>
NS_ALWAYS_INLINE bool nsHashHelper<T>::Equal(const T& a, const U& b)
{
  return a == b;
}



template <>
struct nsHashHelper<nsUInt32>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsUInt32 value)
  {
    // Knuth: multiplication by the golden ratio will minimize gaps in the hash space.
    // 2654435761U: prime close to 2^32/phi with phi = golden ratio (sqrt(5) - 1) / 2
    return value * 2654435761U;
  }

  NS_ALWAYS_INLINE static bool Equal(nsUInt32 a, nsUInt32 b) { return a == b; }
};

template <>
struct nsHashHelper<nsInt32>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsInt32 value) { return nsHashHelper<nsUInt32>::Hash(nsUInt32(value)); }

  NS_ALWAYS_INLINE static bool Equal(nsInt32 a, nsInt32 b) { return a == b; }
};

template <>
struct nsHashHelper<nsUInt64>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsUInt64 value)
  {
    // boost::hash_combine.
    nsUInt32 a = nsUInt32(value >> 32);
    nsUInt32 b = nsUInt32(value);
    return a ^ (b + 0x9e3779b9 + (a << 6) + (b >> 2));
  }

  NS_ALWAYS_INLINE static bool Equal(nsUInt64 a, nsUInt64 b) { return a == b; }
};

template <>
struct nsHashHelper<nsInt64>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsInt64 value) { return nsHashHelper<nsUInt64>::Hash(nsUInt64(value)); }

  NS_ALWAYS_INLINE static bool Equal(nsInt64 a, nsInt64 b) { return a == b; }
};

template <>
struct nsHashHelper<const char*>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(const char* szValue)
  {
    return nsHashingUtils::StringHashTo32(nsHashingUtils::StringHash(szValue));
  }

  NS_ALWAYS_INLINE static bool Equal(const char* a, const char* b) { return nsStringUtils::IsEqual(a, b); }
};

template <>
struct nsHashHelper<nsStringView>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(nsStringView sValue)
  {
    return nsHashingUtils::StringHashTo32(nsHashingUtils::StringHash(sValue));
  }

  NS_ALWAYS_INLINE static bool Equal(nsStringView a, nsStringView b) { return a == b; }
};

template <typename T>
struct nsHashHelper<T*>
{
  NS_ALWAYS_INLINE static nsUInt32 Hash(T* value)
  {
#if NS_ENABLED(NS_PLATFORM_64BIT)
    return nsHashHelper<nsUInt64>::Hash(reinterpret_cast<nsUInt64>(value) >> 4);
#else
    return nsHashHelper<nsUInt32>::Hash(reinterpret_cast<nsUInt32>(value) >> 4);
#endif
  }

  NS_ALWAYS_INLINE static bool Equal(T* a, T* b)
  {
    return a == b;
  }
};

template <size_t N>
constexpr NS_ALWAYS_INLINE nsUInt64 nsHashingUtils::StringHash(const char (&str)[N], nsUInt64 uiSeed)
{
  return xxHash64String(str, uiSeed);
}

NS_ALWAYS_INLINE nsUInt64 nsHashingUtils::StringHash(nsStringView sStr, nsUInt64 uiSeed)
{
  return xxHash64String(sStr, uiSeed);
}

constexpr NS_ALWAYS_INLINE nsUInt32 nsHashingUtils::StringHashTo32(nsUInt64 uiHash)
{
  // just throw away the upper bits
  return static_cast<nsUInt32>(uiHash);
}

constexpr NS_ALWAYS_INLINE nsUInt32 nsHashingUtils::CombineHashValues32(nsUInt32 ui0, nsUInt32 ui1)
{
  // See boost::hash_combine
  return ui0 ^ (ui1 + 0x9e3779b9 + (ui0 << 6) + (ui1 >> 2));
}
