#include <Foundation/Strings/Implementation/StringBase.h>

namespace wdInternal
{
  template <typename T, bool isString>
  struct HashHelperImpl
  {
    static wdUInt32 Hash(const T& value);
  };

  template <typename T>
  struct HashHelperImpl<T, true>
  {
    WD_ALWAYS_INLINE static wdUInt32 Hash(wdStringView sString)
    {
      return wdHashingUtils::StringHashTo32(wdHashingUtils::StringHash(sString));
    }
  };

  template <typename T, bool isString>
  WD_ALWAYS_INLINE wdUInt32 HashHelperImpl<T, isString>::Hash(const T& value)
  {
    WD_CHECK_AT_COMPILETIME_MSG(isString, "wdHashHelper is not implemented for the given type.");
    return 0;
  }
} // namespace wdInternal

template <typename T>
template <typename U>
WD_ALWAYS_INLINE wdUInt32 wdHashHelper<T>::Hash(const U& value)
{
  return wdInternal::HashHelperImpl<T, WD_IS_DERIVED_FROM_STATIC(wdThisIsAString, T)>::Hash(value);
}

template <typename T>
template <typename U>
WD_ALWAYS_INLINE bool wdHashHelper<T>::Equal(const T& a, const U& b)
{
  return a == b;
}



template <>
struct wdHashHelper<wdUInt32>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(wdUInt32 value)
  {
    // Knuth: multiplication by the golden ratio will minimize gaps in the hash space.
    // 2654435761U: prime close to 2^32/phi with phi = golden ratio (sqrt(5) - 1) / 2
    return value * 2654435761U;
  }

  WD_ALWAYS_INLINE static bool Equal(wdUInt32 a, wdUInt32 b) { return a == b; }
};

template <>
struct wdHashHelper<wdInt32>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(wdInt32 value) { return wdHashHelper<wdUInt32>::Hash(wdUInt32(value)); }

  WD_ALWAYS_INLINE static bool Equal(wdInt32 a, wdInt32 b) { return a == b; }
};

template <>
struct wdHashHelper<wdUInt64>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(wdUInt64 value)
  {
    // boost::hash_combine.
    wdUInt32 a = wdUInt32(value >> 32);
    wdUInt32 b = wdUInt32(value);
    return a ^ (b + 0x9e3779b9 + (a << 6) + (b >> 2));
  }

  WD_ALWAYS_INLINE static bool Equal(wdUInt64 a, wdUInt64 b) { return a == b; }
};

template <>
struct wdHashHelper<wdInt64>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(wdInt64 value) { return wdHashHelper<wdUInt64>::Hash(wdUInt64(value)); }

  WD_ALWAYS_INLINE static bool Equal(wdInt64 a, wdInt64 b) { return a == b; }
};

template <>
struct wdHashHelper<const char*>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(const char* szValue)
  {
    return wdHashingUtils::StringHashTo32(wdHashingUtils::StringHash(szValue));
  }

  WD_ALWAYS_INLINE static bool Equal(const char* a, const char* b) { return wdStringUtils::IsEqual(a, b); }
};

template <typename T>
struct wdHashHelper<T*>
{
  WD_ALWAYS_INLINE static wdUInt32 Hash(T* value)
  {
#if WD_ENABLED(WD_PLATFORM_64BIT)
    return wdHashHelper<wdUInt64>::Hash(reinterpret_cast<wdUInt64>(value) >> 4);
#else
    return wdHashHelper<wdUInt32>::Hash(reinterpret_cast<wdUInt32>(value) >> 4);
#endif
  }

  WD_ALWAYS_INLINE static bool Equal(T* a, T* b)
  {
    return a == b;
  }
};

template <size_t N>
constexpr WD_ALWAYS_INLINE wdUInt64 wdHashingUtils::StringHash(const char (&str)[N], wdUInt64 uiSeed)
{
  return xxHash64String(str, uiSeed);
}

WD_ALWAYS_INLINE wdUInt64 wdHashingUtils::StringHash(wdStringView sStr, wdUInt64 uiSeed)
{
  return xxHash64String(sStr, uiSeed);
}

constexpr WD_ALWAYS_INLINE wdUInt32 wdHashingUtils::StringHashTo32(wdUInt64 uiHash)
{
  // just throw away the upper bits
  return static_cast<wdUInt32>(uiHash);
}

constexpr WD_ALWAYS_INLINE wdUInt32 wdHashingUtils::CombineHashValues32(wdUInt32 ui0, wdUInt32 ui1)
{
  // See boost::hash_combine
  return ui0 ^ (ui1 + 0x9e3779b9 + (ui0 << 6) + (ui1 >> 2));
}
