
namespace nsInternal
{
  constexpr nsUInt32 MURMUR_M = 0x5bd1e995;
  constexpr nsUInt32 MURMUR_R = 24;

  template <size_t N, size_t Loop>
  struct CompileTimeMurmurHash
  {
    constexpr NS_ALWAYS_INLINE nsUInt32 operator()(nsUInt32 uiHash, const char (&str)[N], size_t i) const
    {
      return CompileTimeMurmurHash<N, Loop - 4>()(CompileTimeMurmurHash<N, 4>()(uiHash, str, i), str, i + 4);
    }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 4>
  {
    static constexpr NS_ALWAYS_INLINE nsUInt32 helper(nsUInt32 k) { return (k ^ (k >> MURMUR_R)) * MURMUR_M; }

    constexpr NS_ALWAYS_INLINE nsUInt32 operator()(nsUInt32 uiHash, const char (&str)[N], size_t i) const
    {
      // In C++11 constexpr local variables are not allowed. Need to express the following without "nsUInt32 k"
      // (this restriction is lifted in C++14's generalized constexpr)
      // nsUInt32 k = ((str[i + 0]) | ((str[i + 1]) << 8) | ((str[i + 2]) << 16) | ((str[i + 3]) << 24));
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
    constexpr NS_ALWAYS_INLINE nsUInt32 operator()(nsUInt32 uiHash, const char (&str)[N], size_t i) const
    {
      return (uiHash ^ (str[i + 2] << 16) ^ (str[i + 1] << 8) ^ (str[i + 0])) * MURMUR_M;
    }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 2>
  {
    constexpr NS_ALWAYS_INLINE nsUInt32 operator()(nsUInt32 uiHash, const char (&str)[N], size_t i) const
    {
      return (uiHash ^ (str[i + 1] << 8) ^ (str[i])) * MURMUR_M;
    }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 1>
  {
    constexpr NS_ALWAYS_INLINE nsUInt32 operator()(nsUInt32 uiHash, const char (&str)[N], size_t i) const { return (uiHash ^ (str[i])) * MURMUR_M; }
  };

  template <size_t N>
  struct CompileTimeMurmurHash<N, 0>
  {
    constexpr NS_ALWAYS_INLINE nsUInt32 operator()(nsUInt32 uiHash, const char (&str)[N], size_t i) const { return uiHash; }
  };

  constexpr nsUInt32 rightShift_and_xorWithPrevSelf(nsUInt32 h, nsUInt32 uiShift)
  {
    return h ^ (h >> uiShift);
  }
} // namespace nsInternal

template <size_t N>
constexpr NS_ALWAYS_INLINE nsUInt32 nsHashingUtils::MurmurHash32String(const char (&str)[N], nsUInt32 uiSeed)
{
  // In C++11 constexpr local variables are not allowed. Need to express the following without "nsUInt32 h"
  // (this restriction is lifted in C++14's generalized constexpr)
  // const nsUInt32 uiStrlen = (nsUInt32)(N - 1);
  // nsUInt32 h = nsInternal::CompileTimeMurmurHash<N - 1>(uiSeed ^ uiStrlen, str, 0);
  // h ^= h >> 13;
  // h *= nsInternal::MURMUR_M;
  // h ^= h >> 15;
  // return h;

  return nsInternal::rightShift_and_xorWithPrevSelf(
    nsInternal::rightShift_and_xorWithPrevSelf(nsInternal::CompileTimeMurmurHash<N, N - 1>()(uiSeed ^ static_cast<nsUInt32>(N - 1), str, 0), 13) *
      nsInternal::MURMUR_M,
    15);
}

NS_ALWAYS_INLINE nsUInt32 nsHashingUtils::MurmurHash32String(nsStringView sStr, nsUInt32 uiSeed)
{
  return MurmurHash32(sStr.GetStartPointer(), sStr.GetElementCount(), uiSeed);
}
