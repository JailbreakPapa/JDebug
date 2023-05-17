
namespace wdInternal
{
  constexpr wdUInt32 MURMUR_M = 0x5bd1e995;
  constexpr wdUInt32 MURMUR_R = 24;

  template <size_t N, size_t Loop>
  struct CompileTimeMurmurHash
  {
    constexpr WD_ALWAYS_INLINE wdUInt32 operator()(wdUInt32 uiHash, const char (&str)[N], size_t i) const
    {
      return CompileTimeMurmurHash<N, Loop - 4>()(CompileTimeMurmurHash<N, 4>()(uiHash, str, i), str, i + 4);
    }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 4>
  {
    static constexpr WD_ALWAYS_INLINE wdUInt32 helper(wdUInt32 k) { return (k ^ (k >> MURMUR_R)) * MURMUR_M; }

    constexpr WD_ALWAYS_INLINE wdUInt32 operator()(wdUInt32 uiHash, const char (&str)[N], size_t i) const
    {
      // In C++11 constexpr local variables are not allowed. Need to express the following without "wdUInt32 k"
      // (this restriction is lifted in C++14's generalized constexpr)
      // wdUInt32 k = ((str[i + 0]) | ((str[i + 1]) << 8) | ((str[i + 2]) << 16) | ((str[i + 3]) << 24));
      // k *= MURMUR_M;
      // k ^= (k >> MURMUR_R);
      // k *= MURMUR_M;
      // return (hash * MURMUR_M) ^ k;

      return (uiHash * MURMUR_M) ^ helper(((str[i + 0]) | ((str[i + 1]) << 8) | ((str[i + 2]) << 16) | ((str[i + 3]) << 24)) * MURMUR_M);
    }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 3>
  {
    constexpr WD_ALWAYS_INLINE wdUInt32 operator()(wdUInt32 uiHash, const char (&str)[N], size_t i) const
    {
      return (uiHash ^ (str[i + 2] << 16) ^ (str[i + 1] << 8) ^ (str[i + 0])) * MURMUR_M;
    }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 2>
  {
    constexpr WD_ALWAYS_INLINE wdUInt32 operator()(wdUInt32 uiHash, const char (&str)[N], size_t i) const
    {
      return (uiHash ^ (str[i + 1] << 8) ^ (str[i])) * MURMUR_M;
    }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 1>
  {
    constexpr WD_ALWAYS_INLINE wdUInt32 operator()(wdUInt32 uiHash, const char (&str)[N], size_t i) const { return (uiHash ^ (str[i])) * MURMUR_M; }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 0>
  {
    constexpr WD_ALWAYS_INLINE wdUInt32 operator()(wdUInt32 uiHash, const char (&str)[N], size_t i) const { return uiHash; }
  };

  constexpr wdUInt32 rightShift_and_xorWithPrevSelf(wdUInt32 h, wdUInt32 uiShift) { return h ^ (h >> uiShift); }
} // namespace wdInternal

template <size_t N>
constexpr WD_ALWAYS_INLINE wdUInt32 wdHashingUtils::MurmurHash32String(const char (&str)[N], wdUInt32 uiSeed)
{
  // In C++11 constexpr local variables are not allowed. Need to express the following without "wdUInt32 h"
  // (this restriction is lifted in C++14's generalized constexpr)
  // const wdUInt32 uiStrlen = (wdUInt32)(N - 1);
  // wdUInt32 h = wdInternal::CompileTimeMurmurHash<N - 1>(uiSeed ^ uiStrlen, str, 0);
  // h ^= h >> 13;
  // h *= wdInternal::MURMUR_M;
  // h ^= h >> 15;
  // return h;

  return wdInternal::rightShift_and_xorWithPrevSelf(
    wdInternal::rightShift_and_xorWithPrevSelf(wdInternal::CompileTimeMurmurHash<N, N - 1>()(uiSeed ^ static_cast<wdUInt32>(N - 1), str, 0), 13) *
      wdInternal::MURMUR_M,
    15);
}

WD_ALWAYS_INLINE wdUInt32 wdHashingUtils::MurmurHash32String(wdStringView sStr, wdUInt32 uiSeed)
{
  return MurmurHash32(sStr.GetStartPointer(), sStr.GetElementCount(), uiSeed);
}
